/**
 * @file	statsave.cpp
 * @brief	Implementation of State save
 */

#include "compiler.h"
#include "statsave.h"
#include "strres.h"
#include "dosio.h"
#include "commng.h"
#include "scrnmng.h"
#include "soundmng.h"
#include "timemng.h"
#include "mousemng.h"
#include "cpucore.h"
#include "pccore.h"
#include "iocore.h"
#include "gdc_sub.h"
#include "cbuscore.h"
#include "ideio.h"
#include "sasiio.h"
#include "scsiio.h"
#include "pc9861k.h"
#include "mpu98ii.h"
#if defined(SUPPORT_SMPU98)
#include "smpu98.h"
#endif
#include "board14.h"
#include "amd98.h"
#include "bios/bios.h"
#include "vram.h"
#include "palettes.h"
#include "maketext.h"
#include "sound.h"
#include "fmboard.h"
#ifdef SUPPORT_SOUND_SB16
#include "ct1741io.h"
#endif
#include "beep.h"
#include "diskimage/fddfile.h"
#include "fdd/fdd_mtr.h"
#include "fdd/sxsi.h"
#include "font/font.h"
#include "keydisp.h"
#include "hostdrv.h"
#if defined(SUPPORT_HOSTDRVNT)
#include "hostdrvnt.h"
#endif
#include "calendar.h"
#include "keystat.h"
#if defined(SUPPORT_WAB)
#include "wab/wab.h"
#endif
#if defined(SUPPORT_CL_GD5430)
#include "wab/cirrus_vga_extern.h"
#endif
#if defined(SUPPORT_NET)
#include "network/net.h"
#endif
#if defined(SUPPORT_LGY98)
#include "network/lgy98.h"
#include "network/lgy98dev.h"
#endif
#if defined(CPUCORE_IA32)
#include "ia32/instructions/fpu/fp.h"
#endif
#if defined(BIOS_IO_EMULATION)
#include "bios/bios.h"
#endif
#if defined(SUPPORT_IA32_HAXM)
#include	"i386hax/haxfunc.h"
#include	"i386hax/haxcore.h"
#endif

#ifdef USE_MAME
UINT8 YMF262Read(void *chip, INT a);
INT YMF262Write(void *chip, INT a, INT v);
int YMF262FlagSave(void *chip, void *dstbuf);
int YMF262FlagLoad(void *chip, void *srcbuf, int size);
#endif

extern int sxsi_unittbl[];

#if defined(MACOS)
#define	CRCONST		str_cr
#elif defined(WIN32) || defined(X11)
#define	CRCONST		str_lf
#else
#define	CRCONST		str_crlf
#endif

typedef struct {
	char	name[16];
	char	vername[28];
	UINT32	ver;
} NP2FHDR;

typedef struct {
	char	index[10];
	UINT16	ver;
	UINT32	size;
} NP2FENT;

/**
 * @brief handle
 */
struct TagStatFlagHandle
{
	NP2FENT		hdr;
	UINT		pos;
	OEMCHAR		*err;
	int			errlen;
};
typedef struct TagStatFlagHandle _STFLAGH;		/* define */

enum
{
	STATFLAG_BIN			= 0,
	STATFLAG_TERM,
	STATFLAG_COM,
	STATFLAG_DMA,
	STATFLAG_EGC,
	STATFLAG_EPSON,
	STATFLAG_EVT,
	STATFLAG_EXT,
	STATFLAG_FDD,
	STATFLAG_FM,
	STATFLAG_GIJ,
#if defined(SUPPORT_HOSTDRV)
	STATFLAG_HDRV,
#endif
	STATFLAG_MEM,
	STATFLAG_SXSI,
	STATFLAG_HDRVNT,
	STATFLAG_MASK				= 0x3fff,
	
	STATFLAG_BWD_COMPATIBLE			= 0x4000, // ���̃t���O�������Ă���Ƃ��A�Â��o�[�W�����̃X�e�[�g�Z�[�u�ƌ݊���������i����Ȃ��f�[�^��0�Ŗ��߂���̂Œ��ӂ���j���܂̂Ƃ���STATFLAG_BIN�̂݃T�|�[�g
	STATFLAG_FWD_COMPATIBLE			= 0x8000, // ���̃t���O�������Ă���Ƃ��A�V�����o�[�W�����̃X�e�[�g�Z�[�u�ƌ݊���������i����Ȃ��f�[�^�͖����������ƂɂȂ�̂Œ��ӂ���j���܂̂Ƃ���STATFLAG_BIN�̂݃T�|�[�g
};

typedef struct {
	UINT32	id;
	void	*proc;
} PROCTBL;

typedef struct {
	UINT32	id;
	NEVENTID num;
} ENUMTBL;

#define	PROCID(a, b, c, d)	(((d) << 24) + ((c) << 16) + ((b) << 8) + (a))
#define	PROC2NUM(a, b)		proc2num(&(a), (b), sizeof(b)/sizeof(PROCTBL))
#define	NUM2PROC(a, b)		num2proc(&(a), (b), sizeof(b)/sizeof(PROCTBL))

#include "statsave.tbl"


extern	COMMNG	cm_mpu98;
#if defined(SUPPORT_SMPU98)
extern	COMMNG	cm_smpu98[];
#endif
extern	COMMNG	cm_rs232c;

typedef struct {
	OEMCHAR	*buf;
	int		remain;
} ERR_BUF;


// ----

enum {
	SFFILEH_WRITE	= 0x0001,
	SFFILEH_BLOCK	= 0x0002,
	SFFILEH_ERROR	= 0x0004
};

typedef struct {
	_STFLAGH	sfh;
	UINT		stat;
	FILEH		fh;
	UINT		secpos;
	NP2FHDR		f;
} _SFFILEH, *SFFILEH;

static SFFILEH statflag_open(const OEMCHAR *filename, OEMCHAR *err, int errlen) {

	FILEH	fh;
	SFFILEH	ret;

	fh = file_open_rb(filename);
	if (fh == FILEH_INVALID) {
		goto sfo_err1;
	}
	ret = (SFFILEH)_MALLOC(sizeof(_SFFILEH), filename);
	if (ret == NULL) {
		goto sfo_err2;
	}
	if ((file_read(fh, &ret->f, sizeof(NP2FHDR)) == sizeof(NP2FHDR)) &&
		(!memcmp(&ret->f, &np2flagdef, sizeof(np2flagdef)))) {
		ZeroMemory(ret, sizeof(_SFFILEH));
		ret->fh = fh;
		ret->secpos = sizeof(NP2FHDR);
		if ((err) && (errlen > 0)) {
			err[0] = '\0';
			ret->sfh.err = err;
			ret->sfh.errlen = errlen;
		}
		return(ret);
	}
	_MFREE(ret);

sfo_err2:
	file_close(fh);

sfo_err1:
	return(NULL);
}

static int statflag_closesection(SFFILEH sffh) {

	UINT	leng;
	UINT8	zero[16];

	if (sffh == NULL) {
		goto sfcs_err1;
	}
	if (sffh->stat == (SFFILEH_BLOCK | SFFILEH_WRITE)) {
		leng = (0 - sffh->sfh.hdr.size) & 15;
		if (leng) {
			ZeroMemory(zero, sizeof(zero));
			if (file_write(sffh->fh, zero, leng) != leng) {
				goto sfcs_err2;
			}
		}
		if ((file_seek(sffh->fh, (long)sffh->secpos, FSEEK_SET)
												!= (long)sffh->secpos) ||
			(file_write(sffh->fh, &sffh->sfh.hdr, sizeof(sffh->sfh.hdr))
												!= sizeof(sffh->sfh.hdr))) {
			goto sfcs_err2;
		}
	}
	if (sffh->stat & SFFILEH_BLOCK) {
		sffh->stat &= ~SFFILEH_BLOCK;
		sffh->secpos += sizeof(sffh->sfh.hdr) +
									((sffh->sfh.hdr.size + 15) & (~15));
		if (file_seek(sffh->fh, (long)sffh->secpos, FSEEK_SET)
												!= (long)sffh->secpos) {
			goto sfcs_err2;
		}
	}
	return(STATFLAG_SUCCESS);

sfcs_err2:
	sffh->stat = SFFILEH_ERROR;

sfcs_err1:
	return(STATFLAG_FAILURE);
}

static int statflag_readsection(SFFILEH sffh) {

	int		ret;

	ret = statflag_closesection(sffh);
	if (ret != STATFLAG_SUCCESS) {
		return(ret);
	}
	if ((sffh->stat == 0) &&
		(file_read(sffh->fh, &sffh->sfh.hdr, sizeof(sffh->sfh.hdr))
												== sizeof(sffh->sfh.hdr))) {
		sffh->stat = SFFILEH_BLOCK;
		sffh->sfh.pos = 0;
		return(STATFLAG_SUCCESS);
	}
	sffh->stat = SFFILEH_ERROR;
	return(STATFLAG_FAILURE);
}

int statflag_read(STFLAGH sfh, void *buf, UINT size) {

	if ((sfh == NULL) || (buf == NULL) ||
		((sfh->pos + size) > sfh->hdr.size)) {
		goto sfr_err;
	}
	if (size) {
		if (file_read(((SFFILEH)sfh)->fh, buf, size) != size) {
			goto sfr_err;
		}
		sfh->pos += size;
	}
	return(STATFLAG_SUCCESS);

sfr_err:
	return(STATFLAG_FAILURE);
}

static SFFILEH statflag_create(const OEMCHAR *filename) {

	SFFILEH	ret;
	FILEH	fh;

	ret = (SFFILEH)_MALLOC(sizeof(_SFFILEH), filename);
	if (ret == NULL) {
		goto sfc_err1;
	}
	fh = file_create(filename);
	if (fh == FILEH_INVALID) {
		goto sfc_err2;
	}
	if (file_write(fh, &np2flagdef, sizeof(NP2FHDR)) == sizeof(NP2FHDR)) {
		ZeroMemory(ret, sizeof(_SFFILEH));
		ret->stat = SFFILEH_WRITE;
		ret->fh = fh;
		ret->secpos = sizeof(NP2FHDR);
		return(ret);
	}
	file_close(fh);
	file_delete(filename);

sfc_err2:
	_MFREE(ret);

sfc_err1:
	return(NULL);
}

static int statflag_createsection(SFFILEH sffh, const SFENTRY *tbl) {

	int		ret;

	ret = statflag_closesection(sffh);
	if (ret != STATFLAG_SUCCESS) {
		return(ret);
	}
	if (sffh->stat != SFFILEH_WRITE) {
		sffh->stat = SFFILEH_ERROR;
		return(STATFLAG_FAILURE);
	}
	CopyMemory(sffh->sfh.hdr.index, tbl->index, sizeof(sffh->sfh.hdr.index));
	sffh->sfh.hdr.ver = tbl->ver;
	sffh->sfh.hdr.size = 0;
	return(STATFLAG_SUCCESS);
}

int statflag_write(STFLAGH sfh, const void *buf, UINT size) {

	SFFILEH	sffh;

	if (sfh == NULL) {
		goto sfw_err1;
	}
	sffh = (SFFILEH)sfh;
	if (!(sffh->stat & SFFILEH_WRITE)) {
		goto sfw_err2;
	}
	if (!(sffh->stat & SFFILEH_BLOCK)) {
		sffh->stat |= SFFILEH_BLOCK;
		sfh->pos = 0;
		if (file_write(sffh->fh, &sfh->hdr, sizeof(sfh->hdr))
														!= sizeof(sfh->hdr)) {
			goto sfw_err2;
		}
	}
	if (size) {
		if ((buf == NULL) || (file_write(sffh->fh, buf, size) != size)) {
			goto sfw_err2;
		}
		sfh->pos += size;
		if (sfh->hdr.size < sfh->pos) {
			sfh->hdr.size = sfh->pos;
		}
	}
	return(STATFLAG_SUCCESS);

sfw_err2:
	sffh->stat = SFFILEH_ERROR;

sfw_err1:
	return(STATFLAG_FAILURE);
}

static void statflag_close(SFFILEH sffh) {

	if (sffh) {
		statflag_closesection(sffh);
		file_close(sffh->fh);
		_MFREE(sffh);
	}
}

void statflag_seterr(STFLAGH sfh, const OEMCHAR *str) {

	if ((sfh) && (sfh->errlen)) {
		milstr_ncat(sfh->err, str, sfh->errlen);
		milstr_ncat(sfh->err, CRCONST, sfh->errlen);
	}
}


// ---- function

// �֐��|�C���^�� int�ɕύX�B
static BRESULT proc2num(void *func, const PROCTBL *tbl, int size) {

	int		i;

	for (i=0; i<size; i++) {
		if (*(INTPTR *)func == (INTPTR)tbl->proc) {
			*(INTPTR *)func = (INTPTR)tbl->id;
			return(SUCCESS);
		}
		tbl++;
	}
	return(FAILURE);
}

static BRESULT num2proc(void *func, const PROCTBL *tbl, int size) {

	int		i;

	for (i=0; i<size; i++) {
		if (*(INTPTR *)func == (INTPTR)tbl->id) {
			*(INTPTR *)func = (INTPTR)tbl->proc;
			return(SUCCESS);
		}
		tbl++;
	}
	return(FAILURE);
}


// ---- file

typedef struct {
	OEMCHAR	path[MAX_PATH];
	UINT	ftype;
	int		readonly;
	DOSDATE	date;
	DOSTIME	time;
} STATPATH;

static const OEMCHAR str_updated[] = OEMTEXT("%s: updated");
static const OEMCHAR str_notfound[] = OEMTEXT("%s: not found");

static int statflag_writepath(STFLAGH sfh, const OEMCHAR *path,
												UINT ftype, int readonly) {

	STATPATH	sp;
	FILEH		fh;

	ZeroMemory(&sp, sizeof(sp));
	if ((path) && (path[0])) {
		file_cpyname(sp.path, path, NELEMENTS(sp.path));
		sp.ftype = ftype;
		sp.readonly = readonly;
		fh = file_open_rb(path);
		if (fh != FILEH_INVALID) {
			file_getdatetime(fh, &sp.date, &sp.time);
			file_close(fh);
		}
	}
	return(statflag_write(sfh, &sp, sizeof(sp)));
}

static int statflag_checkpath(STFLAGH sfh, const OEMCHAR *dvname) {

	int			ret;
	STATPATH	sp;
	FILEH		fh;
	OEMCHAR		buf[256];
	DOSDATE		dosdate;
	DOSTIME		dostime;

	ret = statflag_read(sfh, &sp, sizeof(sp));
	if (sp.path[0]) {
		fh = file_open_rb(sp.path);
		if (fh != FILEH_INVALID) {
			file_getdatetime(fh, &dosdate, &dostime);
			file_close(fh);
			if ((memcmp(&sp.date, &dosdate, sizeof(dosdate))) ||
				(memcmp(&sp.time, &dostime, sizeof(dostime)))) {
				ret |= STATFLAG_DISKCHG;
				OEMSPRINTF(buf, str_updated, dvname);
				statflag_seterr(sfh, buf);
			}
		}
		else {
			ret |= STATFLAG_DISKCHG;
			OEMSPRINTF(buf, str_notfound, dvname);
			statflag_seterr(sfh, buf);
		}
	}
	return(ret);
}


// ---- common

static int flagsave_common(STFLAGH sfh, const SFENTRY *tbl) {

	return(statflag_write(sfh, tbl->arg1, tbl->arg2));
}

static int flagload_common(STFLAGH sfh, const SFENTRY *tbl) {

	memset(tbl->arg1, 0, tbl->arg2);
	return(statflag_read(sfh, tbl->arg1, min(tbl->arg2, sfh->hdr.size)));
}


// ---- memory

static int flagsave_mem(STFLAGH sfh, const SFENTRY *tbl) {

	int		ret;

	ret = statflag_write(sfh, mem, 0x110000);
	ret |= statflag_write(sfh, mem + VRAM1_B, 0x18000);
	ret |= statflag_write(sfh, mem + VRAM1_E, 0x8000);
	(void)tbl;
	return(ret);
}

static int flagload_mem(STFLAGH sfh, const SFENTRY *tbl) {

	int		ret;

	ret = statflag_read(sfh, mem, 0x110000);
	ret |= statflag_read(sfh, mem + VRAM1_B, 0x18000);
	ret |= statflag_read(sfh, mem + VRAM1_E, 0x8000);
	(void)tbl;
	return(ret);
}


// ---- dma

static int flagsave_dma(STFLAGH sfh, const SFENTRY *tbl) {

	int			i;
	_DMAC		dmabak;

	dmabak = dmac;
	for (i=0; i<4; i++) {
		if ((PROC2NUM(dmabak.dmach[i].proc.outproc, dmaproc)) ||
			(PROC2NUM(dmabak.dmach[i].proc.inproc, dmaproc)) ||
			(PROC2NUM(dmabak.dmach[i].proc.extproc, dmaproc))) {
			return(STATFLAG_FAILURE);
		}
	}
	(void)tbl;
	return(statflag_write(sfh, &dmabak, sizeof(dmabak)));
}

static int flagload_dma(STFLAGH sfh, const SFENTRY *tbl) {

	int		ret;
	int		i;

	ret = statflag_read(sfh, &dmac, sizeof(dmac));

	for (i=0; i<4; i++) {
		if (NUM2PROC(dmac.dmach[i].proc.outproc, dmaproc)) {
			dmac.dmach[i].proc.outproc = dma_dummyout;
			ret |= STATFLAG_WARNING;
		}
		if (NUM2PROC(dmac.dmach[i].proc.inproc, dmaproc)) {
			dmac.dmach[i].proc.inproc = dma_dummyin;
			ret |= STATFLAG_WARNING;
		}
		if (NUM2PROC(dmac.dmach[i].proc.extproc, dmaproc)) {
			dmac.dmach[i].proc.extproc = dma_dummyproc;
			ret |= STATFLAG_WARNING;
		}
	}
	(void)tbl;
	return(ret);
}


// ---- egc

static int flagsave_egc(STFLAGH sfh, const SFENTRY *tbl) {

	_EGC	egcbak;

	egcbak = egc;
	egcbak.inptr -= (INTPTR)egc.buf;
	egcbak.outptr -= (INTPTR)egc.buf;
	(void)tbl;
	return(statflag_write(sfh, &egcbak, sizeof(egcbak)));
}

static int flagload_egc(STFLAGH sfh, const SFENTRY *tbl) {

	int		ret;

	ret = statflag_read(sfh, &egc, sizeof(egc));
	egc.inptr += (INTPTR)egc.buf;
	egc.outptr += (INTPTR)egc.buf;
	(void)tbl;
	return(ret);
}


// ---- epson

static int flagsave_epson(STFLAGH sfh, const SFENTRY *tbl) {

	int		ret;

	if (!(pccore.model & PCMODEL_EPSON)) {
		return(STATFLAG_SUCCESS);
	}
	ret = statflag_write(sfh, &epsonio, sizeof(epsonio));
	ret |= statflag_write(sfh, mem + 0x1c0000, 0x8000);
	ret |= statflag_write(sfh, mem + 0x1e8000, 0x18000);
	(void)tbl;
	return(ret);
}

static int flagload_epson(STFLAGH sfh, const SFENTRY *tbl) {

	int		ret;

	ret = statflag_read(sfh, &epsonio, sizeof(epsonio));
	ret |= statflag_read(sfh, mem + 0x1c0000, 0x8000);
	ret |= statflag_read(sfh, mem + 0x1e8000, 0x18000);
	(void)tbl;
	return(ret);
}


// ---- event

typedef struct {
	UINT		readyevents;
	UINT		waitevents;
} NEVTSAVE;

typedef struct {
	UINT32		id;
	SINT32		clock;
	UINT32		flag;
	NEVENTCB	proc;
} NEVTITEM;

static int nevent_write(STFLAGH sfh, NEVENTID num) {

	NEVTITEM	nit;
	UINT		i;

	ZeroMemory(&nit, sizeof(nit));
	for (i=0; i<NELEMENTS(evtnum); i++) {
		if (evtnum[i].num == num) {
			nit.id = evtnum[i].id;
			break;
		}
	}
	nit.clock = g_nevent.item[num].clock;
	nit.flag = g_nevent.item[num].flag;
	nit.proc = g_nevent.item[num].proc;
	if (PROC2NUM(nit.proc, evtproc)) {
		nit.proc = NULL;
	}
	return(statflag_write(sfh, &nit, sizeof(nit)));
}

static int flagsave_evt(STFLAGH sfh, const SFENTRY *tbl) {

	NEVTSAVE	nevt;
	int			ret;
	UINT		i;

	nevt.readyevents = g_nevent.readyevents;
	nevt.waitevents = g_nevent.waitevents;

	ret = statflag_write(sfh, &nevt, sizeof(nevt));
	for (i=0; i<nevt.readyevents; i++) {
		ret |= nevent_write(sfh, g_nevent.level[i]);
	}
	for (i=0; i<nevt.waitevents; i++) {
		ret |= nevent_write(sfh, g_nevent.waitevent[i]);
	}
	(void)tbl;
	return(ret);
}

static int nevent_read(STFLAGH sfh, NEVENTID *tbl, UINT *pos) {

	int			ret;
	NEVTITEM	nit;
	UINT		i;
	NEVENTID	num;

	ret = statflag_read(sfh, &nit, sizeof(nit));

	for (i=0; i<NELEMENTS(evtnum); i++) {
		if (nit.id == evtnum[i].id) {
			break;
		}
	}
	if (i < NELEMENTS(evtnum)) {
		num = evtnum[i].num;
		g_nevent.item[num].clock = nit.clock;
		g_nevent.item[num].flag = nit.flag;
		g_nevent.item[num].proc = nit.proc;
		if (NUM2PROC(g_nevent.item[num].proc, evtproc)) {
			ret |= STATFLAG_WARNING;
		}
		else {
			tbl[*pos] = num;
			(*pos)++;
		}
	}
	else {
		ret |= STATFLAG_WARNING;
	}
	return(ret);
}

static int flagload_evt(STFLAGH sfh, const SFENTRY *tbl) {

	int			ret;
	NEVTSAVE	nevt;
	UINT		i;

	ret = statflag_read(sfh, &nevt, sizeof(nevt));

	g_nevent.readyevents = 0;
	g_nevent.waitevents = 0;

	for (i=0; i<nevt.readyevents; i++) {
		ret |= nevent_read(sfh, g_nevent.level, &g_nevent.readyevents);
	}
	for (i=0; i<nevt.waitevents; i++) {
		ret |= nevent_read(sfh, g_nevent.waitevent, &g_nevent.waitevents);
	}
	(void)tbl;
	return(ret);
}


// ---- extmem

static int flagsave_ext(STFLAGH sfh, const SFENTRY *tbl) {

	int		ret;

	ret = STATFLAG_SUCCESS;
	if (CPU_EXTMEM) {
		ret = statflag_write(sfh, CPU_EXTMEM, CPU_EXTMEMSIZE);
	}
	(void)tbl;
	return(ret);
}

static int flagload_ext(STFLAGH sfh, const SFENTRY *tbl) {

	int		ret;

	ret = STATFLAG_SUCCESS;
	if (CPU_EXTMEM) {
		ret = statflag_read(sfh, CPU_EXTMEM, CPU_EXTMEMSIZE);
	}
	(void)tbl;
	return(ret);
}


// ---- gaiji

static int flagsave_gij(STFLAGH sfh, const SFENTRY *tbl) {

	int		ret;
	int		i;
	int		j;
const UINT8	*fnt;

	ret = STATFLAG_SUCCESS;
	for (i=0; i<2; i++) {
		fnt = fontrom + ((0x56 + (i << 7)) << 4);
		for (j=0; j<0x80; j++) {
			ret |= statflag_write(sfh, fnt, 32);
			fnt += 0x1000;
		}
	}
	(void)tbl;
	return(ret);
}

static int flagload_gij(STFLAGH sfh, const SFENTRY *tbl) {

	int		ret;
	int		i;
	int		j;
	UINT8	*fnt;

	ret = 0;
	for (i=0; i<2; i++) {
		fnt = fontrom + ((0x56 + (i << 7)) << 4);
		for (j=0; j<0x80; j++) {
			ret |= statflag_read(sfh, fnt, 32);
			fnt += 0x1000;
		}
	}
	(void)tbl;
	return(ret);
}


// ---- FM

#if !defined(DISABLE_SOUND)

/**
 * chip flags
 */
enum
{
	FLAG_MG			= 0x0001,
	FLAG_OPNA1		= 0x0002,
	FLAG_OPNA2		= 0x0004,
#if defined(SUPPORT_PX)
	FLAG_OPNA3		= 0x0008,
	FLAG_OPNA4		= 0x0010,
	FLAG_OPNA5		= 0x0020,
#endif	/* defined(SUPPORT_PX) */
	FLAG_AMD98		= 0x0040,
	FLAG_PCM86		= 0x0080,
	FLAG_CS4231		= 0x0100,
	FLAG_OPL3		= 0x0200,
	FLAG_SB16		= 0x0400
};

/**
 * Gets flags
 * @param[in] nSoundID The sound ID
 * @return The flags
 */
static UINT GetSoundFlags(SOUNDID nSoundID)
{
	switch (nSoundID)
	{
		case SOUNDID_PC_9801_14:
			return FLAG_MG;

		case SOUNDID_PC_9801_26K:
			return FLAG_OPNA1;

		case SOUNDID_PC_9801_86:
			return FLAG_OPNA1 | FLAG_PCM86;

		case SOUNDID_PC_9801_86_26K:
			return FLAG_OPNA1 | FLAG_OPNA2 | FLAG_PCM86;

		case SOUNDID_PC_9801_118:
			return FLAG_OPNA1 | FLAG_OPL3 | FLAG_CS4231;
			
		case SOUNDID_PC_9801_86_WSS:
			return FLAG_OPNA1 | FLAG_PCM86 | FLAG_CS4231;
			
		case SOUNDID_PC_9801_86_118:
			return FLAG_OPNA1 | FLAG_OPNA2 | FLAG_OPL3 | FLAG_PCM86 | FLAG_CS4231;
			
		case SOUNDID_MATE_X_PCM:
			return FLAG_OPNA1 | FLAG_CS4231;
			
		case SOUNDID_PC_9801_86_ADPCM:
			return FLAG_OPNA1 | FLAG_PCM86;
			
		case SOUNDID_WAVESTAR:
			return FLAG_OPNA1 | FLAG_PCM86 | FLAG_CS4231;
			
		case SOUNDID_SPEAKBOARD:
			return FLAG_OPNA1;

		case SOUNDID_SPARKBOARD:
			return FLAG_OPNA1 | FLAG_OPNA2;

		case SOUNDID_AMD98:
			return FLAG_AMD98;

		case SOUNDID_SOUNDORCHESTRA:
		case SOUNDID_SOUNDORCHESTRAV:
			return FLAG_OPNA1 | FLAG_OPL3;
			
#if defined(SUPPORT_SOUND_SB16)
		case SOUNDID_SB16:
			return FLAG_OPL3 | FLAG_SB16;
			
		case SOUNDID_PC_9801_86_SB16:
			return FLAG_OPNA1 | FLAG_PCM86 | FLAG_OPL3 | FLAG_SB16;
			
		case SOUNDID_WSS_SB16:
			return FLAG_CS4231 | FLAG_OPL3 | FLAG_SB16;
			
		case SOUNDID_PC_9801_86_WSS_SB16:
			return FLAG_OPNA1 | FLAG_PCM86 | FLAG_CS4231 | FLAG_OPL3 | FLAG_SB16;
			
		case SOUNDID_PC_9801_118_SB16:
			return FLAG_OPNA1 | FLAG_OPNA2 | FLAG_OPL3 | FLAG_PCM86 | FLAG_CS4231 | FLAG_SB16;

		case SOUNDID_PC_9801_86_118_SB16:
			return FLAG_OPNA1 | FLAG_OPNA2 | FLAG_PCM86 | FLAG_CS4231 | FLAG_OPL3 | FLAG_SB16;
			
#endif	// defined(SUPPORT_SOUND_SB16)

#if defined(SUPPORT_PX)
		case SOUNDID_PX1:
			return FLAG_OPNA1 | FLAG_OPNA2 | FLAG_OPNA3 | FLAG_OPNA4;

		case SOUNDID_PX2:
			return FLAG_OPNA1 | FLAG_OPNA2 | FLAG_OPNA3 | FLAG_OPNA4 | FLAG_OPNA5 | FLAG_PCM86;
#endif	/* defined(SUPPORT_PX) */

		default:
			return 0;
	}
}

static int flagsave_fm(STFLAGH sfh, const SFENTRY *tbl)
{
	int ret;
	UINT nSaveFlags;
	UINT i;
	SOUNDID invalidSoundID = SOUNDID_INVALID;
	UINT32 datalen;
	
	ret = statflag_write(sfh, &invalidSoundID, sizeof(invalidSoundID));
	ret = statflag_write(sfh, &g_nSoundID, sizeof(g_nSoundID));

	nSaveFlags = GetSoundFlags(g_nSoundID);
	if (nSaveFlags & FLAG_MG)
	{
		datalen = sizeof(g_musicgen);
		ret |= statflag_write(sfh, &datalen, sizeof(datalen));
		ret |= statflag_write(sfh, &g_musicgen, sizeof(g_musicgen));
	}
	for (i = 0; i < NELEMENTS(g_opna); i++)
	{
		if (nSaveFlags & (FLAG_OPNA1 << i))
		{
			ret |= opna_sfsave(&g_opna[i], sfh, tbl);
		}
	}
	if (nSaveFlags & FLAG_PCM86)
	{
		datalen = sizeof(g_pcm86);
		ret |= statflag_write(sfh, &datalen, sizeof(datalen));
		ret |= statflag_write(sfh, &g_pcm86, sizeof(g_pcm86));
	}
	if (nSaveFlags & FLAG_CS4231)
	{
		datalen = sizeof(cs4231);
		ret |= statflag_write(sfh, &datalen, sizeof(datalen));
		ret |= statflag_write(sfh, &cs4231, sizeof(cs4231));
	}
	if (nSaveFlags & FLAG_AMD98)
	{
		ret |= amd98_sfsave(sfh, tbl);
	}
	if (nSaveFlags & FLAG_OPL3)
	{
		for (i = 0; i < NELEMENTS(g_opl3); i++)
		{
			ret |= opl3_sfsave(&g_opl3[i], sfh, tbl);
		}
#ifdef USE_MAME
		{
			void* buffer;
			SINT32 bufsize = 0;
			bufsize = YMF262FlagSave(NULL, NULL);
			buffer = malloc(bufsize);
			for (i = 0; i < NELEMENTS(g_mame_opl3); i++)
			{
				if(g_mame_opl3[i]){
					YMF262FlagSave(g_mame_opl3[i], buffer);
					ret |= statflag_write(sfh, &bufsize, sizeof(SINT32));
					ret |= statflag_write(sfh, buffer, bufsize);
				}else{
					SINT32 tmpsize = 0;
					ret |= statflag_write(sfh, &tmpsize, sizeof(SINT32));
				}
			}
			free(buffer);
		}
#endif
	}
#if defined(SUPPORT_SOUND_SB16)
	if (nSaveFlags & FLAG_SB16)
	{
		datalen = sizeof(g_sb16);
		ret |= statflag_write(sfh, &datalen, sizeof(datalen));
		ret |= statflag_write(sfh, &g_sb16, sizeof(g_sb16));
	}
#endif
	return ret;
}

static int flagload_fm(STFLAGH sfh, const SFENTRY *tbl)
{
	int ret;
	SOUNDID nSoundID;
	UINT nSaveFlags;
	UINT i;
	UINT32 datalen;

	ret = statflag_read(sfh, &nSoundID, sizeof(nSoundID));
	if(nSoundID==SOUNDID_INVALID){
		// new statsave
		// �V�����X�e�[�g�Z�[�u�F�����Ƃ��č\���̃T�C�Y�������悤�ɕύX�B�������ɑ���Ȃ�������0�Ŗ��߂���B�����o�������ւ����ǋL���Ă����΍H�v�ɂ��݊����ێ����\�B
		ret = statflag_read(sfh, &nSoundID, sizeof(nSoundID));
		fmboard_reset(&np2cfg, nSoundID);

		nSaveFlags = GetSoundFlags(g_nSoundID);
		if (nSaveFlags & FLAG_MG)
		{
			ret |= statflag_read(sfh, &datalen, sizeof(datalen));
			if(datalen > sizeof(g_musicgen)) return STATFLAG_FAILURE; // ���o�[�W�����œǂ߂Ȃ��悤�ɂ��Ă����i�R�����g�A�E�g����Ɠǂ߂�悤�ɂȂ邪�A�����ƍl���Đ݌v���Ȃ��Ɗ댯�j
			ret |= statflag_read(sfh, &g_musicgen, min(datalen, sizeof(g_musicgen)));
			if(datalen > sizeof(g_musicgen)){
				sfh->pos += datalen - sizeof(g_musicgen);
			}else{
				memset((UINT8*)(&g_musicgen) + datalen, 0, sizeof(g_musicgen) - datalen); // �Ȃ�������0����
			}
			board14_allkeymake();
		}
		for (i = 0; i < NELEMENTS(g_opna); i++)
		{
			if (nSaveFlags & (FLAG_OPNA1 << i))
			{
				ret |= opna_sfload(&g_opna[i], sfh, tbl);
			}
		}
		if (nSaveFlags & FLAG_PCM86)
		{
			ret |= statflag_read(sfh, &datalen, sizeof(datalen));
			if(datalen > sizeof(g_pcm86)) return STATFLAG_FAILURE; // ���o�[�W�����œǂ߂Ȃ��悤�ɂ��Ă����i�R�����g�A�E�g����Ɠǂ߂�悤�ɂȂ邪�A�����ƍl���Đ݌v���Ȃ��Ɗ댯�j
			ret |= statflag_read(sfh, &g_pcm86, min(datalen, sizeof(g_pcm86)));
			if(datalen > sizeof(g_pcm86)){
				sfh->pos += datalen - sizeof(g_pcm86);
			}else{
				memset((UINT8*)(&g_pcm86) + datalen, 0, sizeof(g_pcm86) - datalen); // �Ȃ�������0����
			}
		}
		if (nSaveFlags & FLAG_CS4231)
		{
			ret |= statflag_read(sfh, &datalen, sizeof(datalen));
			if(datalen > sizeof(cs4231)) return STATFLAG_FAILURE; // ���o�[�W�����œǂ߂Ȃ��悤�ɂ��Ă����i�R�����g�A�E�g����Ɠǂ߂�悤�ɂȂ邪�A�����ƍl���Đ݌v���Ȃ��Ɗ댯�j
			ret |= statflag_read(sfh, &cs4231, min(datalen, sizeof(cs4231)));
			if(datalen > sizeof(cs4231)){
				sfh->pos += datalen - sizeof(cs4231);
			}else{
				memset((UINT8*)(&cs4231) + datalen, 0, sizeof(cs4231) - datalen); // �Ȃ�������0����
			}
		}
		if (nSaveFlags & FLAG_AMD98)
		{
			ret |= amd98_sfload(sfh, tbl);
		}
		if (nSaveFlags & FLAG_OPL3)
		{
			for (i = 0; i < NELEMENTS(g_opl3); i++)
			{
				ret |= opl3_sfload(&g_opl3[i], sfh, tbl);
			}
#ifdef USE_MAME
			for (i = 0; i < NELEMENTS(g_mame_opl3); i++)
			{
				void* buffer;
				int bufsize = 0;
				ret |= statflag_read(sfh, &bufsize, sizeof(SINT32));
				if(bufsize!=0){
					if(YMF262FlagSave(NULL, NULL) != bufsize){
						ret = STATFLAG_FAILURE;
						break;
					}else{
						buffer = malloc(bufsize);
						ret |= statflag_read(sfh, buffer, bufsize);
						if(g_mame_opl3[i]){
							YMF262FlagLoad(g_mame_opl3[i], buffer, bufsize);
						}
						free(buffer);
					}
				}
			}
#endif
		}
#if defined(SUPPORT_SOUND_SB16)
		if (nSaveFlags & FLAG_SB16)
		{
			ret |= statflag_read(sfh, &datalen, sizeof(datalen));
			if(datalen > sizeof(g_sb16)) return STATFLAG_FAILURE; // ���o�[�W�����œǂ߂Ȃ��悤�ɂ��Ă����i�R�����g�A�E�g����Ɠǂ߂�悤�ɂȂ邪�A�����ƍl���Đ݌v���Ȃ��Ɗ댯�j
			ret |= statflag_read(sfh, &g_sb16, min(datalen, sizeof(g_sb16)));
			if(datalen > sizeof(g_sb16)){
				sfh->pos += datalen - sizeof(g_sb16);
			}else{
				memset((UINT8*)(&g_sb16) + datalen, 0, sizeof(g_sb16) - datalen); // �Ȃ�������0����
			}
		}
#endif
	}else{
		// old statsave
		nSaveFlags = GetSoundFlags(g_nSoundID);
		if (nSaveFlags & FLAG_MG)
		{
			ret |= statflag_read(sfh, &g_musicgen, sizeof(MUSICGEN_OLD));
			if(sizeof(MUSICGEN_OLD) < sizeof(g_musicgen)){
				memset((UINT8*)(&g_musicgen) + sizeof(MUSICGEN_OLD), 0, sizeof(g_musicgen) - sizeof(MUSICGEN_OLD)); // �Ȃ�������0����
			}
			board14_allkeymake();
		}
		for (i = 0; i < NELEMENTS(g_opna); i++)
		{
			if (nSaveFlags & (FLAG_OPNA1 << i))
			{
				ret |= opna_sfload(&g_opna[i], sfh, tbl);
			}
		}
		if (nSaveFlags & FLAG_PCM86)
		{
			ret |= statflag_read(sfh, &g_pcm86, sizeof(_PCM86_OLD));
			if(sizeof(_PCM86_OLD) < sizeof(g_pcm86)){
				memset((UINT8*)(&g_pcm86) + sizeof(_PCM86_OLD), 0, sizeof(g_pcm86) - sizeof(_PCM86_OLD)); // �Ȃ�������0����
			}
			g_pcm86.lastclock = g_pcm86.lastclock_obsolate;
			g_pcm86.stepclock = g_pcm86.stepclock_obsolate;
		}
		if (nSaveFlags & FLAG_CS4231)
		{
			ret |= statflag_read(sfh, &cs4231, sizeof(_CS4231_OLD));
			if(sizeof(_CS4231_OLD) < sizeof(cs4231)){
				memset((UINT8*)(&cs4231) + sizeof(_CS4231_OLD), 0, sizeof(cs4231) - sizeof(_CS4231_OLD)); // �Ȃ�������0����
			}
		}
		if (nSaveFlags & FLAG_AMD98)
		{
			ret |= amd98_sfload(sfh, tbl);
		}
		if (nSaveFlags & FLAG_OPL3)
		{
			for (i = 0; i < NELEMENTS(g_opl3); i++)
			{
				ret |= opl3_sfload(&g_opl3[i], sfh, tbl);
			}
#ifdef USE_MAME
			for (i = 0; i < NELEMENTS(g_mame_opl3); i++)
			{
				void* buffer;
				int bufsize = 0;
				ret |= statflag_read(sfh, &bufsize, sizeof(SINT32));
				if(bufsize!=0){
					if(YMF262FlagSave(NULL, NULL) != bufsize){
						ret = STATFLAG_FAILURE;
						break;
					}else{
						buffer = malloc(bufsize);
						ret |= statflag_read(sfh, buffer, bufsize);
						if(g_mame_opl3[i]){
							YMF262FlagLoad(g_mame_opl3[i], buffer, bufsize);
						}
						free(buffer);
					}
				}
			}
#endif
		}
#if defined(SUPPORT_SOUND_SB16)
		if (nSaveFlags & FLAG_SB16)
		{
			ret |= statflag_read(sfh, &g_sb16, sizeof(SB16_OLD));
			if(sizeof(SB16_OLD) < sizeof(g_sb16)){
				memset((UINT8*)(&g_sb16) + sizeof(SB16_OLD), 0, sizeof(g_sb16) - sizeof(SB16_OLD)); // �Ȃ�������0����
			}
		}
#endif
	}

	// �����B ����ړ����邱�ƁI
	pcm86gen_update();
	if (nSaveFlags & FLAG_PCM86)
	{
		fmboard_extenable((REG8)(g_pcm86.soundflags & 1));
	}
	if (nSaveFlags & FLAG_CS4231)
	{
		fmboard_extenable((REG8)(cs4231.extfunc & 1));
	}
#if defined(SUPPORT_SOUND_SB16)
	if (nSaveFlags & FLAG_SB16)
	{
		g_sb16.dsp_info.dma.chan = dmac.dmach + g_sb16.dmach; // DMA�`���l������
		dmac_attach(DMADEV_CT1741, g_sb16.dmach); // �Ċ��蓖��
	}
#endif
	return(ret);
}
#endif


// ---- fdd

static const OEMCHAR str_fddx[] = OEMTEXT("FDD%u");

static int flagsave_fdd(STFLAGH sfh, const SFENTRY *tbl) {

	int			ret;
	UINT8		i;
const OEMCHAR	*path;
	UINT		ftype;
	int			ro;

	ret = STATFLAG_SUCCESS;
	for (i=0; i<4; i++) {
		path = fdd_getfileex(i, &ftype, &ro);
		ret |= statflag_writepath(sfh, path, ftype, ro);
	}
	(void)tbl;
	return(ret);
}

static int flagcheck_fdd(STFLAGH sfh, const SFENTRY *tbl) {

	int		ret;
	int		i;
	OEMCHAR	buf[8];

	ret = STATFLAG_SUCCESS;
	for (i=0; i<4; i++) {
		OEMSPRINTF(buf, str_fddx, i+1);
		ret |= statflag_checkpath(sfh, buf);
	}
	(void)tbl;
	return(ret);
}

static int flagload_fdd(STFLAGH sfh, const SFENTRY *tbl) {

	int			ret;
	UINT8		i;
	STATPATH	sp;

	ret = STATFLAG_SUCCESS;
	for (i=0; i<4; i++) {
		ret |= statflag_read(sfh, &sp, sizeof(sp));
		if (sp.path[0]) {
			fdd_set(i, sp.path, sp.ftype, sp.readonly);
		}
	}
	(void)tbl;
	return(ret);
}


// ---- sxsi

typedef struct {
	UINT8	ide[4];
	UINT8	scsi[8];
} SXSIDEVS;

#ifdef SUPPORT_IDEIO
static const OEMCHAR str_sasix[] = OEMTEXT("IDE#%u");
#else
static const OEMCHAR str_sasix[] = OEMTEXT("SASI%u");
#endif
static const OEMCHAR str_scsix[] = OEMTEXT("SCSI%u");

static int flagsave_sxsi(STFLAGH sfh, const SFENTRY *tbl) {

	int			ret;
	UINT		i;
	SXSIDEVS	sds;
const OEMCHAR	*path;

	sxsi_allflash();
	ret = STATFLAG_SUCCESS;
	for (i=0; i<NELEMENTS(sds.ide); i++) {
		sds.ide[i] = sxsi_getdevtype((REG8)i);
	}
	for (i=0; i<NELEMENTS(sds.scsi); i++) {
		sds.scsi[i] = sxsi_getdevtype((REG8)(i + 0x20));
	}
	ret = statflag_write(sfh, &sds, sizeof(sds));
	for (i=0; i<NELEMENTS(sds.ide); i++) {
		if (sds.ide[i] != SXSIDEV_NC) {
#if defined(SUPPORT_IDEIO)&&defined(SUPPORT_PHYSICAL_CDDRV)
			if(sds.ide[i]==SXSIDEV_CDROM){ // CD-ROM�̏ꍇ�Anp2cfg��D��
				path = np2cfg.idecd[i];
			}else
#endif
			{
				path = sxsi_getfilename((REG8)i);
			}
			ret |= statflag_writepath(sfh, path, FTYPE_NONE, 0);
		}
	}
	for (i=0; i<NELEMENTS(sds.scsi); i++) {
		if (sds.scsi[i] != SXSIDEV_NC) {
			path = sxsi_getfilename((REG8)(i + 0x20));
			ret |= statflag_writepath(sfh, path, FTYPE_NONE, 0);
		}
	}
	(void)tbl;
	return(ret);
}

static int flagcheck_sxsi(STFLAGH sfh, const SFENTRY *tbl) {

	int			ret;
	SXSIDEVS	sds;
	UINT		i;
	OEMCHAR		buf[8];

	sxsi_allflash();
	ret = statflag_read(sfh, &sds, sizeof(sds));
	for (i=0; i<NELEMENTS(sds.ide); i++) {
		if (sds.ide[i] != SXSIDEV_NC) {
			if(sds.ide[i] != SXSIDEV_CDROM) {
				OEMSPRINTF(buf, str_sasix, i+1);
				ret |= statflag_checkpath(sfh, buf);
			}else{
				OEMSPRINTF(buf, str_sasix, i+1);
				statflag_checkpath(sfh, buf); // CD�̎��A�t���O�ɂ͉e�������Ȃ�
			}
		}
	}
	for (i=0; i<NELEMENTS(sds.scsi); i++) {
		if (sds.scsi[i] != SXSIDEV_NC) {
			if(sds.ide[i] != SXSIDEV_CDROM) {
				OEMSPRINTF(buf, str_scsix, i);
				ret |= statflag_checkpath(sfh, buf);
			}else{
				OEMSPRINTF(buf, str_scsix, i);
				statflag_checkpath(sfh, buf); // CD�̎��A�t���O�ɂ͉e�������Ȃ�
			}
		}
	}
	(void)tbl;
	return(ret);
}

static int flagload_sxsi(STFLAGH sfh, const SFENTRY *tbl) {

	int			ret;
	SXSIDEVS	sds;
	UINT		i;
	REG8		drv;
	STATPATH	sp;
	
	ret = statflag_read(sfh, &sds, sizeof(sds));
	if (ret != STATFLAG_SUCCESS) {
		return(ret);
	}
	for (i=0; i<NELEMENTS(sds.ide); i++) {
		drv = (REG8)i;
		sxsi_setdevtype(drv, sds.ide[i]);
		if (sds.ide[i] != SXSIDEV_NC) {
			ret |= statflag_read(sfh, &sp, sizeof(sp));
			sxsi_devopen(drv, sp.path);
		}
	}
	for (i=0; i<NELEMENTS(sds.scsi); i++) {
		drv = (REG8)(i + 0x20);
		sxsi_setdevtype(drv, sds.scsi[i]);
		if (sds.scsi[i] != SXSIDEV_NC) {
			ret |= statflag_read(sfh, &sp, sizeof(sp));
			sxsi_devopen(drv, sp.path);
		}
	}
	(void)tbl;
	return(ret);
}


// ---- com

static int flagsave_com(STFLAGH sfh, const SFENTRY *tbl) {

	UINT	device;
	COMMNG	cm;
	int		ret;
	COMFLAG	flag;

	device = (UINT)(INTPTR)tbl->arg1;
	switch(device) {
		case 0:
			cm = cm_mpu98;
			break;

		case 1:
			cm = cm_rs232c;
			break;
			
#if defined(SUPPORT_SMPU98)
		case 2:
			cm = cm_smpu98[0];
			break;
			
		case 3:
			cm = cm_smpu98[1];
			break;
#endif

		default:
			cm = NULL;
			break;
	}
	ret = STATFLAG_SUCCESS;
	if (cm) {
		flag = (COMFLAG)cm->msg(cm, COMMSG_GETFLAG, 0);
		if (flag) {
			ret |= statflag_write(sfh, flag, flag->size);
			_MFREE(flag);
		}
	}
	return(ret);
}

static int flagload_com(STFLAGH sfh, const SFENTRY *tbl) {

	UINT		device;
	COMMNG		cm;
	int			ret;
	_COMFLAG	fhdr;
	COMFLAG		flag;

	ret = statflag_read(sfh, &fhdr, sizeof(fhdr));
	if (ret != STATFLAG_SUCCESS) {
		goto flcom_err1;
	}
	if (fhdr.size < sizeof(fhdr)) {
		goto flcom_err1;
	}
	flag = (COMFLAG)_MALLOC(fhdr.size, "com stat flag");
	if (flag == NULL) {
		goto flcom_err1;
	}
	CopyMemory(flag, &fhdr, sizeof(fhdr));
	ret |= statflag_read(sfh, flag + 1, fhdr.size - sizeof(fhdr));
	if (ret != STATFLAG_SUCCESS) {
		goto flcom_err2;
	}

	device = (UINT)(INTPTR)tbl->arg1;
	switch(device) {
		case 0:
			commng_destroy(cm_mpu98);
			cm = commng_create(COMCREATE_MPU98II, FALSE);
			cm_mpu98 = cm;
			break;

		case 1:
			commng_destroy(cm_rs232c);
			cm = commng_create(COMCREATE_SERIAL, FALSE);
			cm_rs232c = cm;
			break;
			
#if defined(SUPPORT_SMPU98)
		case 2:
			commng_destroy(cm_smpu98[0]);
			cm = commng_create(COMCREATE_SMPU98_A, FALSE);
			cm_smpu98[0] = cm;
			break;

		case 3:
			commng_destroy(cm_smpu98[1]);
			cm = commng_create(COMCREATE_SMPU98_B, FALSE);
			cm_smpu98[1] = cm;
			break;
#endif

		default:
			cm = NULL;
			break;
	}
	if (cm) {
		cm->msg(cm, COMMSG_SETFLAG, (INTPTR)flag);
	}

flcom_err2:
	_MFREE(flag);

flcom_err1:
	return(ret);
}


// ----

static int flagcheck_versize(STFLAGH sfh, const SFENTRY *tbl) {

	if ((sfh->hdr.ver == tbl->ver) && ((sfh->hdr.size == tbl->arg2) || 
		((tbl->type & STATFLAG_BWD_COMPATIBLE) && sfh->hdr.size < tbl->arg2) || 
		((tbl->type & STATFLAG_FWD_COMPATIBLE) && sfh->hdr.size > tbl->arg2))) {
		return(STATFLAG_SUCCESS);
	}
	return(STATFLAG_FAILURE);
}

static int flagcheck_veronly(STFLAGH sfh, const SFENTRY *tbl) {

	if (sfh->hdr.ver == tbl->ver) {
		return(STATFLAG_SUCCESS);
	}
	return(STATFLAG_FAILURE);
}


// ----

int statsave_save(const OEMCHAR *filename) {

	SFFILEH		sffh;
	int			ret;
const SFENTRY	*tbl;
const SFENTRY	*tblterm;

	sffh = statflag_create(filename);
	if (sffh == NULL) {
		return(STATFLAG_FAILURE);
	}
	
#if defined(SUPPORT_CL_GD5430)
	pc98_cirrus_vga_save();
#endif
	
#if defined(SUPPORT_IA32_HAXM)
	memcpy(vramex_base, vramex, sizeof(vramex_base));
	i386haxfunc_vcpu_getMSRs(&np2haxstat.msrstate);
#endif

	ret = STATFLAG_SUCCESS;
	tbl = np2tbl;
	tblterm = tbl + NELEMENTS(np2tbl);
	while(tbl < tblterm) {
		ret |= statflag_createsection(sffh, tbl);
		switch(tbl->type & STATFLAG_MASK) {
			case STATFLAG_BIN:
			case STATFLAG_TERM:
				ret |= flagsave_common(&sffh->sfh, tbl);
				break;

			case STATFLAG_COM:
				ret |= flagsave_com(&sffh->sfh, tbl);
				break;

			case STATFLAG_DMA:
				ret |= flagsave_dma(&sffh->sfh, tbl);
				break;

			case STATFLAG_EGC:
				ret |= flagsave_egc(&sffh->sfh, tbl);
				break;

			case STATFLAG_EPSON:
				ret |= flagsave_epson(&sffh->sfh, tbl);
				break;

			case STATFLAG_EVT:
				ret |= flagsave_evt(&sffh->sfh, tbl);
				break;

			case STATFLAG_EXT:
				ret |= flagsave_ext(&sffh->sfh, tbl);
				break;

			case STATFLAG_FDD:
				ret |= flagsave_fdd(&sffh->sfh, tbl);
				break;

#if !defined(DISABLE_SOUND)
			case STATFLAG_FM:
				ret |= flagsave_fm(&sffh->sfh, tbl);
				break;
#endif

			case STATFLAG_GIJ:
				ret |= flagsave_gij(&sffh->sfh, tbl);
				break;

#if defined(SUPPORT_HOSTDRV)
			case STATFLAG_HDRV:
				ret |= hostdrv_sfsave(&sffh->sfh, tbl);
				break;
#endif
#if defined(SUPPORT_HOSTDRVNT)
			case STATFLAG_HDRVNT:
				ret |= hostdrvNT_sfsave(&sffh->sfh, tbl);
				break;
#endif

			case STATFLAG_MEM:
				ret |= flagsave_mem(&sffh->sfh, tbl);
				break;

			case STATFLAG_SXSI:
				ret |= flagsave_sxsi(&sffh->sfh, tbl);
				break;
		}
		tbl++;
	}
	statflag_close(sffh);
	return(ret);
}

int statsave_check(const OEMCHAR *filename, OEMCHAR *buf, int size) {

	SFFILEH		sffh;
	int			ret;
	BOOL		done;
const SFENTRY	*tbl;
const SFENTRY	*tblterm;

	sffh = statflag_open(filename, buf, size);
	if (sffh == NULL) {
		return(STATFLAG_FAILURE);
	}

	done = FALSE;
	ret = STATFLAG_SUCCESS;
	while((!done) && (ret != STATFLAG_FAILURE)) {
		ret |= statflag_readsection(sffh);
		tbl = np2tbl;
		tblterm = tbl + NELEMENTS(np2tbl);
		while(tbl < tblterm) {
			if (!memcmp(sffh->sfh.hdr.index, tbl->index, sizeof(sffh->sfh.hdr.index))) {
				break;
			}
			tbl++;
		}
		if (tbl < tblterm) {
			switch(tbl->type & STATFLAG_MASK) {
				case STATFLAG_BIN:
				case STATFLAG_MEM:
					ret |= flagcheck_versize(&sffh->sfh, tbl);
					break;

				case STATFLAG_TERM:
					done = TRUE;
					break;

				case STATFLAG_COM:
				case STATFLAG_DMA:
				case STATFLAG_EGC:
				case STATFLAG_EPSON:
				case STATFLAG_EVT:
				case STATFLAG_EXT:
				case STATFLAG_GIJ:
#if !defined(DISABLE_SOUND)
				case STATFLAG_FM:
#endif
					ret |= flagcheck_veronly(&sffh->sfh, tbl);
					break;

#if defined(SUPPORT_HOSTDRV)
				case STATFLAG_HDRV:
#endif
					ret |= flagcheck_veronly(&sffh->sfh, tbl);
					break;

#if defined(SUPPORT_HOSTDRVNT)
				case STATFLAG_HDRVNT:
#endif
					ret |= flagcheck_veronly(&sffh->sfh, tbl);
					break;

				case STATFLAG_FDD:
					ret |= flagcheck_fdd(&sffh->sfh, tbl);
					break;

				case STATFLAG_SXSI:
					ret |= flagcheck_sxsi(&sffh->sfh, tbl);
					break;

				default:
					ret |= STATFLAG_WARNING;
					break;
			}
		}
		else {
			ret |= STATFLAG_WARNING;
		}
	}
	statflag_close(sffh);
	return(ret);
}

int statsave_load(const OEMCHAR *filename) {

	SFFILEH		sffh;
	int			ret;
	BOOL		done;
const SFENTRY	*tbl;
const SFENTRY	*tblterm;
	UINT		i;

	sffh = statflag_open(filename, NULL, 0);
	if (sffh == NULL) {
		return(STATFLAG_FAILURE);
	}

	// PCCORE read!
	ret = statflag_readsection(sffh);
	if ((ret != STATFLAG_SUCCESS) ||
		(memcmp(sffh->sfh.hdr.index, np2tbl[0].index, sizeof(sffh->sfh.hdr.index)))) {
		statflag_close(sffh);
		return(STATFLAG_FAILURE);
	}

	soundmng_stop();
	rs232c_midipanic();
	mpu98ii_midipanic();
#if defined(SUPPORT_SMPU98)
	smpu98_midipanic();
#endif
	pc9861k_midipanic();
	sxsi_alltrash();

	ret |= flagload_common(&sffh->sfh, np2tbl);

	CPU_RESET();
	CPU_SETEXTSIZE((UINT32)pccore.extmem);
	nevent_allreset();

	sound_changeclock();
	beep_changeclock();
	sound_reset();
	fddmtrsnd_bind();

	iocore_reset(&np2cfg);							// �T�E���h��pic���ĂԂ̂Łc
	cbuscore_reset(&np2cfg);
	fmboard_reset(&np2cfg, pccore.sound);

	done = FALSE;
	while((!done) && (ret != STATFLAG_FAILURE)) {
		ret |= statflag_readsection(sffh);
		tbl = np2tbl + 1;
		tblterm = np2tbl + NELEMENTS(np2tbl);
		while(tbl < tblterm) {
			if (!memcmp(sffh->sfh.hdr.index, tbl->index, sizeof(sffh->sfh.hdr.index))) {
				break;
			}
			tbl++;
		}
		if (tbl < tblterm) {
			switch(tbl->type & STATFLAG_MASK) {
				case STATFLAG_BIN:
					ret |= flagload_common(&sffh->sfh, tbl);
					break;

				case STATFLAG_TERM:
					done = TRUE;
					break;

				case STATFLAG_COM:
					ret |= flagload_com(&sffh->sfh, tbl);
					break;

				case STATFLAG_DMA:
					ret |= flagload_dma(&sffh->sfh, tbl);
					break;

				case STATFLAG_EGC:
					ret |= flagload_egc(&sffh->sfh, tbl);
					break;

				case STATFLAG_EPSON:
					ret |= flagload_epson(&sffh->sfh, tbl);
					break;

				case STATFLAG_EVT:
					ret |= flagload_evt(&sffh->sfh, tbl);
					break;

				case STATFLAG_EXT:
					ret |= flagload_ext(&sffh->sfh, tbl);
					break;

				case STATFLAG_FDD:
					ret |= flagload_fdd(&sffh->sfh, tbl);
					break;

#if !defined(DISABLE_SOUND)
				case STATFLAG_FM:
					ret |= flagload_fm(&sffh->sfh, tbl);
					break;
#endif

				case STATFLAG_GIJ:
					ret |= flagload_gij(&sffh->sfh, tbl);
					break;

#if defined(SUPPORT_HOSTDRV)
				case STATFLAG_HDRV:
					ret |= hostdrv_sfload(&sffh->sfh, tbl);
					break;
#endif

#if defined(SUPPORT_HOSTDRVNT)
				case STATFLAG_HDRVNT:
					ret |= hostdrvNT_sfload(&sffh->sfh, tbl);
					break;
#endif

				case STATFLAG_MEM:
					ret |= flagload_mem(&sffh->sfh, tbl);
					break;

				case STATFLAG_SXSI:
					ret |= flagload_sxsi(&sffh->sfh, tbl);
					break;

				default:
					ret |= STATFLAG_WARNING;
					break;
			}
		}
		else {
			ret |= STATFLAG_WARNING;
		}
	}
	statflag_close(sffh);

	// �X�e�[�g�Z�[�u�݊����ێ��p
	if(pccore.maxmultiple == 0) pccore.maxmultiple = pccore.multiple;
	
#if defined(SUPPORT_IA32_HAXM)
	memcpy(vramex, vramex_base, sizeof(vramex_base));
	i386haxfunc_vcpu_setREGs(&np2haxstat.state);
	i386haxfunc_vcpu_setFPU(&np2haxstat.fpustate);
	{
		HAX_MSR_DATA	msrstate_set = {0};
		i386haxfunc_vcpu_setMSRs(&np2haxstat.msrstate, &msrstate_set);
	}
	i386hax_vm_sethmemory(CPU_ADRSMASK != 0x000fffff);
	i386hax_vm_setitfmemory(CPU_ITFBANK);
	i386hax_vm_setvga256linearmemory();
	np2haxcore.clockpersec = GetTickCounter_ClockPerSec();
	np2haxcore.lastclock = GetTickCounter_Clock();
	np2haxcore.clockcount = GetTickCounter_Clock();
	np2haxcore.I_ratio = 0;
#endif

	// I/O��蒼��
	MEMM_ARCH((pccore.model & PCMODEL_EPSON)?1:0);
	iocore_build();
	iocore_bind();
	cbuscore_bind();
	fmboard_bind();
	
	// DA/UA�Ɨv�f�ԍ��̑Ή��֌W��������
	for(i=0;i<4;i++){
		sxsi_unittbl[i] = i;
	}
#if defined(SUPPORT_IDEIO)
	if (pccore.hddif & PCHDD_IDE) {
		int i, idx, ncidx;
		// ���ڑ��̂��̂𖳎����Đڑ�����DA/UA�����蓖�Ă�
		ncidx = idx = 0;
		for(i=0;i<4;i++){
			if(sxsi_getdevtype(i)==SXSIDEV_HDD){
				sxsi_unittbl[idx] = i;
				idx++;
			}else{
				ncidx = i;
			}
		}
		for(;idx<4;idx++){
			sxsi_unittbl[idx] = ncidx; // XXX: �]����DA/UA�͂Ƃ肠�������ڑ��̔ԍ��ɐݒ�
		}
	}
#endif

#if defined(SUPPORT_PC9821)&&defined(SUPPORT_PCI)
	pcidev_bind();
#endif

#if defined(CPUCORE_IA32)
#if defined(USE_CPU_EIPMASK)
	CPU_EIPMASK = CPU_STATSAVE.cpu_inst_default.op_32 ? 0xffffffff : 0xffff;
#endif
	fpu_initialize();
#endif

#if defined(SUPPORT_NET)
	np2net_reset(&np2cfg);
	np2net_bind();
#endif
#if defined(SUPPORT_LGY98)
	lgy98_bind();
#endif
#if defined(SUPPORT_WAB)
	np2wab_bind();
#endif
#if defined(SUPPORT_CL_GD5430)
	pc98_cirrus_vga_bind();
	pc98_cirrus_vga_load();
#endif
	
	// OPNA�{�����[���Đݒ�
	if(g_nSoundID == SOUNDID_WAVESTAR){
		opngen_setvol(np2cfg.vol_fm * cs4231.devvolume[0xff] / 15 * np2cfg.vol_master / 100);
		psggen_setvol(np2cfg.vol_ssg * cs4231.devvolume[0xff] / 15 * np2cfg.vol_master / 100);
		rhythm_setvol(np2cfg.vol_rhythm * cs4231.devvolume[0xff] / 15 * np2cfg.vol_master / 100);
#if defined(SUPPORT_FMGEN)
		if(np2cfg.usefmgen) {
			opna_fmgen_setallvolumeFM_linear(np2cfg.vol_fm * cs4231.devvolume[0xff] / 15 * np2cfg.vol_master / 100);
			opna_fmgen_setallvolumePSG_linear(np2cfg.vol_ssg * cs4231.devvolume[0xff] / 15 * np2cfg.vol_master / 100);
			opna_fmgen_setallvolumeRhythmTotal_linear(np2cfg.vol_rhythm * cs4231.devvolume[0xff] / 15 * np2cfg.vol_master / 100);
		}
#endif
	}else{
		opngen_setvol(np2cfg.vol_fm * np2cfg.vol_master / 100);
		psggen_setvol(np2cfg.vol_ssg * np2cfg.vol_master / 100);
		rhythm_setvol(np2cfg.vol_rhythm * np2cfg.vol_master / 100);
#if defined(SUPPORT_FMGEN)
		if(np2cfg.usefmgen) {
			opna_fmgen_setallvolumeFM_linear(np2cfg.vol_fm * np2cfg.vol_master / 100);
			opna_fmgen_setallvolumePSG_linear(np2cfg.vol_ssg * np2cfg.vol_master / 100);
			opna_fmgen_setallvolumeRhythmTotal_linear(np2cfg.vol_rhythm * np2cfg.vol_master / 100);
		}
#endif
	}
	for (i = 0; i < _countof(g_opna); i++)
	{
		rhythm_update(&g_opna[i].rhythm);
	}

	gdcs.textdisp |= GDCSCRN_EXT;
	gdcs.textdisp |= GDCSCRN_ALLDRAW2;
	gdcs.grphdisp |= GDCSCRN_EXT;
	gdcs.grphdisp |= GDCSCRN_ALLDRAW2;
	gdcs.palchange = GDCSCRN_REDRAW;
	tramflag.renewal = 1;
	cgwindow.writable |= 0x80;
#if defined(CPUSTRUC_FONTPTR)
	FONTPTR_LOW = fontrom + cgwindow.low;
	FONTPTR_HIGH = fontrom + cgwindow.high;
#endif
	MEMM_VRAM(vramop.operate);
	fddmtr_reset();
	soundmng_play();

#if defined(SUPPORT_WAB)
	{
		UINT8 wabaswtmp = np2cfg.wabasw;
		np2cfg.wabasw = 1; // �����[����炳�Ȃ�
		np2wab.relay = 0;
		np2wab_setRelayState(np2wab.relaystateint|np2wab.relaystateext);
		np2wab.realWidth = np2wab.wndWidth; // XXX: ???
		np2wab.realHeight = np2wab.wndHeight; // XXX: ???
		np2wab.lastWidth = 0;
		np2wab.lastHeight = 0;
		np2wab_setScreenSize(np2wab.wndWidth, np2wab.wndHeight);
		np2cfg.wabasw = wabaswtmp;
	}
#endif
	
	pit_setrs232cspeed((pit.ch + 2)->value);
#if defined(SUPPORT_RS232C_FIFO)
	rs232c_vfast_setrs232cspeed(rs232cfifo.vfast);
#endif

	// �J�[�\���\����ԕ���
	mousemng_updateautohidecursor();

	return(ret);
}

// ---- Extended API Implementation for 200-slot functionality

static NP2SLOT_MASTER g_slot_master;
static BOOL g_slot_master_loaded = FALSE;

/**
 * @brief Load slot master file
 */
static int slot_master_load(void)
{
	FILEH fh;
	OEMCHAR path[MAX_PATH];

	if (g_slot_master_loaded) {
		return SUCCESS;
	}

	// Build path to slots master file
	// file_cpyname(path, modulefile, sizeof(path));
	file_cpyname(path, OEMTEXT("."), sizeof(path));
	file_cutname(path);
	file_catname(path, OEMTEXT("np2.slots"), sizeof(path));

	fh = file_open_rb(path);
	if (fh == FILEH_INVALID) {
		// Initialize empty slot master
		ZeroMemory(&g_slot_master, sizeof(g_slot_master));
		g_slot_master.signature = 0x544F4C53; // 'SLOT'
		g_slot_master.version = 1;
		g_slot_master.slot_count = 200;
		g_slot_master.used_count = 0;
		g_slot_master_loaded = TRUE;
		return SUCCESS;
	}

	if (file_read(fh, &g_slot_master, sizeof(g_slot_master)) != sizeof(g_slot_master)) {
		file_close(fh);
		return FAILURE;
	}

	file_close(fh);

	// Validate signature
	if (g_slot_master.signature != 0x544F4C53) {
		return FAILURE;
	}

	g_slot_master_loaded = TRUE;
	return SUCCESS;
}

/**
 * @brief Save slot master file
 */
static int slot_master_save(void)
{
	FILEH fh;
	OEMCHAR path[MAX_PATH];

	// Build path to slots master file
	// file_cpyname(path, modulefile, sizeof(path));
	file_cpyname(path, OEMTEXT("."), sizeof(path));
	file_cutname(path);
	file_catname(path, OEMTEXT("np2.slots"), sizeof(path));

	fh = file_create(path);
	if (fh == FILEH_INVALID) {
		return FAILURE;
	}

	// Update checksum
	g_slot_master.master_checksum = 0; // TODO: Calculate actual checksum

	if (file_write(fh, &g_slot_master, sizeof(g_slot_master)) != sizeof(g_slot_master)) {
		file_close(fh);
		return FAILURE;
	}

	file_close(fh);
	return SUCCESS;
}

/**
 * @brief Validate slot range
 */
static int slot_validate_range(int slot)
{
	return (slot >= 0 && slot < 200) ? SUCCESS : FAILURE;
}

/**
 * @brief Extended save with slot number and comment
 */
int statsave_save_ext(int slot, const char *comment)
{
	OEMCHAR filename[MAX_PATH];
	OEMCHAR ext[8];
	int ret;

	TRACEOUT(("StatsaveExt: Save slot %d requested\n", slot));
	if (slot_validate_range(slot) != SUCCESS) {
		TRACEOUT(("StatsaveExt: Invalid slot range %d\n", slot));
		return FAILURE;
	}

	if (slot_master_load() != SUCCESS) {
		TRACEOUT(("StatsaveExt: Failed to load slot master\n"));
		return FAILURE;
	}

	// Generate filename
	OEMSPRINTF(ext, OEMTEXT("S%03d"), slot);
	// file_cpyname(filename, modulefile, sizeof(filename));
	file_cpyname(filename, OEMTEXT("."), sizeof(filename));
	file_cutname(filename);
	file_catname(filename, OEMTEXT("NP2."), sizeof(filename));
	file_catname(filename, ext, sizeof(filename));

	TRACEOUT(("StatsaveExt: Saving to file: %s\n", filename));
	// Save state file
	ret = statsave_save(filename);
	if (ret == SUCCESS) {
		TRACEOUT(("StatsaveExt: State save successful for slot %d\n", slot));
#ifdef _WIN32
		// Create and save thumbnail automatically
		HBITMAP hThumbnail = statsave_create_thumbnail();
		if (hThumbnail) {
			statsave_save_thumbnail(slot, hThumbnail);
			DeleteObject(hThumbnail);
		}
#endif

		// Update slot info
		NP2SLOT_INFO *info = &g_slot_master.slots[slot];
		info->used = TRUE;
		info->protect_flag = FALSE;
		info->flags = 0;
#ifdef _WIN32
		// Set save time (Windows specific)
		GetSystemTimeAsFileTime((FILETIME*)&info->save_time);
#endif
		// info->file_size = 0; // TODO: Get actual file size

		if (comment) {
			strncpy(info->comment, comment, sizeof(info->comment) - 1);
			info->comment[sizeof(info->comment) - 1] = '\0';
		} else {
			// Clear comment
			info->comment[0] = '\0';
		}

		// Set default title (can be improved with game detection)
		snprintf(info->title, sizeof(info->title), "Slot %03d", slot);

		// Update used count
		int used_count = 0;
		for (int i = 0; i < 200; i++) {
			if (g_slot_master.slots[i].used) {
				used_count++;
			}
		}
		g_slot_master.used_count = used_count;

		slot_master_save();
	}

	return ret;
}

/**
 * @brief Extended load with slot number
 */
int statsave_load_ext(int slot)
{
	OEMCHAR filename[MAX_PATH];
	OEMCHAR ext[8];

	TRACEOUT(("StatsaveExt: Load slot %d requested\n", slot));
	if (slot_validate_range(slot) != SUCCESS) {
		TRACEOUT(("StatsaveExt: Invalid slot range %d\n", slot));
		return FAILURE;
	}

	if (slot_master_load() != SUCCESS) {
		TRACEOUT(("StatsaveExt: Failed to load slot master\n"));
		return FAILURE;
	}

	// Check if slot is used
	if (!g_slot_master.slots[slot].used) {
		TRACEOUT(("StatsaveExt: Slot %d is not used\n", slot));
		return FAILURE;
	}

	// Generate filename
	OEMSPRINTF(ext, OEMTEXT("S%03d"), slot);
	// file_cpyname(filename, modulefile, sizeof(filename));
	file_cpyname(filename, OEMTEXT("."), sizeof(filename));
	file_cutname(filename);
	file_catname(filename, OEMTEXT("NP2."), sizeof(filename));
	file_catname(filename, ext, sizeof(filename));

	TRACEOUT(("StatsaveExt: Loading from file: %s\n", filename));
	int ret = statsave_load(filename);
	if (ret == SUCCESS) {
		TRACEOUT(("StatsaveExt: State load successful for slot %d\n", slot));
	} else {
		TRACEOUT(("StatsaveExt: State load failed for slot %d\n", slot));
	}
	return ret;
}

/**
 * @brief Delete slot
 */
int statsave_delete_ext(int slot)
{
	OEMCHAR filename[MAX_PATH];
	OEMCHAR ext[8];

	if (slot_validate_range(slot) != SUCCESS) {
		return FAILURE;
	}

	if (slot_master_load() != SUCCESS) {
		return FAILURE;
	}

	// Generate filename
	OEMSPRINTF(ext, OEMTEXT("S%03d"), slot);
	// file_cpyname(filename, modulefile, sizeof(filename));
	file_cpyname(filename, OEMTEXT("."), sizeof(filename));
	file_cutname(filename);
	file_catname(filename, OEMTEXT("NP2."), sizeof(filename));
	file_catname(filename, ext, sizeof(filename));

	// Delete state file
	file_delete(filename);

	// Delete thumbnail file
	file_catname(filename, OEMTEXT(".thumb"), sizeof(filename));
	file_delete(filename);

	// Update slot info
	ZeroMemory(&g_slot_master.slots[slot], sizeof(NP2SLOT_INFO));

	// Update used count
	int used_count = 0;
	for (int i = 0; i < 200; i++) {
		if (g_slot_master.slots[i].used) {
			used_count++;
		}
	}
	g_slot_master.used_count = used_count;

	slot_master_save();
	return SUCCESS;
}

/**
 * @brief Get slot information
 */
int statsave_get_info(int slot, NP2SLOT_INFO *info)
{
	if (slot_validate_range(slot) != SUCCESS || !info) {
		return FAILURE;
	}

	if (slot_master_load() != SUCCESS) {
		return FAILURE;
	}

	*info = g_slot_master.slots[slot];
	return SUCCESS;
}

/**
 * @brief Get total slot count
 */
int statsave_get_slot_count(void)
{
	return 200;
}

/**
 * @brief Get used slots
 */
int statsave_get_used_slots(int *slots, int max_count)
{
	if (!slots || max_count <= 0) {
		return 0;
	}

	if (slot_master_load() != SUCCESS) {
		return 0;
	}

	int count = 0;
	for (int i = 0; i < 200 && count < max_count; i++) {
		if (g_slot_master.slots[i].used) {
			slots[count++] = i;
		}
	}

	return count;
}

/**
 * @brief Get slot master
 */
int statsave_get_slot_master(NP2SLOT_MASTER *master)
{
	if (!master) {
		return FAILURE;
	}

	if (slot_master_load() != SUCCESS) {
		return FAILURE;
	}

	*master = g_slot_master;
	return SUCCESS;
}

#ifdef _WIN32
// ---- Windows-specific thumbnail implementation

#include "win9x/np2.h"

/**
 * @brief Create thumbnail from current screen
 */
HBITMAP statsave_create_thumbnail(void)
{
	HDC hdc, hdcMem;
	HBITMAP hBitmap;
	BITMAPINFO bmi;
	void *pBits;

	// Create thumbnail size (80x64 recommended)
	int thumb_width = 80;
	int thumb_height = 64;

	// Create compatible DC and bitmap
	hdc = GetDC(NULL);
	hdcMem = CreateCompatibleDC(hdc);

	// Setup bitmap info for 24-bit thumbnail
	ZeroMemory(&bmi, sizeof(bmi));
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = thumb_width;
	bmi.bmiHeader.biHeight = -thumb_height; // Negative for top-down
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 24;
	bmi.bmiHeader.biCompression = BI_RGB;

	// Create DIB section for thumbnail
	hBitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &pBits, NULL, 0);
	if (!hBitmap) {
		DeleteDC(hdcMem);
		ReleaseDC(NULL, hdc);
		return NULL;
	}

	// Fill with a simple pattern for now
	if (pBits) {
		memset(pBits, 0x80, thumb_width * thumb_height * 3);
	}

	// Cleanup
	DeleteDC(hdcMem);
	ReleaseDC(NULL, hdc);

	return hBitmap;
}

/**
 * @brief Save thumbnail for a slot
 */
int statsave_save_thumbnail(int slot, HBITMAP hBitmap)
{
	OEMCHAR filename[MAX_PATH];
	OEMCHAR ext[8];
	FILEH fh;
	BITMAP bm;
	BITMAPINFOHEADER bih;
	BITMAPFILEHEADER bfh;
	HDC hdc;
	UINT8 *bits;
	DWORD bytesWritten;

	if (slot_validate_range(slot) != SUCCESS || !hBitmap) {
		return FAILURE;
	}

	// Generate thumbnail filename
	OEMSPRINTF(ext, OEMTEXT("S%03d"), slot);
	// file_cpyname(filename, modulefile, sizeof(filename));
	file_cpyname(filename, OEMTEXT("."), sizeof(filename));
	file_cutname(filename);
	file_catname(filename, OEMTEXT("NP2."), sizeof(filename));
	file_catname(filename, ext, sizeof(filename));
	file_catname(filename, OEMTEXT(".thumb"), sizeof(filename));

	// Get bitmap info
	if (!GetObject(hBitmap, sizeof(BITMAP), &bm)) {
		return FAILURE;
	}

	// Setup bitmap headers
	ZeroMemory(&bih, sizeof(bih));
	bih.biSize = sizeof(BITMAPINFOHEADER);
	bih.biWidth = bm.bmWidth;
	bih.biHeight = bm.bmHeight;
	bih.biPlanes = 1;
	bih.biBitCount = 24;
	bih.biCompression = BI_RGB;
	bih.biSizeImage = ((bm.bmWidth * 3 + 3) & ~3) * bm.bmHeight;

	ZeroMemory(&bfh, sizeof(bfh));
	bfh.bfType = 0x4D42; // 'BM'
	bfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + bih.biSizeImage;
	bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	// Allocate buffer for bitmap bits
	bits = (UINT8*)_MALLOC(bih.biSizeImage, "thumbnail bits");
	if (!bits) {
		return FAILURE;
	}

	// Get bitmap bits
	hdc = GetDC(NULL);
	if (!GetDIBits(hdc, hBitmap, 0, bm.bmHeight, bits, (BITMAPINFO*)&bih, DIB_RGB_COLORS)) {
		ReleaseDC(NULL, hdc);
		_MFREE(bits);
		return FAILURE;
	}
	ReleaseDC(NULL, hdc);

	// Write to file
	fh = file_create(filename);
	if (fh == FILEH_INVALID) {
		_MFREE(bits);
		return FAILURE;
	}

	// Write BMP file header
	if (file_write(fh, &bfh, sizeof(bfh)) != sizeof(bfh)) {
		file_close(fh);
		_MFREE(bits);
		return FAILURE;
	}

	// Write BMP info header
	if (file_write(fh, &bih, sizeof(bih)) != sizeof(bih)) {
		file_close(fh);
		_MFREE(bits);
		return FAILURE;
	}

	// Write bitmap data
	if (file_write(fh, bits, bih.biSizeImage) != bih.biSizeImage) {
		file_close(fh);
		_MFREE(bits);
		return FAILURE;
	}

	file_close(fh);
	_MFREE(bits);

	return SUCCESS;
}

/**
 * @brief Load thumbnail for a slot
 */
HBITMAP statsave_load_thumbnail(int slot)
{
	OEMCHAR filename[MAX_PATH];
	OEMCHAR ext[8];
	FILEH fh;
	BITMAPFILEHEADER bfh;
	BITMAPINFOHEADER bih;
	UINT8 *bits;
	HDC hdc;
	HBITMAP hBitmap;

	if (slot_validate_range(slot) != SUCCESS) {
		return NULL;
	}

	// Generate thumbnail filename
	OEMSPRINTF(ext, OEMTEXT("S%03d"), slot);
	// file_cpyname(filename, modulefile, sizeof(filename));
	file_cpyname(filename, OEMTEXT("."), sizeof(filename));
	file_cutname(filename);
	file_catname(filename, OEMTEXT("NP2."), sizeof(filename));
	file_catname(filename, ext, sizeof(filename));
	file_catname(filename, OEMTEXT(".thumb"), sizeof(filename));

	// Open file
	fh = file_open_rb(filename);
	if (fh == FILEH_INVALID) {
		return NULL;
	}

	// Read BMP file header
	if (file_read(fh, &bfh, sizeof(bfh)) != sizeof(bfh)) {
		file_close(fh);
		return NULL;
	}

	// Validate BMP signature
	if (bfh.bfType != 0x4D42) {
		file_close(fh);
		return NULL;
	}

	// Read BMP info header
	if (file_read(fh, &bih, sizeof(bih)) != sizeof(bih)) {
		file_close(fh);
		return NULL;
	}

	// Validate format
	if (bih.biBitCount != 24 || bih.biCompression != BI_RGB) {
		file_close(fh);
		return NULL;
	}

	// Allocate buffer
	bits = (UINT8*)_MALLOC(bih.biSizeImage, "thumbnail load");
	if (!bits) {
		file_close(fh);
		return NULL;
	}

	// Read bitmap data
	if (file_read(fh, bits, bih.biSizeImage) != bih.biSizeImage) {
		file_close(fh);
		_MFREE(bits);
		return NULL;
	}

	file_close(fh);

	// Create bitmap from data
	hdc = GetDC(NULL);
	hBitmap = CreateBitmap(bih.biWidth, bih.biHeight, 1, 24, bits);
	ReleaseDC(NULL, hdc);

	_MFREE(bits);

	return hBitmap;
}

#endif // _WIN32


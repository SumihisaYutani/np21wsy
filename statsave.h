/**
 * @file	statsave.h
 * @brief	Interface of state save
 */

#pragma once

/**
 * Result code
 */
enum
{
	STATFLAG_SUCCESS	= 0,
	STATFLAG_VERCHG		= 0x0002,
	STATFLAG_WARNING	= 0x0080,
	STATFLAG_VERSION	= 0x0100,
	STATFLAG_FAILURE	= -1
};

struct TagStatFlagHandle;
typedef struct TagStatFlagHandle *STFLAGH;

/**
 * @brief The entry of state flag
 */
struct TagStatFlagEntry
{
	char	index[12];
	UINT16	ver;
	UINT16	type;
	void	*arg1;
	UINT	arg2;
};
typedef struct TagStatFlagEntry SFENTRY;

/**
 * @brief Extended state save structures for enhanced functionality
 */

// Extension flags
#define EXT_FLAG_THUMBNAIL      0x00000001  // Contains thumbnail
#define EXT_FLAG_METADATA       0x00000002  // Contains metadata
#define EXT_FLAG_COMPRESSED     0x00000004  // Compressed data
#define EXT_FLAG_ENCRYPTED      0x00000008  // Encrypted (future use)

/**
 * @brief Thumbnail data structure
 */
typedef struct {
    UINT32  signature;      // 'THMB'
    UINT32  width;          // Width (recommended: 80)
    UINT32  height;         // Height (recommended: 64)
    UINT32  bpp;            // Bits per pixel (24)
    UINT32  compression;    // Compression type (0=none, 1=RLE)
    UINT32  data_size;      // Image data size
    UINT8   data[1];        // Variable length image data
} NP2THUMBNAIL;

/**
 * @brief Save state metadata
 */
typedef struct {
    UINT32      signature;      // 'META'
    UINT32      version;        // Metadata version
    UINT64      save_time;      // Save time (FILETIME)
    UINT32      session_time;   // Session time (seconds)
    char        game_title[64]; // Game title (auto-detected)
    char        user_comment[128]; // User comment
    UINT32      cpu_clock;      // CPU clock frequency
    UINT32      mem_size;       // Memory size
    char        disk_names[4][64]; // Inserted disk names
    UINT32      checksum;       // Metadata checksum
} NP2METADATA;

/**
 * @brief Slot information (200 slots)
 */
typedef struct {
    UINT8       used;           // Usage flag
    UINT8       protect_flag;   // Protection flag (renamed from protected)
    UINT16      flags;          // Extension flags
    UINT64      save_time;      // Save time
    UINT32      file_size;      // File size
    char        title[32];      // Short title
    char        comment[64];    // Comment
    UINT32      checksum;       // File checksum
} NP2SLOT_INFO;

/**
 * @brief Slot management master
 */
typedef struct {
    UINT32          signature;      // 'SLOT'
    UINT32          version;        // Version
    UINT32          slot_count;     // Total slot count (200)
    UINT32          used_count;     // Used slot count
    NP2SLOT_INFO    slots[200];     // Slot info array
    UINT32          master_checksum; // Master checksum
} NP2SLOT_MASTER;

#ifdef __cplusplus
extern "C"
{
#endif

int statflag_read(STFLAGH sfh, void *ptr, UINT size);
int statflag_write(STFLAGH sfh, const void *ptr, UINT size);
void statflag_seterr(STFLAGH sfh, const OEMCHAR *str);

int statsave_save(const OEMCHAR *filename);
int statsave_check(const OEMCHAR *filename, OEMCHAR *buf, int size);
int statsave_load(const OEMCHAR *filename);

// Extended API for 200-slot functionality
int statsave_save_ext(int slot, const char *comment);
#ifdef _WIN32
int statsave_save_ext_with_hwnd(int slot, const char *comment, HWND hMainWnd);
#endif
int statsave_load_ext(int slot);
int statsave_delete_ext(int slot);
int statsave_get_info(int slot, NP2SLOT_INFO *info);

// Thumbnail operations
#ifdef _WIN32
HBITMAP statsave_create_thumbnail(HWND hWnd);
int statsave_save_thumbnail(int slot, HBITMAP hBitmap);
HBITMAP statsave_load_thumbnail(int slot);
int statsave_check_slot_exists(int slot);
#endif

// Slot management
int statsave_get_slot_count(void);
int statsave_get_used_slots(int *slots, int max_count);
int statsave_get_slot_master(NP2SLOT_MASTER *master);
int slot_master_reload(void);

#ifdef __cplusplus
}
#endif

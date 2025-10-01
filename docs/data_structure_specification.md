# NP21WSY PC-9801 Emulator - Data Structure Specification

## 1. 概要

本仕様書は、NP21WSY PC-9801エミュレータで使用される主要なデータ構造について詳細に説明します。

---

## 2. 設定管理構造体

### 2.1 NP2CFG構造体

エミュレータの設定を管理するメイン構造体。

```c
struct tagNP2Config {
    // ハードウェア設定（頻繁に参照される項目）
    UINT8   uPD72020;           // μPD72020搭載有無
    UINT8   DISPSYNC;           // 表示同期モード
    UINT8   RASTER;             // ラスター設定
    UINT8   realpal;            // リアルパレット使用
    UINT8   LCD_MODE;           // LCD表示モード
    UINT8   skipline;           // スキップライン
    UINT16  skiplight;          // スキップライト

    // 入力設定
    UINT8   KEY_MODE;           // キーボードモード
    UINT8   XSHIFT;             // Xキーシフト
    UINT8   BTN_RAPID;          // 連射ボタン設定
    UINT8   BTN_MODE;           // ボタンモード

    // システム設定
    UINT8   dipsw[3];           // DIPスイッチ設定（3バイト）
    UINT8   MOUSERAPID;         // マウス連射設定
    UINT8   calendar;           // カレンダー設定
    UINT8   usefd144;           // 1.44MB FDD使用
    UINT8   wait[6];            // 各種ウェイト設定

    // CPU・メモリ設定（リセット時にのみ参照）
    OEMCHAR model[8];           // マシンモデル名
    UINT    baseclock;          // ベースクロック
    UINT    multiple;           // クロック倍率
    UINT8   usebios;            // BIOS使用設定
    UINT8   memsw[8];           // メモリスイッチ
    UINT8   ITF_WORK;           // ITFワーク領域

#if defined(SUPPORT_LARGE_MEMORY)
    UINT16  EXTMEM;             // 拡張メモリサイズ（MB）
#else
    UINT8   EXTMEM;             // 拡張メモリサイズ（MB）
#endif

    // グラフィック設定
    UINT8   grcg;               // GRCG設定
    UINT8   color16;            // 16色モード
    UINT32  BG_COLOR;           // 背景色
    UINT32  FG_COLOR;           // 前景色

    // サウンド設定
    UINT32  samplingrate;       // サンプリングレート
    UINT16  delayms;            // 遅延時間(ms)
    UINT8   SOUND_SW;           // サウンドボード設定
    UINT8   snd_x;              // サウンド拡張

    // 音源固有設定
    UINT8   snd14opt[3];        // PC-9801-14設定
    UINT8   snd26opt;           // PC-9801-26K設定
    UINT8   snd86opt;           // PC-9801-86設定
    UINT8   spbopt;             // スピークボード設定
    UINT8   spb_vrc, spb_vrl, spb_x; // スピークボード詳細

    // PC-9801-118設定
    UINT16  snd118io;           // I/Oアドレス
    UINT8   snd118id;           // ID設定
    UINT8   snd118dma;          // DMAチャンネル
    UINT8   snd118irqf, snd118irqp, snd118irqm; // IRQ設定
    UINT8   snd118rom;          // ROM設定

    // ボリューム設定
    UINT8   BEEP_VOL;           // BEEP音量
    UINT8   vol14[6];           // PC-9801-14音量
    UINT8   vol_master;         // マスター音量
    UINT8   vol_fm;             // FM音量
    UINT8   vol_ssg;            // SSG音量
    UINT8   vol_adpcm;          // ADPCM音量
    UINT8   vol_pcm;            // PCM音量
    UINT8   vol_rhythm;         // リズム音量
    UINT8   vol_midi;           // MIDI音量

    // ストレージ設定
    UINT8   fddequip;           // FDD装備状況
    OEMCHAR fddfile[4][MAX_PATH]; // FDDファイルパス

#if defined(SUPPORT_IDEIO)
    OEMCHAR sasihdd[4][MAX_PATH]; // SASI HDDファイルパス
    OEMCHAR idecd[4][MAX_PATH];   // IDE CDファイルパス
    UINT8   idetype[4];           // IDEタイプ
    UINT8   idebios;              // IDE BIOS使用
    UINT32  iderwait, idewwait;   // IDE待機時間
#endif

    // ファイルパス
    OEMCHAR fontfile[MAX_PATH];   // フォントファイルパス
    OEMCHAR biospath[MAX_PATH];   // BIOSパス
    OEMCHAR hdrvroot[MAX_PATH];   // ホストドライブルート

    // CPU詳細設定
    char    cpu_vendor[16];       // CPUベンダー名
    UINT32  cpu_family;           // CPUファミリー
    UINT32  cpu_model;            // CPUモデル
    UINT32  cpu_stepping;         // CPUステッピング
    UINT32  cpu_feature;          // 機能フラグ
    char    cpu_brandstring[64];  // ブランド文字列
    UINT8   fpu_type;             // FPUタイプ
};
```

**使用パターン:**
- システム起動時に設定ファイルから読み込み
- 設定変更時に即座に反映される項目と、リセット後に反映される項目に分類
- 状態保存時にシリアライズされる

---

### 2.2 PCCORE構造体

実行時のコア状態を管理。

```c
typedef struct {
    UINT32  baseclock;          // ベースクロック周波数
    UINT    multiple;           // クロック倍率
    UINT8   cpumode;            // CPUモード（8MHz等）
    UINT8   model;              // マシンモデル
    UINT8   hddif;              // HDD接続インターフェース

#if defined(SUPPORT_LARGE_MEMORY)
    UINT16  extmem;             // 拡張メモリサイズ
#else
    UINT8   extmem;             // 拡張メモリサイズ
#endif

    UINT8   dipsw[3];           // リセット時のDIPSW設定
    UINT8   rom;                // ROM設定フラグ
    SOUNDID sound;              // サウンドボードID
    UINT32  device;             // デバイスフラグ
    UINT32  realclock;          // 実際のクロック
    UINT    maxmultiple;        // 最大倍率
} PCCORE;
```

**ビットフィールド詳細:**
- `cpumode`: bit5が8MHzモードフラグ
- `model`: 下位6bitがモデル、bit6がPC-9821、bit7がEPSON
- `hddif`: SASI(0x01)/SCSI(0x02)/IDE(0x04)のOR
- `rom`: BIOS(0x01)/SOUND(0x02)/SASI(0x04)/SCSI(0x08)等のOR

---

## 3. CPU関連構造体

### 3.1 I286レジスタ構造体

80286 CPUのレジスタ状態を管理。

```c
// 8ビットレジスタアクセス用共用体
typedef union {
    REG16   w;                  // 16ビットアクセス
    struct {
        REG8    l;              // 下位8ビット
        REG8    h;              // 上位8ビット
    } b;
} I286REG16;

// レジスタセット全体
typedef struct {
    union {
        struct {
            I286REG16 ax, bx, cx, dx, si, di, bp, sp;
            I286REG16 es, cs, ss, ds, ip, flag;
        } w;                    // ワードアクセス
        struct {
            REG8 al, ah, bl, bh, cl, ch, dl, dh;
            REG8 sil, sih, dil, dih, bpl, bph, spl, sph;
            REG8 esl, esh, csl, csh, ssl, ssh, dsl, dsh;
            REG8 ipl, iph, flag_l, flag_h;
        } b;                    // バイトアクセス
    } r;
} I286REG;

// 記述子テーブルレジスタ
typedef struct {
    UINT16  limit;              // セグメント限界
    UINT32  base;               // ベースアドレス
} I286DTR;
```

### 3.2 CPUコア状態構造体

```c
typedef struct {
    I286REG     r;              // レジスタセット
    I286DTR     GDTR;           // グローバル記述子テーブル
    I286DTR     IDTR;           // 割り込み記述子テーブル
    UINT16      LDTR;           // ローカル記述子テーブル
    I286DTR     LDTRC;          // LDTRキャッシュ
    UINT16      TR;             // タスクレジスタ
    I286DTR     TRC;            // TRキャッシュ
    UINT16      MSW;            // マシンステータスワード

    SINT32      remainclock;    // 残りクロック
    UINT32      baseclock;      // ベースクロック
    UINT32      clock;          // 現在クロック
    UINT32      adrsmask;       // アドレスマスク

    UINT32      es_base;        // セグメントベースアドレス
    UINT32      cs_base;
    UINT32      ss_base;
    UINT32      ds_base;
    UINT32      ss_fix;         // セグメント固定化フラグ
    UINT32      ds_fix;

    UINT8       prefix;         // プリフィックス
    UINT8       trap;           // トラップフラグ
    UINT8       ovflag;         // オーバーフローフラグ
} I286CORE;
```

---

## 4. ステートセーブシステム構造体

### 4.1 NP2SLOT_INFO構造体

セーブスロットの情報を管理する構造体。

```c
typedef struct {
    UINT8       used;               // スロット使用フラグ
    UINT8       protect_flag;       // 保護フラグ
    UINT16      flags;              // 拡張フラグ
    UINT64      save_time;          // 保存時刻
    UINT32      file_size;          // ファイルサイズ
    char        title[32];          // タイトル
    char        comment[64];        // コメント
    UINT32      checksum;           // チェックサム
} NP2SLOT_INFO;
```

### 4.2 NP2SLOT_MASTER構造体

全スロットの管理情報を保持する構造体。

```c
typedef struct {
    NP2SLOT_INFO slots[200];        // 200スロット
    UINT32       used_count;        // 使用中スロット数
    UINT32       version;           // バージョン
    char         signature[16];     // 署名
} NP2SLOT_MASTER;
```

### 4.3 ステートファイル構造体

```c
typedef struct {
    char        name[16];           // "Neko Project II"
    char        vername[28];        // "create by NP2.EXE"
    UINT32      ver;                // バージョン
} NP2FHDR;

typedef struct {
    char        index[10];          // セクション名
    UINT16      ver;                // バージョン
    UINT32      size;               // データサイズ
} NP2FENT;
```

---

## 5. イベントシステム構造体

### 5.1 イベントアイテム構造体

```c
typedef void (*NEVENTCB)(NEVENTITEM item);

struct _neventitem {
    SINT32      clock;          // 実行タイミング（クロック）
    UINT32      flag;           // 状態フラグ
    NEVENTCB    proc;           // コールバック関数
    INTPTR      userData;       // ユーザーデータ
};

// フラグビット定義
enum {
    NEVENT_ENABLE    = 0x0001,  // 有効フラグ
    NEVENT_SETEVENT  = 0x0002,  // イベント設定済み
    NEVENT_WAIT      = 0x0004   // 待機中フラグ
};
```

### 4.2 イベントシステム全体構造体

```c
typedef struct {
    UINT        readyevents;                    // 準備済みイベント数
    UINT        waitevents;                     // 待機イベント数
    NEVENTID    level[NEVENT_MAXEVENTS];        // 優先度レベル
    NEVENTID    waitevent[NEVENT_MAXEVENTS];    // 待機イベントID
    _NEVENTITEM item[NEVENT_MAXEVENTS];         // イベントアイテム配列
} _NEVENT;

// イベントID定義（32個まで）
enum tagNEventId {
    NEVENT_FLAMES       = 0,    // フレーム更新
    NEVENT_ITIMER       = 1,    // 内部タイマー
    NEVENT_BEEP         = 2,    // BEEP音
    NEVENT_RS232C       = 3,    // シリアル通信
    NEVENT_MUSICGEN     = 4,    // 音楽生成
    NEVENT_FMTIMERA     = 5,    // FMタイマーA
    NEVENT_FMTIMERB     = 6,    // FMタイマーB
    // ... 最大32個
};
```

---

## 5. メモリ管理構造体

### 5.1 メモリマップ定義

```c
enum {
    VRAM_STEP   = 0x100000,     // VRAMステップサイズ
    VRAM_B      = 0x0a8000,     // VRAMブループレーン
    VRAM_R      = 0x0b0000,     // VRAMレッドプレーン
    VRAM_G      = 0x0b8000,     // VRAMグリーンプレーン
    VRAM_E      = 0x0e0000,     // VRAM拡張プレーン

    FONT_ADRS   = 0x110000,     // フォントROMアドレス
    ITF_ADRS    = 0x1f8000      // ITF ROMアドレス
};

// グローバルメモリ配列
extern UINT8 mem[0x200000];     // 2MBメモリ空間
```

### 5.2 VRAM操作制御構造体

```c
typedef struct {
    UINT    operate;            // 操作フラグ
#if !defined(CPUSTRUC_MEMWAIT)
    UINT    tramwait;           // テキストRAM待機時間
    UINT    vramwait;           // VRAM待機時間
    UINT    grcgwait;           // GRCG待機時間
#endif
#if defined(SUPPORT_PC9821)
    UINT8   mio1[4];            // PEGC MIO1レジスタ
    UINT8   mio2[0x100];        // PEGC MIO2レジスタ
#endif
} _VRAMOP;

// operateビット定義
enum {
    VOPBIT_ACCESS   = 0,        // アクセスページ
    VOPBIT_EGC      = 1,        // EGC有効
    VOPBIT_GRCG     = 2,        // GRCGビット6-7
    VOPBIT_ANALOG   = 4,        // アナログ有効
    VOPBIT_VGA      = 5         // PC-9821 VGA
};
```

---

## 6. I/O・デバイス構造体

### 6.1 I/Oコア構造体

```c
// I/O関数ポインタ型定義
typedef void (IOOUTCALL *IOOUT)(UINT port, REG8 val);
typedef REG8 (IOINPCALL *IOINP)(UINT port);

// I/Oハンドラ配列（内部実装）
// - 8ビットデコード: 共通I/O (0x00-0xFF)
// - 10ビットデコード: システムI/O (0x000-0x3FF)
// - 12ビットデコード: サウンドI/O (0x000-0xFFF)
// - 16ビットデコード: 拡張I/O (0x0000-0xFFFF)
```

### 6.2 割り込みコントローラ (PIC)

```c
typedef struct {
    UINT8   irr;                // 割り込み要求レジスタ
    UINT8   imr;                // 割り込みマスクレジスタ
    UINT8   isr;                // 割り込みサービスレジスタ
    UINT8   icw[5];             // 初期化コマンドワード
    UINT8   ocw[4];             // 動作コマンドワード
    UINT8   prio;               // 優先度
    UINT8   irq;                // 現在のIRQ
    UINT8   mode;               // 動作モード
} _PIC;
```

### 6.3 DMAコントローラ (DMAC)

```c
typedef struct {
    UINT16  adrs;               // アドレス
    UINT16  leng;               // 長さ
    UINT8   mode;               // モード
    UINT8   action;             // アクション
} DMACH;

typedef struct {
    DMACH   dmach[4];           // 4チャンネル
    UINT8   stat;               // ステータス
    UINT8   mask;               // マスク
    UINT8   sreq;               // ソフトウェア要求
    UINT8   mode;               // モード
} _DMAC;
```

---

## 7. グラフィック関連構造体

### 7.1 グラフィックディスプレイコントローラ (GDC)

```c
typedef struct {
    SINT32  clock;              // クロック
    UINT    cnt;                // カウンタ
    UINT8   mod;                // モード
    UINT8   cmd;                // コマンド
    UINT16  para[16];           // パラメータ配列
    UINT8   paracb;             // パラメータカウント
    UINT8   reserved;

    UINT16  cr;                 // カーソル位置
    UINT16  lr;                 // ライトアドレス
    UINT8   active;             // アクティブフラグ
    UINT8   phase;              // フェーズ
    UINT8   stat;               // ステータス
    UINT8   fifocur;            // FIFOカーソル
    UINT16  fifo[16];           // FIFOバッファ
} _GDC;
```

### 7.2 CRTコントローラ (CRTC)

```c
typedef struct {
    UINT8   reg[0x100];         // レジスタ配列
    UINT8   crtcreg;            // 現在のレジスタ番号
    UINT8   mode;               // 表示モード
    UINT8   status;             // ステータス
    UINT8   vblank;             // VBlank状態
} _CRTC;
```

### 7.3 拡張グラフィックコントローラ (EGC)

```c
typedef struct {
    UINT16  ope;                // オペレーション
    UINT16  fgc;                // 前景色
    UINT16  mask;               // マスク
    UINT16  bgc;                // 背景色
    UINT16  sft;                // シフト
    UINT16  leng;               // 長さ
    UINT16  lastvl;             // 最終値
    UINT16  patreg[4];          // パターンレジスタ
    UINT16  fgcreg[4];          // 前景色レジスタ
    UINT16  maskreg[4];         // マスクレジスタ
    UINT16  bgcreg[4];          // 背景色レジスタ

    UINT32  func;               // 機能設定
    UINT32  remain;             // 残りデータ
    UINT32  stack;              // スタック
    UINT8   inptr;              // 入力ポインタ
    UINT8   outptr;             // 出力ポインタ
    UINT8   mask2;              // マスク2
    UINT8   sft8;               // 8ビットシフト
} _EGC;
```

---

## 8. サウンド関連構造体

### 8.1 サウンド設定構造体

```c
typedef struct {
    UINT    rate;               // サンプリングレート
    UINT32  hzbase;             // Hz基準値
    UINT32  clockbase;          // クロック基準値
    UINT32  minclock;           // 最小クロック
    UINT32  lastclock;          // 最終クロック
    UINT    writecount;         // 書き込みカウント
} SOUNDCFG;

// サウンドコールバック型
typedef void (SOUNDCALL * SOUNDCB)(void *hdl, SINT32 *pcm, UINT count);
```

### 8.2 音源ボード識別

```c
enum tagSoundId {
    SOUNDID_NONE                = 0,        // ボードなし
    SOUNDID_PC_9801_14          = 0x01,     // PC-9801-14
    SOUNDID_PC_9801_26K         = 0x02,     // PC-9801-26K
    SOUNDID_PC_9801_86          = 0x04,     // PC-9801-86
    SOUNDID_PC_9801_86_26K      = 0x06,     // 86+26K
    SOUNDID_PC_9801_118         = 0x08,     // PC-9801-118
    SOUNDID_SPEAKBOARD          = 0x20,     // スピークボード
    SOUNDID_SB16                = 0x41,     // Sound Blaster 16
    SOUNDID_AMD98               = 0x80,     // AMD-98
    // ... その他の組み合わせ
};
```

---

## 9. ストレージ関連構造体

### 9.1 フロッピーディスクコントローラ (FDC)

```c
typedef struct {
    UINT8   stat;               // ステータス
    UINT8   drv;                // ドライブ選択
    UINT8   trk;                // トラック
    UINT8   sec;                // セクタ
    UINT8   side;               // サイド
    UINT8   cmd;                // コマンド
    UINT8   phase;              // フェーズ
    UINT8   steptime;           // ステップ時間

    UINT8   result[8];          // リザルトフェーズデータ
    UINT8   resultsize;         // リザルトサイズ
    UINT8   resultpos;          // リザルト位置
} _FDC;
```

### 9.2 SXSI（SASI/SCSI/IDE統合）

```c
typedef struct {
    UINT8   device;             // デバイスタイプ
    UINT8   drivesize;          // ドライブサイズ
    UINT16  headersize;         // ヘッダーサイズ
    UINT32  totals;             // 総セクタ数
    UINT16  sectors;            // セクタ数
    UINT16  surfaces;           // サーフェス数
    UINT16  cylinders;          // シリンダ数
} SXSIDEVINFO;

typedef struct {
    FILEH   fh;                 // ファイルハンドル
    UINT8   flag;               // フラグ
    UINT8   headstep;           // ヘッドステップ
    OEMCHAR fname[MAX_PATH];    // ファイル名
} SXSIDEV;
```

---

## 10. 状態保存・復元構造体

### 10.1 状態フラグエントリ

```c
struct TagStatFlagEntry {
    char    index[12];          // インデックス文字列
    UINT16  ver;                // バージョン
    UINT16  type;               // データタイプ
    void    *arg1;              // データポインタ
    UINT    arg2;               // データサイズ
};

// 状態保存結果コード
enum {
    STATFLAG_SUCCESS    = 0,        // 成功
    STATFLAG_DISKCHG    = 0x0001,   // ディスク変更
    STATFLAG_VERCHG     = 0x0002,   // バージョン変更
    STATFLAG_WARNING    = 0x0080,   // 警告
    STATFLAG_VERSION    = 0x0100,   // バージョン
    STATFLAG_FAILURE    = -1        // 失敗
};
```

---

## 11. データアクセスパターン

### 11.1 設定データの管理
- **読み書き頻度**: 起動時読み込み、設定変更時書き込み
- **アクセス方法**: 構造体メンバー直接アクセス
- **同期**: 設定変更時に`pccore_cfgupdate()`で反映

### 11.2 CPUレジスタアクセス
- **アクセス頻度**: 命令実行毎（高頻度）
- **最適化**: マクロによる直接アクセス（`I286_AX`等）
- **バイト/ワードアクセス**: 共用体による型安全なアクセス

### 11.3 イベント管理
- **優先度管理**: ソート済み配列による高速検索
- **メモリ効率**: 固定サイズ配列（32要素）による予測可能な性能
- **スレッドセーフ**: シングルスレッド前提の設計

### 11.4 メモリマップ
- **物理メモリ**: 線形配列による高速アクセス
- **VRAM**: プレーン分離によるグラフィック処理最適化
- **ROM**: 読み取り専用領域の保護

この仕様書により、NP21WSYの内部データ構造を完全に理解し、効率的な開発・保守が可能になります。
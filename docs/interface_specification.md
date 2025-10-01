# NP21WSY PC-9801 Emulator - Interface Specification

## 1. 概要

本仕様書は、NP21WSY PC-9801エミュレータの主要なインターフェースについて詳細に説明します。

---

## 2. プラットフォーム抽象化インターフェース

### 2.1 画面管理インターフェース

#### 2.1.1 SCRNSURF構造体

```c
typedef struct {
    BYTE*   ptr;        // VRAMポインタ
    int     xalign;     // x方向オフセット
    int     yalign;     // y方向オフセット
    int     width;      // 幅
    int     height;     // 高さ
    UINT    bpp;        // スクリーン色ビット
    int     extend;     // 拡張
} SCRNSURF;
```

#### 2.1.2 画面管理関数

```c
// 画面描画開始
const SCRNSURF* scrnmng_surflock(void);

// 画面描画終了（このタイミングで描画）
void scrnmng_surfunlock(const SCRNSURF* surf);

// 描画サイズの変更
void scrnmng_setwidth(int posx, int width);    // posx, widthは8の倍数
void scrnmng_setextend(int extend);
void scrnmng_setheight(int posy, int height);

// 画面状態取得
BOOL scrnmng_isfullscreen(void);    // フルスクリーン状態の取得
BOOL scrnmng_haveextend(void);      // 拡張機能サポート取得
UINT scrnmng_getbpp(void);          // スクリーン色ビット数の取得

// パレット管理
void scrnmng_palchanged(void);                      // 8bitスクリーン用
RGB16 scrnmng_makepal16(RGB32 pal32);              // 16bitスクリーン用
```

**実装要件:**
- サーフェスサイズは `(width + extend) x height`
- プラットフォーム固有の最適化が可能
- スレッドセーフである必要はない（メインスレッドのみ）

---

### 2.2 サウンド管理インターフェース

#### 2.2.1 サウンドストリーム管理

```c
// サウンドストリームの確保
UINT soundmng_create(UINT rate, UINT ms);
// input:  rate - サンプリングレート(11025/22050/44100)
//         ms   - サンプリングバッファサイズ(ミリ秒)
// return: 実際に確保されたバッファのサンプリング数

// サウンドストリームの終了
void soundmng_destroy(void);

// サウンドストリームのリセット
void soundmng_reset(void);

// サウンドストリームの再生
void soundmng_play(void);

// サウンドストリームの停止
void soundmng_stop(void);

// サウンドストリームのコールバック
void soundmng_sync(void);

// サウンドストリームの出力反転設定
void soundmng_setreverse(BOOL reverse);    // 非0で左右反転
```

#### 2.2.2 PCM再生

```c
// PCM再生
BOOL soundmng_pcmplay(UINT num, BOOL loop);
// input:  num  - PCM番号
//         loop - 非0でループ
// return: 再生開始成功時に非0

// PCM停止
void soundmng_pcmstop(UINT num);
// input:  num - PCM番号
```

**実装要件:**
- msに従う必要はない（プラットフォーム固有の最適化可能）
- NP2のサウンドバッファから戻り値分のみを利用
- リアルタイム性が重要

---

### 2.3 入力デバイスインターフェース

#### 2.3.1 マウス

```c
// マウスの状態取得
BYTE mousemng_getstat(SINT16* x, SINT16* y, int clear);
// input:  clear - 非0でカウンタをリセット
// output: *x    - clearしてからのx方向カウント
//         *y    - clearしてからのy方向カウント
// return: bit7  - 左ボタンの状態 (0:押下)
//         bit5  - 右ボタンの状態 (0:押下)
```

#### 2.3.2 ジョイスティック

```c
// ジョイスティックの状態取得
BYTE joymng_getstat(void);
// return: bit0 - 上ボタンの状態 (0:押下)
//         bit1 - 下ボタンの状態
//         bit2 - 左ボタンの状態
//         bit3 - 右ボタンの状態
//         bit4 - 連射ボタン1の状態
//         bit5 - 連射ボタン2の状態
//         bit6 - ボタン1の状態
//         bit7 - ボタン2の状態
```

---

### 2.4 ファイル・通信インターフェース

#### 2.4.1 シリアル/パラレル/MIDI

```c
// シリアルオープン
COMMNG commng_create(UINT device);
// input:  device - デバイス番号
// return: ハンドル (失敗時NULL)

// シリアルクローズ
void commng_destroy(COMMNG hdl);
// input:  hdl - ハンドル (失敗時NULL)
```

---

## 3. コア制御インターフェース

### 3.1 システム管理

```c
// 状態が変化した場合にコールされる
void sysmng_update(UINT bitmap);

// リセット時にコールされる
void sysmng_cpureset(void);

// システム終了時にコールされる
void taskmng_exit(void);
```

---

## 4. サウンドシステムインターフェース

### 4.1 サウンドデータアクセス

```c
// サウンドPCMロック
const SINT32* sound_pcmlock(void);
// return: PCMデータへのポインタ

// サウンドPCMアンロック
void sound_pcmunlock(const SINT32* hdl);
// input:  hdl - sound_pcmlock()の戻り値
```

**PCMデータフォーマット:**
- 32ビット符号付き整数
- ステレオインターリーブ (L, R, L, R, ...)
- サンプリングレートは設定による

---

### 4.2 サウンドボード制御

#### 4.2.1 レジスタアクセス関数

```c
// 音源ボード出力関数型
typedef void (IOOUTCALL *IOOUT)(UINT port, REG8 val);

// 音源ボード入力関数型
typedef REG8 (IOINPCALL *IOINP)(UINT port);
```

#### 4.2.2 ストリーム登録

```c
// ストリーム登録
void sound_streamregist(void* hdl, SOUNDCB cbfn);
// input:  hdl  - ハンドル
//         cbfn - コールバック関数

// コールバック関数型
typedef void (SOUNDCALL *SOUNDCB)(void* hdl, SINT32* pcm, UINT count);
// input:  hdl   - ハンドル
//         pcm   - PCMバッファ
//         count - サンプル数
```

---

## 5. I/Oシステムインターフェース

### 5.1 I/O関数登録

```c
// 共通I/O - 8ビットデコード
void iocore_attachcmnout(UINT port, IOOUT func);
void iocore_attachcmninp(UINT port, IOINP func);
void iocore_attachcmnoutex(UINT port, UINT mask, const IOOUT* func, UINT funcs);
void iocore_attachcmninpex(UINT port, UINT mask, const IOINP* func, UINT funcs);

// システムI/O - 10ビットデコード
void iocore_attachsysout(UINT port, IOOUT func);
void iocore_attachsysinp(UINT port, IOINP func);
void iocore_attachsysoutex(UINT port, UINT mask, const IOOUT* func, UINT funcs);
void iocore_attachsysinpex(UINT port, UINT mask, const IOINP* func, UINT funcs);

// サウンドI/O - 12ビットデコード
BRESULT iocore_attachsndout(UINT port, IOOUT func);
BRESULT iocore_detachsndout(UINT port);
BRESULT iocore_attachsndinp(UINT port, IOINP func);
BRESULT iocore_detachsndinp(UINT port);

// 拡張I/O - 16ビットデコード
BRESULT iocore_attachout(UINT port, IOOUT func);
BRESULT iocore_detachout(UINT port);
BRESULT iocore_attachinp(UINT port, IOINP func);
BRESULT iocore_detachinp(UINT port);
```

### 5.2 I/Oアクセス関数

```c
// 8ビットI/O
void IOOUTCALL iocore_out8(UINT port, REG8 dat);
REG8 IOINPCALL iocore_inp8(UINT port);

// 16ビットI/O
void IOOUTCALL iocore_out16(UINT port, REG16 dat);
REG16 IOINPCALL iocore_inp16(UINT port);

// 32ビットI/O
void IOOUTCALL iocore_out32(UINT port, UINT32 dat);
UINT32 IOINPCALL iocore_inp32(UINT port);
```

---

## 6. イベントシステムインターフェース

### 6.1 イベント制御

```c
// イベントの追加
void nevent_set(NEVENTID id, SINT32 eventclock, NEVENTCB proc, NEVENTPOSITION absolute);
void nevent_setbyms(NEVENTID id, SINT32 ms, NEVENTCB proc, NEVENTPOSITION absolute);

// イベントの削除
void nevent_reset(NEVENTID id);
void nevent_waitreset(NEVENTID id);

// イベントの動作状況取得
BOOL nevent_iswork(NEVENTID id);

// イベントの強制実行
void nevent_forceexecute(NEVENTID id);

// イベント実行までのクロック数取得
SINT32 nevent_getremain(NEVENTID id);
```

**イベント位置指定:**
```c
enum tagNEventPosition {
    NEVENT_RELATIVE = 0,    // 相対時間
    NEVENT_ABSOLUTE = 1     // 絶対時間
};
```

---

## 7. メモリアクセスインターフェース

### 7.1 物理メモリアクセス

```c
// 物理メモリ読み書き（DMA用）
REG8 MEMCALL memp_read8(UINT32 address);
REG16 MEMCALL memp_read16(UINT32 address);
UINT32 MEMCALL memp_read32(UINT32 address);
void MEMCALL memp_write8(UINT32 address, REG8 value);
void MEMCALL memp_write16(UINT32 address, REG16 value);
void MEMCALL memp_write32(UINT32 address, UINT32 value);

// ブロック転送
void MEMCALL memp_reads(UINT32 address, void* dat, UINT leng);
void MEMCALL memp_writes(UINT32 address, const void* dat, UINT leng);
```

### 7.2 論理メモリアクセス

```c
// セグメントアドレス指定（BIOS用）
REG8 MEMCALL memr_read8(UINT seg, UINT off);
REG16 MEMCALL memr_read16(UINT seg, UINT off);
void MEMCALL memr_write8(UINT seg, UINT off, REG8 value);
void MEMCALL memr_write16(UINT seg, UINT off, REG16 value);
void MEMCALL memr_reads(UINT seg, UINT off, void* dat, UINT leng);
void MEMCALL memr_writes(UINT seg, UINT off, const void* dat, UINT leng);
```

### 7.3 メモリマップ制御

```c
// メモリアーキテクチャ設定
void MEMCALL memm_arch(UINT type);

// VRAM操作設定
void MEMCALL memm_vram(UINT operate);
```

---

## 8. 状態保存・復元インターフェース

### 8.1 状態保存

```c
// 基本状態保存
int statsave_save(const OEMCHAR* filename);

// 状態チェック
int statsave_check(const OEMCHAR* filename, OEMCHAR* buf, int size);

// 基本状態復元
int statsave_load(const OEMCHAR* filename);

// 拡張状態保存（スロット対応）
int statsave_save_ext(int slot, const char* comment);
int statsave_save_ext_with_hwnd(int slot, const char* comment, HWND hMainWnd);

// 拡張状態復元（スロット対応）
int statsave_load_ext(int slot);

// スロット管理
int statsave_delete_slot(int slot);
int statsave_get_info(int slot, NP2SLOT_INFO* info);
int statsave_get_slot_count(void);
int statsave_get_used_slots(int* slots, int max_count);
```

### 8.2 状態フラグハンドル

```c
// データ読み込み
int statflag_read(STFLAGH sfh, void* ptr, UINT size);

// データ書き込み
int statflag_write(STFLAGH sfh, const void* ptr, UINT size);

// エラー設定
void statflag_seterr(STFLAGH sfh, const OEMCHAR* str);
```

---

## 9. CPU制御インターフェース

### 9.1 CPU実行制御

```c
// CPU初期化
void pccore_init(void);

// CPU終了処理
void pccore_term(void);

// CPUリセット
void pccore_reset(void);

// CPU実行
void pccore_exec(BOOL draw);
// input: draw - 描画フラグ
```

### 9.2 割り込み制御

```c
// 割り込み発生
void CPUCALL i286c_intnum(UINT vect, REG16 IP);
// input: vect - 割り込みベクタ番号
//        IP   - 割り込みアドレス

// セレクタ変換
UINT32 i286c_selector(UINT sel);
// input:  sel - セレクタ値
// return: 線形アドレス
```

---

## 10. エラーハンドリング

### 10.1 戻り値の標準化

```c
// 基本結果型
enum {
    SUCCESS = 0,
    FAILURE = 1
};

#ifndef BRESULT
#define BRESULT UINT
#endif
```

### 10.2 エラー状態の報告

- **NULL戻り値**: リソース確保失敗
- **FAILURE戻り値**: 操作失敗
- **負の値**: 致命的エラー

---

## 11. パフォーマンス最適化

### 11.1 高頻度アクセス関数

**CPUレジスタアクセス:**
```c
#define I286_AX    i286core.s.r.w.ax
#define I286_AL    i286core.s.r.b.al
#define I286_FLAG  i286core.s.r.w.flag
```

**メモリアクセス:**
```c
#define i286_memoryread(a)     memp_read8(a)
#define i286_memorywrite(a,v)  memp_write8(a,v)
```

### 11.2 コンパイル時最適化

```c
#ifndef INLINE
#define INLINE
#endif

#ifndef FASTCALL
#define FASTCALL
#endif

#ifndef MEMCALL
#define MEMCALL
#endif
```

---

## 12. 設定可能パラメータ

### 12.1 メモリ最適化

```c
// メモリ最適化レベル
// MEMOPTIMIZE = 0 : x86
// MEMOPTIMIZE = 1 : PowerPC系デスクトップ用RISC
// MEMOPTIMIZE = 2 : StrongARM系組み込み用RISC
```

### 12.2 プラットフォーム固有設定

```c
// OS言語設定
#define OSLANG_SJIS  // Shift-JIS
#define OSLANG_EUC   // EUC

// 改行コード設定
#define OSLINEBREAK_CR    // MacOS "\r"
#define OSLINEBREAK_LF    // Unix "\n"
#define OSLINEBREAK_CRLF  // Windows "\r\n"
```

この仕様書により、NP21WSYの全インターフェースを理解し、効率的な移植・拡張・保守が可能になります。
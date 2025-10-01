# NP21WSY 拡張ステートセーブ機能設計書

## 1. 概要

本設計書では、NP21WSY PC-9801エミュレータのステートセーブ機能を大幅に拡張し、現代的なエミュレータに求められる機能を提供する方針を示します。

---

## 2. 現在の実装分析

### 2.1 既存機能
- **基本保存/復元**: CPU状態、メモリ、デバイス状態の完全な保存・復元
- **セクション構造**: モジュール別のデータ管理
- **互換性機能**: 前方/後方互換性フラグによるバージョン管理
- **エラー検出**: ディスク変更検出、整合性チェック

### 2.2 制限事項
- **単一ファイル**: 1つのセーブファイルのみサポート
- **メタデータ不足**: 保存時刻、説明、識別情報なし
- **UI限定**: 基本的なファイル選択のみ
- **手動操作**: オートセーブ機能なし

---

## 3. 拡張機能設計

### 3.1 マルチスロット機能

#### 3.1.1 スロット管理システム
```c
#define MAX_SAVE_SLOTS 200  // 実装済み: 200スロット対応

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

typedef struct {
    NP2SLOT_INFO slots[200];        // 200スロット
    UINT32       used_count;        // 使用中スロット数
    UINT32       version;           // バージョン
    char         signature[16];     // 署名
} NP2SLOT_MASTER;
```

#### 実装状況
- ✅ **200スロット対応**: 基本的なマルチスロット機能は実装済み
- ✅ **スロット情報管理**: メタデータの永続化機能実装済み
- ❌ **オートセーブ**: 未実装（将来拡張予定）
- ❌ **詳細メタデータ**: プレイ時間、スクリーンショット等は未実装

#### 3.1.2 スロット操作API
```c
// 実装済みAPI
int statsave_save_ext(int slot, const char *comment);
int statsave_save_ext_with_hwnd(int slot, const char *comment, HWND hMainWnd);
int statsave_load_ext(int slot);
int statsave_delete_slot(int slot);
int statsave_get_info(int slot, NP2SLOT_INFO *info);
int statsave_get_slot_count(void);
int statsave_get_used_slots(int *slots, int max_count);

// 将来拡張予定
int statsave_quick_save(void);           // 未実装
int statsave_quick_load(void);           // 未実装
int statsave_auto_save(void);            // 未実装
void statsave_auto_save_enable(BOOL enable);    // 未実装
void statsave_auto_save_set_interval(UINT32 minutes);  // 未実装
```

#### 重要な変更点
- **ディスクパス管理**: 当初設計されたスロット情報内でのディスクパス保存機能は、既存のステートファイル形式に組み込まれた機能と重複するため削除されました。ディスクイメージの自動マウントは既存の`flagsave_sxsi()`/`flagload_sxsi()`/`flagload_fdd()`機能により適切に動作します。

### 3.2 拡張メタデータ

#### 3.2.1 メタデータ構造
```c
typedef struct {
    char            magic[16];          // "NP21WSY_SAVE_V2"
    UINT32          version;
    SYSTEMTIME      save_time;
    UINT32          playtime_total;
    UINT32          save_count;
    OEMCHAR         slot_name[64];
    OEMCHAR         description[256];
    OEMCHAR         disk_names[4][MAX_PATH];
    OEMCHAR         program_name[64];
    UINT8           pc_model;
    UINT32          cpu_clock;
    UINT32          memory_size;
    UINT32          screenshot_size;
    UINT32          data_offset;
    UINT32          data_size;
    UINT32          checksum;
} SAVESTATE_HEADER_V2;
```

#### 3.2.2 スクリーンショット機能
```c
typedef struct {
    UINT16  width;
    UINT16  height;
    UINT16  bits_per_pixel;
    UINT32  image_size;
    UINT8   *image_data;
} SCREENSHOT_DATA;

int statsave_capture_screenshot(SCREENSHOT_DATA *screenshot);
int statsave_save_screenshot(FILEH fh, const SCREENSHOT_DATA *screenshot);
int statsave_load_screenshot(FILEH fh, SCREENSHOT_DATA *screenshot);
```

### 3.3 オートセーブシステム

#### 3.3.1 オートセーブ設定
```c
typedef struct {
    BOOL    enabled;
    UINT32  interval_minutes;
    UINT32  max_slots;
    BOOL    save_on_exit;
    BOOL    save_on_disk_change;
    BOOL    compress_data;
} AUTOSAVE_CONFIG;
```

#### 3.3.2 オートセーブタイマー
```c
void autosave_timer_init(void);
void autosave_timer_callback(void);
void autosave_check_triggers(void);
void autosave_on_exit(void);
```

### 3.4 新ファイル形式

#### 3.4.1 ファイル構造
```
[SAVESTATE_HEADER_V2]
[SCREENSHOT_DATA] (optional)
[COMPRESSED_STATE_DATA]
[CHECKSUM]
```

#### 3.4.2 圧縮サポート
```c
#ifdef SUPPORT_ZLIB
int statsave_compress_data(const void *src, UINT32 src_size, void **dst, UINT32 *dst_size);
int statsave_decompress_data(const void *src, UINT32 src_size, void *dst, UINT32 dst_size);
#endif

typedef struct {
    UINT32  original_size;
    UINT32  compressed_size;
    UINT8   compression_type;   // 0=none, 1=zlib
    UINT8   reserved[3];
} COMPRESSION_HEADER;
```

---

## 4. UI設計

### 4.1 セーブ/ロードダイアログ

#### 4.1.1 機能要件
- **スロット一覧表示**: サムネイル、名前、日時
- **プレビュー機能**: 選択したスロットの詳細表示
- **検索・ソート**: 名前、日時でのフィルタリング
- **バッチ操作**: 複数スロットの削除、エクスポート

#### 4.1.2 ダイアログレイアウト
```
┌─ Save/Load State ────────────────────────────────┐
│ [New Save] [Load] [Delete] [Export] [Import]     │
├──────────────────────────────────────────────────┤
│ Slot │ Screenshot │ Name      │ Date     │ Time  │
├──────┼────────────┼───────────┼──────────┼───────┤
│  1   │ [IMG]      │ Start     │ 01/01/24 │ 12:00 │
│  2   │ [IMG]      │ Boss Fight│ 01/01/24 │ 13:30 │
│ ...  │            │           │          │       │
├──────────────────────────────────────────────────┤
│ Preview: [Screenshot]  │ Details:               │
│                        │ Slot: 2                │
│                        │ Description: Boss...   │
│                        │ Playtime: 1h 30m      │
│                        │ Disk: game.d88         │
└────────────────────────────────────────────────────┘
```

### 4.2 ホットキー拡張

#### 4.2.1 新しいキー割り当て
```c
// 基本操作
#define HOTKEY_QUICK_SAVE       VK_F5
#define HOTKEY_QUICK_LOAD       VK_F9

// 拡張操作 (Shift+キー)
#define HOTKEY_SAVE_AS          (VK_F5 | HOTKEYF_SHIFT)
#define HOTKEY_LOAD_SLOT        (VK_F9 | HOTKEYF_SHIFT)

// スロット直接アクセス (Ctrl+数字)
#define HOTKEY_SAVE_SLOT_1      (VK_1 | HOTKEYF_CONTROL)
#define HOTKEY_LOAD_SLOT_1      (VK_1 | HOTKEYF_ALT)
```

### 4.3 ステータス表示

#### 4.3.1 保存/復元進捗
```c
typedef struct {
    BOOL    in_progress;
    UINT32  current_step;
    UINT32  total_steps;
    OEMCHAR step_name[64];
    UINT32  bytes_processed;
    UINT32  total_bytes;
} SAVESTATE_PROGRESS;

void statsave_show_progress(HWND parent, const SAVESTATE_PROGRESS *progress);
```

---

## 5. 実装計画

### 5.1 Phase 1: 基盤機能
1. **新ファイル形式**: 拡張ヘッダー、メタデータ構造
2. **スロット管理**: 基本的なマルチスロット機能
3. **互換性維持**: 既存形式との読み込み互換性

### 5.2 Phase 2: UI機能
1. **セーブ/ロードダイアログ**: 新しいインターフェース
2. **スクリーンショット**: 画面キャプチャ機能
3. **ホットキー**: 拡張キー操作

### 5.3 Phase 3: 高度な機能
1. **オートセーブ**: 自動保存システム
2. **圧縮**: データ圧縮サポート
3. **エクスポート/インポート**: 外部ファイル連携

### 5.4 Phase 4: 最適化・テスト
1. **パフォーマンス**: 保存/復元速度の最適化
2. **安定性**: 大量テスト、エラーハンドリング
3. **ドキュメント**: ユーザーマニュアル更新

---

## 6. ファイル構成

### 6.1 新規追加ファイル
```
src/
├── statsave_v2.h         # 新しいAPI定義
├── statsave_v2.c         # 拡張機能実装
├── statsave_manager.h    # スロット管理
├── statsave_manager.c    # スロット管理実装
├── statsave_ui.h         # UI関連定義
├── statsave_ui.c         # ダイアログ実装
├── screenshot.h          # スクリーンショット機能
├── screenshot.c          # 画面キャプチャ実装
└── autosave.h/c          # オートセーブ機能
```

### 6.2 既存ファイル変更
```
statsave.h/c              # 互換性関数追加
pccore.h                  # 統計情報追加
win9x/menu.rc             # メニュー項目追加
win9x/resource.h          # リソースID追加
```

---

## 7. 設定項目

### 7.1 新しい設定項目
```ini
[StateSeave]
# マルチスロット設定
MaxSlots=10
AutoSaveEnabled=1
AutoSaveInterval=5
AutoSaveOnExit=1
AutoSaveOnDiskChange=1

# UI設定
ShowScreenshots=1
ShowPlaytime=1
DefaultSlotName=Auto Save
CompressData=1

# ホットキー設定
QuickSaveKey=F5
QuickLoadKey=F9
SaveAsKey=Shift+F5
LoadSlotKey=Shift+F9
```

---

## 8. 互換性・移行

### 8.1 既存ファイル対応
- **自動検出**: ファイル形式の自動判別
- **変換機能**: 旧形式→新形式の一括変換
- **フォールバック**: 新機能使用不可時の従来動作

### 8.2 移行ツール
```c
int statsave_convert_old_format(const OEMCHAR *old_file, UINT8 slot_id);
int statsave_batch_convert(const OEMCHAR *directory);
int statsave_export_to_old_format(UINT8 slot_id, const OEMCHAR *output_file);
```

---

## 9. テスト計画

### 9.1 機能テスト
- **基本動作**: 保存/復元の正確性
- **スロット管理**: マルチスロット動作
- **UI操作**: ダイアログ、ホットキー
- **オートセーブ**: タイマー動作、トリガー

### 9.2 互換性テスト
- **旧ファイル**: 既存セーブファイルの読み込み
- **プラットフォーム**: Windows/SDL2/X11での動作
- **設定**: 各種設定の組み合わせ

### 9.3 性能テスト
- **保存速度**: 大容量メモリでの性能
- **圧縮効果**: ファイルサイズ削減効果
- **メモリ使用**: スクリーンショット等の影響

---

## 10. リスクと対策

### 10.1 互換性リスク
- **リスク**: 既存ユーザーの利用継続性
- **対策**: 完全な後方互換性維持、移行ツール提供

### 10.2 パフォーマンスリスク
- **リスク**: 保存/復元速度の低下
- **対策**: 圧縮・最適化、プログレス表示

### 10.3 実装リスク
- **リスク**: 大規模変更による不安定化
- **対策**: 段階的実装、既存機能の保護

---

この設計に基づき、段階的に機能を実装していきます。
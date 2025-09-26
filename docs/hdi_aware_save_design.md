# HDI対応セーブ機能 基本設計書

## 概要

NP21WSY PC-98エミュレータにHDI（Hard Disk Image）設定を含むセーブステート機能を追加する。
この機能により、ゲーム状態と合わせてディスク構成も保存・復元できるようになる。

## 目的

- ゲーム状態の保存時にHDI/FDD設定も同時に保存
- ロード時に適切なディスク構成を自動復元
- セーブファイルの互換性を維持
- 既存機能への影響を最小限に抑制

## 主要機能

### 1. HDI対応セーブ
- 通常のステートセーブに加えて、現在のHDI/FDD設定を保存
- 保存内容：
  - HDD ファイルパス (4スロット)
  - FDD ファイルパス (4スロット)  
  - IDE デバイスタイプ設定
  - DIPスイッチ設定
  - メモリ設定

### 2. HDI対応ロード
- セーブファイルからHDI設定を検出
- ユーザーに選択肢を提示：
  - HDI設定込みでロード（推奨）
  - ステートのみロード（互換性問題の可能性あり）
- 自動的にディスク構成を復元

### 3. ファイル形式
- 既存の.s00ファイル形式を拡張
- HDI メタデータをファイル末尾に追加
- 下位互換性を維持

## データ構造設計

### HDI メタデータ構造体
```c
typedef struct {
    UINT32      signature;          // 'HDIM' - HDI Metadata識別子
    UINT32      version;            // メタデータバージョン
    UINT64      save_time;          // 保存時刻
    UINT32      session_time;       // セッション時間
    char        game_title[64];     // ゲームタイトル（自動検出）
    char        user_comment[128];  // ユーザーコメント
    UINT32      cpu_clock;          // CPU クロック
    UINT32      mem_size;           // メモリサイズ
    
    // ディスク設定
    char        hdd_files[4][MAX_PATH];     // HDD ファイルパス
    char        fdd_files[4][MAX_PATH];     // FDD ファイルパス
    char        cd_files[4][MAX_PATH];      // CD ファイルパス
    UINT8       ide_types[4];               // IDE デバイスタイプ
    UINT8       fdd_types[4];               // FDD タイプ
    
    // システム設定
    UINT32      dip_switches;       // DIP スイッチ設定
    UINT8       memory_switches[8]; // メモリスイッチ設定
    UINT32      extended_memory;    // 拡張メモリサイズ
    
    UINT32      save_flags;         // 保存対象フラグ
    UINT32      checksum;           // チェックサム
} NP2METADATA_HDI;
```

### 保存対象フラグ
```c
#define HDI_SAVE_HDD            0x00000001  // HDD設定を保存
#define HDI_SAVE_FDD            0x00000002  // FDD設定を保存  
#define HDI_SAVE_CD             0x00000004  // CD設定を保存
#define HDI_SAVE_MEMORY         0x00000008  // メモリ設定を保存
#define HDI_SAVE_DIP            0x00000010  // DIPスイッチを保存
#define HDI_SAVE_ALL            0x0000001F  // すべて保存
```

## ファイル形式

### HDI対応セーブファイル構造
```
[通常のステートセーブデータ]
[HDI メタデータ (NP2METADATA_HDI構造体)]
```

### 検出方法
1. ファイル末尾から`sizeof(NP2METADATA_HDI)`分を読み取り
2. `signature`フィールドが'HDIM'であることを確認
3. HDI対応セーブと判定

## API設計

### 新規追加関数

#### セーブ関数
```c
// HDI対応セーブ（基本）
int statsave_save_hdi_ext(int slot, const char *comment, UINT32 save_flags);

// HDI対応セーブ（UI統合版）
#ifdef _WIN32
int statsave_save_hdi_ext_with_hwnd(int slot, const char *comment, UINT32 save_flags, HWND hMainWnd);
#endif
```

#### ロード関数
```c
// HDI対応ロード
int statsave_load_hdi_ext(int slot);

// HDI設定チェック
int statsave_check_hdi_config(int slot, NP2METADATA_HDI *metadata);
```

### 戻り値
- `STATFLAG_SUCCESS` (0): 成功
- `STATFLAG_FAILURE` (-1): 失敗
- `STATFLAG_WARNING` (0x80): 警告付き成功

## UI設計

### セーブ時の動作
1. セーブボタン押下
2. セーブタイプ選択ダイアログ表示：
   - "HDI対応セーブ（ディスク設定含む）" - 推奨
   - "通常セーブ（ステートのみ）"
   - "キャンセル"
3. 選択に応じてセーブ実行

### ロード時の動作
1. ロードボタン押下
2. ファイルのHDI対応チェック
3. HDI対応の場合、選択ダイアログ表示：
   - "HDI設定込みでロード（推奨）"
   - "ステートのみロード（互換性注意）"
   - "キャンセル"
4. 選択に応じてロード実行

### スロット表示
- HDI対応セーブには "[HDI]" マーカーを表示
- 通常セーブには "[Used]" を表示
- 空きスロットには "[Empty]" を表示

## 実装フェーズ

### フェーズ1: 基本構造
- [ ] データ構造定義（statsave.h）
- [ ] 基本的なHDI設定取得関数
- [ ] ファイル形式の実装

### フェーズ2: セーブ機能
- [ ] HDI対応セーブ関数実装
- [ ] メタデータ生成機能
- [ ] ファイル末尾への追記機能

### フェーズ3: ロード機能  
- [ ] HDI検出機能
- [ ] HDI設定復元機能
- [ ] 設定適用機能

### フェーズ4: UI統合
- [ ] セーブタイプ選択ダイアログ
- [ ] ロードタイプ選択ダイアログ
- [ ] スロット表示の拡張

### フェーズ5: テスト・最適化
- [ ] 各種ゲームでのテスト
- [ ] パフォーマンス最適化
- [ ] エラーハンドリング強化

## 互換性

### 下位互換性
- 既存の.s00ファイルは引き続き読み込み可能
- HDI非対応の古いセーブファイルも正常動作

### 上位互換性
- HDI対応セーブは古いバージョンでも基本的なロードは可能
- HDI設定は復元されないが、ステート自体は利用可能

## 注意事項

### 制限事項
- MAX_PATH長を超えるパスは切り詰められる
- ファイルが移動された場合は手動で再設定が必要
- ネットワークドライブ上のファイルは非推奨

### セキュリティ
- ファイルパスの検証を実施
- 不正なパスや危険な場所へのアクセスを防止
- チェックサムによるデータ整合性確認

## 将来拡張

### 考慮事項
- 圧縮機能の追加
- 暗号化対応
- クラウド同期対応
- 自動バックアップ機能

---

作成日: 2025年1月
バージョン: 1.0
作成者: Claude Code Assistant
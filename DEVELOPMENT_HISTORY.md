# NP21WSY 開発履歴

## 概要

NP21WSY（PC-9801/9821エミュレータ）の拡張開発における実装内容と問題解決の記録です。

## 実装した機能

### 1. 200スロット拡張ステートセーブ機能

#### 背景
- 従来の10スロット制限を200スロットに拡張
- T98-NEXT風のGUI管理画面を提供
- サムネイル表示機能を実装

#### 実装内容
- **ファイル**: `statsave.c`, `statsave.h`
- **機能**:
  - 200スロット対応のセーブ/ロード
  - 自動サムネイル生成・保存
  - SaveStatesフォルダによる整理
  - エラーハンドリングの改善

#### 技術詳細
```c
// 拡張されたAPI
int statsave_save_ext_with_hwnd(int slot, const char *comment, HWND hMainWnd);
int statsave_load_ext(int slot);
HBITMAP statsave_load_thumbnail(int slot);
int statsave_check_slot_exists(int slot);
```

### 2. State Manager GUI

#### 背景
- T98-NEXT風のユーザーフレンドリーなGUI
- 起動時自動表示機能
- メインウィンドウとの位置調整

#### 実装内容
- **ファイル**: `win9x/subwnd/statemgr.cpp`, `statemgr.h`
- **機能**:
  - ListView形式のスロット一覧表示
  - サムネイル表示
  - セーブ/ロード操作
  - 重複回避の配置ロジック

#### 技術詳細
```cpp
// メインクラス
class CStateManagerWnd : public CWndBase
{
    static CStateManagerWnd* CreateInstance();
    BOOL Create();
    void Show(BOOL bShow);
    void Refresh();
};
```

### 3. ファイル構造の整理

#### SaveStatesフォルダ構造
```
bin/x64/Release/SaveStates/
├── state_000.dat          # ステートファイル
├── state_000.dat.png      # サムネイル画像
├── state_001.dat
├── state_001.dat.png
└── ... (最大200スロット)
```

## 解決した技術的問題

### 1. コンパイルエラーの修正

#### 問題: 関数重複定義
- **エラー**: `'build_savestates_dir_path': 再定義されています`
- **原因**: ヘッダファイルの重複インクルード
- **解決**: 重複削除とマクロ→直接実装への変更

#### 問題: 外部シンボル未解決
- **エラー**: `BUILD_SAVESTATE_DIR_PATH は未解決です`
- **原因**: マクロ定義の不適切な使用
- **解決**: 直接コード実装に変更

#### 問題: Windows API互換性
- **エラー**: `GetWindowRect': 関数に 2 個の引数を指定できません`
- **原因**: CWndBaseクラスとWin32 APIの名前衝突
- **解決**: `::GetWindowRect()`で明示的にWin32 API呼び出し

### 2. 表示・UI問題の修正

#### 問題: ボタンテキストが見えない
- **症状**: 拡張ウィンドウのボタンが黒背景で読めない
- **解決**: ボタンスタイルとカラー設定の調整

#### 問題: サムネイル表示の改善
- **症状**: グレー一色のサムネイル
- **解決**: ウィンドウキャプチャロジックの改良

### 3. ウィンドウサイズ管理問題の対処

#### 問題: 表示不具合を引き起こすウィンドウサイズ保存機能
- **症状**: 画面表示が拡大されず表示が破綻
- **解決アプローチ**:
  1. 構造体拡張でのサイズ保存機能実装
  2. INI設定による独自管理
  3. **最終決定**: 機能を削除して安定性優先

#### 実装履歴
```c
// 試行: NP2OSCFG構造体拡張
typedef struct {
    int winx, winy;
    int winwidth, winheight;  // 追加→削除
    // ...
} NP2OSCFG;

// 最終: 元の安定した状態に復元
typedef struct {
    int winx, winy;
    UINT paddingx, paddingy;
    // winwidth, winheightは削除
} NP2OSCFG;
```

## 初期化子リスト修正

### 問題
構造体フィールド削除に伴うコンパイルエラー:
```
'初期化中': 'initializer list' から 'UINT8' に変換できません。
```

### 解決
```c
// 修正前（フィールド不一致）
NP2OSCFG np2oscfg = {
    CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, 1, 1, 0, 0, 0, 1, 0, 1,
    // winwidth, winheightが削除されたため値が2つ多い
};

// 修正後（正しいフィールド対応）
NP2OSCFG np2oscfg = {
    CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, 640, 400, 1, 1, 0, 0,
    // winx, winy, paddingx, paddingy, fscrn_cx, fscrn_cy, flags...
};
```

## 通知ポップアップの最適化

### 改善内容
- セーブ成功時の不要な通知を削除
- ロード時の成功通知を削除
- 警告・エラー時のみポップアップ表示

```cpp
// 修正前: 全操作で通知表示
if (result == STATFLAG_SUCCESS) {
    MessageBox(m_hWnd, _T("State saved successfully"), ...);
}

// 修正後: 成功時は無音、問題時のみ通知
if (result == STATFLAG_SUCCESS || (result & STATFLAG_WARNING)) {
    // 成功時は何も表示しない
} else {
    // エラー時のみ表示
    MessageBox(...);
}
```

## 開発方針と判断

### ウィンドウサイズ保存機能の削除判断

#### 検討した解決策
1. **A案**: 構造体拡張による実装
2. **B案**: 独立したINI管理
3. **C案**: 機能削除（採用）

#### 削除理由
- 表示システムとの相性問題
- scrnmng_setsize()呼び出しによる画面破綻
- 安定性を優先し、コア機能に集中

### State Manager優先開発
- ウィンドウサイズ保存より使用頻度の高い機能
- ユーザビリティ向上への直接的貢献
- 技術リスクの低い実装

## 品質管理

### テスト項目
- [x] State Manager起動時自動表示
- [x] 200スロット全範囲での保存/読み込み
- [x] サムネイル生成・表示
- [x] ファイル整理（SaveStatesフォルダ）
- [x] エラーハンドリング
- [x] メインウィンドウとの重複回避
- [x] コンパイル成功（x64 Release）

### 既知の制限事項
- BIOSファイルは別途入手が必要
- Windows専用実装
- Direct3D依存

## 今後の改善案

### 短期的改善
1. サムネイル品質の向上
2. キーボードショートカット対応
3. セーブコメント機能

### 長期的拡張
1. ステート間の差分表示
2. 自動セーブ機能
3. クラウド同期対応

## まとめ

NP21WSYの拡張開発において、200スロットステートセーブ機能とState Manager GUIを正常に実装しました。ウィンドウサイズ保存機能については、表示システムとの技術的課題により削除し、安定性を確保しました。

全体として、エミュレータの使用性を大幅に向上させる実用的な機能追加となりました。

---
**作成日**: 2025-09-23
**プロジェクト**: NP21WSY Enhanced State Save Feature
**開発者**: Claude Code & User
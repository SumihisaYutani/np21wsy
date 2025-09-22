# NP21WSY ウィンドウサイズ設定システム分析（最終報告）

## 開発経緯と結論

### 実装試行の履歴

このドキュメントは、NP21WSYにウィンドウサイズ保存・復元機能を実装しようとした試行とその結果をまとめています。

#### 試行1: 構造体拡張アプローチ
```c
// NP2OSCFG構造体への追加を試行
typedef struct {
    int winx, winy;
    int winwidth, winheight;  // 追加フィールド
    // ...
} NP2OSCFG;
```

**実装内容**:
- `np2oscfg.winwidth`, `np2oscfg.winheight` フィールド追加
- INI設定に `WinWidth`, `WinHeight` エントリ追加
- WM_CLOSE時の保存処理実装
- 起動時の復元処理実装

#### 試行2: タイマー遅延復元
```c
// DirectDraw初期化後の遅延復元
case WM_TIMER:
    if (wParam == 8888) {
        SetWindowPos(hWnd, NULL, 0, 0, np2oscfg.winwidth, np2oscfg.winheight, ...);
        // 画面内容の更新試行
        scrnmng_setsize(0, 0, clientWidth, clientHeight);
    }
```

### 遭遇した技術的問題

#### 1. 表示システムとの競合
- **問題**: ウィンドウサイズ変更後に画面内容が正しくスケールされない
- **症状**: 画面表示が破綻、コンテンツが縮小されたまま
- **原因**: DirectDrawベースの描画システムとの相性問題

#### 2. 初期化タイミングの課題
- **問題**: DirectDraw初期化前後でのサイズ設定競合
- **試行**: タイマーベースの遅延復元実装
- **結果**: 根本的解決に至らず

#### 3. scrnmng_setsize()の副作用
- **問題**: 画面管理関数呼び出しによる表示不具合
- **症状**: 「表示がおかしくなって拡大されていません」
- **判断**: これ以上の修正は困難

## 最終的な判断と対応

### 機能削除の決定理由

1. **安定性の優先**: 既存の動作に悪影響を与えるリスク
2. **技術的複雑さ**: DirectDraw描画システムとの深刻な相性問題
3. **開発効率**: State Manager等のより重要な機能への集中

### 実装された削除処理

```c
// 削除されたフィールド
typedef struct {
    int winx, winy;
    // int winwidth, winheight; // 削除
    UINT paddingx, paddingy;
    // ...
} NP2OSCFG;

// 修正された初期化子リスト
NP2OSCFG np2oscfg = {
    CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, 640, 400, 1, 1, 0, 0,
    // winwidth, winheightの値を削除し、正しいフィールド数に調整
};
```

## 現在の仕様（確定版）

### 1. ウィンドウ管理の範囲

#### 保存される設定
- **ウィンドウ位置**: `np2oscfg.winx`, `np2oscfg.winy`
- **その他設定**: 各種エミュレーション設定

#### 保存されない設定
- **ウィンドウサイズ**: 意図的に削除、毎回デフォルトサイズで起動

### 2. wincentering()関数の動作

```c
static void wincentering(HWND hWnd) {
    RECT rc;
    int width, height;

    // 現在のウィンドウサイズを取得（保存値は使用しない）
    GetWindowRect(hWnd, &rc);
    width = rc.right - rc.left;
    height = rc.bottom - rc.top;

    // 画面中央に配置
    np2oscfg.winx = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
    np2oscfg.winy = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;
    // ...
}
```

## 学習された教訓

### 1. エミュレータの描画システムの複雑さ
- DirectDrawベースの描画システムは独自の制約がある
- ウィンドウサイズ変更は単純なWin32 APIだけでは解決できない
- 画面管理モジュール(`scrnmng_*`)との協調が必要

### 2. 後方互換性の重要性
- 既存の動作を維持することの価値
- 新機能追加時の影響範囲の慎重な評価
- 段階的実装とロールバック戦略の必要性

### 3. 優先順位の明確化
- ユーザビリティ向上（State Manager）vs システム機能（ウィンドウサイズ）
- 実装コストと効果のバランス
- 技術的リスクの評価

## 代替案と今後の方向性

### 短期的対応
- **現状維持**: ウィンドウサイズ保存機能なしで運用
- **手動調整**: ユーザーが必要に応じて手動でサイズ調整

### 長期的検討
- **描画システム更新**: DirectDrawからDirect3D/OpenGLへの移行時に再検討
- **設定外部化**: 外部ツールによるウィンドウ管理
- **プロファイル機能**: 複数の設定プロファイル対応時に含める

## まとめ

ウィンドウサイズ保存機能の実装を試行しましたが、技術的制約により最終的に削除する判断を行いました。この経験により、以下の価値ある成果を得ました：

1. **安定した State Manager機能**: より重要な機能に集中できた
2. **システム理解の向上**: NP21WSYの内部構造への理解が深まった
3. **技術的知見**: エミュレータ開発における制約と課題の理解

この判断により、NP21WSYは安定性を保ちながら、200スロット拡張ステートセーブ機能という実用的な改善を実現できました。

---
**最終更新**: 2025-09-23
**ステータス**: 削除完了・安定化確認済み
**プロジェクト**: NP21WSY Enhanced State Save Feature
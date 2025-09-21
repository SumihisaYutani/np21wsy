# NP21WSY ビルドガイド

## 概要

NP21WSY（PC-9801/9821エミュレータ）のビルド環境とコンパイル手順についてのガイドです。

## プロジェクト構成

### メインソリューション
- **ソリューションファイル**: `win9x/np2vs2019.sln`
- **プロジェクト**:
  - `np2w` - PC-9801エミュレータ (`np2vs2019.vcxproj`)
  - `np21w` - PC-9821エミュレータ (`np21vs2019.vcxproj`)

### ビルド環境要件

#### 必須ツール
- **Visual Studio 2022** (Platform Toolset: v143)
- **MSBuild 17.14.23** 以上
- **NASM** (Netwide Assembler) - x86アセンブリ用
- **YASM** (Yet Another Assembler) - x64アセンブリ用

#### サポートプラットフォーム
- Win32 (x86)
- x64 (x86-64)

#### ビルド構成
- **Debug** - デバッグビルド
- **Release** - リリースビルド
- **ReleaseHAXM** - HAXM対応リリースビルド

## アセンブラ設定

### NASM (x86プラットフォーム)
- **設定ファイル**: `x86/vs2010/x86.props`
- **用途**: x86アセンブリコードのコンパイル
- **インストール**: NASMをシステムPATHに追加

### YASM (x64プラットフォーム)
- **設定ファイル**: `x64/vs2010/x64.props`, `x64/vs2010/x64.targets`
- **用途**: x64アセンブリコードのコンパイル
- **カスタムビルドルール**: `x64.xml`でプロパティ定義
- **インストール場所**: `C:\yasm\yasm.exe`

## ソースファイル構成

### コアモジュール
```
../
├── bios/           # BIOS関連
├── calendar.c      # カレンダー機能
├── cbus/           # C-Bus関連
├── codecnv/        # コード変換
├── common/         # 共通ライブラリ
├── debugsub.c      # デバッグ機能
├── diskimage/      # ディスクイメージ処理
├── fdd/            # フロッピーディスク
├── font/           # フォント処理
├── generic/        # 汎用機能
├── i286c/          # CPU エミュレーション
├── io/             # I/O処理
├── keystat.c       # キーボード状態
├── lio/            # LIO機能
├── mem/            # メモリ管理
├── nevent.c        # イベント処理
├── pccore.c        # PCコア機能
├── sound/          # サウンド処理
├── statsave.c      # ステートセーブ機能（拡張済み）
├── vram/           # VRAM処理
└── zlib/           # 圧縮ライブラリ
```

### Windows固有モジュール
```
win9x/
├── compiler.h      # コンパイラ設定
├── np2.cpp         # メインアプリケーション
├── subwnd/         # サブウィンドウ
│   ├── statemgr.cpp # State Manager Window（新規）
│   └── statemgr.h   # State Manager Header（新規）
└── resources/      # リソースファイル
```

## 拡張機能

### 200スロットステートセーブ機能
- **ファイル**: `statsave.c`, `statsave.h`
- **機能**: 従来の10スロットから200スロットに拡張
- **管理画面**: T98-NEXT風のState Manager Window
- **サムネイル**: セーブ時点の画面キャプチャ表示

## ビルド手順

### 1. 環境準備
```bash
# YASMをPATHに追加
export PATH="/c/yasm:$PATH"
```

### 2. ビルド実行
```bash
# プロジェクトディレクトリに移動
cd /j/project/np21wsy/win9x

# x64 Releaseビルド
"C:/Program Files/Microsoft Visual Studio/2022/Community/MSBuild/Current/Bin/MSBuild.exe" np2vs2019.vcxproj -p:Configuration=Release -p:Platform=x64

# x86 Releaseビルド
"C:/Program Files/Microsoft Visual Studio/2022/Community/MSBuild/Current/Bin/MSBuild.exe" np2vs2019.vcxproj -p:Configuration=Release -p:Platform=Win32
```

### 3. 出力ディレクトリ
```
np21wsy/
├── bin/
│   ├── x64/Release/          # x64リリースビルド出力
│   │   └── np2w.exe
│   └── np2vs2019/            # その他のビルド出力
└── obj/
    └── x64/Release/np2w/     # 中間ファイル（.obj等）
```

## トラブルシューティング

### よくある問題

#### 1. アセンブラが見つからない
```
error: 'nasm' は認識されていません
error: 'yasm' は認識されていません
```
**解決方法**:
- NASMとYASMをインストールし、PATHに追加
- YASMは `C:\yasm\` に配置

#### 2. プラットフォームツールセットエラー
```
error MSB8020: v142 のビルド ツールが見つかりません
```
**解決方法**:
- `.vcxproj`ファイルの`<PlatformToolset>`を`v143`に変更

#### 3. WindowsSDKバージョンエラー
**解決方法**:
- `<WindowsTargetPlatformVersion>`を適切なバージョンに設定

### デバッグ情報

#### ビルド成功の確認
```bash
# 実行ファイルの確認
ls -la bin/x64/Release/np2w.exe

# 依存関係の確認
dumpbin /dependents bin/x64/Release/np2w.exe
```

## 開発メモ

### 現在の状況（2025-09-21）
- ✅ NASM/YASMアセンブラインストール完了
- ✅ `statsave.c`コンパイルエラー修正完了
- ✅ 200スロット拡張機能実装完了
- ⚠️ `statemgr.cpp`のWindowsAPI関数呼び出しエラー修正中

### 次のステップ
1. State Manager WindowのWindowsAPI互換性修正
2. 基底クラス(`CWndBase`)メソッド引数調整
3. 未定義識別子の解決
4. ビルド完了とテスト

## 参考情報

### 関連ファイル
- `compiler.h` - コンパイラ固有の設定とマクロ定義
- `np2.h` - アプリケーション共通ヘッダ
- `resource.h` - リソースID定義

### ビルドログ
ビルド時の詳細ログは `bin/` ディレクトリ内に出力されます。

---
**最終更新**: 2025-09-21
**プロジェクト**: NP21WSY Enhanced State Save Feature
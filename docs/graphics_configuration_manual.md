# NP21WSY グラフィック設定マニュアル

## 1. 概要

本マニュアルでは、NP21WSY PC-9801エミュレータのグラフィック設定について詳しく説明します。
PC-9801シリーズの多彩な表示機能を正確に再現するため、適切な設定が重要です。

---

## 2. 基本表示設定

### 2.1 表示同期設定

#### 設定項目: `DispSync`
画面の表示同期を制御します。

| 設定値 | 動作 | 特徴 |
|--------|------|------|
| `0` | 同期なし | 高速描画、画面が乱れる場合あり |
| `1` | 垂直同期 | 標準設定、なめらかな表示 |
| `2` | 厳密同期 | 最高品質、高負荷 |

**推奨設定:**
```ini
DispSync=1
```

### 2.2 フレームスキップ設定

#### 設定項目: `skipline`, `skiplight`

**ライン飛ばし: `skipline`**
```ini
skipline=0
```
- **範囲**: 0-100
- **効果**: 描画負荷軽減
- **0**: 全ライン描画（推奨）
- **1以上**: 指定したライン数をスキップ

**ライトスキップ: `skiplight`**
```ini
skiplight=0
```
- **範囲**: 0-10000
- **効果**: 軽量化処理
- **0**: 標準処理（推奨）

---

## 3. グラフィック機能設定

### 3.1 GRCG/EGC設定

#### 設定項目: `GRCG_EGC`
PC-9801のグラフィック機能を選択します。

| 設定値 | 機能 | 特徴 |
|--------|------|------|
| `0` | GRCG | 基本グラフィック機能 |
| `1` | EGC | 拡張グラフィック機能 |
| `2` | EGC (互換) | 推奨設定 |

**推奨設定:**
```ini
GRCG_EGC=2
```

**機能説明:**
- **GRCG**: Graphic Charger、基本的なグラフィック描画
- **EGC**: Enhanced Graphic Controller、高速描画・ROP機能

### 3.2 16色モード設定

#### 設定項目: `color16b`
16色表示モードの有効/無効を設定します。

```ini
color16b=1
```
- **0**: 8色モード
- **1**: 16色モード（推奨）

### 3.3 カラーパレット設定

#### 背景色: `BG_COLOR`
```ini
BG_COLOR=000000
```
- **形式**: RRGGBB (16進数)
- **デフォルト**: 000000 (黒)

#### 前景色: `FG_COLOR`
```ini
FG_COLOR=ffffff
```
- **形式**: RRGGBB (16進数)
- **デフォルト**: ffffff (白)

**色指定例:**
- 黒: `000000`
- 白: `ffffff`
- 赤: `ff0000`
- 緑: `00ff00`
- 青: `0000ff`
- 黄: `ffff00`
- 紫: `ff00ff`
- 水色: `00ffff`

---

## 4. 表示モード設定

### 4.1 LCD表示設定

#### 設定項目: `LCD_MODE`
LCD風表示の設定です。

| 設定値 | 表示 | 特徴 |
|--------|------|------|
| `0` | 通常表示 | CRT風表示 |
| `1` | LCD表示 | 液晶風表示 |

**推奨設定:**
```ini
LCD_MODE=0
```

### 4.2 実色表示設定

#### 設定項目: `realpal`
カラーパレットの表示方法を設定します。

```ini
realpal=0
```
- **0**: エミュレートパレット
- **1**: 実機相当パレット（推奨）

---

## 5. 高度なグラフィック設定

### 5.1 PEGC設定 (PC-9821シリーズ)

PC-9821の拡張グラフィック機能です。

#### PEGC有効化
```ini
usepegcplane=1
```

#### PEGC設定詳細
```ini
# PEGC関連設定 (上級者向け)
pegc_enable=1
pegc_640=1
pegc_480=1
```

### 5.2 Cirrus Logic GD5430設定

高解像度グラフィック対応設定です。

```ini
usegd5430=1
gd5430type=1
gd5430fakecur=0
gd5430melofs=0
ga98nb_bigscrn_ex=0
```

**パラメータ詳細:**
- **usegd5430**: GD5430使用有無
- **gd5430type**: チップタイプ
- **gd5430fakecur**: フェイクカーソル
- **gd5430melofs**: メモリオフセット
- **ga98nb_bigscrn_ex**: 大画面拡張

### 5.3 VGAモード設定

#### 設定項目: `usemodex`
DOS/V風VGAモードの使用設定です。

```ini
usemodex=0
```
- **0**: PC-9801ネイティブ（推奨）
- **1**: VGA互換モード

---

## 6. プラットフォーム別表示設定

### 6.1 Windows版表示設定

#### ウィンドウ設定
```ini
windtype=0      # ウィンドウタイプ
thickfrm=0      # 厚いフレーム
toolwind=0      # ツールウィンドウ
```

#### DirectDraw設定
```ini
DRAWTYPE=0      # 描画方式
EMUDDRAW=0      # DirectDrawエミュレーション
```

**描画方式 (DRAWTYPE):**
- **0**: GDI (互換性重視)
- **1**: DirectDraw (標準)
- **2**: Direct3D (高性能)

#### フルスクリーン設定
```ini
fullscreen=0    # フルスクリーン
fullscrn_out=0  # フルスクリーン出力方式
```

### 6.2 SDL2版表示設定

#### ウィンドウ設定
```ini
winposx=100     # ウィンドウX座標
winposy=100     # ウィンドウY座標
multiple=1      # 表示倍率
```

#### レンダリング設定
```ini
render=1        # レンダリング方式
hwaccel=1       # ハードウェアアクセラレーション
```

### 6.3 X11版表示設定

#### X11固有設定
```ini
x11_display=    # X11ディスプレイ
x11_screen=0    # スクリーン番号
```

---

## 7. 表示解像度・倍率設定

### 7.1 表示倍率

#### Windows版
```ini
multiple=1      # 表示倍率 (1-4)
```

#### SDL2版
```ini
multiple=2      # 表示倍率 (1-8)
stretch=1       # 拡大補間
```

### 7.2 アスペクト比

#### 設定項目: `aspect`
```ini
aspect=0        # アスペクト比維持
```
- **0**: オリジナル比率
- **1**: 画面いっぱい

---

## 8. フォント設定

### 8.1 フォントファイル設定

#### 設定項目: `fontfile`
外部フォントファイルを指定します。

```ini
fontfile=
```

**対応フォント形式:**
- PC-98フォントファイル (.fnt)
- ビットマップフォント
- システムフォント

### 8.2 フォント表示設定

#### 文字表示品質
```ini
font_quality=0   # フォント品質 (0-2)
font_smooth=0    # フォントスムージング
```

---

## 9. 用途別推奨設定

### 9.1 ゲーム用設定

**アクションゲーム（高速描画重視）**
```ini
DispSync=1
skipline=0
skiplight=0
GRCG_EGC=2
color16b=1
realpal=1
LCD_MODE=0
```

**RPG・アドベンチャー（画質重視）**
```ini
DispSync=2
skipline=0
skiplight=0
GRCG_EGC=2
color16b=1
realpal=1
LCD_MODE=0
```

### 9.2 アプリケーション用設定

**テキストエディタ・一般用途**
```ini
DispSync=1
GRCG_EGC=1
color16b=0
BG_COLOR=000080
FG_COLOR=ffffff
LCD_MODE=0
```

**グラフィック制作**
```ini
DispSync=2
GRCG_EGC=2
color16b=1
realpal=1
usepegcplane=1
```

### 9.3 エミュレータ開発・デバッグ用

**正確性重視**
```ini
DispSync=2
skipline=0
skiplight=0
GRCG_EGC=2
realpal=1
```

---

## 10. トラブルシューティング

### 10.1 画面が表示されない

**原因と対策:**

1. **表示同期設定**
   ```ini
   DispSync=0  # 同期無効で確認
   ```

2. **描画方式変更**
   ```ini
   DRAWTYPE=0  # GDI描画に変更
   ```

3. **グラフィック機能無効**
   ```ini
   GRCG_EGC=0  # 基本機能のみ
   ```

### 10.2 画面が乱れる

**原因と対策:**

1. **フレームスキップ無効**
   ```ini
   skipline=0
   skiplight=0
   ```

2. **表示同期強化**
   ```ini
   DispSync=2
   ```

3. **ハードウェアアクセラレーション無効**
   ```ini
   hwaccel=0
   ```

### 10.3 色が正しくない

**原因と対策:**

1. **パレット設定確認**
   ```ini
   realpal=1
   ```

2. **色深度設定**
   ```ini
   color16b=1
   ```

3. **カラー設定リセット**
   ```ini
   BG_COLOR=000000
   FG_COLOR=ffffff
   ```

### 10.4 文字が読めない

**原因と対策:**

1. **フォント設定確認**
   ```ini
   fontfile=      # 空に設定
   ```

2. **LCD設定変更**
   ```ini
   LCD_MODE=0     # 通常表示
   ```

3. **コントラスト調整**
   ```ini
   BG_COLOR=000080  # 濃い青背景
   FG_COLOR=ffffff  # 白文字
   ```

### 10.5 低性能PC向け設定

**軽量化設定:**
```ini
DispSync=0
skipline=2
skiplight=100
GRCG_EGC=0
DRAWTYPE=0
hwaccel=0
multiple=1
```

---

## 11. 設定テンプレート集

### 11.1 標準設定
```ini
[NekoProjectII]
DispSync=1
skipline=0
skiplight=0
GRCG_EGC=2
color16b=1
realpal=1
BG_COLOR=000000
FG_COLOR=ffffff
LCD_MODE=0
fontfile=
```

### 11.2 高品質設定
```ini
[NekoProjectII]
DispSync=2
skipline=0
skiplight=0
GRCG_EGC=2
color16b=1
realpal=1
BG_COLOR=000000
FG_COLOR=ffffff
LCD_MODE=0
usepegcplane=1
fontfile=
```

### 11.3 軽量設定
```ini
[NekoProjectII]
DispSync=0
skipline=1
skiplight=50
GRCG_EGC=0
color16b=0
realpal=0
BG_COLOR=000000
FG_COLOR=ffffff
LCD_MODE=0
fontfile=
```

### 11.4 レトロ風設定
```ini
[NekoProjectII]
DispSync=1
GRCG_EGC=1
color16b=0
realpal=0
BG_COLOR=000080
FG_COLOR=00ff00
LCD_MODE=1
```

この設定マニュアルを参考に、お使いの環境と用途に最適なグラフィック設定を行ってください。
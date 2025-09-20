# NP21WSY CPU・メモリ設定マニュアル

## 1. 概要

本マニュアルでは、NP21WSY PC-9801エミュレータのCPU・メモリ設定について詳しく説明します。
適切な設定により、ソフトウェアの動作速度と互換性を最適化できます。

---

## 2. CPU基本設定

### 2.1 CPUモデル設定

#### PC-98シリーズ対応CPU

| CPU | 対応機種 | 特徴 |
|-----|----------|------|
| 8086 | PC-9801(無印)〜VM | 初期CPU、低速 |
| V30 | PC-9801VX/UV | NEC製8086互換、高速 |
| 80286 | PC-9801FA/FS/FX | 16bit保護モード対応 |
| 80386SX | PC-9801DX/DS | 32bit CPU、16bitバス |
| 80386DX | PC-9801DA/BA | 32bit CPU、32bitバス |

### 2.2 クロック設定

#### 基本クロック: `clk_base`
PC-9801シリーズの基本動作周波数です。

```ini
clk_base=2457600
```

**標準値:**
- `2457600`: PC-9801(無印)〜VX系 (2.4576MHz)
- `1996800`: 一部特殊機種 (1.9968MHz)

#### クロック倍率: `clk_mult`
基本クロックに対する倍率設定です。

| 設定値 | 実効速度 | 用途 | 互換性 |
|--------|----------|------|---------|
| `1` | 2.5MHz | 古いソフト | 最高 |
| `2` | 5MHz | 標準動作 | 高 |
| `4` | 10MHz | 推奨設定 | 良 |
| `8` | 20MHz | 高速動作 | 中 |
| `16` | 40MHz | 最高速度 | 低 |
| `32` | 80MHz | 超高速 | 最低 |

**推奨設定:**
```ini
clk_mult=4
```

### 2.3 CPUコア選択

NP21WSYは複数のCPUエミュレーションコアを提供します。

#### i286c (デフォルト)
- **特徴**: 80286互換、高い互換性
- **用途**: 一般的な用途
- **性能**: 中程度

#### i286x (拡張版)
- **特徴**: 最適化された80286エミュレーション
- **用途**: 高速動作が必要な場合
- **性能**: 高速

#### i386c (32bit)
- **特徴**: 80386互換、完全な32bit対応
- **用途**: Windows等の32bitソフト
- **性能**: 高性能、高負荷

#### i386hax (HAXM)
- **特徴**: Intel HAXM使用、ハードウェア支援
- **用途**: 最高性能が必要な場合
- **性能**: 最高速度
- **要件**: Intel CPU + HAXM

---

## 3. メモリ設定

### 3.1 基本メモリ構成

#### PC-9801のメモリマップ
```
000000h - 09FFFFh : 基本メモリ (640KB)
0A0000h - 0A3FFFh : テキストVRAM (16KB)
0A8000h - 0BFFFFh : グラフィックVRAM (96KB)
0C0000h - 0FFFFFh : システム領域・ROM
100000h -         : 拡張メモリ
```

### 3.2 拡張メモリ設定

#### 設定項目: `ExMemory`
640KB超のメモリサイズを設定します。

| 設定値 | 総メモリ | 用途 | 対応ソフト |
|--------|----------|------|------------|
| `0` | 640KB | 最小構成 | 初期ソフト |
| `1` | 1MB | 基本構成 | 一般ソフト |
| `2` | 2MB | 標準構成 | 多くのソフト |
| `4` | 4MB | 中容量 | Windows 3.1 |
| `8` | 8MB | 大容量 | ゲーム・CAD |
| `13` | 13MB | 推奨設定 | 万能設定 |
| `16` | 16MB | 大容量 | 大型ソフト |
| `32` | 32MB | 最大容量 | 特殊用途 |

**推奨設定:**
```ini
ExMemory=13
```

### 3.3 メモリ管理設定

#### EMS (Expanded Memory)
```ini
EMSSIZE=8192    # EMSサイズ (KB)
EMSENABLE=1     # EMS有効
```

#### XMS (Extended Memory)
```ini
XMSENABLE=1     # XMS有効
```

#### UMB (Upper Memory Block)
```ini
UMBENABLE=1     # UMB有効
UMBSIZE=384     # UMBサイズ (KB)
```

---

## 4. 高度なCPU設定

### 4.1 CPU識別設定

#### CPU ベンダー文字列
```ini
cpu_vendor=GenuineIntel     # CPUベンダー名
```

**設定可能値:**
- `GenuineIntel`: Intel CPU
- `AuthenticAMD`: AMD CPU
- `NexGenDriven`: NexGen CPU

#### CPU ファミリ・モデル
```ini
cpu_family=5        # CPUファミリ (Pentium=5)
cpu_model=4         # CPUモデル
cpu_stepping=3      # CPUステッピング
```

#### CPU 機能フラグ
```ini
cpu_feature=00000001        # 基本機能フラグ
cpu_feature_ex=00000000     # 拡張機能フラグ
cpu_feature_ecx=00000000    # ECX機能フラグ
cpu_feature_ex_ecx=00000000 # ECX拡張機能フラグ
```

#### CPU ブランド文字列
```ini
cpu_brandstring=Intel(R) Pentium(R) CPU
cpu_brandid=0               # ブランドID
```

### 4.2 FPU (浮動小数点演算器) 設定

#### FPU タイプ: `fpu_type`
```ini
fpu_type=1
```

**設定値:**
- **0**: Berkeley SoftFloat (正確、低速)
- **1**: DOSBox FPU (推奨、高速)
- **2**: DOSBox FPU+INT64 (最高速)

### 4.3 CPU 動作制御

#### タイマー固定化
```ini
timerfix=1          # タイマー修正有効
```

#### TSC (Time Stamp Counter)
```ini
consttsc=1          # 一定間隔TSC
```

#### 非同期CPU (対応版のみ)
```ini
asynccpu=0          # 非同期CPU無効
asynctgt=100        # 非同期CPU目標負荷
```

---

## 5. メモリアクセス最適化

### 5.1 メモリ待機時間

#### VRAM待機時間
```ini
vramwait=0          # VRAM待機時間 (0=なし)
```

#### テキストRAM待機時間
```ini
tramwait=0          # テキストRAM待機時間
```

#### GRCG待機時間
```ini
grcgwait=0          # GRCG待機時間
```

### 5.2 メモリチェック高速化

#### 高速メモリチェック
```ini
memcheckspeed=1     # 高速メモリチェック
```

#### メモリチェック最大値
```ini
memchkmx=1          # メモリチェック最大値
```

---

## 6. 用途別最適設定

### 6.1 ゲーム用設定

#### 高互換性設定 (古いゲーム)
```ini
clk_base=2457600
clk_mult=2
ExMemory=2
fpu_type=0
timerfix=1
vramwait=1
tramwait=1
```

#### 標準ゲーム設定
```ini
clk_base=2457600
clk_mult=4
ExMemory=8
fpu_type=1
timerfix=1
vramwait=0
tramwait=0
```

#### 高速ゲーム設定
```ini
clk_base=2457600
clk_mult=8
ExMemory=16
fpu_type=2
timerfix=0
vramwait=0
tramwait=0
```

### 6.2 アプリケーション用設定

#### 一般アプリケーション
```ini
clk_base=2457600
clk_mult=4
ExMemory=13
fpu_type=1
EMSSIZE=8192
EMSENABLE=1
XMSENABLE=1
```

#### Windows 3.1用設定
```ini
clk_base=2457600
clk_mult=8
ExMemory=16
fpu_type=1
EMSSIZE=16384
EMSENABLE=1
XMSENABLE=1
UMBENABLE=1
```

#### CAD・グラフィック用設定
```ini
clk_base=2457600
clk_mult=16
ExMemory=32
fpu_type=2
EMSSIZE=32768
XMSENABLE=1
```

### 6.3 開発・プログラミング用設定

#### コンパイラ・開発環境
```ini
clk_base=2457600
clk_mult=8
ExMemory=16
fpu_type=1
EMSSIZE=16384
EMSENABLE=1
XMSENABLE=1
UMBENABLE=1
UMBSIZE=384
```

---

## 7. パフォーマンスチューニング

### 7.1 速度重視設定

```ini
clk_mult=16         # 最高速度
ExMemory=32         # 大容量メモリ
fpu_type=2          # 高速FPU
timerfix=0          # タイマー修正無効
asynccpu=1          # 非同期CPU
vramwait=0          # 待機時間なし
tramwait=0
grcgwait=0
memcheckspeed=1     # 高速メモリチェック
```

### 7.2 互換性重視設定

```ini
clk_mult=2          # 低速動作
ExMemory=2          # 少ないメモリ
fpu_type=0          # 正確なFPU
timerfix=1          # タイマー修正有効
asynccpu=0          # 同期CPU
vramwait=1          # 待機時間あり
tramwait=1
grcgwait=1
memcheckspeed=0     # 通常メモリチェック
```

### 7.3 バランス設定

```ini
clk_mult=4          # 標準速度
ExMemory=13         # 適度なメモリ
fpu_type=1          # バランス型FPU
timerfix=1          # タイマー修正有効
vramwait=0          # 待機時間なし
tramwait=0
grcgwait=0
memcheckspeed=1     # 高速メモリチェック
```

---

## 8. トラブルシューティング

### 8.1 ソフトが起動しない

**原因と対策:**

1. **メモリ不足**
   ```ini
   ExMemory=16     # メモリ増量
   ```

2. **CPU速度問題**
   ```ini
   clk_mult=2      # 速度を下げる
   ```

3. **FPU問題**
   ```ini
   fpu_type=0      # 正確なFPU
   ```

### 8.2 動作が不安定

**原因と対策:**

1. **タイミング問題**
   ```ini
   timerfix=1      # タイマー修正
   vramwait=1      # 待機時間追加
   tramwait=1
   ```

2. **CPU速度調整**
   ```ini
   clk_mult=4      # 適度な速度
   ```

### 8.3 メモリエラー

**原因と対策:**

1. **メモリサイズ調整**
   ```ini
   ExMemory=8      # メモリ減量
   ```

2. **EMS/XMS無効化**
   ```ini
   EMSENABLE=0
   XMSENABLE=0
   ```

### 8.4 浮動小数点演算エラー

**原因と対策:**

1. **FPU設定変更**
   ```ini
   fpu_type=0      # 最高精度FPU
   ```

2. **CPU機能制限**
   ```ini
   cpu_feature=00000000    # 機能無効化
   ```

---

## 9. 設定テンプレート集

### 9.1 推奨設定
```ini
[NekoProjectII]
clk_base=2457600
clk_mult=4
ExMemory=13
fpu_type=1
timerfix=1
EMSENABLE=1
XMSENABLE=1
EMSSIZE=8192
memcheckspeed=1
```

### 9.2 高性能設定
```ini
[NekoProjectII]
clk_base=2457600
clk_mult=8
ExMemory=32
fpu_type=2
timerfix=0
asynccpu=1
EMSENABLE=1
XMSENABLE=1
UMBENABLE=1
EMSSIZE=16384
UMBSIZE=384
vramwait=0
tramwait=0
grcgwait=0
memcheckspeed=1
```

### 9.3 互換性最優先
```ini
[NekoProjectII]
clk_base=2457600
clk_mult=1
ExMemory=1
fpu_type=0
timerfix=1
asynccpu=0
EMSENABLE=0
XMSENABLE=0
vramwait=1
tramwait=1
grcgwait=1
memcheckspeed=0
```

この設定マニュアルを参考に、使用するソフトウェアに最適なCPU・メモリ設定を行ってください。
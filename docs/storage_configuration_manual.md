# NP21WSY ストレージ設定マニュアル

## 1. 概要

本マニュアルでは、NP21WSY PC-9801エミュレータのストレージ設定について詳しく説明します。
フロッピーディスク、ハードディスク、CD-ROMの設定方法を解説します。

---

## 2. フロッピーディスク設定

### 2.1 FDD装備設定

#### 設定項目: `fddequip`
フロッピーディスクドライブの装備状況を設定します。

| 設定値 | ドライブ構成 | 説明 |
|--------|-------------|------|
| `0` | なし | FDDなし |
| `1` | 1台 | ドライブA:のみ |
| `2` | 1台 (2DD) | 2DDドライブA:のみ |
| `3` | 2台 | ドライブA:, B: |
| `4` | 2台 (2DD) | 2DD ドライブA:, B: |

**推奨設定:**
```ini
fddequip=3
```

### 2.2 1.44MBフロッピー対応

#### 設定項目: `usefd144`
1.44MBフロッピーディスクの使用可否を設定します。

```ini
usefd144=1
```
- **0**: 使用しない（1.2MB/640KBのみ）
- **1**: 使用する（推奨）

### 2.3 フロッピーディスクファイル設定

#### 設定項目: `FDD?FILE`
各ドライブにマウントするディスクイメージファイルを指定します。

```ini
FDD1FILE=C:\disk\game.d88
FDD2FILE=C:\disk\system.d88
FDD3FILE=
FDD4FILE=
```

**対応ファイル形式:**
- **.d88**: T98-NEXT形式（推奨）
- **.fdi**: Anex86形式
- **.nfd**: T98形式
- **.dcp**: DiskExplorer形式
- **.88d**: D88形式
- **.xdf**: IBM XDF形式
- **.hdm**: HDM形式

### 2.4 FDD動作設定

#### モーター音設定
```ini
MOTOR=1         # モーター音有効
MOTORVOL=50     # モーター音量 (0-100)
```

#### アクセス音設定
```ini
SEEK=0          # シーク音有効
SEEKVOL=50      # シーク音量 (0-100)
```

#### ファイル保存設定
```ini
savefddfile=1   # ディスクファイル自動保存
```

---

## 3. ハードディスク設定

### 3.1 HDD接続インターフェース

#### SASI接続
```ini
HDD1FILE=C:\hdd\sasi1.hdd
HDD2FILE=C:\hdd\sasi2.hdd
```

#### SCSI接続 (対応版のみ)
```ini
SCSI1FILE=C:\hdd\scsi1.hdd
SCSI2FILE=C:\hdd\scsi2.hdd
SCSI3FILE=C:\hdd\scsi3.hdd
SCSI4FILE=C:\hdd\scsi4.hdd
```

#### IDE接続 (対応版のみ)
```ini
IDE1FILE=C:\hdd\ide1.hdd
IDE2FILE=C:\hdd\ide2.hdd
IDE3FILE=C:\hdd\ide3.hdd
IDE4FILE=C:\hdd\ide4.hdd
```

### 3.2 HDDファイル形式

**対応形式:**
- **.hdd**: NP2標準形式
- **.thd**: T98-NEXT形式
- **.nhd**: T98形式
- **.hdi**: Anex86形式
- **.vhd**: Virtual PC形式

### 3.3 IDE設定 (対応版のみ)

#### IDE タイプ設定
```ini
idetype1=0      # IDE1のタイプ
idetype2=0      # IDE2のタイプ
idetype3=0      # IDE3のタイプ
idetype4=0      # IDE4のタイプ
```

**タイプ設定値:**
- **0**: 未使用
- **1**: ハードディスク
- **2**: CD-ROM

#### IDE BIOS設定
```ini
idebios=1       # IDE BIOS使用
autoidebios=1   # 自動IDE BIOS
idebaddr=d8     # IDE BIOSアドレス
```

#### IDE待機時間設定
```ini
iderwait=50     # 読み込み待機時間
idewwait=50     # 書き込み待機時間
```

---

## 4. CD-ROM設定

### 4.1 CD-ROMファイル設定

#### ファイル指定
```ini
CDROM1FILE=C:\cd\game.iso
CDROM2FILE=C:\cd\music.cue
```

**対応ファイル形式:**
- **.iso**: ISO9660形式
- **.cue/.bin**: CUE/BIN形式
- **.ccd/.img**: CloneCD形式
- **.mds/.mdf**: Media Descriptor形式
- **.nrg**: Nero形式

### 4.2 CD-ROM動作設定

#### ファイル保存設定
```ini
savecdfile=1    # CD-ROMファイル自動保存
```

#### 非同期読み込み
```ini
useasynccd=1    # 非同期CD読み込み
```

#### トレイコマンド許可
```ini
allowcdtraycmd=1 # トレイ開閉コマンド許可
```

### 4.3 CD-ROM機能設定

#### EDC/ECC エミュレーション
```ini
usecdecc=1      # EDC/ECC サポート
```

#### データトラックスキップ
```ini
cddtskip=1      # データトラックスキップ
```

---

## 5. ホストドライブ設定

### 5.1 ホストドライブ有効化

#### 設定項目: `hdrvenable`
ホストPC側のディスクにアクセスする機能です。

```ini
hdrvenable=1    # ホストドライブ有効
hdrvacc=1       # ホストドライブアクセス
```

#### ルートディレクトリ設定
```ini
hdrvroot=C:\np21w\hostdrive
```

### 5.2 Windows NT系対応

#### 設定項目: `hdrvntenable`
Windows NT/2000/XP系でのホストドライブ使用設定です。

```ini
hdrvntenable=1  # NT系ホストドライブ
```

---

## 6. ストレージ詳細設定

### 6.1 メモリ保護設定

#### 設定項目: `PROTECTMEM`
メモリ保護機能の設定です。

```ini
PROTECTMEM=1    # メモリ保護有効
```

### 6.2 高速メモリチェック

#### 設定項目: `memcheckspeed`
起動時メモリチェックの速度設定です。

```ini
memcheckspeed=1 # 高速メモリチェック
```

### 6.3 拡張メモリ設定

#### D0000h-DFFFFh RAM使用
```ini
useram_d=1      # D0000h-DFFFFh RAM使用
```

---

## 7. ディスクイメージ管理

### 7.1 ディスクイメージ作成

#### 空のFDD作成
- サイズ: 1.44MB, 1.2MB, 720KB, 640KB
- フォーマット: MS-DOS, PC-98 DOS
- ファイル形式: .d88推奨

#### 空のHDD作成
- サイズ: 10MB〜2GB
- ジオメトリ: 自動設定
- フォーマット: 未フォーマット

### 7.2 実ディスクからのイメージ作成

#### FDDイメージ化
1. 物理フロッピーディスクをセット
2. ディスクイメージ作成ツール使用
3. .d88形式で保存

#### CD-ROMイメージ化
1. 物理CD-ROMをセット
2. CD-ROMイメージ作成ツール使用
3. .iso/.cue形式で保存

---

## 8. ネットワークストレージ

### 8.1 ネットワーク共有

#### SMB/CIFS共有
```ini
# ネットワークパス例
HDD1FILE=\\server\share\disk.hdd
FDD1FILE=\\nas\images\game.d88
```

### 8.2 クラウドストレージ

#### 同期フォルダ使用
```ini
# Dropbox/OneDrive等の同期フォルダ
hdrvroot=C:\Users\user\Dropbox\PC98
FDD1FILE=C:\Users\user\OneDrive\Images\system.d88
```

---

## 9. 用途別設定例

### 9.1 ゲーム用設定

```ini
[NekoProjectII]
fddequip=3
usefd144=1
FDD1FILE=
FDD2FILE=
HDD1FILE=C:\np21w\hdd\games.hdd
HDD2FILE=
hdrvenable=0
MOTOR=1
MOTORVOL=30
savefddfile=1
```

### 9.2 開発・作業用設定

```ini
[NekoProjectII]
fddequip=3
usefd144=1
HDD1FILE=C:\np21w\hdd\system.hdd
HDD2FILE=C:\np21w\hdd\work.hdd
hdrvenable=1
hdrvroot=C:\np21w\exchange
MOTOR=0
savefddfile=1
useasynchd=1
```

### 9.3 音楽・マルチメディア用設定

```ini
[NekoProjectII]
fddequip=3
usefd144=1
HDD1FILE=C:\np21w\hdd\music.hdd
CDROM1FILE=C:\np21w\cd\album.cue
useasynccd=1
usecdecc=1
cddtskip=0
```

---

## 10. トラブルシューティング

### 10.1 ディスクが認識されない

**原因と対策:**

1. **ファイルパス確認**
   - パスに日本語・特殊文字が含まれていないか
   - ファイルが実在するか

2. **ファイル形式確認**
   - 対応形式かチェック
   - ファイルが破損していないか

3. **ドライブ設定確認**
   ```ini
   fddequip=3    # 適切なドライブ数
   ```

### 10.2 ディスクアクセスが遅い

**原因と対策:**

1. **非同期アクセス有効**
   ```ini
   useasynchd=1
   useasynccd=1
   ```

2. **待機時間調整**
   ```ini
   iderwait=10   # 待機時間短縮
   idewwait=10
   ```

3. **SSD使用**
   - HDDからSSDに移行

### 10.3 ディスクが書き込めない

**原因と対策:**

1. **書き込み保護確認**
   - ファイルの読み取り専用属性
   - フォルダのアクセス権限

2. **ディスク容量確認**
   - 十分な空き容量があるか

3. **自動保存設定**
   ```ini
   savefddfile=1
   savecdfile=1
   ```

### 10.4 ホストドライブが使えない

**原因と対策:**

1. **設定確認**
   ```ini
   hdrvenable=1
   hdrvacc=1
   ```

2. **ルートパス確認**
   ```ini
   hdrvroot=C:\valid\path
   ```

3. **NT系設定** (Windows NT系)
   ```ini
   hdrvntenable=1
   ```

---

## 11. 設定テンプレート集

### 11.1 基本設定
```ini
[NekoProjectII]
fddequip=3
usefd144=1
HDD1FILE=
savefddfile=1
MOTOR=1
MOTORVOL=50
```

### 11.2 高性能設定
```ini
[NekoProjectII]
fddequip=3
usefd144=1
HDD1FILE=C:\np21w\hdd\system.hdd
HDD2FILE=C:\np21w\hdd\work.hdd
CDROM1FILE=
useasynchd=1
useasynccd=1
iderwait=10
idewwait=10
savefddfile=1
savecdfile=1
```

### 11.3 互換性重視設定
```ini
[NekoProjectII]
fddequip=3
usefd144=0
HDD1FILE=
MOTOR=1
MOTORVOL=100
useasynchd=0
iderwait=100
idewwait=100
```

この設定マニュアルを参考に、お使いの用途に最適なストレージ設定を行ってください。
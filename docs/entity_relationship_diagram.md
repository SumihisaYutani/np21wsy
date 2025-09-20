# NP21WSY PC-9801 Emulator - Entity Relationship Diagram

## データエンティティ関係図

このER図は、NP21WSYエミュレータ内の主要なデータ構造体とその関係性を示しています。

```
                                 ┌─────────────────┐
                                 │     NP2CFG      │
                                 │  (Global Config)│
                                 │                 │
                                 │ • Hardware      │
                                 │ • CPU Settings  │
                                 │ • Sound Config  │
                                 │ • Memory Config │
                                 └─────────┬───────┘
                                           │ (1:1)
                                           │ configures
                                           ▼
┌─────────────────┐                ┌─────────────────┐                ┌─────────────────┐
│     PCCORE      │                │     PCSTAT      │                │    _NEVENT      │
│  (Core State)   │                │ (Runtime State) │                │ (Event System)  │
│                 │                │                 │                │                 │
│ • CPU Mode      │◄──────────────►│ • Screen Update │                │ • Event Queue   │
│ • Model Info    │     (1:1)      │ • Draw Frame    │                │ • Timer Mgmt    │
│ • Sound ID      │   synced with  │ • Hardware Reset│                │ • 32 Event Slots│
│ • Device Flags  │                │                 │                │                 │
└─────────┬───────┘                └─────────────────┘                └─────────┬───────┘
          │ (1:1)                                                               │ (1:N)
          │ controls                                                            │ manages
          ▼                                                                     ▼
┌─────────────────┐                                                   ┌─────────────────┐
│   I286 Core     │                                                   │  _NEVENTITEM    │
│                 │                                                   │ (Event Entry)   │
│ ┌─────────────┐ │                                                   │                 │
│ │ I286REG8/16 │ │                                                   │ • Clock         │
│ │ • AX,BX,CX, │ │                                                   │ • Flag          │
│ │   DX,SI,DI, │ │                                                   │ • Callback      │
│ │   BP,SP     │ │                                                   │ • User Data     │
│ └─────────────┘ │                                                   └─────────────────┘
│                 │
│ ┌─────────────┐ │
│ │ I286DTR     │ │
│ │ • GDTR,IDTR │ │
│ │ • LDTR,TR   │ │
│ │ • MSW       │ │
│ └─────────────┘ │
└─────────┬───────┘
          │ (1:1)
          │ accesses
          ▼
┌─────────────────┐                ┌─────────────────┐                ┌─────────────────┐
│   Memory Map    │                │    _VRAMOP      │                │    _IOCORE      │
│                 │                │ (VRAM Control)  │                │ (I/O System)    │
│ • mem[0x200000] │                │                 │                │                 │
│ • VRAM Layout   │◄──────────────►│ • Access Page   │                │ • Port Decode   │
│ • Font ROM      │     (1:1)      │ • EGC Enable    │                │ • Handler Array │
│ • ITF ROM       │   coordinates  │ • GRCG Bits     │                │ • 8/10/12/16bit │
│                 │                │ • Analog Enable │                │                 │
└─────────┬───────┘                └─────────────────┘                └─────────┬───────┘
          │ (1:N)                                                               │ (1:N)
          │ contains                                                            │ manages
          ▼                                                                     ▼
┌─────────────────┐                ┌─────────────────┐                ┌─────────────────┐
│   VRAM Banks    │                │     Graphics    │                │   I/O Devices   │
│                 │                │   Controllers   │                │                 │
│ • VRAM_B        │                │                 │                │ ┌─────────────┐ │
│ • VRAM_R        │                │ ┌─────────────┐ │                │ │    _PIC     │ │
│ • VRAM_G        │◄──────────────►│ │    _GDC     │ │                │ │             │ │
│ • VRAM_E        │     (1:1)      │ │ (Graphics   │ │◄──────────────►│ │ • IRQ Mask  │ │
│                 │   controls     │ │ Display     │ │     (1:1)      │ │ • IRQ Req   │ │
└─────────────────┘                │ │ Controller) │ │   interrupts   │ └─────────────┘ │
                                   │ └─────────────┘ │                │                 │
                                   │                 │                │ ┌─────────────┐ │
                                   │ ┌─────────────┐ │                │ │    _PIT     │ │
                                   │ │   _CRTC     │ │                │ │             │ │
                                   │ │ (CRT        │ │                │ │ • Counters  │ │
                                   │ │ Controller) │ │                │ │ • Clock Div │ │
                                   │ └─────────────┘ │                │ └─────────────┘ │
                                   │                 │                │                 │
                                   │ ┌─────────────┐ │                │ ┌─────────────┐ │
                                   │ │    _EGC     │ │                │ │   _DMAC     │ │
                                   │ │ (Enhanced   │ │                │ │             │ │
                                   │ │ Graphics    │ │                │ │ • Channels  │ │
                                   │ │ Controller) │ │                │ │ • Transfer  │ │
                                   │ └─────────────┘ │                │ └─────────────┘ │
                                   └─────────────────┘                └─────────┬───────┘
                                                                                │ (1:N)
                                                                                │ controls
                                                                                ▼
┌─────────────────┐                ┌─────────────────┐                ┌─────────────────┐
│  Sound System   │                │   Storage       │                │   Serial I/F    │
│                 │                │                 │                │                 │
│ ┌─────────────┐ │                │ ┌─────────────┐ │                │ ┌─────────────┐ │
│ │  SOUNDCFG   │ │                │ │    _FDC     │ │                │ │  _RS232C    │ │
│ │             │ │                │ │             │ │                │ │             │ │
│ │ • Rate      │ │                │ │ • Drive Sel │ │                │ │ • Port Cfg  │ │
│ │ • Clock Base│ │                │ │ • Track/Sec │ │                │ │ • Baud Rate │ │
│ └─────────────┘ │                │ │ • Status    │ │                │ └─────────────┘ │
│                 │                │ └─────────────┘ │                │                 │
│ ┌─────────────┐ │                │                 │                │ ┌─────────────┐ │
│ │ Sound Chips │ │                │ ┌─────────────┐ │                │ │ _MOUSEIF    │ │
│ │             │ │                │ │    SXSI     │ │                │ │             │ │
│ │ • OPN/OPNA  │ │                │ │ (SASI/SCSI/ │ │                │ │ • Position  │ │
│ │ • PSG       │ │                │ │ IDE HDDs)   │ │                │ │ • Buttons   │ │
│ │ • ADPCM     │ │                │ └─────────────┘ │                │ └─────────────┘ │
│ └─────────────┘ │                └─────────────────┘                └─────────────────┘
└─────────────────┘
```

## 主要なエンティティ関係

### 設定管理系
- **NP2CFG** ↔ **PCCORE**: 1:1 - 設定が実行時コア状態を決定
- **PCCORE** ↔ **PCSTAT**: 1:1 - コア状態と実行時状態の同期

### CPU・メモリ系
- **I286 Core** ↔ **Memory Map**: 1:1 - CPUがメモリ空間にアクセス
- **Memory Map** ↔ **VRAM Banks**: 1:N - メモリ内に複数のVRAMバンク
- **VRAM Banks** ↔ **_VRAMOP**: 1:1 - VRAM操作制御

### イベント系
- **_NEVENT** ↔ **_NEVENTITEM**: 1:N - イベントシステムが複数のイベントを管理
- **All Components** ↔ **_NEVENT**: N:1 - 全コンポーネントがイベントシステムを利用

### I/O・デバイス系
- **_IOCORE** ↔ **I/O Devices**: 1:N - I/Oコアが複数デバイスを管理
- **I/O Devices** ↔ **_PIC**: N:1 - デバイスが割り込みコントローラを共有

### グラフィック系
- **Graphics Controllers** ↔ **VRAM Banks**: 1:1 - グラフィックコントローラがVRAMを制御
- **_GDC** ↔ **_PIC**: 1:1 - 表示同期割り込み

## データフロー関係

```
Configuration → Core State → CPU Execution → Memory/I/O Access → Device State Update → Events → Timing Sync
      ↑                                                                    ↓
State Save/Load ←←←←←←←←←←←←←←←←←←←←←←←←←←←←←←←←←←←← Platform Output (Screen/Audio)
```

## 状態保存・復元システム

```
┌─────────────────┐
│   SFENTRY       │
│ (State Entry)   │
│                 │
│ • Index[12]     │◄────┐
│ • Version       │     │ (N:1)
│ • Type          │     │ serializes
│ • Data Ptr      │     │
└─────────────────┘     │
                        │
                ┌─────────────────┐
                │  All Entities   │
                │                 │
                │ • CPU State     │
                │ • Memory State  │
                │ • Device State  │
                │ • Config State  │
                └─────────────────┘
```

このER図は、NP21WSYエミュレータの複雑なデータ構造間の関係を視覚化し、開発者がシステムの全体像を理解できるよう設計されています。
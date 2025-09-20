# NP21WSY PC-9801 Emulator - Class Diagram

## 主要なクラス構造図（C構造体とモジュール）

このクラス図は、CプロジェクトであるNP21WSYの構造体と関数モジュールを、オブジェクト指向のクラス図風に表現しています。

```
                             ┌─────────────────────────────┐
                             │         NP2CFG              │
                             │      <<Configuration>>      │
                             ├─────────────────────────────┤
                             │ + baseclock: UINT32         │
                             │ + multiple: UINT            │
                             │ + SOUND_SW: UINT8           │
                             │ + samplingrate: UINT32      │
                             │ + vol_master: UINT8         │
                             │ + EXTMEM: UINT8/UINT16      │
                             │ + fddfile[4]: OEMCHAR[]     │
                             │ + sasihdd[4]: OEMCHAR[]     │
                             ├─────────────────────────────┤
                             │ + pccore_cfgupdate(): void  │
                             │ + getbiospath(): void       │
                             └─────────────┬───────────────┘
                                           │ configures
                                           ▼
                             ┌─────────────────────────────┐
                             │         PCCORE              │
                             │       <<Core State>>        │
                             ├─────────────────────────────┤
                             │ + baseclock: UINT32         │
                             │ + multiple: UINT            │
                             │ + cpumode: UINT8            │
                             │ + model: UINT8              │
                             │ + sound: SOUNDID            │
                             │ + device: UINT32            │
                             ├─────────────────────────────┤
                             │ + pccore_init(): void       │
                             │ + pccore_reset(): void      │
                             │ + pccore_exec(): void       │
                             └─────────────┬───────────────┘
                                           │ manages
                                           ▼
    ┌─────────────────────────────┐       ┌─────────────────────────────┐       ┌─────────────────────────────┐
    │        _NEVENT              │       │       I286CORE             │       │        _IOCORE              │
    │      <<Event System>>       │       │      <<CPU Core>>           │       │      <<I/O Manager>>        │
    ├─────────────────────────────┤       ├─────────────────────────────┤       ├─────────────────────────────┤
    │ + readyevents: UINT         │       │ + s.r: I286REG              │       │ + out8[]: IOOUT[]           │
    │ + waitevents: UINT          │       │ + s.remainclock: SINT32     │       │ + inp8[]: IOINP[]           │
    │ + level[32]: NEVENTID       │       │ + s.baseclock: UINT32       │       │ + snd_out[]: IOOUT[]        │
    │ + item[32]: _NEVENTITEM     │       │ + s.GDTR: I286DTR           │       │ + snd_inp[]: IOINP[]        │
    ├─────────────────────────────┤       │ + s.IDTR: I286DTR           │       ├─────────────────────────────┤
    │ + nevent_set(): void        │       │ + s.MSW: UINT16             │       │ + iocore_out8(): void       │
    │ + nevent_reset(): void      │       ├─────────────────────────────┤       │ + iocore_inp8(): REG8       │
    │ + nevent_progress(): void   │       │ + i286c_step(): void        │       │ + iocore_attachout(): void  │
    │ + nevent_execule(): void    │       │ + i286c_interrupt(): void   │       │ + iocore_attachinp(): void  │
    └─────────────┬───────────────┘       │ + i286c_selector(): UINT32  │       └─────────────┬───────────────┘
                  │ triggers              └─────────────┬───────────────┘                     │ manages
                  ▼                                     │ accesses                            ▼
    ┌─────────────────────────────┐                     ▼                       ┌─────────────────────────────┐
    │      _NEVENTITEM            │       ┌─────────────────────────────┐       │         _PIC                │
    │     <<Event Entry>>         │       │       Memory Manager        │       │  <<Interrupt Controller>>   │
    ├─────────────────────────────┤       │      <<Memory Access>>      │       ├─────────────────────────────┤
    │ + clock: SINT32             │       ├─────────────────────────────┤       │ + irr: UINT8                │
    │ + flag: UINT32              │       │ + mem[0x200000]: UINT8      │       │ + imr: UINT8                │
    │ + proc: NEVENTCB            │       │ + VRAM_B: 0x0a8000          │       │ + isr: UINT8                │
    │ + userData: INTPTR          │       │ + VRAM_R: 0x0b0000          │       │ + icw: UINT8[5]             │
    ├─────────────────────────────┤       │ + VRAM_G: 0x0b8000          │       ├─────────────────────────────┤
    │ + NEVENTCB(): void          │       │ + FONT_ADRS: 0x110000       │       │ + pic_irq(): void           │
    └─────────────────────────────┘       ├─────────────────────────────┤       │ + pic_setirq(): void        │
                                          │ + memp_read8(): REG8        │       │ + pic_resetirq(): void      │
                                          │ + memp_write8(): void       │       └─────────────────────────────┘
                                          │ + memr_read16(): REG16      │
                                          │ + memr_write16(): void      │
                                          └─────────────┬───────────────┘
                                                        │ contains
                                                        ▼
    ┌─────────────────────────────┐                     ┌─────────────────────────────┐
    │       _VRAMOP               │                     │        _DMAC                │
    │     <<VRAM Control>>        │                     │    <<DMA Controller>>       │
    ├─────────────────────────────┤                     ├─────────────────────────────┤
    │ + operate: UINT             │                     │ + dmach[4]: DMACH           │
    │ + tramwait: UINT            │                     │ + stat: UINT8               │
    │ + vramwait: UINT            │                     │ + mask: UINT8               │
    │ + grcgwait: UINT            │                     ├─────────────────────────────┤
    ├─────────────────────────────┤                     │ + dmac_check(): void        │
    │ + vram_initialize(): void   │                     │ + dmac_dword(): void        │
    └─────────────┬───────────────┘                     └─────────────────────────────┘
                  │ controls
                  ▼
    ┌─────────────────────────────┐       ┌─────────────────────────────┐       ┌─────────────────────────────┐
    │         _GDC                │       │         _CRTC               │       │         _EGC                │
    │  <<Graphics Controller>>    │       │    <<CRT Controller>>       │       │  <<Enhanced Graphics>>      │
    ├─────────────────────────────┤       ├─────────────────────────────┤       ├─────────────────────────────┤
    │ + clock: SINT32             │       │ + reg[0x100]: UINT8         │       │ + ope: UINT16               │
    │ + cnt: UINT                 │       │ + mode: UINT8               │       │ + fgc: UINT16               │
    │ + mod: UINT8                │       │ + status: UINT8             │       │ + mask: UINT16              │
    │ + cmd: UINT8                │       ├─────────────────────────────┤       │ + bgc: UINT16               │
    │ + para[16]: UINT16          │       │ + crtc_out(): void          │       │ + pat[4]: UINT16            │
    ├─────────────────────────────┤       │ + crtc_inp(): REG8          │       ├─────────────────────────────┤
    │ + gdc_cmd(): void           │       └─────────────────────────────┘       │ + egc_vram_write(): void    │
    │ + gdc_sync(): void          │                                             │ + egc_vram_read(): UINT32   │
    │ + gdc_vsync(): void         │                                             └─────────────────────────────┘
    └─────────────────────────────┘

    ┌─────────────────────────────┐       ┌─────────────────────────────┐       ┌─────────────────────────────┐
    │      SOUNDCFG               │       │         _FDC                │       │       _RS232C               │
    │    <<Sound Config>>         │       │  <<Floppy Controller>>      │       │   <<Serial Interface>>      │
    ├─────────────────────────────┤       ├─────────────────────────────┤       ├─────────────────────────────┤
    │ + rate: UINT                │       │ + stat: UINT8               │       │ + mode: UINT8               │
    │ + hzbase: UINT32            │       │ + drv: UINT8                │       │ + baudrate: UINT8           │
    │ + clockbase: UINT32         │       │ + trk: UINT8                │       │ + fifo[16]: UINT8           │
    │ + minclock: UINT32          │       │ + sec: UINT8                │       ├─────────────────────────────┤
    ├─────────────────────────────┤       │ + cmd: UINT8                │       │ + rs232c_out(): void        │
    │ + sound_create(): BRESULT   │       ├─────────────────────────────┤       │ + rs232c_inp(): REG8        │
    │ + sound_sync(): void        │       │ + fdc_out(): void           │       └─────────────────────────────┘
    │ + sound_pcmlock(): SINT32*  │       │ + fdc_inp(): REG8           │
    └─────────────┬───────────────┘       │ + fdc_readdata(): REG8      │
                  │ manages               └─────────────────────────────┘
                  ▼
    ┌─────────────────────────────┐       ┌─────────────────────────────┐
    │      Sound Boards           │       │        SXSI                 │
    │    <<Board Collection>>     │       │   <<Storage Interface>>     │
    ├─────────────────────────────┤       ├─────────────────────────────┤
    │ + board14: BOARD14          │       │ + drv[16]: SXSIDEV          │
    │ + board26k: BOARD26K        │       │ + devinfo[16]: SXSIDEVINFO  │
    │ + board86: BOARD86          │       ├─────────────────────────────┤
    │ + boardspb: BOARDSPB        │       │ + sxsi_reset(): void        │
    │ + boardsb16: BOARDSB16      │       │ + sxsi_getptr(): SXSIDEV*   │
    ├─────────────────────────────┤       │ + sxsi_read(): REG8         │
    │ + cbuscore_reset(): void    │       │ + sxsi_write(): void        │
    │ + cbuscore_bind(): void     │       └─────────────────────────────┘
    └─────────────────────────────┘
```

## インターフェース継承関係

```
                    ┌─────────────────────────────┐
                    │      IOOUT/IOINP            │
                    │     <<Function Ptr>>        │
                    ├─────────────────────────────┤
                    │ + IOOUT(port, val): void    │
                    │ + IOINP(port): REG8         │
                    └─────────────┬───────────────┘
                                  │ implements
                                  ▼
    ┌─────────────────────────────┐ ┌─────────────────────────────┐ ┌─────────────────────────────┐
    │    System I/O Handlers      │ │    Sound I/O Handlers       │ │   Device I/O Handlers       │
    │                             │ │                             │ │                             │
    │ + pic_out(): void           │ │ + opn_out(): void           │ │ + fdc_out(): void           │
    │ + pit_out(): void           │ │ + psg_out(): void           │ │ + crtc_out(): void          │
    │ + dmac_out(): void          │ │ + pcm_out(): void           │ │ + sasi_out(): void          │
    │ + gdc_out(): void           │ │ + midi_out(): void          │ │ + serial_out(): void        │
    └─────────────────────────────┘ └─────────────────────────────┘ └─────────────────────────────┘
```

## コールバック・委譲パターン

```
                    ┌─────────────────────────────┐
                    │       NEVENTCB              │
                    │    <<Function Pointer>>     │
                    ├─────────────────────────────┤
                    │ + (*proc)(NEVENTITEM): void │
                    └─────────────┬───────────────┘
                                  │ callback
                                  ▼
    ┌─────────────────────────────┐ ┌─────────────────────────────┐ ┌─────────────────────────────┐
    │    Timer Callbacks          │ │   Device Callbacks          │ │   Platform Callbacks       │
    │                             │ │                             │ │                             │
    │ + itimer_callback(): void   │ │ + fmtimera_callback(): void │ │ + screenvsync(): void       │
    │ + beep_callback(): void     │ │ + fdc_callback(): void      │ │ + soundmng_sync(): void     │
    │ + mouse_callback(): void    │ │ + rs232c_callback(): void   │ │ + taskmng_exit(): void      │
    └─────────────────────────────┘ └─────────────────────────────┘ └─────────────────────────────┘
```

## ファクトリーパターン（CPU選択）

```
                    ┌─────────────────────────────┐
                    │       CPU Factory           │
                    │     <<Static Factory>>      │
                    ├─────────────────────────────┤
                    │ + create_cpu(type): void*   │
                    └─────────────┬───────────────┘
                                  │ creates
                                  ▼
    ┌─────────────────────────────┐ ┌─────────────────────────────┐ ┌─────────────────────────────┐
    │       I286C Core            │ │        I286X Core           │ │       I386C Core            │
    │     <<Concrete CPU>>        │ │     <<Concrete CPU>>        │ │     <<Concrete CPU>>        │
    │                             │ │                             │ │                             │
    │ + i286c_step(): void        │ │ + i286x_step(): void        │ │ + i386c_step(): void        │
    │ + i286c_interrupt(): void   │ │ + i286x_interrupt(): void   │ │ + i386c_interrupt(): void   │
    │ + i286c_reset(): void       │ │ + i286x_reset(): void       │ │ + i386c_reset(): void       │
    └─────────────────────────────┘ └─────────────────────────────┘ └─────────────────────────────┘
```

## Strategyパターン（プラットフォーム抽象化）

```
                    ┌─────────────────────────────┐
                    │   Platform Interface        │
                    │     <<Abstract>>            │
                    ├─────────────────────────────┤
                    │ + scrnmng_*(): void         │
                    │ + soundmng_*(): void        │
                    │ + sysmng_*(): void          │
                    └─────────────┬───────────────┘
                                  │ implements
                                  ▼
    ┌─────────────────────────────┐ ┌─────────────────────────────┐ ┌─────────────────────────────┐
    │     SDL2 Platform           │ │     Win9x Platform          │ │      X11 Platform           │
    │  <<Concrete Platform>>      │ │  <<Concrete Platform>>      │ │  <<Concrete Platform>>      │
    │                             │ │                             │ │                             │
    │ + SDL2 specific impl.       │ │ + Windows specific impl.    │ │ + X11 specific impl.        │
    └─────────────────────────────┘ └─────────────────────────────┘ └─────────────────────────────┘
```

この設計により、NP21WSYは高度にモジュール化され、拡張可能で保守しやすいエミュレータとなっています。
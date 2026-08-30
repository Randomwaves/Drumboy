# Drumboy

**A touch-controlled hardware groovebox** — sequencer, sampler, and DSP effects engine built around an STM32H723 (Cortex-M7), with a 480×854 color display, a fully capacitive-touch control surface, and MIDI/Sync I/O.

This repository holds the complete hardware design (Eagle/KiCad) and firmware (STM32CubeIDE / C++) for Drumboy, released by [Randomwaves](https://randomwaves.io) as an open hardware/software project. Drumboy is the predecessor of [Drumboy Pro](https://github.com/Randomwaves/Drumboy-Pro), sharing the same sequencer and DSP engine on a simpler, touch-based control surface.

---

## Overview

The MCU sits at the center of the same core subsystems as Drumboy Pro: an NT35510 display over a 16-bit parallel GPIO bus, external SDRAM on the FMC bus for sample streaming, an SGTL5000 audio codec over I2S/I2C, a microSD card over SDMMC2 for storage, and MIDI/Sync connectivity over USART/GPIO. In place of mechanical keys and encoders, the control surface is driven entirely by three capacitive-touch channels.

| | |
|---|---|
| **MCU** | STM32H723ZGT6 — Cortex-M7, LQFP-144 |
| **Display** | NT35510, 480×854, 16-bit parallel |
| **SDRAM** | External, FMC bus (sample streaming) |
| **Audio codec** | SGTL5000 — I2S3 audio / I2C3 control |
| **Storage** | microSD, SDMMC2, FAT filesystem |
| **Control surface** | 3× capacitive-touch channels (CT0–CT2) |
| **Connectivity** | MIDI In/Out, Sync In/Out |
| **Firmware** | v1.05 |

## Features

- **10 sample layers**, each with independent LFO-driven modulation
- **Sequencer** with 5 song banks per layer, 64 beats per bank, and algorithmic complex-fill generation (12 sub-hits per fill slot)
- **9 effect types** per send slot: Delay, Chorus, Flanger, Phaser, Compressor, Expander, Overdrive, Distortion, Bitcrusher
- **2 multimode filters + 2 send effects + stereo reverb** in the DSP chain
- **MIDI In/Out** and **Sync In/Out** for external clock/gate sync
- **SD-card firmware updates** via a dedicated bootloader, mirroring the same two-project split as Drumboy Pro

## Repository structure

```
01 Hardware/
  Main/       Main board — Eagle CAD (Main.sch / Main.brd)
  Top/        Top/control-surface board — Eagle CAD (Top.sch / Top.brd)
  Connector/  Interconnect board — KiCad (migrated from Eagle)
  Bottom/     Base board — KiCad (migrated from Eagle)

02 Software/
  Drumboy-Boot-H723/   Bootloader — STM32CubeIDE project
  Drumboy-App-H723/    Application firmware — STM32CubeIDE project
  Firmware/            Prebuilt firmware binaries
  Sd Card/             Reference SD card contents (samples, presets, drumkits, system data)
```

## Hardware

Main and Top were originally designed in **Autodesk Eagle** (`.sch` / `.brd`); Connector and Bottom have since been migrated to **KiCad**.

## Firmware

Built with **STM32CubeIDE**, written in C/C++ against the STM32 HAL. Firmware is split into `Drumboy-Boot-H723` (bootloader) and `Drumboy-App-H723` (application), sharing one flash device — the same split used in Drumboy Pro.

## License

Released under the [MIT License](LICENSE).

## Community

- 💬 [Discord community](https://discord.gg/5atevNH6UU) — join the discussion
- 🌐 [randomwaves.io](https://www.randomwaves.io)
- ✉️ [hello@randomwaves.io](mailto:hello@randomwaves.io)

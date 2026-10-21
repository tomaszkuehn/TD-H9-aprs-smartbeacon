# TD-H9 APRS Smart Beaconing

<img width="230" height="250" alt="IMG_2" src="https://github.com/user-attachments/assets/8e8798c1-ace5-46ec-9609-d54a3274ffbf" />


<img width="220" height="250" alt="IMG_1" src="https://github.com/user-attachments/assets/f4145d49-a2d4-48fa-84bb-81b370670d8a" />


Reverse engineering a handheld radio (TIDRADIO TD-H9) to replace its fixed
interval APRS beacon with **smart beaconing** driven by real GNSS data:
speed-dependent intervals, corner pegging and a final beacon on stop.
There was no source code, no documentation and no debugger, only the
stock firmware update files and the radio itself.

> **What this repository contains:** project write-up, architecture,
> method, and the portable core of the smart beaconing logic with tests.
> **What it does not contain:** vendor firmware, disassembly, encryption
> details, patch offsets or flashable images. This is deliberate: the
> vendor code is not mine to publish, and a bad image can brick a radio.

## Results 

| Milestone | Status |
|---|---|
| Patched SoC firmware boots and runs (flashed over USB-C, no brick) | verified |
| APRS module sends its own frame inside the radio's TX window | verified |
| Module triggers a beacon on demand through the radio's own TX path | verified |
| Smart beaconing on real GNSS, urban route | verified |
| Custom radio alias shown in the menu | verified |
| Menu entry *Timed Beacon: OFF / ON / Smart* | built, awaiting field test |

## How it works (short)

```
 GNSS ──NMEA──▶ APRS module (ARM MCU) ──SLIP/UART──▶ main SoC (JieLi AC695X)
                 │  RMC parser                         │  menu, config, PTT, RF
                 │  smart beacon scheduler ──"TX now"─▶│  stock beacon timer path
                 └◀────────── AX.25 frame ─────────────┘
                 AFSK 1200 Bd modem ──▶ RF
```

The module decides *when* to transmit. The SoC still owns *how*:
PTT, RF sequencing, AX.25 framing and user settings. Reusing the stock
TX path instead of re-implementing it was the key to a safe result.
Details: [docs/architecture.md](docs/architecture.md).

## Repository layout

| Path | Content |
|---|---|
| [docs/architecture.md](docs/architecture.md) | hardware/software split, data flow, design decisions |
| [docs/method.md](docs/method.md) | reverse engineering and test method, lessons learned |
| [docs/results.md](docs/results.md) | what was tested, how, and what was observed |
| [src/smartbeacon.c](src/smartbeacon.c) | beacon scheduler (portable C99, no hardware deps) |
| [src/nmea_rmc.c](src/nmea_rmc.c) | streaming RMC parser, no buffers, no floats |
| [test/test_main.c](test/test_main.c) | host-side tests |

```sh
make test      # gcc or clang
```

## Skills demonstrated

- **Radio systems:** APRS/AX.25, AFSK 1200 Bd, PTT keying and RF sequencing,
  NMEA, smart beaconing parameters tuned for walking, cycling and driving.
- **Hardware and firmware architecture:** mapping roles between a SoC and a
  co-processor, an inter-chip protocol, the RAM layout, non-volatile config,
  menu tables and the update file format, all recovered from binaries only.
- **Debugging and testing:** one hypothesis per iteration, a registry of
  failed attempts, offline verification gates before every flash
  (integrity checks, round-trip decode, instruction-set emulator run against
  a stock baseline), field tests as the final judge.
- **AI-assisted analysis:** used an AI coding agent (Claude Code) for
  disassembly triage, script and tool generation and cross-checking.
  I set the hypotheses, ran the hardware tests, and rejected
  conclusions that measurements did not confirm.

## Author

**Tomasz Kuehn**: reverse engineering, firmware, radio integration and field testing.  
Callsign: SP3FHI · [LinkedIn](https://www.linkedin.com/in/tomasz-kuehn/)

If you use or build on this work, please credit the author and link to this
repository. GitHub's **"Cite this repository"** button (from
[CITATION.cff](CITATION.cff)) gives a ready-made citation.

## Disclaimer

Independent hobby project for interoperability and education, not affiliated
with the radio manufacturer. Transmitting requires an amateur radio licence.

## License

- **Code** (`src/`, `test/`, `Makefile`): MIT, see [LICENSE](LICENSE).
- **Documentation and text** (`README.md`, `docs/`): CC BY 4.0, see
  [LICENSE-DOCS.md](LICENSE-DOCS.md). You may share and adapt it, including
  commercially, as long as you credit **Tomasz Kuehn** and link to this repository.

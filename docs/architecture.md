# Architecture

## Hardware

| Block | Role |
|---|---|
| Main SoC: JieLi AC695X (pi32v2 core) | UI and menu, user config in NV storage, PTT, RF sequencing, AX.25 frame building, beacon timer |
| APRS module: ARM Cortex-M MCU | AFSK 1200 Bd modem (AX.25 with X.25 FCS), GNSS receiver interface, UART link to the SoC |
| GNSS receiver | NMEA over UART to the module; its power rail is switched by a GPIO |

One early assumption was wrong: a GPIO that looked like PTT turned out to be
the **GNSS power** switch. Toggling it explained a long-standing "no fix"
symptom. This is a good example of why every hypothesis was field-tested.

## Inter-chip link

- SLIP-framed UART between SoC and module: command byte, sub-type, payload.
- SoC → module: version/status queries, PTT control, frames to transmit, config bytes.
- Module → SoC: responses, NMEA relay (the SoC shows GNSS status from it),
  and, after this project, a **"transmit now" trigger**.

## Design: who decides what

```
            ┌───────────── APRS module ─────────────┐
 NMEA ─────▶│ RMC parser ─▶ smart beacon scheduler  │
            │                     │ trigger         │
            └─────────────────────┼─────────────────┘
                                  ▼
            ┌──────────────── main SoC ─────────────┐
            │ beacon timer ─▶ PTT + RF ─▶ AX.25 ────┼──▶ module modem ─▶ RF
            │ menu: Timed Beacon OFF / ON / Smart   │
            └───────────────────────────────────────┘
```

Rejected alternatives (each tried or analysed, each failed for a documented
reason):

1. **The module keys the transmitter on its own.** It bypasses the SoC RF
   sequencing, so it is unsafe and was never reliable.
2. **Cloning the SoC RF sequence inside the module command handler.** It did
   not reproduce the radio's timing.
3. **Injecting frames into SoC internal queues.** Wrong layer: the stock timer
   path ignored them.

The chosen design adds a small handler in the SoC that, on the module's
trigger, advances the **stock beacon timer** past its threshold. The
transmission then follows the stock, already proven path, with the user's
Timed Beacon interval kept as a safety net.

## SoC firmware patching (overview only)

- The update container format was decoded: encryption, header and payload
  integrity checks. A patcher rebuilds a flashable image and verifies it by
  decoding it back.
- Patches live in unused code space and only after verifying the original
  bytes at each site.
- The new menu option reuses existing NV storage. Space came from removing an
  unused UI language (now redirected to English).

Offsets, keys and patch bytes are intentionally not published.

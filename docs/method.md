# Method

## Principles

- **One iteration = one hypothesis.** Each firmware build tests exactly one
  idea, so a field result cannot be ambiguous.
- **A field test beats disassembly.** Static analysis proposes; the radio on
  the air decides. Nothing was declared working before a hardware test.
- **Failed attempts registry.** Each failed path gets an entry with the
  observed behaviour and the root cause, so it is never retried by accident
  (by a person or by the AI agent).

## Pre-flash gates (every SoC build)

1. Verify the original bytes at every patch site before writing.
2. Rebuild the update container and recompute all integrity checks.
3. Round-trip: decode the produced image and compare it with the intended binary.
4. Boot the patched app in a custom **pi32v2 instruction-set emulator** and
   compare the instruction count, final PC and unimplemented opcodes with
   the stock baseline.
5. Record a SHA-256 of the image and keep the previous image for rollback.

Result: no bricked radio across the whole project.

## Debugging examples

| Symptom | Root cause | Fix |
|---|---|---|
| No GNSS fix after a firmware change | GPIO assumed to be PTT was GNSS power | stopped toggling it |
| SoC drops GNSS data under load | NMEA relayed in chunks cut mid-sentence | flush after UART idle, like the stock module |
| Unstable fix shown by the radio | bad NMEA checksum formatting in relay | correct hex checksum, dynamic timing |
| Beacons while standing still | GNSS speed noise | parked threshold of 2 km/h |
| Beacon cadence did not follow module | three wrong "cadence source" leads | traced the real SoC beacon timer |

## Tooling

Python (patcher, container codec, table parsers, emulator), the vendor pi32v2
toolchain (clang/objdump) for instruction encoding checks, arm-none-eabi
for the module, Ghidra, UART sniffers, WSL, git.

## Working with AI

The AI agent sped up the repetitive parts: mapping functions in disassembly,
writing one-off scripts, checking instruction encodings, keeping notes.
It also produced confident but wrong conclusions, which is why the
failed-attempts registry and the "field test first" rule existed. The
direction, the hypotheses and the go/no-go on every flash stayed with me.

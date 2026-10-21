# Results

All tests used a real radio on air, with reception checked on a second
receiver and on APRS-IS.

| Step | Test | Observation |
|---|---|---|
| Patched SoC boots | flash over USB-C, power cycle, menu walk | normal operation, no regression |
| Module frame in TX window | module replaces frame content every 15 s | frames decoded correctly on air |
| Triggered TX | module sends "transmit now" | radio keys up through stock path |
| Smart beaconing | urban route with real GNSS | rate follows speed, corners and stops trigger beacons |
| Radio alias | custom alias in config | shown in menu |
| OFF / ON / Smart menu | built and verified offline (emulator, round-trip) | **pending field test** |

Default scheduler parameters (see [src/smartbeacon.c](../src/smartbeacon.c)):

| Situation | Beacon |
|---|---|
| parked (< 2 km/h) or no fix | every 10 min |
| walking 5 km/h | ~every 216 s (~300 m) |
| cycling 15 km/h | ~every 72 s |
| driving >= 18 km/h | every 60 s |
| corner | > 30 deg + 2400/speed, at least 15 s apart |
| stop | one beacon after 30 s parked |
| fix acquired / regained | immediately |

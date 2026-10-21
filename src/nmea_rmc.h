/*
 * nmea_rmc.h - byte-at-a-time RMC parser (status, speed, course).
 * No buffers, no floats: suitable for an ISR-fed UART on a small MCU.
 */
#ifndef NMEA_RMC_H
#define NMEA_RMC_H

#include <stdint.h>

typedef struct {
    uint8_t  fld, rmc, st, frac;
    uint32_t kn10, crs10;
} rmc_parser_t;

typedef struct {
    uint8_t  valid;     /* status 'A'        */
    uint16_t kmh10;     /* speed [0.1 km/h]  */
    uint16_t course;    /* course [deg]      */
} rmc_fix_t;

/* Returns 1 when a complete RMC sentence was parsed into *out. */
int rmc_feed(rmc_parser_t *p, char c, rmc_fix_t *out);

#endif

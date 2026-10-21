/*
 * nmea_rmc.c - byte-at-a-time RMC parser (fields 2: status, 7: knots, 8: course).
 * Any talker ID is accepted ($GPRMC, $GNRMC, ...).
 */
#include "nmea_rmc.h"

int rmc_feed(rmc_parser_t *p, char c, rmc_fix_t *out)
{
    if (c == '$') { *p = (rmc_parser_t){0}; return 0; }
    if (c == ',') {
        if (p->fld == 0) p->rmc = (p->st == 3);   /* "..RMC" matched */
        if (p->fld == 7 && p->frac == 0) p->kn10 *= 10u;
        if (p->fld == 8 && p->frac == 0) p->crs10 *= 10u;
        p->fld++; p->frac = 0;
        if (p->fld == 1) p->st = 0;
        return 0;
    }
    if (c == '*') {
        int done = p->rmc && p->fld >= 8;
        if (done) {
            out->valid  = (p->st == 'A');
            out->kmh10  = (uint16_t)((p->kn10 * 1852u) / 1000u);
            out->course = (uint16_t)((p->crs10 / 10u) % 360u);
        }
        p->rmc = 0;
        return done;
    }
    if (p->fld == 0) {                  /* match the "RMC" suffix */
        if (c == 'R') p->st = 1;
        else if (c == 'M' && p->st == 1) p->st = 2;
        else if (c == 'C' && p->st == 2) p->st = 3;
        else p->st = 0;
        return 0;
    }
    if (!p->rmc) return 0;
    if (p->fld == 2) { p->st = (uint8_t)c; return 0; }
    if (p->fld == 7 || p->fld == 8) {
        uint32_t *v = (p->fld == 7) ? &p->kn10 : &p->crs10;
        if (c == '.') { if (p->frac == 0) { *v *= 10u; p->frac = 1; } return 0; }
        if (c < '0' || c > '9') return 0;
        if (p->frac == 0) *v = *v * 10u + (uint32_t)(c - '0');
        else if (p->frac == 1) { *v += (uint32_t)(c - '0'); p->frac = 2; }
    }
    return 0;
}

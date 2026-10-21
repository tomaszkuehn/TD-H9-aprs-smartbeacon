/*
 * test_main.c - host-side tests for the RMC parser and the beacon scheduler.
 * Build and run: make test
 */
#include <stdio.h>
#include <string.h>
#include "../src/nmea_rmc.h"
#include "../src/smartbeacon.h"

static int fails;
#define CHECK(cond) do { if (!(cond)) { \
    printf("FAIL %s:%d  %s\n", __FILE__, __LINE__, #cond); fails++; } } while (0)

static int feed_str(rmc_parser_t *p, const char *s, rmc_fix_t *f)
{
    int got = 0;
    while (*s) got |= rmc_feed(p, *s++, f);
    return got;
}

static void test_rmc(void)
{
    rmc_parser_t p = {0};
    rmc_fix_t f = {0};

    CHECK(feed_str(&p, "$GNRMC,123519,A,5212.000,N,02101.000,E,10.0,084.4,211026,,,A*6A\r\n", &f));
    CHECK(f.valid == 1);
    CHECK(f.kmh10 == 185);            /* 10.0 kn = 18.52 km/h */
    CHECK(f.course == 84);

    CHECK(feed_str(&p, "$GPRMC,123520,V,,,,,,,211026,,,N*00\r\n", &f));
    CHECK(f.valid == 0);

    memset(&f, 0, sizeof f);
    CHECK(!feed_str(&p, "$GNGGA,123519,5212.000,N,02101.000,E,1,08,0.9,100.0,M,,M,,*00\r\n", &f));
    CHECK(feed_str(&p, "$GPRMC,1,A,0,N,0,E,3,270,211026,,,A*00", &f));
    CHECK(f.kmh10 == 55 && f.course == 270);   /* integer fields */
}

/* Simulate `secs` seconds at 1 Hz with constant GNSS input, count beacons. */
static int run(sb_state_t *s, uint32_t *t, uint32_t secs,
               uint8_t fix, uint16_t kmh10, uint16_t course)
{
    int n = 0;
    for (uint32_t i = 0; i < secs; i++, *t += 1000u) {
        if (fix) sb_on_rmc(s, *t, 1, kmh10, course);
        n += sb_tick(s, *t, 0);
    }
    return n;
}

static void test_scheduler(void)
{
    sb_state_t s;
    uint32_t t = 0;

    sb_init(&s, &SB_DEFAULTS);
    CHECK(run(&s, &t, 29, 1, 0, 0) == 0);       /* nothing during boot */
    CHECK(run(&s, &t, 2, 1, 0, 0) == 1);        /* first fix -> beacon */
    CHECK(run(&s, &t, 1200, 1, 0, 0) == 2);     /* parked: every 10 min */

    sb_init(&s, &SB_DEFAULTS); t = 0;
    run(&s, &t, 31, 1, 500, 0);                 /* fix beacon */
    CHECK(run(&s, &t, 600, 1, 500, 0) == 10);   /* 50 km/h: every 60 s */

    sb_init(&s, &SB_DEFAULTS); t = 0;
    run(&s, &t, 31, 1, 50, 0);
    CHECK(run(&s, &t, 216 * 3, 1, 50, 0) == 3); /* 5 km/h: every 216 s */

    sb_init(&s, &SB_DEFAULTS); t = 0;
    run(&s, &t, 31, 1, 500, 0);
    run(&s, &t, 20, 1, 500, 0);
    CHECK(run(&s, &t, 1, 1, 500, 90) == 1);     /* 90 deg corner at 50 km/h */
    CHECK(run(&s, &t, 10, 1, 500, 180) == 0);   /* guard: not within 15 s */

    sb_init(&s, &SB_DEFAULTS); t = 0;
    run(&s, &t, 31, 1, 500, 0);
    run(&s, &t, 20, 1, 500, 0);
    CHECK(run(&s, &t, 31, 1, 0, 0) == 1);       /* stop beacon after 30 s */
    CHECK(run(&s, &t, 300, 1, 0, 0) == 0);      /* then quiet while parked */

    sb_init(&s, &SB_DEFAULTS); t = 0;
    run(&s, &t, 31, 1, 0, 0);
    run(&s, &t, 60, 0, 0, 0);                   /* fix lost */
    CHECK(run(&s, &t, 1, 1, 0, 0) == 1);        /* fix back -> beacon */

    sb_init(&s, &SB_DEFAULTS); t = 0;
    CHECK(run(&s, &t, 599, 0, 0, 0) == 0);      /* no fix: wait park_s */
    CHECK(run(&s, &t, 2, 0, 0, 0) == 1);
}

int main(void)
{
    test_rmc();
    test_scheduler();
    printf(fails ? "%d test(s) FAILED\n" : "all tests passed\n", fails);
    return fails != 0;
}

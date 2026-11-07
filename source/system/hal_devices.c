#include "hal_devices.h"

#include <ogc/card.h>
#include <ogc/lwp_watchdog.h>

static s32 s_card_init_status = CARD_ERROR_BUSY;
static u64 s_last_probe_ticks = 0;

void hal_devices_init(system_snapshot_t *snap)
{
    s_card_init_status = CARD_Init(NULL, NULL);
    s_last_probe_ticks = 0;

    snap->card_init_status = s_card_init_status;
    snap->card_status[0] = s_card_init_status;
    snap->card_status[1] = s_card_init_status;
}

void hal_devices_update(system_snapshot_t *snap, int force_refresh)
{
    u64 now = gettime();

    snap->card_init_status = s_card_init_status;

    if (!force_refresh && s_last_probe_ticks != 0) {
        u64 elapsed_us = ticks_to_microsecs(now - s_last_probe_ticks);

        if (elapsed_us < 1000000)
            return;
    }

    s_last_probe_ticks = now;

    if (s_card_init_status != CARD_ERROR_READY) {
        snap->card_status[0] = s_card_init_status;
        snap->card_status[1] = s_card_init_status;
        return;
    }

    snap->card_status[0] = CARD_ProbeEx(0, NULL, NULL);
    snap->card_status[1] = CARD_ProbeEx(1, NULL, NULL);
}

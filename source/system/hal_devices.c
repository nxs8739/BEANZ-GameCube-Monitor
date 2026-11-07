#include "hal_devices.h"

#include <ogc/card.h>
#include <ogc/exi.h>
#include <sdcard/gcsd.h>

/*
 * An EXI device-ID response is evidence of a responding peripheral, not
 * a universal physical-presence signal. The direct transaction avoids
 * libogc's cached SP1 EXI_GetID result.
 *
 * Return 1 for a plausible response, 0 for a completed transaction without
 * a plausible response, -1 if the bus was unavailable or transfer failed.
 */
static s32 probe_exi_id(s32 channel, s32 device)
{
    u32 command = 0;
    u32 id = 0xFFFFFFFF;
    s32 selected = 0;
    s32 result = -1;

    if (!EXI_Lock(channel, device, NULL))
        return -1;

    if (!EXI_Select(channel, device, EXI_SPEED1MHZ))
        goto done;

    selected = 1;

    if (!EXI_Imm(channel, &command, 2, EXI_WRITE, NULL) ||
        !EXI_Sync(channel) ||
        !EXI_Imm(channel, &id, 4, EXI_READ, NULL) ||
        !EXI_Sync(channel))
        goto done;

    result = (id != 0x00000000 && id != 0xFFFFFFFF) ? 1 : 0;

done:
    if (selected)
        EXI_Deselect(channel);
    EXI_Unlock(channel);
    return result;
}

/*
 * SD2SP2 is a passive adapter: its SD card does not answer the ordinary
 * EXI device-ID command. Use libogc's SD interface to check for a card
 * response. No filesystem mount, sector read, or sector write is performed.
 */
static s32 probe_sp2(void)
{
    s32 result;

    if (__io_gcsd2.startup()) {
        result = __io_gcsd2.isInserted() ? 1 : 0;
        __io_gcsd2.shutdown();
        if (result)
            return 1;
    }

    return probe_exi_id(EXI_CHANNEL_2, EXI_DEVICE_0);
}

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
    snap->serial_port_status[0] = SERIAL_NOT_OCCUPIED;
    snap->serial_port_status[1] = SERIAL_NOT_OCCUPIED;
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

    /* Keep the existing 1-second, DEVICES-page polling cadence. */
    {
        s32 sp1 = probe_exi_id(EXI_CHANNEL_0, EXI_DEVICE_2);
        s32 sp2 = probe_sp2();

        /* An EXI lock/transfer failure is not proof of an empty port. */
        if (sp1 >= 0)
            snap->serial_port_status[0] = sp1 ? SERIAL_OCCUPIED : SERIAL_NOT_OCCUPIED;
        if (sp2 >= 0)
            snap->serial_port_status[1] = sp2 ? SERIAL_OCCUPIED : SERIAL_NOT_OCCUPIED;
    }

    if (s_card_init_status != CARD_ERROR_READY) {
        snap->card_status[0] = s_card_init_status;
        snap->card_status[1] = s_card_init_status;
        return;
    }

    snap->card_status[0] = CARD_ProbeEx(0, NULL, NULL);
    snap->card_status[1] = CARD_ProbeEx(1, NULL, NULL);
}

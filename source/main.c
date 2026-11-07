#include <stdio.h>
#include <stdlib.h>
#include <ogc/system.h>
#include <ogc/video.h>
#include <ogc/gx.h>
#include <ogc/lwp_watchdog.h>
#include <ogc/console.h>

#include "snapshot.h"
#include "hal_memory.h"
#include "hal_video.h"
#include "hal_time.h"
#include "hal_input.h"
#include "hal_devices.h"
#include "render.h"

static GXRModeObj *rmode;

int main(void)
{
    void *xfb;
    system_snapshot_t snapshot = {0};
    monitor_page_t page = PAGE_SYSTEM;
    u64 application_start_ticks;
    int force_redraw = 1;

    /* VIDEO INITIALIZATION */
    VIDEO_Init();
    rmode = VIDEO_GetPreferredMode(NULL);
    VIDEO_Configure(rmode);

    hal_video_cache_mode(rmode);

    xfb = SYS_AllocateFramebuffer(rmode);

    if (xfb == NULL) {
        while (1) {
        }
    }

    xfb = MEM_K0_TO_K1(xfb);

    VIDEO_ClearFrameBuffer(rmode, xfb, COLOR_BLACK);

    console_init(xfb, 20, 20,
                 rmode->fbWidth,
                 rmode->xfbHeight,
                 rmode->fbWidth * VI_DISPLAY_PIX_SZ);

    VIDEO_SetNextFramebuffer(xfb);
    VIDEO_SetBlack(FALSE);
    VIDEO_Flush();
    VIDEO_WaitVSync();

    if (rmode->viTVMode & VI_NON_INTERLACE)
        VIDEO_WaitVSync();

    /* SUBSYSTEM INITIALIZATION */
    render_init();
    hal_input_init();
    hal_devices_init(&snapshot);

    application_start_ticks = gettime();

    /* MAIN APPLICATION LOOP */
    while (1) {
        u32 navigation_events = NAV_EVENT_NONE;
        monitor_page_t previous_page = page;

        hal_time_start_frame();

        /* One direct controller read per frame. */
        hal_input_update(&snapshot, &navigation_events);

        if (navigation_events & NAV_EVENT_EXIT)
            break;

        if (navigation_events & NAV_EVENT_PREVIOUS) {
            page = (page == PAGE_SYSTEM) ? PAGE_DEVICES : PAGE_SYSTEM;
        }

        if (navigation_events & NAV_EVENT_NEXT) {
            page = (page == PAGE_DEVICES) ? PAGE_SYSTEM : PAGE_DEVICES;
        }

        force_redraw = (page != previous_page);

        /* SYSTEM TELEMETRY */
        hal_memory_get_snapshot(&snapshot);
        hal_video_get_snapshot(&snapshot);

        /*
         * Card probing is intentionally page-scoped.
         * Entering DEVICES forces an immediate probe.
         */
        if (page == PAGE_DEVICES)
            hal_devices_update(&snapshot, force_redraw);

        snapshot.uptime_us =
            ticks_to_microsecs(gettime() - application_start_ticks);

        hal_time_mark_collection_complete();

        /* VSYNC TIMING */
        {
            u64 pre_retrace_ticks = gettime();

            VIDEO_WaitVSync();

            {
                u64 post_retrace_ticks = gettime();
                u32 retrace_wait_us =
                    ticks_to_microsecs(post_retrace_ticks - pre_retrace_ticks);

                hal_time_end_frame_and_get_snapshot(&snapshot,
                                                     retrace_wait_us);
            }
        }

        render_draw_snapshot(&snapshot, page, force_redraw);
    }

    return 0;
}

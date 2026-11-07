#include "hal_time.h"
#include <ogc/lwp_watchdog.h>

static u64 s_frame_start_ticks = 0;
static u64 s_collection_end_ticks = 0;
static u64 s_previous_frame_ticks = 0;

void hal_time_start_frame(void) {
    s_frame_start_ticks = gettime();
}

void hal_time_mark_collection_complete(void) {
    s_collection_end_ticks = gettime();
}

void hal_time_end_frame_and_get_snapshot(system_snapshot_t *snap, u32 retrace_wait_us) {
    u64 current_ticks = gettime();

    // Frame interval: time between successive frame boundaries
    if (s_previous_frame_ticks != 0) {
        snap->frame_interval_us = ticks_to_microsecs(current_ticks - s_previous_frame_ticks);
        snap->frame_interval_status = METRIC_AVAILABLE;
    } else {
        snap->frame_interval_us = 0;
        snap->frame_interval_status = METRIC_NOT_INITIALIZED;
    }
    s_previous_frame_ticks = current_ticks;

    // Collection time: from frame start through completion of HAL collectors
    snap->collection_time_us = ticks_to_microsecs(s_collection_end_ticks - s_frame_start_ticks);
    snap->collection_time_status = METRIC_AVAILABLE;

    // Retrace wait
    snap->retrace_wait_us = retrace_wait_us;
    snap->retrace_wait_status = METRIC_AVAILABLE;

    // Draw completion (Phase 1: Unavailable)
    snap->draw_completion_barrier_wait_us = 0;
    snap->draw_completion_status = METRIC_UNAVAILABLE;
}

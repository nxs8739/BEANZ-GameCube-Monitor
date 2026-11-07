#ifndef SNAPSHOT_H
#define SNAPSHOT_H

#include <gctypes.h>
#include <ogc/video.h>

typedef enum {
    METRIC_UNAVAILABLE,
    METRIC_NOT_INITIALIZED,
    METRIC_AVAILABLE
} metric_status_t;

/* A positive response from the corresponding serial-port interface. */
typedef enum {
    SERIAL_NOT_OCCUPIED = 0,
    SERIAL_OCCUPIED = 1
} serial_presence_t;

/* A positive response from the corresponding serial-port interface. */


typedef struct {
    /* MEMORY */
    u32 mem1_size_bytes;
    metric_status_t mem1_status;

    u32 arena1_remaining_span_bytes;
    metric_status_t arena1_status;

    u32 allocator_in_use_bytes;
    metric_status_t allocator_in_use_status;

    u32 allocator_free_chunk_bytes;
    metric_status_t allocator_free_status;

    u32 aram_size_bytes;
    metric_status_t aram_status;

    /* VIDEO */
    u32 tv_mode;
    metric_status_t tv_mode_status;

    u32 scan_mode;
    metric_status_t scan_mode_status;

    GXRModeObj cached_video_mode;
    metric_status_t video_mode_cached_status;

    /* TIMING */
    u32 frame_interval_us;
    metric_status_t frame_interval_status;

    u32 collection_time_us;
    metric_status_t collection_time_status;

    u32 retrace_wait_us;
    metric_status_t retrace_wait_status;

    u32 draw_completion_barrier_wait_us;
    metric_status_t draw_completion_status;

    u64 uptime_us;

    /* CONTROLLERS: PAD_ERR_* values */
    s32 controller_error[4];

    /* MEMORY CARDS: CARD_ERROR_* values */
    s32 card_init_status;
    s32 card_status[2];

    /* SERIAL PORTS: response-based occupancy, not physical presence. */
    serial_presence_t serial_port_status[2];

    /* SERIAL PORTS: response-based occupancy, not physical presence. */

} system_snapshot_t;

#endif

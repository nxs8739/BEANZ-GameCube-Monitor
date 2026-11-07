#include "hal_video.h"
#include <ogc/video.h>
#include <string.h>

static GXRModeObj s_cached_mode;
static int s_mode_cached = 0; // Explicit int to avoid bool header dependencies

void hal_video_cache_mode(const GXRModeObj *rmode) {
    if (rmode != NULL) {
        memcpy(&s_cached_mode, rmode, sizeof(GXRModeObj));
        s_mode_cached = 1;
    }
}

void hal_video_get_snapshot(system_snapshot_t *snap) {
    snap->tv_mode = VIDEO_GetCurrentTvMode();
    snap->tv_mode_status = METRIC_AVAILABLE;

    snap->scan_mode = VIDEO_GetVideoScanMode();
    snap->scan_mode_status = METRIC_AVAILABLE;

    if (s_mode_cached) {
        memcpy(&snap->cached_video_mode, &s_cached_mode, sizeof(GXRModeObj));
        snap->video_mode_cached_status = METRIC_AVAILABLE;
    } else {
        memset(&snap->cached_video_mode, 0, sizeof(GXRModeObj));
        snap->video_mode_cached_status = METRIC_NOT_INITIALIZED;
    }
}

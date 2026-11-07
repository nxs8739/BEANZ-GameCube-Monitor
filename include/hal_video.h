#ifndef HAL_VIDEO_H
#define HAL_VIDEO_H

#include "snapshot.h"

void hal_video_cache_mode(const GXRModeObj *rmode);
void hal_video_get_snapshot(system_snapshot_t *snap);

#endif // HAL_VIDEO_H

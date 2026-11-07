#ifndef HAL_TIME_H
#define HAL_TIME_H

#include "snapshot.h"

void hal_time_start_frame(void);
void hal_time_mark_collection_complete(void);
void hal_time_end_frame_and_get_snapshot(system_snapshot_t *snap, u32 retrace_wait_us);

#endif // HAL_TIME_H

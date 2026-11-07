#ifndef HAL_INPUT_H
#define HAL_INPUT_H

#include "snapshot.h"

#define NAV_EVENT_NONE     0
#define NAV_EVENT_PREVIOUS 1
#define NAV_EVENT_NEXT     2
#define NAV_EVENT_EXIT     4

void hal_input_init(void);
void hal_input_update(system_snapshot_t *snap, u32 *navigation_events);

#endif

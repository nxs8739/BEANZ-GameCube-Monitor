#ifndef HAL_DEVICES_H
#define HAL_DEVICES_H

#include "snapshot.h"

void hal_devices_init(system_snapshot_t *snap);
void hal_devices_update(system_snapshot_t *snap, int force_refresh);

#endif

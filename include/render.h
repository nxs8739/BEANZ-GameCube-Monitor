#ifndef RENDER_H
#define RENDER_H

#include "snapshot.h"

typedef enum {
    PAGE_SYSTEM = 0,
    PAGE_DEVICES = 1
} monitor_page_t;

void render_init(void);
void render_draw_snapshot(const system_snapshot_t *snap,
                          monitor_page_t page,
                          int force_redraw);

#endif

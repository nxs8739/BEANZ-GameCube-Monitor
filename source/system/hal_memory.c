#include "hal_memory.h"
#include <ogc/system.h>
#include <ogc/aram.h>
#include <malloc.h>      // Compile/link verification for mallinfo

void hal_memory_get_snapshot(system_snapshot_t *snap) {
    // 1. MEM1 Size: Read from libogc low-memory initialization field (0x80000028)
    snap->mem1_size_bytes = *(volatile u32*)0x80000028;
    snap->mem1_status = METRIC_AVAILABLE;

    // 2. Arena 1 Remaining Span (Safe integer comparison)
    u32 lo = (u32)SYS_GetArena1Lo();
    u32 hi = (u32)SYS_GetArena1Hi();
    if (hi > lo) {
        snap->arena1_remaining_span_bytes = hi - lo;
        snap->arena1_status = METRIC_AVAILABLE;
    } else {
        snap->arena1_remaining_span_bytes = 0;
        snap->arena1_status = METRIC_UNAVAILABLE;
    }

    // 3. Allocator Statistics
    // Note: On 32-bit PowerPC, size_t is a 32-bit unsigned integer. 
    // Direct casting to u32 is inherently safe from overflow or negative wrapping.
    struct mallinfo mi = mallinfo();

    snap->allocator_in_use_bytes = (u32)mi.uordblks;
    snap->allocator_in_use_status = METRIC_AVAILABLE;

    snap->allocator_free_chunk_bytes = (u32)mi.fordblks;
    snap->allocator_free_status = METRIC_AVAILABLE;

    // 4. ARAM Capacity (NON-DESTRUCTIVE CHECK ONLY)
    if (AR_CheckInit()) {
        snap->aram_size_bytes = AR_GetSize();
        snap->aram_status = METRIC_AVAILABLE;
    } else {
        snap->aram_size_bytes = 0;
        snap->aram_status = METRIC_NOT_INITIALIZED;
    }
}

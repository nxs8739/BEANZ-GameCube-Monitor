#include "render.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <ogc/console.h>
#include <ogc/pad.h>
#include <ogc/card.h>
#include <ogc/video_types.h>

#define RENDER_MAX_ROWS 64
#define RENDER_MAX_COLS 255
#define RENDER_LINE_COUNT 64

static int s_columns = 0;
static int s_rows = 0;
static int s_initialized = 0;
static monitor_page_t s_previous_page = PAGE_SYSTEM;

static char s_previous_lines[RENDER_LINE_COUNT][RENDER_MAX_COLS + 1];
static u8 s_previous_valid[RENDER_LINE_COUNT];

static const char *video_standard_name(u32 mode)
{
    switch (mode) {
        case VI_NTSC:      return "NTSC";
        case VI_PAL:       return "PAL";
        case VI_MPAL:      return "MPAL";
        case VI_EURGB60:   return "EURGB60";
        case VI_DEBUG:     return "DEBUG NTSC";
        case VI_DEBUG_PAL: return "DEBUG PAL";
        default:           return "UNKNOWN";
    }
}

static const char *scan_mode_name(u32 mode)
{
    switch (mode) {
        case VI_INTERLACE:     return "Interlaced";
        case VI_NON_INTERLACE: return "Non-interlaced";
        case VI_PROGRESSIVE:   return "Progressive";
        default:               return "UNKNOWN";
    }
}

static void add_line(char lines[][RENDER_MAX_COLS + 1],
                     int *count,
                     const char *format,
                     ...)
{
    va_list args;

    if (*count >= RENDER_LINE_COUNT)
        return;

    va_start(args, format);
    vsnprintf(lines[*count], RENDER_MAX_COLS + 1, format, args);
    va_end(args);

    (*count)++;
}

static const char *controller_status(s32 error)
{
    switch (error) {
        case PAD_ERR_NONE:
            return "NO ERROR REPORTED";
        case PAD_ERR_NO_CONTROLLER:
            return "NO CONTROLLER";
        case PAD_ERR_NOT_READY:
            return "NOT READY";
        case PAD_ERR_TRANSFER:
            return "TRANSFER ERROR";
        default:
            return "UNKNOWN";
    }
}

static const char *card_status(s32 status)
{
    switch (status) {
        case CARD_ERROR_READY:
            return "READY";
        case CARD_ERROR_BUSY:
            return "BUSY";
        case CARD_ERROR_WRONGDEVICE:
            return "WRONG DEVICE";
        case CARD_ERROR_NOCARD:
            return "NO CARD";
        case CARD_ERROR_IOERROR:
            return "I/O ERROR";
        case CARD_ERROR_BROKEN:
            return "BROKEN";
        default:
            return "UNKNOWN / INIT ERROR";
    }
}

static void format_uptime(u64 uptime_us, char *buffer, size_t size)
{
    u64 total_seconds = uptime_us / 1000000;
    u64 hours = total_seconds / 3600;
    u64 minutes = (total_seconds / 60) % 60;
    u64 seconds = total_seconds % 60;

    snprintf(buffer, size, "%llu:%02llu:%02llu",
             (unsigned long long)hours,
             (unsigned long long)minutes,
             (unsigned long long)seconds);
}

static int build_system_page(const system_snapshot_t *snap,
                             char lines[][RENDER_MAX_COLS + 1])
{
    int count = 0;
    char uptime[32];

    format_uptime(snap->uptime_us, uptime, sizeof(uptime));

    add_line(lines, &count, "BEANZ GAMECUBE MONITOR  |  SYSTEM");
    add_line(lines, &count, "");
    add_line(lines, &count, "[ MEMORY ]");

    if (snap->mem1_status == METRIC_AVAILABLE)
        add_line(lines, &count, "Configured MEM1 Size: %u bytes",
                 (unsigned int)snap->mem1_size_bytes);

    if (snap->arena1_status == METRIC_AVAILABLE)
        add_line(lines, &count, "Arena 1 Span: %u bytes",
                 (unsigned int)snap->arena1_remaining_span_bytes);

    if (snap->allocator_in_use_status == METRIC_AVAILABLE)
        add_line(lines, &count, "Allocator Used: %u bytes",
                 (unsigned int)snap->allocator_in_use_bytes);

    if (snap->allocator_free_status == METRIC_AVAILABLE)
        add_line(lines, &count, "Allocator Free Chunks: %u bytes",
                 (unsigned int)snap->allocator_free_chunk_bytes);

    add_line(lines, &count, "");
    add_line(lines, &count, "[ ARAM ]");

    if (snap->aram_status == METRIC_AVAILABLE) {
        add_line(lines, &count, "Initialization: INITIALIZED");
        add_line(lines, &count, "Reported Size: %u bytes",
                 (unsigned int)snap->aram_size_bytes);
    } else if (snap->aram_status == METRIC_NOT_INITIALIZED) {
        add_line(lines, &count, "Initialization: NOT INITIALIZED");
        add_line(lines, &count, "Reported Size: N/A");
    } else {
        add_line(lines, &count, "Initialization: UNAVAILABLE");
        add_line(lines, &count, "Reported Size: N/A");
    }

    add_line(lines, &count, "");
    add_line(lines, &count, "[ VIDEO ]");

   if (snap->tv_mode_status == METRIC_AVAILABLE)
       add_line(lines, &count, "Video Standard: %s",
                video_standard_name(snap->tv_mode));

   if (snap->video_mode_cached_status == METRIC_AVAILABLE) {
       add_line(lines, &count, "Resolution: %ux%u",
                (unsigned int)snap->cached_video_mode.viWidth,
                (unsigned int)snap->cached_video_mode.viHeight);
   }

   if (snap->scan_mode_status == METRIC_AVAILABLE)
       add_line(lines, &count, "Scan Mode: %s",
                scan_mode_name(snap->scan_mode));

    add_line(lines, &count, "");
    add_line(lines, &count, "[ PROCESSOR ]");
    add_line(lines, &count, "Nominal CPU Clock: 486 MHz");

    add_line(lines, &count, "");
    add_line(lines, &count, "[ TIMING ]");
    add_line(lines, &count, "Application Uptime: %s", uptime);

    if (snap->collection_time_status == METRIC_AVAILABLE)
        add_line(lines, &count, "Collection Time: %u us",
                 (unsigned int)snap->collection_time_us);

    if (snap->retrace_wait_status == METRIC_AVAILABLE)
        add_line(lines, &count, "VSync Wait: %u us",
                 (unsigned int)snap->retrace_wait_us);

    if (snap->frame_interval_status == METRIC_AVAILABLE)
        add_line(lines, &count, "Frame Interval: %u us",
                 (unsigned int)snap->frame_interval_us);
    else
        add_line(lines, &count, "Frame Interval: Waiting for second frame");

    add_line(lines, &count, "");
    add_line(lines, &count, "L: DEVICES     R: DEVICES     START: EXIT");

    return count;
}

static int build_devices_page(const system_snapshot_t *snap,
                              char lines[][RENDER_MAX_COLS + 1])
{
    int count = 0;

    add_line(lines, &count, "BEANZ GAMECUBE MONITOR  |  DEVICES");
    add_line(lines, &count, "");
    add_line(lines, &count, "[ CONTROLLERS ]");

    add_line(lines, &count, "Port 1: %s", controller_status(snap->controller_error[0]));
    add_line(lines, &count, "Port 2: %s", controller_status(snap->controller_error[1]));
    add_line(lines, &count, "Port 3: %s", controller_status(snap->controller_error[2]));
    add_line(lines, &count, "Port 4: %s", controller_status(snap->controller_error[3]));

    add_line(lines, &count, "");
    add_line(lines, &count, "[ MEMORY CARDS ]");

    if (snap->card_init_status == CARD_ERROR_READY) {
        add_line(lines, &count, "Slot A: %s", card_status(snap->card_status[0]));
        add_line(lines, &count, "Slot B: %s", card_status(snap->card_status[1]));
    } else {
        add_line(lines, &count, "Card subsystem: INITIALIZATION ERROR");
        add_line(lines, &count, "Slot A: UNAVAILABLE");
        add_line(lines, &count, "Slot B: UNAVAILABLE");
    }

    add_line(lines, &count, "");
    add_line(lines, &count, "[ SERIAL PORTS ]");
    add_line(lines, &count, "SP1: STATUS UNKNOWN");
    add_line(lines, &count, "SP2: RESPONSE UNKNOWN");

    add_line(lines, &count, "");
    add_line(lines, &count, "L: SYSTEM      R: SYSTEM      START: EXIT");

    return count;
}

void render_init(void)
{
    int columns = 0;
    int rows = 0;

    CON_GetMetrics(&columns, &rows);

    if (columns <= 0 || rows <= 0)
        return;

    s_columns = columns;
    s_rows = rows;

    if (s_columns > RENDER_MAX_COLS)
        s_columns = RENDER_MAX_COLS;

    if (s_rows > RENDER_MAX_ROWS)
        s_rows = RENDER_MAX_ROWS;

    memset(s_previous_valid, 0, sizeof(s_previous_valid));
    s_initialized = 1;
}

void render_draw_snapshot(const system_snapshot_t *snap,
                          monitor_page_t page,
                          int force_redraw)
{
    char lines[RENDER_LINE_COUNT][RENDER_MAX_COLS + 1] = {{0}};
    int line_count;
    int row;

    if (!s_initialized || s_columns <= 0 || s_rows <= 0)
        return;

    if (page == PAGE_SYSTEM)
        line_count = build_system_page(snap, lines);
    else
        line_count = build_devices_page(snap, lines);

    if (force_redraw || page != s_previous_page) {
        printf("\x1b[2J");
        memset(s_previous_valid, 0, sizeof(s_previous_valid));
    }

    if (line_count > s_rows)
        line_count = s_rows;

    for (row = 0; row < line_count; row++) {
        if (!s_previous_valid[row] ||
            strcmp(lines[row], s_previous_lines[row]) != 0) {

            printf("\x1b[%d;1H%-*.*s",
                   row + 1,
                   s_columns,
                   s_columns,
                   lines[row]);

            memcpy(s_previous_lines[row], lines[row],
                   sizeof(s_previous_lines[row]));
            s_previous_valid[row] = 1;
        }
    }

    s_previous_page = page;
}

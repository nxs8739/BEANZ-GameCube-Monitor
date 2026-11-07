#include "hal_input.h"

#include <string.h>
#include <ogc/pad.h>

static const u32 s_channel_masks[PAD_CHANMAX] = {
    PAD_CHAN0_BIT,
    PAD_CHAN1_BIT,
    PAD_CHAN2_BIT,
    PAD_CHAN3_BIT
};

static u16 s_previous_buttons[PAD_CHANMAX];
static u8 s_baseline_valid[PAD_CHANMAX];

void hal_input_init(void)
{
    memset(s_previous_buttons, 0, sizeof(s_previous_buttons));
    memset(s_baseline_valid, 0, sizeof(s_baseline_valid));

    PAD_Init();
}

void hal_input_update(system_snapshot_t *snap, u32 *navigation_events)
{
    PADStatus status[PAD_CHANMAX];
    u32 reset_mask = 0;
    u32 read_mask;
    u32 events = NAV_EVENT_NONE;
    int channel;

    memset(status, 0, sizeof(status));

    read_mask = PAD_Read(status);

    for (channel = 0; channel < PAD_CHANMAX; channel++) {
        s32 error = status[channel].err;
        u16 buttons = status[channel].button;
        int normal_response = (read_mask & s_channel_masks[channel]) != 0;

        snap->controller_error[channel] = error;

        if (error == PAD_ERR_NO_CONTROLLER) {
            s_baseline_valid[channel] = 0;
            s_previous_buttons[channel] = 0;
            reset_mask |= s_channel_masks[channel];
            continue;
        }

        if (error == PAD_ERR_NOT_READY || error == PAD_ERR_TRANSFER) {
            continue;
        }

        if (error != PAD_ERR_NONE) {
            continue;
        }

        /*
         * Policy B:
         * A returned channel bit OR nonzero buttons is treated as a
         * normal sample. A zero-button sample without a returned bit
         * is ambiguous and does not generate button edges.
         */
        if (normal_response || buttons != 0) {
            if (s_baseline_valid[channel]) {
                u16 pressed = buttons & (u16)~s_previous_buttons[channel];

                if (channel == PAD_CHAN0) {
                    if (pressed & PAD_TRIGGER_L)
                        events |= NAV_EVENT_PREVIOUS;

                    if (pressed & PAD_TRIGGER_R)
                        events |= NAV_EVENT_NEXT;

                    if (pressed & PAD_BUTTON_START)
                        events |= NAV_EVENT_EXIT;
                }
            } else {
                /* First valid sample establishes a baseline only. */
                s_baseline_valid[channel] = 1;
            }

            s_previous_buttons[channel] = buttons;
        } else {
            /*
             * Ambiguous zero-button sample:
             * seed an invalid baseline, otherwise update the previous
             * state without generating an edge.
             */
            if (!s_baseline_valid[channel]) {
                s_baseline_valid[channel] = 1;
            }

            s_previous_buttons[channel] = 0;
        }
    }

    if (reset_mask != 0)
        PAD_Reset(reset_mask);

    if (navigation_events != NULL)
        *navigation_events = events;
}

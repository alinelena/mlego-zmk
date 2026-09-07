/*
 *
 * Copyright (c) 2023 The ZMK Contributors
 * SPDX-License-Identifier: MIT
 *
 */

#include <zephyr/kernel.h>

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <zmk/battery.h>
#include <zmk/display.h>
#include "status.h"
#include <zmk/events/usb_conn_state_changed.h>
#include <zmk/event_manager.h>
#include <zmk/events/battery_state_changed.h>
#include <zmk/events/ble_active_profile_changed.h>
#include <zmk/events/endpoint_changed.h>
#include <zmk/events/wpm_state_changed.h>
#include <zmk/events/layer_state_changed.h>
#include <zmk/usb.h>
#include <zmk/ble.h>
#include <zmk/endpoints.h>
#include <zmk/keymap.h>

#if IS_ENABLED(CONFIG_ZMK_WIDGET_WPM_STATUS)
#include <zmk/wpm.h>
#endif

#if !IS_ENABLED(CONFIG_MLEGO_BONGO_CAT) && CONFIG_DISP_HEIGHT>103
LV_IMAGE_DECLARE(elep);
#endif
static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

struct output_status_state {
    struct zmk_endpoint_instance selected_endpoint;
    int active_profile_index;
    bool active_profile_connected;
    bool active_profile_bonded;
    bool profiles_connected[MLEGO_PROFILE_COUNT];
    bool profiles_bonded[MLEGO_PROFILE_COUNT];
};

struct layer_status_state {
    zmk_keymap_layer_index_t index;
    const char *label;
};

#if IS_ENABLED(CONFIG_ZMK_WIDGET_WPM_STATUS)
struct wpm_status_state {
    uint8_t wpm;
};
#endif

#if !IS_ENABLED(CONFIG_MLEGO_BONGO_CAT) && CONFIG_DISP_HEIGHT>103
static void draw_image(lv_obj_t *widget) {
    lv_obj_t *canvas = lv_obj_get_child(widget, 3);
    if (canvas != NULL) {
        rotate_img_to_canvas(canvas, &elep);
    }
}
#endif

#if IS_ENABLED(CONFIG_ZMK_BLE)
static void draw_top(lv_obj_t *widget, const struct status_state *state) {
    lv_obj_t *canvas = lv_obj_get_child(widget, 0);
    if (canvas == NULL) {
        return;
    }

    lv_draw_label_dsc_t label_dsc;
    init_label_dsc(&label_dsc, LVGL_FOREGROUND, &lv_font_montserrat_18, LV_TEXT_ALIGN_RIGHT);
    lv_draw_label_dsc_t label_dsc_wpm;
    init_label_dsc(&label_dsc_wpm, LVGL_FOREGROUND, &lv_font_unscii_8, LV_TEXT_ALIGN_RIGHT);
    lv_draw_rect_dsc_t rect_black_dsc;
    init_rect_dsc(&rect_black_dsc, LVGL_BACKGROUND);
    lv_draw_rect_dsc_t rect_white_dsc;
    init_rect_dsc(&rect_white_dsc, LVGL_FOREGROUND);
    lv_draw_line_dsc_t line_dsc;
    init_line_dsc(&line_dsc, LVGL_FOREGROUND, 1);

    // Fill background
    lv_canvas_fill_bg(canvas, LVGL_BACKGROUND, LV_OPA_COVER);

    // Draw battery
    draw_battery(canvas, state);
    char percentage[5] = {};
    snprintf(percentage, sizeof(percentage), "%3u%%", state->battery);
    canvas_draw_text(canvas, 0, 20, CONFIG_DISP_CANVAS-18, &label_dsc, percentage);

    // Draw output status
    char output_text[10] = {};
    switch (state->selected_endpoint.transport) {
    case ZMK_TRANSPORT_USB:
        strcat(output_text, LV_SYMBOL_USB);
        break;
    case ZMK_TRANSPORT_BLE:
        if (state->active_profile_bonded) {
            if (state->active_profile_connected) {
                strcat(output_text, LV_SYMBOL_BLUETOOTH);
            } else {
                strcat(output_text, LV_SYMBOL_CLOSE);
            }
        } else {
            strcat(output_text, LV_SYMBOL_SETTINGS);
        }
        break;
    default:
        break;
    }

    canvas_draw_text(canvas, 0, 40, 20, &label_dsc, output_text);
#if IS_ENABLED(CONFIG_ZMK_WIDGET_WPM_STATUS)

    // Draw WPM
    canvas_draw_rect(canvas, 0, 21, 68, 42, &rect_white_dsc);
    canvas_draw_rect(canvas, 1, 22, 66, 40, &rect_black_dsc);

    char wpm_text[6] = {};
    snprintf(wpm_text, sizeof(wpm_text), "%d", state->wpm[9]);
    canvas_draw_text(canvas, 42, 52, 24, &label_dsc_wpm, wpm_text);

    int max = 0;
    int min = 256;

    for (int i = 0; i < 10; i++) {
        if (state->wpm[i] > max) {
            max = state->wpm[i];
        }
        if (state->wpm[i] < min) {
            min = state->wpm[i];
        }
    }

    int range = max - min;
    if (range == 0) {
        range = 1;
    }

    lv_point_t points[10];
    for (int i = 0; i < 10; i++) {
        points[i].x = 2 + i * 7;
        points[i].y = 60 - (state->wpm[i] - min) * 36 / range;
    }
    canvas_draw_line(canvas, points, 10, &line_dsc);
#endif

    // Rotate canvas
    rotate_canvas(canvas);
}
#endif


#if IS_ENABLED(CONFIG_ZMK_BLE)
static void draw_middle(lv_obj_t *widget, const struct status_state *state) {
    lv_obj_t *canvas = lv_obj_get_child(widget, 1);
    if (canvas == NULL) {
        return;
    }

    // Fill background
    lv_canvas_fill_bg(canvas, LVGL_BACKGROUND, LV_OPA_COVER);

    int profile_idx = state->active_profile_index;
    if (profile_idx < 0 || profile_idx >= MLEGO_PROFILE_COUNT) {
        profile_idx = 0;
    }

    bool connected = state->active_profile_connected;
    bool bonded = state->active_profile_bonded;

    // Draw active BT profile in a good size circle (centered at 32, 32)
    lv_draw_arc_dsc_t arc_dsc;
    init_arc_dsc(&arc_dsc, LVGL_FOREGROUND, connected ? 3 : 2);
    canvas_draw_arc(canvas, 32, 32, 22, 0, 360, &arc_dsc);

    if (connected) {
        // Outer concentric ring for connected state
        lv_draw_arc_dsc_t outer_dsc;
        init_arc_dsc(&outer_dsc, LVGL_FOREGROUND, 1);
        canvas_draw_arc(canvas, 32, 32, 26, 0, 360, &outer_dsc);
    } else if (!bonded) {
        // Inner ring for unbonded/open profile
        lv_draw_arc_dsc_t inner_dsc;
        init_arc_dsc(&inner_dsc, LVGL_FOREGROUND, 1);
        canvas_draw_arc(canvas, 32, 32, 18, 0, 360, &inner_dsc);
    }

    // Active profile number in the center of the circle
    lv_draw_label_dsc_t label_dsc;
    init_label_dsc(&label_dsc, LVGL_FOREGROUND, &lv_font_montserrat_18, LV_TEXT_ALIGN_CENTER);

    char label[4];
    snprintf(label, sizeof(label), "%d", profile_idx + 1);
    canvas_draw_text(canvas, 12, 23, 40, &label_dsc, label);

    // Rotate canvas
    rotate_canvas(canvas);
}
#endif


static void draw_bottom(lv_obj_t *widget, const struct status_state *state) {
    lv_obj_t *canvas = lv_obj_get_child(widget, 2);
    if (canvas == NULL) {
        return;
    }

    lv_draw_rect_dsc_t rect_black_dsc;
    init_rect_dsc(&rect_black_dsc, LVGL_BACKGROUND);
    lv_draw_label_dsc_t label_dsc;
    init_label_dsc(&label_dsc, LVGL_FOREGROUND, &lv_font_montserrat_18, LV_TEXT_ALIGN_CENTER);

    // Fill background
    lv_canvas_fill_bg(canvas, LVGL_BACKGROUND, LV_OPA_COVER);

    char keyb[10] = {};
    strcat(keyb, LV_SYMBOL_KEYBOARD);
    canvas_draw_text(canvas, 0, 0, 20, &label_dsc, keyb);

    // Draw layer
    if (state->layer_label == NULL) {
        char text[10] = {};
        sprintf(text, "%i", state->layer_index);
        canvas_draw_text(canvas, 0, 20, 20, &label_dsc, text);
    } else {
        canvas_draw_text(canvas, 0, 20, 20, &label_dsc, state->layer_label);
    }

    // Rotate canvas
    rotate_canvas(canvas);
}

#if IS_ENABLED(CONFIG_ZMK_BLE)
static void set_battery_status(struct zmk_widget_status *widget,
                               struct battery_status_state state) {
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
    widget->state.charging = state.usb_present;
#endif /* IS_ENABLED(CONFIG_USB_DEVICE_STACK) */
    widget->state.battery = state.level;

    draw_top(widget->obj, &widget->state);
}

static void battery_status_update_cb(struct battery_status_state state) {
    struct zmk_widget_status *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) { set_battery_status(widget, state); }
}

static struct battery_status_state battery_status_get_state(const zmk_event_t *eh) {
    const struct zmk_battery_state_changed *ev = as_zmk_battery_state_changed(eh);

    return (struct battery_status_state) {
        .level = (ev != NULL) ? ev->state_of_charge : zmk_battery_state_of_charge(),
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
        .usb_present = zmk_usb_is_powered(),
#endif /* IS_ENABLED(CONFIG_USB_DEVICE_STACK) */
    };
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_battery_status, struct battery_status_state,
                            battery_status_update_cb, battery_status_get_state)

ZMK_SUBSCRIPTION(widget_battery_status, zmk_battery_state_changed);
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
ZMK_SUBSCRIPTION(widget_battery_status, zmk_usb_conn_state_changed);
#endif /* IS_ENABLED(CONFIG_USB_DEVICE_STACK) */

static void set_output_status(struct zmk_widget_status *widget,
                              const struct output_status_state *state) {
    widget->state.selected_endpoint = state->selected_endpoint;
    widget->state.active_profile_index = state->active_profile_index;
    widget->state.active_profile_connected = state->active_profile_connected;
    widget->state.active_profile_bonded = state->active_profile_bonded;
    for (int i = 0; i < MLEGO_PROFILE_COUNT; ++i) {
        widget->state.profiles_connected[i] = state->profiles_connected[i];
        widget->state.profiles_bonded[i] = state->profiles_bonded[i];
    }

    draw_top(widget->obj, &widget->state);
    draw_middle(widget->obj, &widget->state);
}

static void output_status_update_cb(struct output_status_state state) {
    struct zmk_widget_status *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) { set_output_status(widget, &state); }
}

static struct output_status_state output_status_get_state(const zmk_event_t *_eh) {
    struct output_status_state state = {
        .selected_endpoint = zmk_endpoint_get_selected(),
        .active_profile_index = zmk_ble_active_profile_index(),
        .active_profile_connected = zmk_ble_active_profile_is_connected(),
        .active_profile_bonded = !zmk_ble_active_profile_is_open(),
    };
    for (int i = 0; i < MIN(MLEGO_PROFILE_COUNT, ZMK_BLE_PROFILE_COUNT); ++i) {
        state.profiles_connected[i] = zmk_ble_profile_is_connected(i);
        state.profiles_bonded[i] = !zmk_ble_profile_is_open(i);
    }
    return state;
}


ZMK_DISPLAY_WIDGET_LISTENER(widget_output_status, struct output_status_state,
                            output_status_update_cb, output_status_get_state)
ZMK_SUBSCRIPTION(widget_output_status, zmk_endpoint_changed);

#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
ZMK_SUBSCRIPTION(widget_output_status, zmk_usb_conn_state_changed);
#endif

#if defined(CONFIG_ZMK_BLE)
ZMK_SUBSCRIPTION(widget_output_status, zmk_ble_active_profile_changed);
#endif
#endif

static void set_layer_status(struct zmk_widget_status *widget, struct layer_status_state state) {
    widget->state.layer_index = state.index;
    widget->state.layer_label = state.label;

    draw_bottom(widget->obj, &widget->state);
}

static void layer_status_update_cb(struct layer_status_state state) {
    struct zmk_widget_status *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) { set_layer_status(widget, state); }
}

static struct layer_status_state layer_status_get_state(const zmk_event_t *eh) {
    zmk_keymap_layer_index_t index = zmk_keymap_highest_layer_active();
    return (struct layer_status_state){
        .index = index, .label = zmk_keymap_layer_name(zmk_keymap_layer_index_to_id(index))};
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_layer_status, struct layer_status_state, layer_status_update_cb,
                            layer_status_get_state)

ZMK_SUBSCRIPTION(widget_layer_status, zmk_layer_state_changed);

#if IS_ENABLED(CONFIG_ZMK_WIDGET_WPM_STATUS)
static void set_wpm_status(struct zmk_widget_status *widget, struct wpm_status_state state) {
    for (int i = 0; i < 9; i++) {
        widget->state.wpm[i] = widget->state.wpm[i + 1];
    }
    widget->state.wpm[9] = state.wpm;

    draw_top(widget->obj, &widget->state);
}

static void wpm_status_update_cb(struct wpm_status_state state) {
    struct zmk_widget_status *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) { set_wpm_status(widget, state); }
}

struct wpm_status_state wpm_status_get_state(const zmk_event_t *eh) {
    return (struct wpm_status_state){.wpm = zmk_wpm_get_state()};
};

ZMK_DISPLAY_WIDGET_LISTENER(widget_wpm_status, struct wpm_status_state, wpm_status_update_cb,
                            wpm_status_get_state)
ZMK_SUBSCRIPTION(widget_wpm_status, zmk_wpm_state_changed);
#endif

int zmk_widget_status_init(struct zmk_widget_status *widget, lv_obj_t *parent) {
    widget->obj = lv_obj_create(parent);
    lv_obj_set_size(widget->obj, CONFIG_DISP_WIDTH, CONFIG_DISP_HEIGHT);
    lv_obj_set_style_bg_color(widget->obj, LVGL_BACKGROUND, 0);
    lv_obj_set_style_bg_opa(widget->obj, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(widget->obj, 0, 0);
    lv_obj_set_style_pad_all(widget->obj, 0, 0);

    lv_obj_t *top = lv_canvas_create(widget->obj);
    lv_obj_set_size(top, CONFIG_DISP_CANVAS, CONFIG_DISP_CANVAS);
#if CONFIG_DISP_ROTATE == 900
    lv_obj_align(top, LV_ALIGN_TOP_RIGHT, 0, 0);
#elif CONFIG_DISP_ROTATE == 2700
    lv_obj_align(top, LV_ALIGN_BOTTOM_LEFT, 0, 0);
#elif CONFIG_DISP_ROTATE == 1800
    lv_obj_align(top, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
#endif
    lv_canvas_set_buffer(top, widget->cbuf1, CONFIG_DISP_CANVAS, CONFIG_DISP_CANVAS, CANVAS_COLOR_FORMAT);

    lv_obj_t *middle = lv_canvas_create(widget->obj);
    lv_obj_set_size(middle, CONFIG_DISP_CANVAS, CONFIG_DISP_CANVAS);
#if CONFIG_DISP_ROTATE == 900
    lv_obj_align(middle, LV_ALIGN_TOP_LEFT, CONFIG_DISP_CANVAS, CONFIG_DISP_CANVAS);
#elif CONFIG_DISP_ROTATE == 2700
    lv_obj_align(middle, LV_ALIGN_BOTTOM_LEFT, 0, -48);
#elif CONFIG_DISP_ROTATE == 1800
    lv_obj_align(middle, LV_ALIGN_BOTTOM_RIGHT, -CONFIG_DISP_CANVAS, 0);
#endif
    lv_canvas_set_buffer(middle, widget->cbuf2, CONFIG_DISP_CANVAS, CONFIG_DISP_CANVAS, CANVAS_COLOR_FORMAT);

    lv_obj_t *bottom = lv_canvas_create(widget->obj);
    lv_obj_set_size(bottom, CONFIG_DISP_CANVAS, CONFIG_DISP_CANVAS);
#if CONFIG_DISP_ROTATE == 900
    lv_obj_align(bottom, LV_ALIGN_BOTTOM_LEFT, CONFIG_DISP_CANVAS, 0);
#elif CONFIG_DISP_ROTATE == 1800
    lv_obj_align(bottom, LV_ALIGN_BOTTOM_RIGHT, -2 * CONFIG_DISP_CANVAS, 0);
#elif CONFIG_DISP_ROTATE == 2700
    lv_obj_align(bottom, LV_ALIGN_BOTTOM_LEFT, 0, -100);
#endif
    lv_canvas_set_buffer(bottom, widget->cbuf3, CONFIG_DISP_CANVAS, CONFIG_DISP_CANVAS, CANVAS_COLOR_FORMAT);

#if !IS_ENABLED(CONFIG_MLEGO_BONGO_CAT) && CONFIG_DISP_HEIGHT>103
    lv_obj_t *picture = lv_canvas_create(widget->obj);

#if CONFIG_DISP_ROTATE == 900 || CONFIG_DISP_ROTATE == 2700
    lv_obj_set_size(picture, elep.header.h, elep.header.w);
    lv_canvas_set_buffer(picture, widget->cbuf4, elep.header.h, elep.header.w, CANVAS_COLOR_FORMAT);
#if CONFIG_DISP_ROTATE == 900
    lv_obj_align(picture, LV_ALIGN_CENTER, -30, 0);
#else
    lv_obj_align(picture, LV_ALIGN_CENTER, 30, 0);
#endif
#elif CONFIG_DISP_ROTATE == 1800
    lv_obj_set_size(picture, elep.header.w, elep.header.h);
    lv_canvas_set_buffer(picture, widget->cbuf4, elep.header.w, elep.header.h, CANVAS_COLOR_FORMAT);
    lv_obj_align(picture, LV_ALIGN_CENTER, 0, -20);
#else
    lv_obj_set_size(picture, elep.header.w, elep.header.h);
    lv_canvas_set_buffer(picture, widget->cbuf4, elep.header.w, elep.header.h, CANVAS_COLOR_FORMAT);
    lv_obj_align(picture, LV_ALIGN_CENTER, 0, -20);
#endif

    draw_image(widget->obj);
#endif
    sys_slist_append(&widgets, &widget->node);
#if IS_ENABLED(CONFIG_ZMK_BLE)
    widget_battery_status_init();
    widget_output_status_init();
#endif
    widget_layer_status_init();
#if IS_ENABLED(CONFIG_ZMK_WIDGET_WPM_STATUS)
    widget_wpm_status_init();
#endif
    return 0;
}

lv_obj_t *zmk_widget_status_obj(struct zmk_widget_status *widget) { return widget->obj; }
 
void zmk_widget_status_refresh(struct zmk_widget_status *widget) {
    if (widget == NULL || widget->obj == NULL) {
        return;
    }
#if IS_ENABLED(CONFIG_ZMK_BLE)
    widget->state.battery = zmk_battery_state_of_charge();
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
    widget->state.charging = zmk_usb_is_powered();
#endif
    widget->state.selected_endpoint = zmk_endpoint_get_selected();
    widget->state.active_profile_index = zmk_ble_active_profile_index();
    widget->state.active_profile_connected = zmk_ble_active_profile_is_connected();
    widget->state.active_profile_bonded = !zmk_ble_active_profile_is_open();
    for (int i = 0; i < MIN(MLEGO_PROFILE_COUNT, ZMK_BLE_PROFILE_COUNT); ++i) {
        widget->state.profiles_connected[i] = zmk_ble_profile_is_connected(i);
        widget->state.profiles_bonded[i] = !zmk_ble_profile_is_open(i);
    }
    draw_middle(widget->obj, &widget->state);
    draw_top(widget->obj, &widget->state);
#endif
    zmk_keymap_layer_index_t index = zmk_keymap_highest_layer_active();
    widget->state.layer_index = index;
    widget->state.layer_label = zmk_keymap_layer_name(zmk_keymap_layer_index_to_id(index));
    draw_bottom(widget->obj, &widget->state);

#if !IS_ENABLED(CONFIG_MLEGO_BONGO_CAT) && CONFIG_DISP_HEIGHT>103
    draw_image(widget->obj);
#endif

}

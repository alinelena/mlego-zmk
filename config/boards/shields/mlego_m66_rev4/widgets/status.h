/*
 *
 * Copyright (c) 2023 The ZMK Contributors
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

#include <lvgl.h>
#include <zephyr/kernel.h>
#include "util.h"

struct zmk_widget_status {
    sys_snode_t node;
    lv_obj_t *obj;
    lv_color_t cbuf1[CONFIG_DISP_CANVAS * CONFIG_DISP_CANVAS];
    lv_color_t cbuf2[CONFIG_DISP_CANVAS * CONFIG_DISP_CANVAS];
    lv_color_t cbuf3[CONFIG_DISP_CANVAS * CONFIG_DISP_CANVAS];
#if !IS_ENABLED(CONFIG_MLEGO_BONGO_CAT) && CONFIG_DISP_HEIGHT>103
    lv_color_t cbuf4[40 * 128];
#endif
    struct status_state state;
};

int zmk_widget_status_init(struct zmk_widget_status *widget, lv_obj_t *parent);
lv_obj_t *zmk_widget_status_obj(struct zmk_widget_status *widget);

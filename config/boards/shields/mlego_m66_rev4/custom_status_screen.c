/*
 *
 * Copyright (c) 2023 The ZMK Contributors
 * SPDX-License-Identifier: MIT
 *
 */

#include "widgets/status.h"

#include "widgets/bongo_cat.h"

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

static struct zmk_widget_status status_widget;

static struct mlego_bongo_cat_widget bongo_widget;

lv_obj_t *zmk_display_status_screen() {

    lv_obj_t *screen;
    screen = lv_obj_create(NULL);

    zmk_widget_status_init(&status_widget, screen);
    lv_obj_align(zmk_widget_status_obj(&status_widget), LV_ALIGN_TOP_LEFT, 0, 0);

    lv_obj_t *bongo = lv_obj_create(screen);
    lv_obj_align(bongo, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_obj_center(bongo);

    mlego_bongo_cat_widget_init(&bongo_widget, bongo);
    return screen;
}

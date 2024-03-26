/*
 *
 * Copyright (c) 2023 The ZMK Contributors
 * SPDX-License-Identifier: MIT
 *
 */

#include "widgets/status.h"

#if IS_ENABLED(CONFIG_MLEGO_BONGO_CAT)
#include "widgets/bongo_cat.h"
#endif

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

static struct zmk_widget_status status_widget;

#if IS_ENABLED(CONFIG_MLEGO_BONGO_CAT)
static struct mlego_bongo_cat_widget bongo_widget;
#endif

lv_obj_t *zmk_display_status_screen() {

    lv_obj_t *screen;
    screen = lv_obj_create(NULL);

    zmk_widget_status_init(&status_widget, screen);
    lv_obj_align(zmk_widget_status_obj(&status_widget), LV_ALIGN_TOP_LEFT, 0, 0);

#if IS_ENABLED(CONFIG_MLEGO_BONGO_CAT)
    lv_obj_t *bongo = lv_obj_create(screen);
    mlego_bongo_cat_widget_init(&bongo_widget, bongo);
#endif

    return screen;
}

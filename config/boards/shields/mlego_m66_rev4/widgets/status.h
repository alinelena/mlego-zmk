#pragma once

#include <lvgl.h>
#include <zephyr/kernel.h>
#include "util.h"

#define ELEP_CANVAS_BUF_SIZE                                                                       \
    LV_CANVAS_BUF_SIZE(128, 40, LV_COLOR_FORMAT_GET_BPP(CANVAS_COLOR_FORMAT),                      \
                       LV_DRAW_BUF_STRIDE_ALIGN)

struct zmk_widget_status {
    sys_snode_t node;
    lv_obj_t *obj;
    uint8_t cbuf1[CANVAS_BUF_SIZE];
    uint8_t cbuf2[CANVAS_BUF_SIZE];
    uint8_t cbuf3[CANVAS_BUF_SIZE];
#if !IS_ENABLED(CONFIG_MLEGO_BONGO_CAT) && CONFIG_DISP_HEIGHT>103
    uint8_t cbuf4[ELEP_CANVAS_BUF_SIZE];
#endif
    struct status_state state;
};

int zmk_widget_status_init(struct zmk_widget_status *widget, lv_obj_t *parent);
lv_obj_t *zmk_widget_status_obj(struct zmk_widget_status *widget);

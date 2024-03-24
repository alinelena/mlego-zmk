/*
 * Copyright (c) 2021 Pete Johanson
 *
 * SPDX-License-Identifier: MIT
 */


#include <lvgl.h>

struct mlego_bongo_cat_widget {
    sys_snode_t node;
    lv_obj_t *obj;
    lv_anim_t anim;
};

int mlego_bongo_cat_widget_init(struct mlego_bongo_cat_widget *widget, lv_obj_t *parent);
lv_obj_t *mlego_bongo_cat_widget_obj(struct mlego_bongo_cat_widget *widget);

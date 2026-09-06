#include <zephyr/kernel.h>
#include <zephyr/version.h>
#include <zephyr/app_version.h>
#include <lvgl.h>
#include <zmk/display.h>
#include <zmk/event_manager.h>
#include <zmk/events/position_state_changed.h>

#include "widgets/status.h"

#if IS_ENABLED(CONFIG_MLEGO_BONGO_CAT)
#include "widgets/bongo_cat.h"
#endif

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#ifdef CONFIG_USB_DEVICE_PRODUCT
#define SPLASH_TITLE CONFIG_USB_DEVICE_PRODUCT
#else
#define SPLASH_TITLE "mlego m66"
#endif

#define SPLASH_TIMEOUT_MS 10000

#define SPLASH_W 120
#define SPLASH_H 64
#define SPLASH_BUF_SIZE                                                                                LV_CANVAS_BUF_SIZE(SPLASH_W, SPLASH_H, LV_COLOR_FORMAT_GET_BPP(CANVAS_COLOR_FORMAT),                                 LV_DRAW_BUF_STRIDE_ALIGN)

static uint8_t splash_buf_src[SPLASH_BUF_SIZE];
#if defined(CONFIG_DISP_ROTATE) && (CONFIG_DISP_ROTATE != 0)
static uint8_t splash_buf_dest[SPLASH_BUF_SIZE];
#endif

static struct zmk_widget_status status_widget;

#if IS_ENABLED(CONFIG_MLEGO_BONGO_CAT)
static struct mlego_bongo_cat_widget bongo_widget;
#endif

static lv_obj_t *status_screen;
static lv_obj_t *splash_screen;
static struct k_work_delayable splash_timeout_work;
static bool splash_active = false;

static void dismiss_splash_work_handler(struct k_work *work) {
    if (!splash_active) {
        return;
    }
    splash_active = false;
    if (status_screen != NULL) {
        lv_scr_load_anim(status_screen, LV_SCR_LOAD_ANIM_NONE, 0, 0, true);
        splash_screen = NULL;
    }
}

static void dismiss_splash(void) {
    if (splash_active) {
        k_work_cancel_delayable(&splash_timeout_work);
        k_work_submit_to_queue(zmk_display_work_q(), &splash_timeout_work.work);
    }
}

static int splash_position_listener(const zmk_event_t *eh) {
    const struct zmk_position_state_changed *ev = as_zmk_position_state_changed(eh);
    if (ev != NULL && ev->state && splash_active) {
        dismiss_splash();
    }
    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(splash_dismiss, splash_position_listener);
ZMK_SUBSCRIPTION(splash_dismiss, zmk_position_state_changed);

lv_obj_t *zmk_display_status_screen() {
    status_screen = lv_obj_create(NULL);
    zmk_widget_status_init(&status_widget, status_screen);
    lv_obj_align(zmk_widget_status_obj(&status_widget), LV_ALIGN_TOP_LEFT, 0, 0);

#if IS_ENABLED(CONFIG_MLEGO_BONGO_CAT)
    lv_obj_t *bongo = lv_obj_create(status_screen);
    mlego_bongo_cat_widget_init(&bongo_widget, bongo);
#endif

    splash_screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(splash_screen, LVGL_BACKGROUND, 0);
    lv_obj_set_style_bg_opa(splash_screen, LV_OPA_COVER, 0);

    lv_obj_t *splash_canvas = lv_canvas_create(splash_screen);
    lv_canvas_set_buffer(splash_canvas, splash_buf_src, SPLASH_W, SPLASH_H, CANVAS_COLOR_FORMAT);
    lv_canvas_fill_bg(splash_canvas, LVGL_BACKGROUND, LV_OPA_COVER);

    lv_draw_label_dsc_t label_dsc;
    init_label_dsc(&label_dsc, LVGL_FOREGROUND, &lv_font_montserrat_12, LV_TEXT_ALIGN_CENTER);
    canvas_draw_text(splash_canvas, 0, 8, SPLASH_W, &label_dsc,
                     SPLASH_TITLE "\nZephyr: " KERNEL_VERSION_STRING "\nZMK: " APP_VERSION_STRING);

#if CONFIG_DISP_ROTATE == 900
    const uint32_t src_stride = lv_draw_buf_width_to_stride(SPLASH_W, CANVAS_COLOR_FORMAT);
    const uint32_t dest_stride = lv_draw_buf_width_to_stride(SPLASH_H, CANVAS_COLOR_FORMAT);
    lv_draw_sw_rotate(splash_buf_src, splash_buf_dest, SPLASH_W, SPLASH_H, src_stride, dest_stride,
                      LV_DISPLAY_ROTATION_270, CANVAS_COLOR_FORMAT);
    lv_canvas_set_buffer(splash_canvas, splash_buf_dest, SPLASH_H, SPLASH_W, CANVAS_COLOR_FORMAT);
#elif CONFIG_DISP_ROTATE == 2700
    const uint32_t src_stride = lv_draw_buf_width_to_stride(SPLASH_W, CANVAS_COLOR_FORMAT);
    const uint32_t dest_stride = lv_draw_buf_width_to_stride(SPLASH_H, CANVAS_COLOR_FORMAT);
    lv_draw_sw_rotate(splash_buf_src, splash_buf_dest, SPLASH_W, SPLASH_H, src_stride, dest_stride,
                      LV_DISPLAY_ROTATION_90, CANVAS_COLOR_FORMAT);
    lv_canvas_set_buffer(splash_canvas, splash_buf_dest, SPLASH_H, SPLASH_W, CANVAS_COLOR_FORMAT);
#elif CONFIG_DISP_ROTATE == 1800
    const uint32_t stride = lv_draw_buf_width_to_stride(SPLASH_W, CANVAS_COLOR_FORMAT);
    lv_draw_sw_rotate(splash_buf_src, splash_buf_dest, SPLASH_W, SPLASH_H, stride, stride,
                      LV_DISPLAY_ROTATION_180, CANVAS_COLOR_FORMAT);
    lv_canvas_set_buffer(splash_canvas, splash_buf_dest, SPLASH_W, SPLASH_H, CANVAS_COLOR_FORMAT);
#endif

    lv_obj_center(splash_canvas);

    splash_active = true;
    k_work_init_delayable(&splash_timeout_work, dismiss_splash_work_handler);
    k_work_schedule_for_queue(zmk_display_work_q(), &splash_timeout_work, K_MSEC(SPLASH_TIMEOUT_MS));

    return splash_screen;
}

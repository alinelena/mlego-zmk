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
    lv_obj_t *label = lv_label_create(splash_screen);
    lv_label_set_text(label, SPLASH_TITLE "\nZephyr: " KERNEL_VERSION_STRING "\nZMK: " APP_VERSION_STRING);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);

#if defined(CONFIG_DISP_ROTATE) && (CONFIG_DISP_ROTATE != 0)
    lv_obj_set_style_transform_rotation(label, CONFIG_DISP_ROTATE, 0);
#endif

    lv_obj_center(label);

    splash_active = true;
    k_work_init_delayable(&splash_timeout_work, dismiss_splash_work_handler);
    k_work_schedule_for_queue(zmk_display_work_q(), &splash_timeout_work, K_MSEC(SPLASH_TIMEOUT_MS));

    return splash_screen;
}


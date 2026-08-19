/*
 * Philarmony Filament Dryer ESP32 Firmware
 * Copyright (C) 2026 Philarmony Contributors
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "LvglPort.hpp"

#include <new>

#include "../drivers/touch/TouchManager.hpp"

#if !defined(UNIT_TEST) && defined(PHILARMONY_HAS_LVGL)
#include <lvgl.h>
#define PHILARMONY_LVGL_ENABLED 1
#elif !defined(UNIT_TEST) && defined(__has_include)
#if __has_include(<lvgl.h>)
#include <lvgl.h>
#define PHILARMONY_LVGL_ENABLED 1
#endif
#endif

#if defined(PHILARMONY_LVGL_ENABLED)
#if defined(LVGL_VERSION_MAJOR)
#define PHILARMONY_LVGL_MAJOR LVGL_VERSION_MAJOR
#else
#define PHILARMONY_LVGL_MAJOR LV_VERSION_MAJOR
#endif
#endif

namespace filament_dryer {

#if defined(PHILARMONY_LVGL_ENABLED) && PHILARMONY_LVGL_MAJOR >= 9

struct LvglPort::Impl {
    explicit Impl(LvglPort* port) : owner(port) {}

    LvglPort* owner;
    lv_display_t* display = nullptr;
    lv_indev_t* input = nullptr;
    uint8_t* draw_buffer = nullptr;
    size_t draw_buffer_size = 0;

    static void flushCallback(lv_display_t* display, const lv_area_t* area,
                              uint8_t* pixels) {
        Impl* self = static_cast<Impl*>(lv_display_get_user_data(display));
        if (self && self->owner) {
            self->owner->flush(area->x1, area->y1, area->x2, area->y2,
                               reinterpret_cast<const uint16_t*>(pixels));
        }
        lv_display_flush_ready(display);
    }

    static void inputCallback(lv_indev_t* input, lv_indev_data_t* data) {
        Impl* self = static_cast<Impl*>(lv_indev_get_user_data(input));
        const TouchPoint point =
            (self && self->owner) ? self->owner->readInput() : TouchPoint{};
        data->state = point.pressed ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
        data->point.x = point.x;
        data->point.y = point.y;
    }
};

#elif defined(PHILARMONY_LVGL_ENABLED)

struct LvglPort::Impl {
    explicit Impl(LvglPort* port) : owner(port) {}

    LvglPort* owner;
    lv_disp_draw_buf_t draw_buffer;
    lv_disp_drv_t display_driver;
    lv_indev_drv_t input_driver;
    lv_color_t* pixels = nullptr;
    lv_disp_t* display = nullptr;
    lv_indev_t* input = nullptr;

    static void flushCallback(lv_disp_drv_t* driver, const lv_area_t* area,
                              lv_color_t* pixels) {
        Impl* self = static_cast<Impl*>(driver->user_data);
        if (self && self->owner) {
            self->owner->flush(area->x1, area->y1, area->x2, area->y2,
                               reinterpret_cast<const uint16_t*>(pixels));
        }
        lv_disp_flush_ready(driver);
    }

    static void inputCallback(lv_indev_drv_t* driver, lv_indev_data_t* data) {
        Impl* self = static_cast<Impl*>(driver->user_data);
        const TouchPoint point =
            (self && self->owner) ? self->owner->readInput() : TouchPoint{};
        data->state = point.pressed ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;
        data->point.x = point.x;
        data->point.y = point.y;
    }
};

#else

struct LvglPort::Impl {
    explicit Impl(LvglPort* port) : owner(port) {}
    LvglPort* owner;
};

#endif

LvglPort::~LvglPort() {
    deinit();
}

bool LvglPort::init(TouchManager& touch_manager, uint16_t width, uint16_t height,
                    FlushCallback flush_callback, void* context) {
    deinit();
    if (width == 0 || height == 0) {
        return false;
    }

    touch_manager_ = &touch_manager;
    flush_callback_ = flush_callback;
    callback_context_ = context;
    last_touch_ms_ = millis();
    last_touch_point_ = TouchPoint{};
    impl_ = new (std::nothrow) Impl(this);
    if (!impl_) {
        return false;
    }

#if defined(PHILARMONY_LVGL_ENABLED)
    static bool lvgl_initialized = false;
    if (!lvgl_initialized) {
        lv_init();
        lvgl_initialized = true;
    }
    const uint16_t rows = height < 10 ? height : 10;
#if PHILARMONY_LVGL_MAJOR >= 9
    const size_t bytes_per_pixel = (LV_COLOR_DEPTH + 7) / 8;
    impl_->draw_buffer_size =
        static_cast<size_t>(width) * rows * bytes_per_pixel;
    impl_->draw_buffer = new (std::nothrow) uint8_t[impl_->draw_buffer_size];
    if (!impl_->draw_buffer) {
        deinit();
        return false;
    }

    impl_->display = lv_display_create(width, height);
    lv_display_set_user_data(impl_->display, impl_);
    lv_display_set_flush_cb(impl_->display, Impl::flushCallback);
    lv_display_set_buffers(impl_->display, impl_->draw_buffer, nullptr,
                           impl_->draw_buffer_size, LV_DISPLAY_RENDER_MODE_PARTIAL);

    impl_->input = lv_indev_create();
    lv_indev_set_type(impl_->input, LV_INDEV_TYPE_POINTER);
    lv_indev_set_user_data(impl_->input, impl_);
    lv_indev_set_read_cb(impl_->input, Impl::inputCallback);
#else
    impl_->pixels =
        new (std::nothrow) lv_color_t[static_cast<size_t>(width) * rows];
    if (!impl_->pixels) {
        deinit();
        return false;
    }

    lv_disp_draw_buf_init(&impl_->draw_buffer, impl_->pixels, nullptr,
                          static_cast<uint32_t>(width) * rows);
    lv_disp_drv_init(&impl_->display_driver);
    impl_->display_driver.hor_res = width;
    impl_->display_driver.ver_res = height;
    impl_->display_driver.draw_buf = &impl_->draw_buffer;
    impl_->display_driver.flush_cb = Impl::flushCallback;
    impl_->display_driver.user_data = impl_;
    impl_->display = lv_disp_drv_register(&impl_->display_driver);

    lv_indev_drv_init(&impl_->input_driver);
    impl_->input_driver.type = LV_INDEV_TYPE_POINTER;
    impl_->input_driver.read_cb = Impl::inputCallback;
    impl_->input_driver.user_data = impl_;
    impl_->input = lv_indev_drv_register(&impl_->input_driver);
#endif
#endif

    initialized_ = true;
    return true;
}

void LvglPort::deinit() {
#if defined(PHILARMONY_LVGL_ENABLED)
    if (impl_) {
#if PHILARMONY_LVGL_MAJOR >= 9
        if (impl_->input) {
            lv_indev_delete(impl_->input);
        }
        if (impl_->display) {
            lv_display_delete(impl_->display);
        }
        delete[] impl_->draw_buffer;
#else
        if (impl_->input) {
            lv_indev_delete(impl_->input);
        }
        if (impl_->display) {
            lv_disp_remove(impl_->display);
        }
        delete[] impl_->pixels;
#endif
    }
#endif
    delete impl_;
    impl_ = nullptr;
    touch_manager_ = nullptr;
    flush_callback_ = nullptr;
    callback_context_ = nullptr;
    initialized_ = false;
    last_touch_ms_ = 0;
    last_touch_point_ = TouchPoint{};
}

void LvglPort::tick() {
#if defined(PHILARMONY_LVGL_ENABLED)
    if (initialized_) {
        lv_timer_handler();
    }
#endif
}

void LvglPort::flush(int16_t x1, int16_t y1, int16_t x2, int16_t y2,
                     const uint16_t* pixels) {
    if (flush_callback_) {
        flush_callback_(x1, y1, x2, y2, pixels, callback_context_);
    }
}

TouchPoint LvglPort::readInput() {
    TouchPoint point = touch_manager_ ? touch_manager_->read() : TouchPoint{};
    last_touch_point_ = point;
    if (point.pressed) {
        last_touch_ms_ = point.timestamp_ms ? point.timestamp_ms : millis();
    }
    return point;
}

} // namespace filament_dryer

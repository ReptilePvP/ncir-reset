#pragma once

#include <lvgl.h>

// Original CoreS3 adaptation of the DECK//OS NETRUNNER visual palette.
// Reference: https://github.com/hey-its-brian/tab5-cyberdeck
namespace ncir_ui {
constexpr uint32_t background = 0x07070D;
constexpr uint32_t panel = 0x0D0F1C;
constexpr uint32_t line = 0x2A3050;
constexpr uint32_t text = 0xD8E1F0;
// Brighter than the reference dim text for legibility on the small CoreS3 LCD.
constexpr uint32_t muted = 0x929DB8;
constexpr uint32_t cyan = 0x00F0FF;
constexpr uint32_t magenta = 0xFF2A6D;
constexpr uint32_t yellow = 0xF3E600;
constexpr uint32_t green = 0x39FF14;
constexpr uint32_t red = 0xFF3B3B;

inline void style_page(lv_obj_t* page) {
  lv_obj_set_style_bg_color(page, lv_color_hex(background), 0);
  lv_obj_set_style_bg_opa(page, LV_OPA_COVER, 0);
  lv_obj_set_style_text_color(page, lv_color_hex(text), 0);
  lv_obj_set_style_border_width(page, 0, 0);
  lv_obj_set_style_radius(page, 0, 0);
  lv_obj_set_style_pad_all(page, 0, 0);
}

inline void style_tabs(lv_obj_t* view) {
  lv_obj_set_style_bg_color(view, lv_color_hex(background), 0);
  lv_obj_t* bar = lv_tabview_get_tab_bar(view);
  lv_obj_set_style_bg_color(bar, lv_color_hex(panel), 0);
  lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, 0);
  // LVGL 9.3 tab bars contain real buttons, rather than button-matrix items.
  for (uint32_t i = 0; i < lv_obj_get_child_count(bar); ++i) {
    lv_obj_t* button = lv_obj_get_child(bar, i);
    lv_obj_set_style_text_color(button, lv_color_hex(muted), 0);
    lv_obj_set_style_bg_color(button, lv_color_hex(panel), 0);
    lv_obj_set_style_radius(button, 0, 0);
    lv_obj_set_style_shadow_width(button, 0, 0);
    lv_obj_set_style_pad_all(button, 0, 0);
    lv_obj_set_style_border_color(button, lv_color_hex(line), 0);
    lv_obj_set_style_border_width(button, 1, 0);
    lv_obj_set_style_text_color(button, lv_color_hex(cyan), LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x171A30), LV_STATE_CHECKED);
    lv_obj_set_style_border_color(button, lv_color_hex(magenta), LV_STATE_CHECKED);
  }
}
}  // namespace ncir_ui

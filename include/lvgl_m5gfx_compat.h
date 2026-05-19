#pragma once

#ifdef __cplusplus
#include <lvgl/lvgl.h>

#if LVGL_VERSION_MAJOR == 9 && LVGL_VERSION_MINOR <= 3
enum {
  LV_FONT_GLYPH_FORMAT_A1_ALIGNED = 0x011,
  LV_FONT_GLYPH_FORMAT_A2_ALIGNED = 0x012,
  LV_FONT_GLYPH_FORMAT_A4_ALIGNED = 0x014
};
#endif
#endif

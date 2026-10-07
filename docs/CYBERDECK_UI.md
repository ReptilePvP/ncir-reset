# NCIR//OS visual adaptation

The CoreS3 UI takes visual inspiration from [DECK//OS](https://github.com/hey-its-brian/tab5-cyberdeck), inspected on 2026-10-07: near-black surfaces, cyan primary accents, magenta active-tab borders, flat outlined panels, and a persistent header. This is an original small-screen adaptation; no upstream application code, fonts, or assets are bundled.

The palette and shared page/tab styling live in `include/ncir_ui_theme.h`. Dim text is brighter than the reference for readability on the 320 × 240 LCD. Built-in Montserrat fonts keep the firmware independent of external font files.

All six NCIR tabs retain their existing order and commands. The Live page retains the temperature, zone colors, two fan controls, asynchronous request notices, Wi-Fi state, and emissivity. Battery information in the header still comes from the existing power readings. Alert highlighting and lift fault messages retain their meaning. Settings remains scrollable to reach its footer.

The Live temperature panel also includes [embedded Lottie temperature animations](../assets/lottie/README.md): snowflake for COLD, check for GOOD, and flame for TOO HOT. The temperature and bar have dedicated space to the right of the icon.

The adaptation does not add Cyberdeck's SSH, music, portal, OTA, keyboard integration, boot animation, or CRT effects. Those are separate features; full-screen effects could obscure readings and increase redraw costs.

## Device review

After uploading, check all six tabs with the joystick, confirm the active tab has a cyan label and magenta border, and confirm the header and temperature remain readable. Scroll Settings to its footer. Exercise both fan selections and request notices, units, sliders, and alert highlighting. Check lift messages with the existing hardware precautions in [NCIR_LIFT.md](NCIR_LIFT.md).

A firmware build establishes compilation only. Display appearance, touch/joystick interaction, webhook service behavior, and physical lift behavior require device checks.

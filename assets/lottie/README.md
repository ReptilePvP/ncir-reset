# Temperature Lottie assets

The three temperature animations are original NCIR assets in standard Lottie JSON format. The two fan animations are distinct LottieFiles community assets, embedded as part of this firmware under the [Lottie Simple License](https://lottiefiles.com/page/license).

## Fan animation credits and behavior

- `fan.json`: [Rotating Fan by Mudit Khandelwal](https://lottiefiles.com/free-animation/rotating-fan-rl8QVBYjhQ). Three-second rotating-blade loop.
- `smoke_fan.json`: [Wind by Jesse Sohn](https://lottiefiles.com/free-animation/wind-FPAGvbFHgf). A separate 0.75-second airflow loop.

Both assets are rendered at 28 × 28 beside their existing labels. ON plays the relevant loop, OFF freezes and dims it, and unknown state remains frozen/dim with `--`. Each state is still inferred locally after its successful webhook response; this is not authoritative smart-plug or motor feedback. Leaving Live pauses both animations. No webhook is sent by loading or playing an animation.

The fan buffers together use 6,272 bytes. The Arduino loop task retains its 32 KiB stack to accommodate ThorVG's nested parsing/rendering calls. The one-time 30-second serial checkpoint reports minimum remaining stack and free heap after startup.

| File | Calibrated object temperature | Visual |
| --- | --- | --- |
| cold.json | Below 500°F (260°C) | Cyan pulsing snowflake |
| good.json | 500–610°F inclusive (260–321.1°C) | Green pulsing check |
| hot.json | Above 610°F (321.1°C) | Red pulsing flame |

Each temperature asset has a transparent 48 × 48 canvas and a two-second loop (120 frames at 60 fps). Simple paths and strokes keep rendering small. The temperature widget has one shared 9,216-byte ARGB buffer and loads JSON only when the temperature zone changes. It pauses/hides the widget away from Live or when the reading is invalid. Sleep already stops LVGL processing.

To substitute a LottieFiles export, download its **Lottie JSON** with permission under its license, simplify it to a 48 × 48 canvas and a 120-frame/60-fps loop, and replace the relevant JSON here. Avoid external images, fonts, expressions, and complex effects; support depends on the bundled ThorVG renderer. Run `python tools/embed_lottie.py`, then build. This embeds the JSON in firmware; no filesystem or network fetch is used. A `.lottie` ZIP container is not a JSON replacement.

Playback uses LVGL 9.3's native Lottie widget and internal ThorVG renderer. Build validation does not establish CoreS3 frame rate or responsiveness. On-device review should check all three ranges, threshold crossings, unit changes, unavailable readings, switching away from Live, sleep/wake, and fan controls while the icon plays. Use simulated readings or an appropriate controlled setup to review ranges.

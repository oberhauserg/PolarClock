#pragma once

#include "pcmath.h"

namespace polarclock {

struct RingColor {
    math::Vec3 base;       // Base color at arc start
    math::Vec3 bright;     // Brighter color at arc end
};

struct Theme {
    const char* name;
    math::Vec3 background;
    RingColor seconds;
    RingColor minutes;
    RingColor hours;
    RingColor dayOfMonth;
    RingColor month;
    math::Vec3 textColor;
};

// Default vibrant theme
inline Theme createDefaultTheme() {
    Theme theme;
    theme.name = "Vibrant";
    theme.background = math::Vec3(0.04f, 0.04f, 0.06f);  // #0A0A0F

    // Seconds: Cyan
    theme.seconds.base = math::Vec3(0.0f, 0.6f, 0.8f);
    theme.seconds.bright = math::Vec3(0.0f, 0.83f, 1.0f);  // #00D4FF

    // Minutes: Blue
    theme.minutes.base = math::Vec3(0.15f, 0.3f, 0.7f);
    theme.minutes.bright = math::Vec3(0.255f, 0.412f, 0.882f);  // #4169E1

    // Hours: Purple
    theme.hours.base = math::Vec3(0.4f, 0.3f, 0.7f);
    theme.hours.bright = math::Vec3(0.576f, 0.439f, 0.859f);  // #9370DB

    // Day of Month: Orange
    theme.dayOfMonth.base = math::Vec3(0.8f, 0.4f, 0.0f);
    theme.dayOfMonth.bright = math::Vec3(1.0f, 0.55f, 0.0f);  // #FF8C00

    // Month: Red/Coral
    theme.month.base = math::Vec3(0.8f, 0.2f, 0.25f);
    theme.month.bright = math::Vec3(1.0f, 0.278f, 0.341f);  // #FF4757

    theme.textColor = math::Vec3(1.0f, 1.0f, 1.0f);

    return theme;
}

// Cool icy blue theme
// Palette: #09479A, #6190BA, #E8ECEF, #A0C1D0, #788D9E
// Colors animate from bright (light) at 0 to base (dark) at 1
inline Theme createCoolTheme() {
    Theme theme;
    theme.name = "Ice";
    theme.background = math::Vec3(0.910f, 0.922f, 0.937f);  // #E8ECEF light gray

    // Deep blue #09479A (8, 70, 154)
    math::Vec3 deepBlue(0.031f, 0.275f, 0.604f);
    // Medium blue #6190BA (96, 144, 186)
    math::Vec3 mediumBlue(0.376f, 0.565f, 0.729f);
    // Light blue #A0C1D0 (160, 193, 207)
    math::Vec3 lightBlue(0.627f, 0.757f, 0.812f);
    // Gray-blue #788D9E (119, 140, 158)
    math::Vec3 grayBlue(0.467f, 0.549f, 0.620f);

    // Assign colors: bright (start/reset) -> base (end/full)
    theme.seconds.bright = lightBlue;
    theme.seconds.base = deepBlue;

    theme.minutes.bright = lightBlue;
    theme.minutes.base = mediumBlue;

    theme.hours.bright = lightBlue;
    theme.hours.base = grayBlue;

    theme.dayOfMonth.bright = lightBlue;
    theme.dayOfMonth.base = mediumBlue;

    theme.month.bright = lightBlue;
    theme.month.base = deepBlue;

    theme.textColor = math::Vec3(0.910f, 0.922f, 0.937f);  // Light for contrast on dark arcs

    return theme;
}

inline Theme createBlueTheme() {
    Theme theme;
    theme.name = "Blue";
    theme.background = math::Vec3(0.04f, 0.04f, 0.06f);  // #0A0A0F

    // Deep blue #09479A (8, 70, 154)
    math::Vec3 deepBlue(0.031f, 0.275f, 0.604f);
    // Medium blue #6190BA (96, 144, 186)
    math::Vec3 mediumBlue(0.376f, 0.565f, 0.729f);

    // Assign colors: bright (start/reset) -> base (end/full)
    theme.seconds.bright = mediumBlue;
    theme.seconds.base = deepBlue;

    theme.minutes.bright = mediumBlue;
    theme.minutes.base = deepBlue;

    theme.hours.bright = mediumBlue;
    theme.hours.base = deepBlue;

    theme.dayOfMonth.bright = mediumBlue;
    theme.dayOfMonth.base = deepBlue;

    theme.month.bright = mediumBlue;
    theme.month.base = deepBlue;

    theme.textColor = math::Vec3(0.910f, 0.922f, 0.937f);  // Light for contrast on dark arcs

    return theme;
}

inline Theme createPurpleTheme() {
    Theme theme;
    theme.name = "Blue";
    theme.background = math::Vec3(0.04f, 0.04f, 0.06f);  // #0A0A0F

    // #CC5FEA (204, 95, 234)
    math::Vec3 pink(204.0/256.0, 95.0/256.0, 234.0/256.0);
    // #B41DDE (180, 29, 22)
    math::Vec3 purple(180.0/256.0, 29.0/256.0, 22.0/256.0);

    // Assign colors: bright (start/reset) -> base (end/full)
    theme.seconds.bright = pink;
    theme.seconds.base = purple;

    theme.minutes.bright = pink;
    theme.minutes.base = purple;

    theme.hours.bright = pink;
    theme.hours.base = purple;

    theme.dayOfMonth.bright = pink;
    theme.dayOfMonth.base = purple;

    theme.month.bright = pink;
    theme.month.base = purple;

    theme.textColor = math::Vec3(0.910f, 0.922f, 0.937f);  // Light for contrast on dark arcs

    return theme;
}


} // namespace polarclock

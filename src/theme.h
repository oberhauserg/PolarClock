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

// Cool blue theme
inline Theme createCoolTheme() {
    Theme theme;
    theme.name = "Cool";
    theme.background = math::Vec3(0.02f, 0.05f, 0.1f);

    float saturation = 0.7f;
    float value = 0.9f;

    theme.seconds.base = math::hsvToRgb(0.55f, saturation * 0.6f, value * 0.7f);
    theme.seconds.bright = math::hsvToRgb(0.55f, saturation, value);

    theme.minutes.base = math::hsvToRgb(0.6f, saturation * 0.6f, value * 0.7f);
    theme.minutes.bright = math::hsvToRgb(0.6f, saturation, value);

    theme.hours.base = math::hsvToRgb(0.65f, saturation * 0.6f, value * 0.7f);
    theme.hours.bright = math::hsvToRgb(0.65f, saturation, value);

    theme.dayOfMonth.base = math::hsvToRgb(0.5f, saturation * 0.6f, value * 0.7f);
    theme.dayOfMonth.bright = math::hsvToRgb(0.5f, saturation, value);

    theme.month.base = math::hsvToRgb(0.45f, saturation * 0.6f, value * 0.7f);
    theme.month.bright = math::hsvToRgb(0.45f, saturation, value);

    theme.textColor = math::Vec3(0.9f, 0.95f, 1.0f);

    return theme;
}

} // namespace polarclock

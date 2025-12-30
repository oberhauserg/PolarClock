#pragma once

#include "pcmath.h"
#include "theme.h"
#include <string>
#include <array>

namespace polarclock {

enum class RingType {
    Seconds = 0,
    Minutes,
    Hours,
    DayOfMonth,
    Month,
    COUNT
};

struct Ring {
    RingType type;
    float currentValue;     // Current animated value (0.0 - 1.0)
    float targetValue;      // Target value to animate towards
    float innerRadius;
    float outerRadius;
    std::string label;
    std::string valueText;
    RingColor colors;
};

class PolarClock {
public:
    PolarClock();

    void update(float deltaTime);
    void setTheme(const Theme& theme);

    const Ring& getRing(RingType type) const { return m_rings[static_cast<size_t>(type)]; }
    const std::array<Ring, 5>& getRings() const { return m_rings; }

    // Get current time values for display
    int getSeconds() const { return m_seconds; }
    int getMinutes() const { return m_minutes; }
    int getHours() const { return m_hours; }
    int getDayOfMonth() const { return m_dayOfMonth; }
    int getMonth() const { return m_month; }
    int getYear() const { return m_year; }

private:
    void updateTime();
    void updateRingValues();
    float animateValue(float current, float target, float deltaTime);

    std::array<Ring, 5> m_rings;

    // Current time values
    int m_seconds;
    int m_minutes;
    int m_hours;
    int m_dayOfMonth;
    int m_month;
    int m_year;
    float m_fractionalSecond;  // For smooth second hand

    // Animation speed
    float m_animationSpeed;
};

} // namespace polarclock

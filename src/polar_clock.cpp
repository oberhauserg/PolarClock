#include "polar_clock.h"
#include <ctime>
#include <chrono>
#include <cmath>

namespace polarclock {

PolarClock::PolarClock()
    : m_seconds(0)
    , m_minutes(0)
    , m_hours(0)
    , m_dayOfMonth(1)
    , m_month(1)
    , m_year(2024)
    , m_fractionalSecond(0)
    , m_animationSpeed(8.0f)
{
    // Configure rings from inner to outer
    float baseRadius = 0.15f;
    float ringWidth = 0.08f;
    float gap = 0.02f;

    for (int i = 0; i < 5; ++i) {
        m_rings[i].innerRadius = baseRadius + i * (ringWidth + gap);
        m_rings[i].outerRadius = m_rings[i].innerRadius + ringWidth;
        m_rings[i].currentValue = 0.0f;
        m_rings[i].targetValue = 0.0f;
    }

    m_rings[0].type = RingType::Seconds;
    m_rings[0].label = "SECONDS";
    m_rings[1].type = RingType::Minutes;
    m_rings[1].label = "MINUTES";
    m_rings[2].type = RingType::Hours;
    m_rings[2].label = "HOURS";
    m_rings[3].type = RingType::DayOfMonth;
    m_rings[3].label = "DAY";
    m_rings[4].type = RingType::Month;
    m_rings[4].label = "MONTH";

    // Set default theme
    setTheme(createDefaultTheme());

    // Initialize time
    updateTime();
    updateRingValues();

    // Start with current values (no animation on load)
    for (auto& ring : m_rings) {
        ring.currentValue = ring.targetValue;
    }
}

void PolarClock::setTheme(const Theme& theme) {
    m_rings[0].colors = theme.seconds;
    m_rings[1].colors = theme.minutes;
    m_rings[2].colors = theme.hours;
    m_rings[3].colors = theme.dayOfMonth;
    m_rings[4].colors = theme.month;
}

void PolarClock::update(float deltaTime) {
    updateTime();
    updateRingValues();

    // Animate each ring
    for (auto& ring : m_rings) {
        ring.currentValue = animateValue(ring.currentValue, ring.targetValue, deltaTime);
    }
}

void PolarClock::updateTime() {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    auto local_time = std::localtime(&time_t_now);

    // Get fractional seconds for smooth animation
    auto duration = now.time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    m_fractionalSecond = (millis % 1000) / 1000.0f;

    m_seconds = local_time->tm_sec;
    m_minutes = local_time->tm_min;
    m_hours = local_time->tm_hour;
    m_dayOfMonth = local_time->tm_mday;
    m_month = local_time->tm_mon + 1;  // tm_mon is 0-11
    m_year = local_time->tm_year + 1900;
}

void PolarClock::updateRingValues() {
    // Helper for zero-padded numbers
    auto pad2 = [](int n) {
        return (n < 10 ? "0" : "") + std::to_string(n);
    };

    // Seconds: 0-59, include fractional for smoothness
    float secondsValue = (m_seconds + m_fractionalSecond) / 60.0f;
    m_rings[0].targetValue = secondsValue;
    m_rings[0].valueText = pad2(m_seconds) + " seconds";

    // Minutes: 0-59
    float minutesValue = (m_minutes + m_seconds / 60.0f) / 60.0f;
    m_rings[1].targetValue = minutesValue;
    m_rings[1].valueText = pad2(m_minutes) + " minutes";

    // Hours: 0-23 (24-hour clock)
    float hoursValue = (m_hours + m_minutes / 60.0f) / 24.0f;
    m_rings[2].targetValue = hoursValue;
    m_rings[2].valueText = pad2(m_hours) + " hours";

    // Day of month: 1-31 (varies by month, but we use 31 for consistency)
    float daysInMonth = 31.0f;  // Could calculate actual days, but 31 works visually
    float dayValue = m_dayOfMonth / daysInMonth;
    m_rings[3].targetValue = dayValue;
    m_rings[3].valueText = pad2(m_dayOfMonth) + " days";

    // Month: 1-12
    float monthValue = m_month / 12.0f;
    m_rings[4].targetValue = monthValue;

    // Month names
    static const char* monthNames[] = {
        "January", "February", "March", "April", "May", "June",
        "July", "August", "September", "October", "November", "December"
    };
    m_rings[4].valueText = monthNames[m_month - 1];
}

float PolarClock::animateValue(float current, float target, float deltaTime) {
    float diff = target - current;

    // If the difference is very large negative (like going from 0.98 to 0.01),
    // we're wrapping around - sweep backwards quickly to reset
    if (diff < -0.5f) {
        // Animate backward quickly (sweep back to 0)
        float resetSpeed = m_animationSpeed * 1.0f;  // Faster for reset
        current -= resetSpeed * deltaTime;
        if (current < 0.0f) {
            current = 0.0f;
        }
    } else if (diff > 0.5f) {
        // Rare case: animate forward through 1.0
        current += m_animationSpeed * deltaTime;
        if (current >= 1.0f) {
            current -= 1.0f;
        }
    } else {
        // Normal interpolation
        float speed = m_animationSpeed * deltaTime;
        if (std::abs(diff) < speed) {
            current = target;
        } else {
            current += (diff > 0 ? speed : -speed);
        }
    }

    return current;
}

} // namespace polarclock

#include "polar_clock.h"
#include <ctime>
#include <chrono>
#include <cmath>

namespace polarclock {

static int getDaysInMonth(int month, int year) {
    static const int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month == 2) {
        bool isLeap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
        return isLeap ? 29 : 28;
    }
    return days[month - 1];
}

PolarClock::PolarClock()
    : m_seconds(0)
    , m_minutes(0)
    , m_hours(0)
    , m_dayOfMonth(1)
    , m_month(1)
    , m_year(2024)
    , m_fractionalSecond(0)
    , m_animationSpeed(2.0f)
{
    // Configure rings from inner to outer
    float baseRadius = 0.15f;
    float ringWidth = 0.08f;
    float gap = 0.01f;

    for (int i = 0; i < 5; ++i) {
        m_rings[i].innerRadius = baseRadius + i * (ringWidth + gap);
        m_rings[i].outerRadius = m_rings[i].innerRadius + ringWidth;
        m_rings[i].currentValue = 0.0f;
        m_rings[i].targetValue = 0.0f;
    }

    m_rings[0].type = RingType::Month;
    m_rings[0].label = "MONTH";
    m_rings[1].type = RingType::DayOfMonth;
    m_rings[1].label = "DAY";
    m_rings[2].type = RingType::Hours;
    m_rings[2].label = "HOURS";
    m_rings[3].type = RingType::Minutes;
    m_rings[3].label = "MINUTES";
    m_rings[4].type = RingType::Seconds;
    m_rings[4].label = "SECONDS";

    // Set default theme
    setTheme(createDefaultTheme());

    // Initialize time and target values
    // currentValue stays at 0 so rings animate in on startup
    updateTime();
    updateRingValues();
}

void PolarClock::setTheme(const Theme& theme) {
    m_theme = theme;
    m_rings[0].colors = theme.seconds;
    m_rings[1].colors = theme.minutes;
    m_rings[2].colors = theme.hours;
    m_rings[3].colors = theme.dayOfMonth;
    m_rings[4].colors = theme.month;
}

void PolarClock::setValue(RingType type, float value, int text_value)
{
    auto pad2 = [](int n) {
        return (n < 10 ? "0" : "") + std::to_string(n);
    };

    for(auto& ring: m_rings)
    {
        if(ring.type == type)
        {
            ring.targetValue = value;

            switch(type)
            {
                case RingType::Hours:
                    ring.valueText = pad2(text_value) + " hours";
                    break;
                case RingType::Minutes:
                    ring.valueText = pad2(text_value) + " minutes";
                    break;
                case RingType::Seconds:
                    ring.valueText = pad2(text_value) + " seconds";
                    break;
                case RingType::DayOfMonth:
                    ring.valueText = pad2(text_value) + " days";
                    break;
                case RingType::Month:
                    static const char* monthNames[] = {
                        "January", "February", "March", "April", "May", "June",
                        "July", "August", "September", "October", "November", "December"
                    };
                    ring.valueText = monthNames[text_value - 1];
            }
        }
    }
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


    // Each ring cascades from the previous one, so all rings move continuously.
    // secondsValue represents fraction of minute elapsed (0-1)
    // minutesValue represents fraction of hour elapsed (0-1), including seconds contribution
    // And so on...

    // Seconds: 0-59, include fractional for smoothness
    float secondsValue = (m_seconds + m_fractionalSecond) / 60.0f;
    setValue(RingType::Seconds, secondsValue, m_seconds);

    // Minutes: cascade from seconds
    float minutesValue = (m_minutes + secondsValue) / 60.0f;
    setValue(RingType::Minutes, minutesValue, m_minutes);

    // Hours: cascade from minutes (24-hour clock)
    float hoursValue = (m_hours + minutesValue) / 24.0f;
    setValue(RingType::Hours, hoursValue, m_hours);

    // Day of month: cascade from hours, use actual days in current month
    int daysInMonth = getDaysInMonth(m_month, m_year);
    float dayValue = ((m_dayOfMonth - 1) + hoursValue) / static_cast<float>(daysInMonth);
    setValue(RingType::DayOfMonth, dayValue, m_dayOfMonth);
    
    // Month: cascade from days (1-12)
    float monthValue = ((m_month - 1) + dayValue) / 12.0f;
    setValue(RingType::Month, monthValue, m_month);
}

float PolarClock::animateValue(float current, float target, float deltaTime) {
    float diff = target - current;
    float speed = m_animationSpeed * deltaTime;

    // Simple linear interpolation - always move toward target
    if (std::abs(diff) <= speed) {
        return target;
    }

    return current + (diff > 0 ? speed : -speed);
}

} // namespace polarclock

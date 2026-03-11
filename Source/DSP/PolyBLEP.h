#pragma once
#include <cmath>

namespace mono101 {

inline float polyBlep(float t, float dt)
{
    if (t < dt) {
        t /= dt;
        return t + t - t * t - 1.0f;
    }
    if (t > 1.0f - dt) {
        t = (t - 1.0f) / dt;
        return t * t + t + t + 1.0f;
    }
    return 0.0f;
}

inline float blSaw(float phase, float dt)
{
    return (2.0f * phase - 1.0f) - polyBlep(phase, dt);
}

inline float blPulse(float phase, float dt, float pw)
{
    float p1 = 2.0f * phase - 1.0f;
    float phase2 = std::fmod(phase + (1.0f - pw), 1.0f);
    float p2 = 2.0f * phase2 - 1.0f;
    p1 -= polyBlep(phase, dt);
    p2 -= polyBlep(phase2, dt);
    return 0.5f * (p1 - p2);
}

} // namespace mono101

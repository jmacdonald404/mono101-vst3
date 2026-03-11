#pragma once
#include <cmath>

namespace mono101 {

class Portamento
{
public:
    void prepare(float sampleRate) { sr = sampleRate; }

    void setTime(float t)
    {
        if (t < 0.001f) {
            coeff = 1.0f;
            active = false;
        } else {
            coeff = 1.0f - std::exp(-1.0f / (t * sr));
            active = true;
        }
    }

    void setTarget(float f) { target = f; }

    float process()
    {
        if (!active) { current = target; return current; }
        float lc = std::log(current);
        float lt = std::log(target);
        current = std::exp(lc + coeff * (lt - lc));
        return current;
    }

private:
    float sr = 44100.0f;
    float current = 440.0f;
    float target = 440.0f;
    float coeff = 1.0f;
    bool active = false;
};

} // namespace mono101

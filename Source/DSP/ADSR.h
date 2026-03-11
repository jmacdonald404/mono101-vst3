#pragma once
#include <cmath>
#include <algorithm>

namespace mono101 {

class ADSR
{
public:
    enum Stage { Idle, Attack, Decay, Sustain, Release };

    void prepare(float sampleRate) { sr = sampleRate; }

    void retrigger() { stage = Attack; }

    float process(float gate, float a, float d, float s, float r)
    {
        const bool hi = gate > 0.5f;
        const bool ph = prevGate > 0.5f;
        if (hi && !ph) stage = Attack;
        if (!hi && ph) stage = Release;
        prevGate = gate;

        switch (stage) {
            case Attack:
                value += coeff(a) * (1.001f - value);
                if (value >= 1.0f) { value = 1.0f; stage = Decay; }
                break;
            case Decay:
                value += coeff(d) * (s - value);
                if (std::abs(value - s) < 0.0001f) { value = s; stage = Sustain; }
                break;
            case Sustain:
                value = s;
                break;
            case Release:
                value += coeff(r) * (0.00001f - value);
                if (value < 0.0001f) { value = 0.0f; stage = Idle; }
                break;
            default:
                value = 0.0f;
        }
        return std::max(0.0f, value);
    }

    Stage getStage() const { return stage; }

private:
    float coeff(float t) const
    {
        return 1.0f - std::exp(-1.0f / (std::max(t, 0.0005f) * sr));
    }

    float sr = 44100.0f;
    float value = 0.0f;
    float prevGate = 0.0f;
    Stage stage = Idle;
};

} // namespace mono101

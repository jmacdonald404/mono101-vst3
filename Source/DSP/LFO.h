#pragma once
#include <cmath>
#include <cstdlib>

namespace mono101 {

enum class LFOShape { Sine, Triangle, Square, Saw, SampleAndHold };

class LFO
{
public:
    void prepare(float sampleRate) { sr = sampleRate; }

    float process(float rate, LFOShape shape)
    {
        phase += rate / sr;
        if (phase >= 1.0f) {
            phase -= 1.0f;
            if (shape == LFOShape::SampleAndHold)
                shValue = (float)rand() / (float)RAND_MAX * 2.0f - 1.0f;
        }

        switch (shape) {
            case LFOShape::Sine:            return std::sin(2.0f * 3.14159265f * phase);
            case LFOShape::Triangle:        return phase < 0.5f ? 4.0f * phase - 1.0f : 3.0f - 4.0f * phase;
            case LFOShape::Square:          return phase < 0.5f ? 1.0f : -1.0f;
            case LFOShape::Saw:             return 2.0f * phase - 1.0f;
            case LFOShape::SampleAndHold:   return shValue;
        }
        return 0.0f;
    }

    float getPhase() const { return phase; }

private:
    float sr = 44100.0f;
    float phase = 0.0f;
    float shValue = 0.0f;
};

} // namespace mono101

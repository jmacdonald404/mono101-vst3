#pragma once
#include <cmath>
#include <algorithm>
#include <cstdlib>

namespace mono101 {

inline float tanhApprox(float x)
{
    if (x >  4.0f) return  1.0f;
    if (x < -4.0f) return -1.0f;
    float x2 = x * x;
    return x * (27.0f + x2) / (27.0f + 9.0f * x2);
}

static constexpr float THERMAL = 0.5f;

// 15-tap half-band FIR for 2x decimation
class Decimator
{
public:
    Decimator()
    {
        static constexpr float HB_RAW[] = {
            -0.00176508f, 0.0f, 0.01902222f, 0.0f, -0.11605382f, 0.0f,
             0.59679058f, 1.0f,
             0.59679058f, 0.0f, -0.11605382f, 0.0f, 0.01902222f, 0.0f, -0.00176508f
        };
        float sum = 0.0f;
        for (int i = 0; i < HBL; ++i) sum += HB_RAW[i];
        for (int i = 0; i < HBL; ++i) hbc[i] = HB_RAW[i] / sum;
        for (int i = 0; i < HBL; ++i) buf[i] = 0.0f;
    }

    float process(float s0, float s1)
    {
        push(s0);
        push(s1);
        return read();
    }

private:
    static constexpr int HBL = 15;
    float hbc[HBL]{};
    float buf[HBL]{};
    int idx = 0;

    void push(float v)
    {
        buf[idx] = v;
        idx = (idx + 1) % HBL;
    }

    float read() const
    {
        float o = 0.0f;
        for (int j = 0; j < HBL; ++j)
            o += hbc[j] * buf[(idx + j) % HBL];
        return o;
    }
};

class LadderFilter
{
public:
    void prepare(float sampleRate)
    {
        sr = sampleRate;
        osr = sampleRate * 2.0f;
        reset();
    }

    void reset()
    {
        for (auto& v : s) v = 0.0f;
    }

    float process(float input, float cutoff, float res)
    {
        float f = 2.0f * std::tan(3.14159265f * std::min(cutoff, osr * 0.45f) / osr);
        float k = res * 4.0f;
        float s0 = step(input, f, k);
        float s1 = step(input, f, k);
        return dec.process(s0, s1);
    }

private:
    float step(float input, float f, float k)
    {
        // predictor
        float fb = s[3];
        float inp = input - k * fb;
        float t[4];
        t[0] = s[0] + f * (tanhApprox(inp    / (2.0f * THERMAL)) - tanhApprox(s[0] / (2.0f * THERMAL)));
        t[1] = s[1] + f * (tanhApprox(t[0]   / (2.0f * THERMAL)) - tanhApprox(s[1] / (2.0f * THERMAL)));
        t[2] = s[2] + f * (tanhApprox(t[1]   / (2.0f * THERMAL)) - tanhApprox(s[2] / (2.0f * THERMAL)));
        t[3] = s[3] + f * (tanhApprox(t[2]   / (2.0f * THERMAL)) - tanhApprox(s[3] / (2.0f * THERMAL)));
        // corrector
        inp = input - k * 0.5f * (t[3] + s[3]);
        s[0] += f * (tanhApprox(inp    / (2.0f * THERMAL)) - tanhApprox(s[0] / (2.0f * THERMAL)));
        s[1] += f * (tanhApprox(s[0]   / (2.0f * THERMAL)) - tanhApprox(s[1] / (2.0f * THERMAL)));
        s[2] += f * (tanhApprox(s[1]   / (2.0f * THERMAL)) - tanhApprox(s[2] / (2.0f * THERMAL)));
        s[3] += f * (tanhApprox(s[2]   / (2.0f * THERMAL)) - tanhApprox(s[3] / (2.0f * THERMAL)));
        // noise dither
        s[3] += ((float)rand() / (float)RAND_MAX * 2.0f - 1.0f) * 1e-6f;
        return s[3];
    }

    float sr = 44100.0f;
    float osr = 88200.0f;
    float s[4] = {};
    Decimator dec;
};

} // namespace mono101

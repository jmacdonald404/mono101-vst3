#pragma once
#include "PolyBLEP.h"
#include "ADSR.h"
#include "LFO.h"
#include "Portamento.h"
#include "LadderFilter.h"
#include <cmath>
#include <algorithm>
#include <cstdlib>

namespace mono101 {

enum class PWMode  { LFO, Manual, Env };
enum class SubMode { PW25_2Oct, Square_2Oct, Square_1Oct };
enum class EnvMode { LFODriven, Gate, GateTrig };
enum class VCAMode { Env, Gate };
enum class PortaMode { On, Off, Auto };

struct VoiceParams
{
    // oscillator mix
    float sawLevel   = 1.0f;
    float pulseLevel = 0.0f;
    float subLevel   = 0.0f;
    float noiseLevel = 0.0f;

    // filter
    float cutoff     = 2000.0f;
    float resonance  = 0.1f;
    float envModAmt  = 0.5f;
    float keyTracking = 0.5f;

    // LFO modulation amounts
    float lfoPitchAmt  = 0.0f;
    float lfoCutoffAmt = 0.0f;
    float lfoPWMAmt    = 0.0f;
    float lfoRate      = 2.0f;

    // envelope
    float attack  = 0.005f;
    float decay   = 0.2f;
    float sustain = 0.7f;
    float release = 0.3f;

    // output
    float volume   = 0.7f;
    float velocity = 1.0f;

    // transpose
    float octaveShift = 0.0f;
    float fineTune    = 0.0f;

    // pulse width
    float pulseWidth = 0.5f;

    // modes
    LFOShape  lfoShape = LFOShape::Triangle;
    PWMode    pwMode   = PWMode::Manual;
    SubMode   subMode  = SubMode::Square_2Oct;
    EnvMode   envMode  = EnvMode::Gate;
    VCAMode   vcaMode  = VCAMode::Env;
};

class Mono101Voice
{
public:
    void prepare(float sampleRate)
    {
        sr = sampleRate;
        adsr.prepare(sampleRate);
        lfo.prepare(sampleRate);
        porta.prepare(sampleRate);
        filter.prepare(sampleRate);
    }

    void noteOn(float frequency, float vel)
    {
        noteFreq = frequency;
        porta.setTarget(frequency);
        gate = 1.0f;
        velocityCur = vel;
    }

    void noteOff()
    {
        gate = 0.0f;
    }

    void retrigger()
    {
        adsr.retrigger();
    }

    void setPortamentoTime(float t)
    {
        porta.setTime(t);
    }

    void setNoteTarget(float f)
    {
        noteFreq = f;
        porta.setTarget(f);
    }

    bool isActive() const
    {
        return gate > 0.5f || adsr.getStage() != ADSR::Idle;
    }

    float processSample(const VoiceParams& p)
    {
        float transposeRatio = std::pow(2.0f, p.octaveShift + p.fineTune / 1200.0f);

        // drift (slow random walk, CEM3340 tempco simulation)
        driftPhase += 1.0f / (sr * 4.0f);
        if (driftPhase >= 1.0f) {
            driftPhase -= 1.0f;
            driftTarget = ((float)rand() / (float)RAND_MAX * 2.0f - 1.0f) * 0.002f;
        }
        driftSmooth += (driftTarget - driftSmooth) * 0.00005f;

        // portamento
        porta.setTarget(noteFreq);
        float slewedFreq = porta.process() * (1.0f + driftSmooth) * transposeRatio;

        // LFO
        float lfoOut = lfo.process(p.lfoRate, p.lfoShape);

        // ADSR — compute before VCO so envOut available for PW env mode
        float effectiveGate = gate;
        if (p.envMode == EnvMode::LFODriven) {
            effectiveGate = lfo.getPhase() < 0.5f ? 1.0f : 0.0f;
        }
        float envOut = adsr.process(effectiveGate, p.attack, p.decay, p.sustain, p.release);

        // VCO
        float pitchMod = lfoOut * p.lfoPitchAmt;
        float finalFreq = slewedFreq * std::pow(2.0f, pitchMod / 12.0f);
        float dt = finalFreq / sr;

        // pulse width modulation
        float modPW;
        switch (p.pwMode) {
            case PWMode::LFO:
                modPW = std::clamp(p.pulseWidth + lfoOut * p.lfoPWMAmt * 0.4f, 0.01f, 0.99f);
                break;
            case PWMode::Env:
                modPW = std::clamp(envOut, 0.01f, 0.99f);
                break;
            default: // Manual
                modPW = std::clamp(p.pulseWidth, 0.01f, 0.99f);
                break;
        }

        // main oscillator phase
        oscPhase += dt;
        if (oscPhase >= 1.0f) oscPhase -= 1.0f;

        float saw   = blSaw(oscPhase, dt);
        float pulse = blPulse(oscPhase, dt, modPW);

        // sub oscillator
        subPhase1 = std::fmod(subPhase1 + dt * 0.5f, 1.0f);
        subPhase2 = std::fmod(subPhase2 + dt * 0.25f, 1.0f);
        float sub;
        switch (p.subMode) {
            case SubMode::Square_1Oct:  sub = subPhase1 < 0.5f ? 1.0f : -1.0f; break;
            case SubMode::Square_2Oct:  sub = subPhase2 < 0.5f ? 1.0f : -1.0f; break;
            default:                    sub = subPhase2 < 0.25f ? 1.0f : -1.0f; break; // PW25_2Oct
        }

        float noise = (float)rand() / (float)RAND_MAX * 2.0f - 1.0f;

        // mix and normalize
        float norm = std::max(p.sawLevel + p.pulseLevel + p.subLevel + p.noiseLevel, 1.0f);
        float osc = (saw * p.sawLevel + pulse * p.pulseLevel + sub * p.subLevel + noise * p.noiseLevel) / norm;

        // VCF
        float keyOffset  = p.cutoff * (std::pow(noteFreq / 440.0f, p.keyTracking) - 1.0f);
        float envBoost   = envOut * p.envModAmt * (20000.0f - p.cutoff);
        float lfoBoost   = lfoOut * p.lfoCutoffAmt * 10000.0f;
        float finalCutoff = std::clamp(p.cutoff + envBoost + keyOffset + lfoBoost, 10.0f, 20000.0f);
        float filtered = filter.process(osc, finalCutoff, p.resonance);

        // VCA
        gateSmooth += gateSlewCoeff * (gate - gateSmooth);
        float vcaEnv = (p.vcaMode == VCAMode::Gate) ? gateSmooth : envOut;
        float vcaLevel = vcaEnv * p.volume * velocityCur;

        return filtered * vcaLevel;
    }

private:
    float sr = 44100.0f;
    float gate = 0.0f;
    float noteFreq = 440.0f;
    float velocityCur = 1.0f;

    // oscillator state
    float oscPhase = 0.0f;
    float subPhase1 = 0.0f;
    float subPhase2 = 0.0f;

    // drift
    float driftPhase = 0.0f;
    float driftTarget = 0.0f;
    float driftSmooth = 0.0f;

    // gate slew (2ms ramp for click-free gate mode)
    float gateSmooth = 0.0f;
    float gateSlewCoeff = 1.0f - std::exp(-1.0f / (0.002f * 44100.0f));

    // DSP components
    ADSR adsr;
    LFO lfo;
    Portamento porta;
    LadderFilter filter;
};

} // namespace mono101

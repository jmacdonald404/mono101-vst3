#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_core/juce_core.h>
#include <cmath>
#include <map>
#include <vector>

namespace mono101
{

// ---------------------------------------------------------------------------
// Raw patch data — mirrors the web version's JSON format.
// All slider values are stored as raw 0–100 integers (or small ints for
// switches), exactly as the web UI stores them.
// ---------------------------------------------------------------------------
struct RawPatchSettings
{
    int fineTune    = 0;   // -50..50
    int lfoRate     = 25;  // 0..100
    int lfoWaveform = 1;   // 0=sine 1=triangle 2=square 3=saw 4=sh
    int lfoPitch    = 0;   // 0..100
    int octaveShift = 0;   // -5..5
    int pulseWidth  = 50;  // 0..100
    int pwMode      = 1;   // web: 0=env 1=man 2=lfo
    int lfoPWM      = 0;   // 0..100
    int pulseLevel  = 0;   // 0..100
    int sawLevel    = 100; // 0..100
    int subLevel    = 0;   // 0..100
    int subMode     = 1;   // 0..2
    int noiseLevel  = 0;   // 0..100
    int cutoff      = 50;  // 0..100
    int resonance   = 10;  // 0..100
    int envMod      = 50;  // 0..100
    int lfoCutoff   = 0;   // 0..100
    int keyTrack    = 50;  // 0..100
    int vcaMode     = 0;   // 0=env 1=gate
    int envMode     = 1;   // 0=LFO 1=Gate 2=Gate+Trig
    int attack      = 5;   // 0..100
    int decay       = 20;  // 0..100
    int sustain     = 70;  // 0..100
    int release     = 30;  // 0..100
    int volume      = 70;  // 0..100
    int glide       = 0;   // 0..100
    int portaMode   = 1;   // 0=On 1=Off 2=Auto
};

struct Patch
{
    juce::String name;
    RawPatchSettings raw;
};

// ---------------------------------------------------------------------------
// Conversion helpers — raw slider values  <->  actual VST3 parameter values
// ---------------------------------------------------------------------------
namespace Convert
{
    // Web: lfoRate slider 0–100 → lfoHz = 0.05 * 600^(v/100)
    // VST3 lfoRate range 0.01–30 Hz
    inline float lfoRateFromRaw (int raw)  { return 0.05f * std::pow (600.0f, raw / 100.0f); }

    // Inverse: lfoRate Hz → raw 0–100
    inline int   lfoRateToRaw (float hz)   { return juce::roundToInt (100.0f * std::log (hz / 0.05f) / std::log (600.0f)); }

    // Web: cutoff slider 0–100 → 10 * 2000^(v/100)
    inline float cutoffFromRaw (int raw)   { return 10.0f * std::pow (2000.0f, raw / 100.0f); }
    inline int   cutoffToRaw (float hz)    { return juce::roundToInt (100.0f * std::log (hz / 10.0f) / std::log (2000.0f)); }

    // Web: attack slider 0–100 → 0.0015 * 2667^(v/100)
    inline float attackFromRaw (int raw)   { return 0.0015f * std::pow (2667.0f, raw / 100.0f); }
    inline int   attackToRaw (float s)     { return juce::roundToInt (100.0f * std::log (s / 0.0015f) / std::log (2667.0f)); }

    // Web: decay slider 0–100 → 0.002 * 5000^(v/100)
    inline float decayFromRaw (int raw)    { return 0.002f * std::pow (5000.0f, raw / 100.0f); }
    inline int   decayToRaw (float s)      { return juce::roundToInt (100.0f * std::log (s / 0.002f) / std::log (5000.0f)); }

    // Web: release slider 0–100 → 0.002 * 5000^(v/100)
    inline float releaseFromRaw (int raw)  { return 0.002f * std::pow (5000.0f, raw / 100.0f); }
    inline int   releaseToRaw (float s)    { return juce::roundToInt (100.0f * std::log (s / 0.002f) / std::log (5000.0f)); }

    // Web: glide slider 0–100 → raw < 1 ? 0 : 0.01 * 150^(raw/100)
    inline float glideFromRaw (int raw)    { return raw < 1 ? 0.0f : 0.01f * std::pow (150.0f, raw / 100.0f); }
    inline int   glideToRaw (float s)      { return s <= 0.0f ? 0 : juce::roundToInt (100.0f * std::log (s / 0.01f) / std::log (150.0f)); }

    // Web: pulseWidth slider 1–100 → max(0.01, raw/200)
    inline float pulseWidthFromRaw (int raw) { return std::max (0.01f, raw / 200.0f); }
    inline int   pulseWidthToRaw (float pw)  { return juce::roundToInt (pw * 200.0f); }

    // Linear 0–100 → 0.0–1.0
    inline float normFromRaw (int raw)     { return raw / 100.0f; }
    inline int   normToRaw (float v)       { return juce::roundToInt (v * 100.0f); }

    // lfoPitch: raw 0–100 → 0–12 semitones
    inline float lfoPitchFromRaw (int raw) { return raw / 100.0f * 12.0f; }
    inline int   lfoPitchToRaw (float st)  { return juce::roundToInt (st / 12.0f * 100.0f); }

    // Web pwMode mapping: web 0=env → VST3 2=Env, web 1=man → VST3 1=Manual, web 2=lfo → VST3 0=LFO
    inline int   pwModeWebToVST (int web)
    {
        switch (web) {
            case 0: return 2;  // env
            case 1: return 1;  // man
            case 2: return 0;  // lfo
            default: return 1;
        }
    }
    inline int   pwModeVSTToWeb (int vst)
    {
        switch (vst) {
            case 0: return 2;  // LFO
            case 1: return 1;  // Manual
            case 2: return 0;  // Env
            default: return 1;
        }
    }

    // lfoWaveform: string → index (same order in web and VST3)
    inline int lfoWaveformFromString (const juce::String& s)
    {
        if (s == "sine")     return 0;
        if (s == "triangle") return 1;
        if (s == "square")   return 2;
        if (s == "saw")      return 3;
        if (s == "sh")       return 4;
        return 1; // default triangle
    }

    inline juce::String lfoWaveformToString (int idx)
    {
        switch (idx) {
            case 0: return "sine";
            case 1: return "triangle";
            case 2: return "square";
            case 3: return "saw";
            case 4: return "sh";
            default: return "triangle";
        }
    }
}

// ---------------------------------------------------------------------------
// PatchBank — manages factory + custom patches, import/export as JSON
// ---------------------------------------------------------------------------
class PatchBank
{
public:
    PatchBank()
    {
        loadFactoryPatches();
    }

    // ----- Access -----------------------------------------------------------

    const std::vector<Patch>& getFactoryPatches() const { return factoryPatches; }
    const std::vector<Patch>& getCustomPatches()  const { return customPatches; }

    std::vector<juce::String> getFactoryNames() const
    {
        std::vector<juce::String> names;
        for (auto& p : factoryPatches) names.push_back (p.name);
        return names;
    }

    std::vector<juce::String> getCustomNames() const
    {
        std::vector<juce::String> names;
        for (auto& p : customPatches) names.push_back (p.name);
        return names;
    }

    std::vector<juce::String> getAllNames() const
    {
        auto names = getFactoryNames();
        auto custom = getCustomNames();
        names.insert (names.end(), custom.begin(), custom.end());
        return names;
    }

    // ----- Load / Save ------------------------------------------------------

    /** Find a patch by name (searches custom first, then factory). */
    const Patch* findPatch (const juce::String& name) const
    {
        for (auto& p : customPatches)
            if (p.name == name) return &p;
        for (auto& p : factoryPatches)
            if (p.name == name) return &p;
        return nullptr;
    }

    /** Save a custom patch (overwrites if name exists). */
    void savePatch (const Patch& patch)
    {
        for (auto& p : customPatches)
        {
            if (p.name == patch.name)
            {
                p = patch;
                return;
            }
        }
        customPatches.push_back (patch);
    }

    /** Delete a custom patch by name. Returns true if found and deleted. */
    bool deletePatch (const juce::String& name)
    {
        auto it = std::remove_if (customPatches.begin(), customPatches.end(),
                                  [&](const Patch& p) { return p.name == name; });
        if (it == customPatches.end()) return false;
        customPatches.erase (it, customPatches.end());
        return true;
    }

    // ----- APVTS integration ------------------------------------------------

    /** Apply a patch's raw settings to an AudioProcessorValueTreeState. */
    static void applyToAPVTS (const Patch& patch, juce::AudioProcessorValueTreeState& apvts)
    {
        const auto& r = patch.raw;

        setParam (apvts, "fineTune",     (float) r.fineTune);
        setParam (apvts, "lfoRate",      Convert::lfoRateFromRaw (r.lfoRate));
        setParam (apvts, "lfoShape",     (float) r.lfoWaveform);
        setParam (apvts, "lfoPitchAmt",  Convert::lfoPitchFromRaw (r.lfoPitch));
        setParam (apvts, "octaveShift",  (float) r.octaveShift);
        setParam (apvts, "pulseWidth",   Convert::pulseWidthFromRaw (r.pulseWidth));
        setParam (apvts, "pwMode",       (float) Convert::pwModeWebToVST (r.pwMode));
        setParam (apvts, "lfoPWMAmt",    Convert::normFromRaw (r.lfoPWM));
        setParam (apvts, "pulseLevel",   Convert::normFromRaw (r.pulseLevel));
        setParam (apvts, "sawLevel",     Convert::normFromRaw (r.sawLevel));
        setParam (apvts, "subLevel",     Convert::normFromRaw (r.subLevel));
        setParam (apvts, "subMode",      (float) r.subMode);
        setParam (apvts, "noiseLevel",   Convert::normFromRaw (r.noiseLevel));
        setParam (apvts, "cutoff",       Convert::cutoffFromRaw (r.cutoff));
        setParam (apvts, "resonance",    Convert::normFromRaw (r.resonance));
        setParam (apvts, "envModAmt",    Convert::normFromRaw (r.envMod));
        setParam (apvts, "lfoCutoffAmt", Convert::normFromRaw (r.lfoCutoff));
        setParam (apvts, "keyTracking",  Convert::normFromRaw (r.keyTrack));
        setParam (apvts, "vcaMode",      (float) r.vcaMode);
        setParam (apvts, "envMode",      (float) r.envMode);
        setParam (apvts, "attack",       Convert::attackFromRaw (r.attack));
        setParam (apvts, "decay",        Convert::decayFromRaw (r.decay));
        setParam (apvts, "sustain",      Convert::normFromRaw (r.sustain));
        setParam (apvts, "release",      Convert::releaseFromRaw (r.release));
        setParam (apvts, "volume",       Convert::normFromRaw (r.volume));
        setParam (apvts, "glideTime",    Convert::glideFromRaw (r.glide));
        setParam (apvts, "portaMode",    (float) r.portaMode);
    }

    /** Capture current APVTS state as a Patch with raw slider values. */
    static Patch captureFromAPVTS (const juce::String& name,
                                   juce::AudioProcessorValueTreeState& apvts)
    {
        Patch patch;
        patch.name = name;
        auto& r = patch.raw;

        auto gv = [&](const juce::String& id) { return apvts.getRawParameterValue(id)->load(); };

        r.fineTune    = juce::roundToInt (gv ("fineTune"));
        r.lfoRate     = Convert::lfoRateToRaw (gv ("lfoRate"));
        r.lfoWaveform = juce::roundToInt (gv ("lfoShape"));
        r.lfoPitch    = Convert::lfoPitchToRaw (gv ("lfoPitchAmt"));
        r.octaveShift = juce::roundToInt (gv ("octaveShift"));
        r.pulseWidth  = Convert::pulseWidthToRaw (gv ("pulseWidth"));
        r.pwMode      = Convert::pwModeVSTToWeb (juce::roundToInt (gv ("pwMode")));
        r.lfoPWM      = Convert::normToRaw (gv ("lfoPWMAmt"));
        r.pulseLevel  = Convert::normToRaw (gv ("pulseLevel"));
        r.sawLevel    = Convert::normToRaw (gv ("sawLevel"));
        r.subLevel    = Convert::normToRaw (gv ("subLevel"));
        r.subMode     = juce::roundToInt (gv ("subMode"));
        r.noiseLevel  = Convert::normToRaw (gv ("noiseLevel"));
        r.cutoff      = Convert::cutoffToRaw (gv ("cutoff"));
        r.resonance   = Convert::normToRaw (gv ("resonance"));
        r.envMod      = Convert::normToRaw (gv ("envModAmt"));
        r.lfoCutoff   = Convert::normToRaw (gv ("lfoCutoffAmt"));
        r.keyTrack    = Convert::normToRaw (gv ("keyTracking"));
        r.vcaMode     = juce::roundToInt (gv ("vcaMode"));
        r.envMode     = juce::roundToInt (gv ("envMode"));
        r.attack      = Convert::attackToRaw (gv ("attack"));
        r.decay       = Convert::decayToRaw (gv ("decay"));
        r.sustain     = Convert::normToRaw (gv ("sustain"));
        r.release     = Convert::releaseToRaw (gv ("release"));
        r.volume      = Convert::normToRaw (gv ("volume"));
        r.glide       = Convert::glideToRaw (gv ("glideTime"));
        r.portaMode   = juce::roundToInt (gv ("portaMode"));

        return patch;
    }

    // ----- JSON import / export (web-compatible format) ---------------------

    /** Export all custom patches as a web-compatible JSON string. */
    juce::String exportToJSON() const
    {
        auto arr = juce::Array<juce::var>();
        for (auto& p : customPatches)
            arr.add (patchToVar (p));

        auto root = std::make_unique<juce::DynamicObject>();
        root->setProperty ("format", "mono101-patch-bank");
        root->setProperty ("version", 1);
        root->setProperty ("patches", arr);

        return juce::JSON::toString (juce::var (root.release()));
    }

    /** Import patches from a web-compatible JSON string. Returns true on success. */
    bool importFromJSON (const juce::String& jsonStr, bool replaceCustom = false)
    {
        auto parsed = juce::JSON::parse (jsonStr);
        if (! parsed.isObject()) return false;

        auto* obj = parsed.getDynamicObject();
        if (obj == nullptr) return false;

        auto format = obj->getProperty ("format").toString();
        if (format != "mono101-patch-bank") return false;

        auto patchesVar = obj->getProperty ("patches");
        auto* patchArr = patchesVar.getArray();
        if (patchArr == nullptr) return false;

        std::vector<Patch> imported;
        for (auto& pv : *patchArr)
        {
            Patch p;
            if (varToPatch (pv, p))
                imported.push_back (std::move (p));
        }

        if (imported.empty()) return false;

        if (replaceCustom)
            customPatches = std::move (imported);
        else
            for (auto& p : imported)
                savePatch (p);

        return true;
    }

private:
    std::vector<Patch> factoryPatches;
    std::vector<Patch> customPatches;

    // ----- Parameter helper -------------------------------------------------

    static void setParam (juce::AudioProcessorValueTreeState& apvts,
                          const juce::String& id, float value)
    {
        if (auto* param = apvts.getParameter (id))
            param->setValueNotifyingHost (param->convertTo0to1 (value));
    }

    // ----- JSON serialization helpers ---------------------------------------

    static juce::var patchToVar (const Patch& p)
    {
        auto settings = std::make_unique<juce::DynamicObject>();
        const auto& r = p.raw;

        settings->setProperty ("fineTune",    r.fineTune);
        settings->setProperty ("lfoRate",     r.lfoRate);
        settings->setProperty ("lfoWaveform", Convert::lfoWaveformToString (r.lfoWaveform));
        settings->setProperty ("lfoPitch",    r.lfoPitch);
        settings->setProperty ("octaveShift", r.octaveShift);
        settings->setProperty ("pulseWidth",  r.pulseWidth);
        settings->setProperty ("pwMode",      r.pwMode);
        settings->setProperty ("lfoPWM",      r.lfoPWM);
        settings->setProperty ("pulseLevel",  r.pulseLevel);
        settings->setProperty ("sawLevel",    r.sawLevel);
        settings->setProperty ("subLevel",    r.subLevel);
        settings->setProperty ("subMode",     r.subMode);
        settings->setProperty ("noiseLevel",  r.noiseLevel);
        settings->setProperty ("cutoff",      r.cutoff);
        settings->setProperty ("resonance",   r.resonance);
        settings->setProperty ("envMod",      r.envMod);
        settings->setProperty ("lfoCutoff",   r.lfoCutoff);
        settings->setProperty ("keyTrack",    r.keyTrack);
        settings->setProperty ("vcaMode",     r.vcaMode == 1 ? "gate" : "env");
        settings->setProperty ("envMode",     r.envMode);
        settings->setProperty ("attack",      r.attack);
        settings->setProperty ("decay",       r.decay);
        settings->setProperty ("sustain",     r.sustain);
        settings->setProperty ("release",     r.release);
        settings->setProperty ("volume",      r.volume);
        settings->setProperty ("glide",       r.glide);
        settings->setProperty ("portaMode",   r.portaMode);

        auto patch = std::make_unique<juce::DynamicObject>();
        patch->setProperty ("name", p.name);
        patch->setProperty ("settings", juce::var (settings.release()));

        return juce::var (patch.release());
    }

    static bool varToPatch (const juce::var& v, Patch& p)
    {
        if (! v.isObject()) return false;
        auto* obj = v.getDynamicObject();
        if (obj == nullptr) return false;

        p.name = obj->getProperty ("name").toString();
        if (p.name.isEmpty()) return false;

        auto settingsVar = obj->getProperty ("settings");
        if (! settingsVar.isObject()) return false;
        auto* s = settingsVar.getDynamicObject();
        if (s == nullptr) return false;

        auto& r = p.raw;

        r.fineTune    = (int) s->getProperty ("fineTune");
        r.lfoRate     = (int) s->getProperty ("lfoRate");
        r.lfoPitch    = (int) s->getProperty ("lfoPitch");
        r.octaveShift = (int) s->getProperty ("octaveShift");
        r.pulseWidth  = (int) s->getProperty ("pulseWidth");
        r.pwMode      = (int) s->getProperty ("pwMode");
        r.lfoPWM      = (int) s->getProperty ("lfoPWM");
        r.pulseLevel  = (int) s->getProperty ("pulseLevel");
        r.sawLevel    = (int) s->getProperty ("sawLevel");
        r.subLevel    = (int) s->getProperty ("subLevel");
        r.subMode     = (int) s->getProperty ("subMode");
        r.noiseLevel  = (int) s->getProperty ("noiseLevel");
        r.cutoff      = (int) s->getProperty ("cutoff");
        r.resonance   = (int) s->getProperty ("resonance");
        r.envMod      = (int) s->getProperty ("envMod");
        r.lfoCutoff   = (int) s->getProperty ("lfoCutoff");
        r.keyTrack    = (int) s->getProperty ("keyTrack");
        r.envMode     = (int) s->getProperty ("envMode");
        r.attack      = (int) s->getProperty ("attack");
        r.decay       = (int) s->getProperty ("decay");
        r.sustain     = (int) s->getProperty ("sustain");
        r.release     = (int) s->getProperty ("release");
        r.volume      = (int) s->getProperty ("volume");
        r.glide       = (int) s->getProperty ("glide");
        r.portaMode   = (int) s->getProperty ("portaMode");

        // lfoWaveform: string or int
        auto wfVar = s->getProperty ("lfoWaveform");
        if (wfVar.isString())
            r.lfoWaveform = Convert::lfoWaveformFromString (wfVar.toString());
        else
            r.lfoWaveform = (int) wfVar;

        // vcaMode: string "env"/"gate" or int
        auto vcaVar = s->getProperty ("vcaMode");
        if (vcaVar.isString())
            r.vcaMode = (vcaVar.toString() == "gate") ? 1 : 0;
        else
            r.vcaMode = (int) vcaVar;

        return true;
    }

    // ----- Factory patches (hardcoded from web presets/factory.json) ---------

    void loadFactoryPatches()
    {
        factoryPatches.clear();

        // 1. roygbiv
        {
            Patch p;
            p.name = "roygbiv";
            auto& r = p.raw;
            r.fineTune = 18;   r.lfoRate = 54;    r.lfoWaveform = 1; // triangle
            r.lfoPitch = 0;    r.octaveShift = 1; r.pulseWidth = 88;
            r.pwMode = 2;      r.lfoPWM = 82;     r.pulseLevel = 61;
            r.sawLevel = 0;    r.subLevel = 89;   r.subMode = 0;
            r.noiseLevel = 3;  r.cutoff = 59;     r.resonance = 11;
            r.envMod = 17;     r.lfoCutoff = 0;   r.keyTrack = 0;
            r.vcaMode = 1;     r.envMode = 2;     r.attack = 25;
            r.decay = 36;      r.sustain = 35;    r.release = 0;
            r.volume = 89;     r.glide = 18;      r.portaMode = 0;
            factoryPatches.push_back (p);
        }

        // 2. teartear
        {
            Patch p;
            p.name = "teartear";
            auto& r = p.raw;
            r.fineTune = 11;   r.lfoRate = 62;    r.lfoWaveform = 1;
            r.lfoPitch = 0;    r.octaveShift = 1; r.pulseWidth = 74;
            r.pwMode = 2;      r.lfoPWM = 47;     r.pulseLevel = 47;
            r.sawLevel = 0;    r.subLevel = 73;   r.subMode = 0;
            r.noiseLevel = 3;  r.cutoff = 44;     r.resonance = 3;
            r.envMod = 23;     r.lfoCutoff = 0;   r.keyTrack = 0;
            r.vcaMode = 0;     r.envMode = 2;     r.attack = 23;
            r.decay = 39;      r.sustain = 66;    r.release = 76;
            r.volume = 89;     r.glide = 0;       r.portaMode = 0;
            factoryPatches.push_back (p);
        }

        // 3. silent shout
        {
            Patch p;
            p.name = "silent shout";
            auto& r = p.raw;
            r.fineTune = 41;   r.lfoRate = 56;    r.lfoWaveform = 1;
            r.lfoPitch = 0;    r.octaveShift = 2; r.pulseWidth = 64;
            r.pwMode = 0;      r.lfoPWM = 95;     r.pulseLevel = 72;
            r.sawLevel = 95;   r.subLevel = 57;   r.subMode = 2;
            r.noiseLevel = 3;  r.cutoff = 64;     r.resonance = 45;
            r.envMod = 44;     r.lfoCutoff = 0;   r.keyTrack = 30;
            r.vcaMode = 1;     r.envMode = 2;     r.attack = 7;
            r.decay = 18;      r.sustain = 20;    r.release = 18;
            r.volume = 100;    r.glide = 0;       r.portaMode = 0;
            factoryPatches.push_back (p);
        }

        // 4. aegis
        {
            Patch p;
            p.name = "aegis";
            auto& r = p.raw;
            r.fineTune = 14;   r.lfoRate = 25;    r.lfoWaveform = 1;
            r.lfoPitch = 0;    r.octaveShift = 3; r.pulseWidth = 47;
            r.pwMode = 1;      r.lfoPWM = 9;      r.pulseLevel = 0;
            r.sawLevel = 0;    r.subLevel = 75;   r.subMode = 2;
            r.noiseLevel = 0;  r.cutoff = 14;     r.resonance = 9;
            r.envMod = 49;     r.lfoCutoff = 0;   r.keyTrack = 51;
            r.vcaMode = 0;     r.envMode = 2;     r.attack = 0;
            r.decay = 34;      r.sustain = 9;     r.release = 40;
            r.volume = 100;    r.glide = 0;       r.portaMode = 1;
            factoryPatches.push_back (p);
        }

        // 5. further
        {
            Patch p;
            p.name = "further";
            auto& r = p.raw;
            r.fineTune = 14;   r.lfoRate = 25;    r.lfoWaveform = 1;
            r.lfoPitch = 0;    r.octaveShift = 3; r.pulseWidth = 47;
            r.pwMode = 1;      r.lfoPWM = 9;      r.pulseLevel = 0;
            r.sawLevel = 0;    r.subLevel = 93;   r.subMode = 2;
            r.noiseLevel = 0;  r.cutoff = 14;     r.resonance = 9;
            r.envMod = 49;     r.lfoCutoff = 0;   r.keyTrack = 51;
            r.vcaMode = 0;     r.envMode = 2;     r.attack = 0;
            r.decay = 0;       r.sustain = 0;     r.release = 0;
            r.volume = 100;    r.glide = 0;       r.portaMode = 1;
            factoryPatches.push_back (p);
        }

        // 6. eutow
        {
            Patch p;
            p.name = "eutow";
            auto& r = p.raw;
            r.fineTune = 3;    r.lfoRate = 69;    r.lfoWaveform = 1;
            r.lfoPitch = 4;    r.octaveShift = 1; r.pulseWidth = 72;
            r.pwMode = 2;      r.lfoPWM = 50;     r.pulseLevel = 58;
            r.sawLevel = 32;   r.subLevel = 0;    r.subMode = 2;
            r.noiseLevel = 0;  r.cutoff = 36;     r.resonance = 3;
            r.envMod = 12;     r.lfoCutoff = 0;   r.keyTrack = 44;
            r.vcaMode = 0;     r.envMode = 1;     r.attack = 68;
            r.decay = 0;       r.sustain = 0;     r.release = 60;
            r.volume = 78;     r.glide = 0;       r.portaMode = 1;
            factoryPatches.push_back (p);
        }
    }
};

} // namespace mono101

#include "PluginProcessor.h"
#include "PluginEditor.h"

static float midiToFreq(int note)
{
    return 440.0f * std::pow(2.0f, (note - 69) / 12.0f);
}

Mono101Processor::Mono101Processor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::mono(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout Mono101Processor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Oscillator mix
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("sawLevel", 1),   "Saw Level",   0.0f, 1.0f, 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("pulseLevel", 1), "Pulse Level", 0.0f, 1.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("subLevel", 1),   "Sub Level",   0.0f, 1.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("noiseLevel", 1), "Noise Level", 0.0f, 1.0f, 0.0f));

    // Pulse width
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("pulseWidth", 1), "Pulse Width", 0.01f, 0.99f, 0.5f));

    // Filter
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("cutoff", 1), "Cutoff",
        juce::NormalisableRange<float>(10.0f, 20000.0f, 0.0f, 0.3f), 2000.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("resonance", 1), "Resonance", 0.0f, 1.2f, 0.1f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("envModAmt", 1), "Env Mod",   0.0f, 1.0f, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("keyTracking", 1), "Key Track", 0.0f, 1.0f, 0.5f));

    // LFO
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("lfoRate", 1), "LFO Rate",
        juce::NormalisableRange<float>(0.01f, 30.0f, 0.0f, 0.4f), 2.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("lfoPitchAmt", 1),  "LFO Pitch",  0.0f, 12.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("lfoCutoffAmt", 1), "LFO Cutoff", 0.0f, 1.0f,  0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("lfoPWMAmt", 1),    "LFO PWM",    0.0f, 1.0f,  0.0f));

    // Envelope
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("attack", 1), "Attack",
        juce::NormalisableRange<float>(0.0015f, 4.0f, 0.0f, 0.3f), 0.005f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("decay", 1), "Decay",
        juce::NormalisableRange<float>(0.002f, 10.0f, 0.0f, 0.3f), 0.2f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("sustain", 1), "Sustain", 0.0f, 1.0f, 0.7f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("release", 1), "Release",
        juce::NormalisableRange<float>(0.002f, 10.0f, 0.0f, 0.3f), 0.3f));

    // Output
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("volume", 1), "Volume", 0.0f, 1.0f, 0.7f));

    // Transpose
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("octaveShift", 1), "Octave", -5.0f, 5.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("fineTune", 1), "Fine Tune", -100.0f, 100.0f, 0.0f));

    // Glide
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("glideTime", 1), "Glide Time",
        juce::NormalisableRange<float>(0.0f, 1.5f, 0.0f, 0.4f), 0.0f));

    // Mode choices
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("lfoShape", 1), "LFO Shape",
        juce::StringArray{"Sine", "Triangle", "Square", "Saw", "S&H"}, 1));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("pwMode", 1), "PW Mode",
        juce::StringArray{"LFO", "Manual", "Env"}, 1));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("subMode", 1), "Sub Mode",
        juce::StringArray{"25% -2Oct", "Sq -2Oct", "Sq -1Oct"}, 1));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("envMode", 1), "Env Mode",
        juce::StringArray{"LFO", "Gate", "Gate+Trig"}, 1));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("vcaMode", 1), "VCA Mode",
        juce::StringArray{"Env", "Gate"}, 0));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("portaMode", 1), "Porta Mode",
        juce::StringArray{"On", "Off", "Auto"}, 1));

    // Arpeggiator
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("arpPattern", 1), "Arp Pattern",
        juce::StringArray{"Up", "Down", "Up/Down", "Random"}, 0));

    return { params.begin(), params.end() };
}

void Mono101Processor::prepareToPlay(double sampleRate, int /*samplesPerBlock*/)
{
    currentSampleRate = sampleRate;
    voice.prepare(static_cast<float>(sampleRate));
    noteStack.clear();
}

void Mono101Processor::handleMidiEvent(const juce::MidiMessage& msg)
{
    if (msg.isNoteOn())
    {
        int note = msg.getNoteNumber();
        float vel = msg.getFloatVelocity();

        if (arpEnabled.load())
        {
            arp.noteOn(note, vel);
            return;
        }

        bool wasHolding = !noteStack.empty();
        noteStack.erase(
            std::remove_if(noteStack.begin(), noteStack.end(),
                           [note](const NoteEntry& e) { return e.note == note; }),
            noteStack.end());
        noteStack.push_back({note, vel});

        applyGlide(wasHolding);

        if (voiceParams.envMode == mono101::EnvMode::GateTrig)
            voice.retrigger();

        voice.setNoteTarget(midiToFreq(note));
        voice.noteOn(midiToFreq(note), vel);
    }
    else if (msg.isNoteOff())
    {
        int note = msg.getNoteNumber();

        if (arpEnabled.load())
        {
            arp.noteOff(note);
            return;
        }

        noteStack.erase(
            std::remove_if(noteStack.begin(), noteStack.end(),
                           [note](const NoteEntry& e) { return e.note == note; }),
            noteStack.end());

        if (!noteStack.empty())
        {
            auto& prev = noteStack.back();
            applyGlide(true);
            voice.setNoteTarget(midiToFreq(prev.note));
        }
        else
        {
            voice.noteOff();
        }
    }
    else if (msg.isAllNotesOff() || msg.isAllSoundOff())
    {
        noteStack.clear();
        voice.noteOff();
        if (arpEnabled.load())
            arp.stop();
    }
}

void Mono101Processor::applyGlide(bool isLegato)
{
    float t;
    switch (portaMode) {
        case mono101::PortaMode::On:   t = glideTime; break;
        case mono101::PortaMode::Off:  t = 0.0f; break;
        case mono101::PortaMode::Auto: t = isLegato ? glideTime : 0.0f; break;
    }
    voice.setPortamentoTime(t);
}

void Mono101Processor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    buffer.clear();

    // read parameters into voiceParams
    voiceParams.sawLevel    = *apvts.getRawParameterValue("sawLevel");
    voiceParams.pulseLevel  = *apvts.getRawParameterValue("pulseLevel");
    voiceParams.subLevel    = *apvts.getRawParameterValue("subLevel");
    voiceParams.noiseLevel  = *apvts.getRawParameterValue("noiseLevel");
    voiceParams.pulseWidth  = *apvts.getRawParameterValue("pulseWidth");
    voiceParams.cutoff      = *apvts.getRawParameterValue("cutoff");
    voiceParams.resonance   = *apvts.getRawParameterValue("resonance");
    voiceParams.envModAmt   = *apvts.getRawParameterValue("envModAmt");
    voiceParams.keyTracking  = *apvts.getRawParameterValue("keyTracking");
    voiceParams.lfoRate      = *apvts.getRawParameterValue("lfoRate");
    voiceParams.lfoPitchAmt  = *apvts.getRawParameterValue("lfoPitchAmt");
    voiceParams.lfoCutoffAmt = *apvts.getRawParameterValue("lfoCutoffAmt");
    voiceParams.lfoPWMAmt    = *apvts.getRawParameterValue("lfoPWMAmt");
    voiceParams.attack       = *apvts.getRawParameterValue("attack");
    voiceParams.decay        = *apvts.getRawParameterValue("decay");
    voiceParams.sustain      = *apvts.getRawParameterValue("sustain");
    voiceParams.release      = *apvts.getRawParameterValue("release");
    voiceParams.volume       = *apvts.getRawParameterValue("volume");
    voiceParams.octaveShift  = *apvts.getRawParameterValue("octaveShift");
    voiceParams.fineTune     = *apvts.getRawParameterValue("fineTune");
    glideTime                = *apvts.getRawParameterValue("glideTime");

    // read choice parameters
    voiceParams.lfoShape = static_cast<mono101::LFOShape>((int)*apvts.getRawParameterValue("lfoShape"));
    voiceParams.pwMode   = static_cast<mono101::PWMode>((int)*apvts.getRawParameterValue("pwMode"));
    voiceParams.subMode  = static_cast<mono101::SubMode>((int)*apvts.getRawParameterValue("subMode"));
    voiceParams.envMode  = static_cast<mono101::EnvMode>((int)*apvts.getRawParameterValue("envMode"));
    voiceParams.vcaMode  = static_cast<mono101::VCAMode>((int)*apvts.getRawParameterValue("vcaMode"));
    portaMode = static_cast<mono101::PortaMode>((int)*apvts.getRawParameterValue("portaMode"));

    // Sync arp BPM to LFO rate (lfoHz * 60, matching web version)
    arp.setBPM(voiceParams.lfoRate * 60.0);
    arp.setPattern(static_cast<mono101::ArpPattern>((int)*apvts.getRawParameterValue("arpPattern")));

    auto* channelData = buffer.getWritePointer(0);
    int numSamples = buffer.getNumSamples();

    // Collect all MIDI-triggered events into a combined event list
    // so we can interleave with arp events sample-accurately
    struct Event { int sample; int note; float vel; bool isOn; };
    std::vector<Event> events;

    // Process incoming MIDI
    for (const auto metadata : midiMessages)
    {
        const auto& msg = metadata.getMessage();
        int pos = metadata.samplePosition;

        if (msg.isNoteOn())
        {
            if (arpEnabled.load())
            {
                arp.noteOn(msg.getNoteNumber(), msg.getFloatVelocity());
            }
            else
            {
                events.push_back({ pos, msg.getNoteNumber(), msg.getFloatVelocity(), true });
            }
        }
        else if (msg.isNoteOff())
        {
            if (arpEnabled.load())
            {
                arp.noteOff(msg.getNoteNumber());
            }
            else
            {
                events.push_back({ pos, msg.getNoteNumber(), 0.0f, false });
            }
        }
        else if (msg.isAllNotesOff() || msg.isAllSoundOff())
        {
            noteStack.clear();
            voice.noteOff();
            if (arpEnabled.load()) arp.stop();
        }
    }

    // Get arp events for this block
    if (arpEnabled.load() && arp.isRunning())
    {
        auto arpEvents = arp.process(numSamples, currentSampleRate);
        for (auto& ae : arpEvents)
            events.push_back({ ae.sampleOffset, ae.midiNote, ae.velocity, ae.isNoteOn });
    }

    // Sort all events by sample position
    std::sort(events.begin(), events.end(),
              [](const Event& a, const Event& b) { return a.sample < b.sample; });

    // Render with sample-accurate event processing
    int currentSample = 0;
    for (auto& ev : events)
    {
        int eventSample = juce::jmin(ev.sample, numSamples);

        // render audio up to this event
        for (; currentSample < eventSample; ++currentSample)
        {
            channelData[currentSample] = voice.isActive()
                ? voice.processSample(voiceParams) : 0.0f;
        }

        // Handle event
        if (ev.isOn)
        {
            if (arpEnabled.load())
            {
                // Arp drives voice directly — no note stack
                voice.setNoteTarget(midiToFreq(ev.note));
                voice.noteOn(midiToFreq(ev.note), ev.vel);
            }
            else
            {
                // Normal note-on through note stack
                bool wasHolding = !noteStack.empty();
                noteStack.erase(
                    std::remove_if(noteStack.begin(), noteStack.end(),
                                   [&](const NoteEntry& e) { return e.note == ev.note; }),
                    noteStack.end());
                noteStack.push_back({ ev.note, ev.vel });
                applyGlide(wasHolding);
                if (voiceParams.envMode == mono101::EnvMode::GateTrig)
                    voice.retrigger();
                voice.setNoteTarget(midiToFreq(ev.note));
                voice.noteOn(midiToFreq(ev.note), ev.vel);
            }
        }
        else
        {
            if (arpEnabled.load())
            {
                voice.noteOff();
            }
            else
            {
                noteStack.erase(
                    std::remove_if(noteStack.begin(), noteStack.end(),
                                   [&](const NoteEntry& e) { return e.note == ev.note; }),
                    noteStack.end());
                if (!noteStack.empty())
                {
                    auto& prev = noteStack.back();
                    applyGlide(true);
                    voice.setNoteTarget(midiToFreq(prev.note));
                }
                else
                {
                    voice.noteOff();
                }
            }
        }
    }

    // render remaining samples
    for (; currentSample < numSamples; ++currentSample)
    {
        channelData[currentSample] = voice.isActive()
            ? voice.processSample(voiceParams) : 0.0f;
    }
}

juce::AudioProcessorEditor* Mono101Processor::createEditor()
{
    return new Mono101Editor(*this);
}

void Mono101Processor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void Mono101Processor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml && xml->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Mono101Processor();
}

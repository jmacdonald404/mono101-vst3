#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "DSP/Mono101Voice.h"
#include "DSP/Arpeggiator.h"
#include "PatchBank.h"

class Mono101Processor : public juce::AudioProcessor
{
public:
    Mono101Processor();
    ~Mono101Processor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Mono-101"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;
    mono101::PatchBank patchBank;

    // Arp state — readable from editor for UI sync
    mono101::Arpeggiator arp;
    std::atomic<bool> arpEnabled { false };

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    void handleMidiEvent(const juce::MidiMessage& msg);

    mono101::Mono101Voice voice;
    mono101::VoiceParams voiceParams;
    double currentSampleRate = 44100.0;

    // note stack for mono voice (same as JS version)
    struct NoteEntry { int note; float velocity; };
    std::vector<NoteEntry> noteStack;

    // porta mode and glide
    mono101::PortaMode portaMode = mono101::PortaMode::Off;
    float glideTime = 0.0f;

    void applyGlide(bool isLegato);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Mono101Processor)
};

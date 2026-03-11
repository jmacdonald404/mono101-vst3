#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

//==============================================================================
class Mono101LookAndFeel : public juce::LookAndFeel_V4
{
public:
    Mono101LookAndFeel();

    void drawLinearSlider(juce::Graphics&, int x, int y, int width, int height,
                          float sliderPos, float minSliderPos, float maxSliderPos,
                          juce::Slider::SliderStyle, juce::Slider&) override;

    void drawButtonBackground(juce::Graphics&, juce::Button&,
                              const juce::Colour& bg, bool isOver, bool isDown) override;
    void drawButtonText(juce::Graphics&, juce::TextButton&,
                        bool isOver, bool isDown) override;

    // Colours
    static constexpr juce::uint32 kPanel       = 0xff2c2c2e;
    static constexpr juce::uint32 kPanelDark   = 0xff1c1c1e;
    static constexpr juce::uint32 kPanelRaised = 0xff343436;
    static constexpr juce::uint32 kPanelSunken = 0xff202022;
    static constexpr juce::uint32 kAccentRed   = 0xffc8191a;
    static constexpr juce::uint32 kCream       = 0xffddd8c4;
    static constexpr juce::uint32 kCreamDark   = 0xffb8b09a;
    static constexpr juce::uint32 kSilverHi    = 0xffd8d8d8;
    static constexpr juce::uint32 kLabel       = 0xffe0ddd5;
    static constexpr juce::uint32 kLabelDim    = 0xff888880;
    static constexpr juce::uint32 kSectionEdge = 0xff484848;
    static constexpr juce::uint32 kTrack       = 0xff141416;
    static constexpr juce::uint32 kLedOn       = 0xffff5500;
    static constexpr juce::uint32 kLedBlue     = 0xff4499ff;
};

//==============================================================================
// A vertical slider with label above and name below
class Mono101Slider : public juce::Component
{
public:
    Mono101Slider(const juce::String& name, const juce::String& topLabel = {});

    void resized() override;

    juce::Slider slider;
    juce::Label  nameLabel;
    juce::Label  topLabel;

    // Set to true for 3-position xs switches
    bool isXS = false;
};

//==============================================================================
// A toggle button with LED indicator
class Mono101ToggleButton : public juce::TextButton
{
public:
    Mono101ToggleButton(const juce::String& name);
    void paintButton(juce::Graphics&, bool isOver, bool isDown) override;

    juce::Colour ledColour { juce::Colour(Mono101LookAndFeel::kLedOn) };
};

//==============================================================================
class Mono101Editor : public juce::AudioProcessorEditor
{
public:
    explicit Mono101Editor(Mono101Processor&);
    ~Mono101Editor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    Mono101Processor& processor;
    Mono101LookAndFeel mono101LnF;

    // ---- Section rectangles (computed in resized, used in paint) ----
    struct Section {
        juce::Rectangle<int> bounds;
        juce::String title;
        juce::Colour titleColour;
    };
    std::vector<Section> sections;

    // ---- Tune ----
    Mono101Slider fineTuneSlider { "Fine", "+50" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> fineTuneAtt;

    // ---- LFO ----
    Mono101Slider lfoRateSlider   { "Rate", "10" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lfoRateAtt;

    // LFO shape buttons
    Mono101ToggleButton lfoSinBtn  { "Sin" };
    Mono101ToggleButton lfoTriBtn  { "Tri" };
    Mono101ToggleButton lfoSqrBtn  { "Sqr" };
    Mono101ToggleButton lfoSawBtn  { "Saw" };
    Mono101ToggleButton lfoShBtn   { "S&H" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> lfoShapeAtt;
    // hidden combo to drive the choice param
    juce::ComboBox lfoShapeCombo;
    void lfoShapeButtonClicked(int index);

    // ---- VCO ----
    Mono101Slider vcoModSlider     { "Mod", "10" };
    Mono101Slider octaveSlider     { "Octave", "+5" };
    Mono101Slider pWidthSlider     { "P.Width", "Sqr" };
    Mono101Slider pwModeSlider     { "P.Mode" };
    Mono101Slider pwmSlider        { "PWM", "10" };

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> vcoModAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> octaveAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> pWidthAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> pwmAtt;
    // pwMode is a choice -> use combo attachment
    juce::ComboBox pwModeCombo;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> pwModeAtt;

    // ---- Source Mixer ----
    Mono101Slider pulseSlider { "Pulse", "10" };
    Mono101Slider sawSlider   { "Saw", "10" };
    Mono101Slider subSlider   { "Sub", "10" };
    Mono101Slider subModeSlider { "Sub Oct" };
    Mono101Slider noiseSlider { "Noise", "10" };

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> pulseAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sawAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> subAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> noiseAtt;
    juce::ComboBox subModeCombo;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> subModeAtt;

    // ---- VCF ----
    Mono101Slider cutoffSlider  { "Cutoff", "10" };
    Mono101Slider resoSlider    { "Reso", "10" };
    Mono101Slider envModSlider  { "Env Mod", "10" };
    Mono101Slider lfoModSlider  { "LFO Mod", "10" };
    Mono101Slider keyTrkSlider  { "Key Trk", "10" };

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> cutoffAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> resoAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> envModAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lfoModAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> keyTrkAtt;

    // ---- VCA ----
    Mono101ToggleButton vcaEnvBtn  { "Env" };
    Mono101ToggleButton vcaGateBtn { "Gate" };
    juce::ComboBox vcaModeCombo;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> vcaModeAtt;
    void vcaModeButtonClicked(int index);

    // ---- Envelope ----
    Mono101Slider envModeSlider { "Trig" };
    Mono101Slider attackSlider  { "Attack", "10" };
    Mono101Slider decaySlider   { "Decay", "10" };
    Mono101Slider sustainSlider { "Sustain", "10" };
    Mono101Slider releaseSlider { "Release", "10" };

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attackAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> decayAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sustainAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> releaseAtt;
    juce::ComboBox envModeCombo;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> envModeAtt;

    // ---- Portamento (Row 3) ----
    Mono101Slider volumeSlider    { "Volume", "10" };
    Mono101Slider portaTimeSlider { "Porta", "10" };
    Mono101Slider portaModeSlider { "Mode" };

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> volumeAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> portaTimeAtt;
    juce::ComboBox portaModeCombo;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> portaModeAtt;

    // ---- Arpeggio (Row 2) ----
    Mono101ToggleButton arpUpBtn   { "Up" };
    Mono101ToggleButton arpDownBtn { "Down" };
    Mono101ToggleButton arpUDBtn   { "Up/Dn" };
    void arpButtonClicked(int patternIndex);

    // ---- Patch Bank (Row 2) ----
    juce::TextButton pbSaveBtn   { "Save" };
    juce::TextButton pbLoadBtn   { "Load" };
    juce::ComboBox   pbSelect;
    juce::TextButton pbImportBtn { "Import" };
    juce::TextButton pbExportBtn { "Export" };
    std::unique_ptr<juce::FileChooser> fileChooser;
    void populatePatchDropdown();

    // ---- Helpers ----
    void setupSlider(Mono101Slider& s, bool xs = false);
    void setupXSSlider(Mono101Slider& s, juce::ComboBox& combo,
                       const juce::String& paramId, int numItems);
    juce::Rectangle<int> layoutSection(int x, int y, int w, int h,
                                        const juce::String& title,
                                        juce::Colour titleColour);
    void paintSections(juce::Graphics& g);
    void paintBrandBar(juce::Graphics& g, juce::Rectangle<int> area);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Mono101Editor)
};

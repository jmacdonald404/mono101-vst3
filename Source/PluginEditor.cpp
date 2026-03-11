#include "PluginEditor.h"

//==============================================================================
// Mono101LookAndFeel
//==============================================================================
Mono101LookAndFeel::Mono101LookAndFeel()
{
    setColour(juce::Slider::backgroundColourId,   juce::Colour(kTrack));
    setColour(juce::Slider::thumbColourId,         juce::Colour(kCream));
    setColour(juce::Slider::trackColourId,         juce::Colour(kTrack));
    setColour(juce::Label::textColourId,           juce::Colour(kLabelDim));
    setColour(juce::TextButton::buttonColourId,    juce::Colour(kPanelSunken));
    setColour(juce::TextButton::textColourOffId,   juce::Colour(kLabelDim));
    setColour(juce::TextButton::textColourOnId,    juce::Colour(kLabel));
    setColour(juce::ComboBox::backgroundColourId,  juce::Colour(kPanelSunken));
    setColour(juce::ComboBox::textColourId,        juce::Colour(kLabel));
    setColour(juce::ComboBox::outlineColourId,     juce::Colour(kSectionEdge));
    setColour(juce::PopupMenu::backgroundColourId, juce::Colour(kPanelDark));
    setColour(juce::PopupMenu::textColourId,       juce::Colour(kLabel));
    setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(kAccentRed));
}

void Mono101LookAndFeel::drawLinearSlider(juce::Graphics& g,
                                         int x, int y, int width, int height,
                                         float sliderPos,
                                         float /*minSliderPos*/, float /*maxSliderPos*/,
                                         juce::Slider::SliderStyle style,
                                         juce::Slider& slider)
{
    const bool vertical = (style == juce::Slider::LinearVertical
                        || style == juce::Slider::LinearBarVertical);

    if (vertical)
    {
        // Track
        const float trackW = 4.0f;
        const float cx = x + width * 0.5f;
        juce::Rectangle<float> trackRect(cx - trackW * 0.5f, (float)y,
                                          trackW, (float)height);
        g.setColour(juce::Colour(kTrack));
        g.fillRoundedRectangle(trackRect, 2.0f);

        // Scale marks (11 dashes along the track)
        g.setColour(juce::Colour(kSectionEdge).withAlpha(0.5f));
        for (int i = 0; i <= 10; ++i)
        {
            float frac = i / 10.0f;
            float yy = y + height - frac * height;
            float dashX = cx + trackW * 0.5f + 2.0f;
            g.drawHorizontalLine((int)yy, dashX, dashX + 4.0f);
        }

        // Thumb
        const float thumbH = 10.0f;
        const float thumbW = 14.0f;
        float thumbY = sliderPos - thumbH * 0.5f;
        juce::Rectangle<float> thumbRect(cx - thumbW * 0.5f, thumbY, thumbW, thumbH);

        // Shadow
        g.setColour(juce::Colours::black.withAlpha(0.4f));
        g.fillRoundedRectangle(thumbRect.translated(0, 1), 2.0f);

        // Thumb body
        auto thumbGrad = juce::ColourGradient(juce::Colour(kCream), thumbRect.getX(), thumbRect.getY(),
                                               juce::Colour(kCreamDark), thumbRect.getX(), thumbRect.getBottom(),
                                               false);
        g.setGradientFill(thumbGrad);
        g.fillRoundedRectangle(thumbRect, 2.0f);

        // Thumb groove
        g.setColour(juce::Colour(kCreamDark).darker(0.3f));
        float grooveY = thumbRect.getCentreY();
        g.drawHorizontalLine((int)grooveY, thumbRect.getX() + 2.0f, thumbRect.getRight() - 2.0f);
    }
    else
    {
        // Horizontal fallback (minimal)
        g.setColour(juce::Colour(kTrack));
        g.fillRoundedRectangle((float)x, y + height * 0.5f - 2.0f, (float)width, 4.0f, 2.0f);

        g.setColour(juce::Colour(kCream));
        float thumbX = sliderPos;
        g.fillRoundedRectangle(thumbX - 5.0f, (float)y, 10.0f, (float)height, 2.0f);
    }
}

void Mono101LookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                              const juce::Colour&, bool isOver, bool /*isDown*/)
{
    auto bounds = button.getLocalBounds().toFloat().reduced(0.5f);
    bool isOn = button.getToggleState();

    // Background
    auto bgColour = isOn ? juce::Colour(kPanelDark).brighter(0.1f)
                         : juce::Colour(kPanelSunken);
    if (isOver) bgColour = bgColour.brighter(0.05f);

    g.setColour(bgColour);
    g.fillRoundedRectangle(bounds, 3.0f);

    // Border
    g.setColour(juce::Colour(kSectionEdge));
    g.drawRoundedRectangle(bounds, 3.0f, 1.0f);
}

void Mono101LookAndFeel::drawButtonText(juce::Graphics& g, juce::TextButton& button,
                                       bool /*isOver*/, bool /*isDown*/)
{
    auto bounds = button.getLocalBounds();
    bool isOn = button.getToggleState();

    // LED dot
    const float ledSize = 5.0f;
    float ledX = bounds.getX() + 5.0f;
    float ledY = bounds.getCentreY() - ledSize * 0.5f;

    if (isOn)
    {
        g.setColour(juce::Colour(kLedOn));
        g.fillEllipse(ledX, ledY, ledSize, ledSize);
        // glow
        g.setColour(juce::Colour(kLedOn).withAlpha(0.3f));
        g.fillEllipse(ledX - 2, ledY - 2, ledSize + 4, ledSize + 4);
    }
    else
    {
        g.setColour(juce::Colour(0xff2a1200));
        g.fillEllipse(ledX, ledY, ledSize, ledSize);
    }

    // Text
    g.setColour(isOn ? juce::Colour(kLabel) : juce::Colour(kLabelDim));
    g.setFont(juce::Font("Barlow Condensed", 9.5f, juce::Font::plain));
    auto textBounds = bounds.reduced(4, 0).withTrimmedLeft(10);
    g.drawText(button.getButtonText(), textBounds, juce::Justification::centredLeft, false);
}

//==============================================================================
// Mono101Slider
//==============================================================================
Mono101Slider::Mono101Slider(const juce::String& name, const juce::String& top)
{
    slider.setSliderStyle(juce::Slider::LinearVertical);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    addAndMakeVisible(slider);

    nameLabel.setText(name, juce::dontSendNotification);
    nameLabel.setJustificationType(juce::Justification::centred);
    nameLabel.setFont(juce::Font("Barlow Condensed", 8.5f, juce::Font::plain));
    nameLabel.setColour(juce::Label::textColourId, juce::Colour(Mono101LookAndFeel::kLabelDim));
    addAndMakeVisible(nameLabel);

    if (top.isNotEmpty())
    {
        topLabel.setText(top, juce::dontSendNotification);
        topLabel.setJustificationType(juce::Justification::centred);
        topLabel.setFont(juce::Font("Barlow Condensed", 7.5f, juce::Font::plain));
        topLabel.setColour(juce::Label::textColourId, juce::Colour(Mono101LookAndFeel::kLabelDim));
        addAndMakeVisible(topLabel);
    }
}

void Mono101Slider::resized()
{
    auto b = getLocalBounds();
    int topH = topLabel.getText().isNotEmpty() ? 12 : 0;
    int bottomH = 12;

    if (topH > 0)
        topLabel.setBounds(b.removeFromTop(topH));

    nameLabel.setBounds(b.removeFromBottom(bottomH));
    slider.setBounds(b);
}

//==============================================================================
// Mono101ToggleButton
//==============================================================================
Mono101ToggleButton::Mono101ToggleButton(const juce::String& name)
    : juce::TextButton(name)
{
    setClickingTogglesState(true);
}

void Mono101ToggleButton::paintButton(juce::Graphics& g, bool isOver, bool isDown)
{
    auto bounds = getLocalBounds().toFloat().reduced(0.5f);
    bool isOn = getToggleState();

    // Background
    auto bgColour = isOn ? juce::Colour(Mono101LookAndFeel::kPanelDark).brighter(0.1f)
                         : juce::Colour(Mono101LookAndFeel::kPanelSunken);
    if (isOver) bgColour = bgColour.brighter(0.05f);
    g.setColour(bgColour);
    g.fillRoundedRectangle(bounds, 3.0f);

    // Border
    g.setColour(juce::Colour(Mono101LookAndFeel::kSectionEdge));
    g.drawRoundedRectangle(bounds, 3.0f, 1.0f);

    // LED dot
    const float ledSize = 5.0f;
    float ledX = bounds.getCentreX() - ledSize * 0.5f;
    float ledY = 4.0f;
    if (isOn)
    {
        g.setColour(ledColour);
        g.fillEllipse(ledX, ledY, ledSize, ledSize);
        g.setColour(ledColour.withAlpha(0.3f));
        g.fillEllipse(ledX - 2, ledY - 2, ledSize + 4, ledSize + 4);
    }
    else
    {
        g.setColour(juce::Colour(0xff2a1200));
        g.fillEllipse(ledX, ledY, ledSize, ledSize);
    }

    // Text
    g.setColour(isOn ? juce::Colour(Mono101LookAndFeel::kLabel) : juce::Colour(Mono101LookAndFeel::kLabelDim));
    g.setFont(juce::Font("Barlow Condensed", 8.5f, juce::Font::plain));
    g.drawText(getButtonText(), getLocalBounds().withTrimmedTop(10), juce::Justification::centred, false);
}

//==============================================================================
// Mono101Editor
//==============================================================================
Mono101Editor::Mono101Editor(Mono101Processor& p)
    : AudioProcessorEditor(p),
      processor(p)
{
    setLookAndFeel(&mono101LnF);
    setSize(1100, 400);

    // ---- TUNE ----
    setupSlider(fineTuneSlider);
    fineTuneAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.apvts, "fineTune", fineTuneSlider.slider);

    // ---- LFO ----
    setupSlider(lfoRateSlider);
    lfoRateAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.apvts, "lfoRate", lfoRateSlider.slider);

    // LFO shape buttons
    auto setupLfoBtn = [this](Mono101ToggleButton& btn, int idx) {
        btn.ledColour = juce::Colour(Mono101LookAndFeel::kLedBlue);
        btn.setRadioGroupId(1001, juce::dontSendNotification);
        btn.onClick = [this, idx] { lfoShapeButtonClicked(idx); };
        addAndMakeVisible(btn);
    };
    setupLfoBtn(lfoSinBtn, 0);
    setupLfoBtn(lfoTriBtn, 1);
    setupLfoBtn(lfoSqrBtn, 2);
    setupLfoBtn(lfoSawBtn, 3);
    setupLfoBtn(lfoShBtn,  4);
    // Hidden combo for attachment
    lfoShapeCombo.addItemList({"Sine","Triangle","Square","Saw","S&H"}, 1);
    addChildComponent(lfoShapeCombo);
    lfoShapeAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        processor.apvts, "lfoShape", lfoShapeCombo);
    // Sync button states from combo
    lfoShapeCombo.onChange = [this] {
        int idx = lfoShapeCombo.getSelectedItemIndex();
        Mono101ToggleButton* btns[] = { &lfoSinBtn, &lfoTriBtn, &lfoSqrBtn, &lfoSawBtn, &lfoShBtn };
        for (int i = 0; i < 5; ++i)
            btns[i]->setToggleState(i == idx, juce::dontSendNotification);
    };
    // Initial sync
    {
        int idx = (int)*processor.apvts.getRawParameterValue("lfoShape");
        Mono101ToggleButton* btns[] = { &lfoSinBtn, &lfoTriBtn, &lfoSqrBtn, &lfoSawBtn, &lfoShBtn };
        for (int i = 0; i < 5; ++i)
            btns[i]->setToggleState(i == idx, juce::dontSendNotification);
    }

    // ---- VCO ----
    setupSlider(vcoModSlider);
    vcoModAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.apvts, "lfoPitchAmt", vcoModSlider.slider);

    setupSlider(octaveSlider);
    octaveAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.apvts, "octaveShift", octaveSlider.slider);

    setupSlider(pWidthSlider);
    pWidthAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.apvts, "pulseWidth", pWidthSlider.slider);

    setupSlider(pwmSlider);
    pwmAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.apvts, "lfoPWMAmt", pwmSlider.slider);

    // pwMode xs switch via hidden combo
    setupXSSlider(pwModeSlider, pwModeCombo, "pwMode", 3);

    // ---- SOURCE MIXER ----
    setupSlider(pulseSlider);
    pulseAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.apvts, "pulseLevel", pulseSlider.slider);

    setupSlider(sawSlider);
    sawAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.apvts, "sawLevel", sawSlider.slider);

    setupSlider(subSlider);
    subAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.apvts, "subLevel", subSlider.slider);

    setupSlider(noiseSlider);
    noiseAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.apvts, "noiseLevel", noiseSlider.slider);

    // subMode xs
    setupXSSlider(subModeSlider, subModeCombo, "subMode", 3);

    // ---- VCF ----
    setupSlider(cutoffSlider);
    cutoffAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.apvts, "cutoff", cutoffSlider.slider);

    setupSlider(resoSlider);
    resoAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.apvts, "resonance", resoSlider.slider);

    setupSlider(envModSlider);
    envModAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.apvts, "envModAmt", envModSlider.slider);

    setupSlider(lfoModSlider);
    lfoModAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.apvts, "lfoCutoffAmt", lfoModSlider.slider);

    setupSlider(keyTrkSlider);
    keyTrkAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.apvts, "keyTracking", keyTrkSlider.slider);

    // ---- VCA ----
    vcaEnvBtn.setRadioGroupId(1002, juce::dontSendNotification);
    vcaGateBtn.setRadioGroupId(1002, juce::dontSendNotification);
    vcaEnvBtn.onClick = [this] { vcaModeButtonClicked(0); };
    vcaGateBtn.onClick = [this] { vcaModeButtonClicked(1); };
    addAndMakeVisible(vcaEnvBtn);
    addAndMakeVisible(vcaGateBtn);
    vcaModeCombo.addItemList({"Env","Gate"}, 1);
    addChildComponent(vcaModeCombo);
    vcaModeAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        processor.apvts, "vcaMode", vcaModeCombo);
    vcaModeCombo.onChange = [this] {
        int idx = vcaModeCombo.getSelectedItemIndex();
        vcaEnvBtn.setToggleState(idx == 0, juce::dontSendNotification);
        vcaGateBtn.setToggleState(idx == 1, juce::dontSendNotification);
    };
    {
        int idx = (int)*processor.apvts.getRawParameterValue("vcaMode");
        vcaEnvBtn.setToggleState(idx == 0, juce::dontSendNotification);
        vcaGateBtn.setToggleState(idx == 1, juce::dontSendNotification);
    }

    // ---- ENVELOPE ----
    setupXSSlider(envModeSlider, envModeCombo, "envMode", 3);

    setupSlider(attackSlider);
    attackAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.apvts, "attack", attackSlider.slider);

    setupSlider(decaySlider);
    decayAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.apvts, "decay", decaySlider.slider);

    setupSlider(sustainSlider);
    sustainAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.apvts, "sustain", sustainSlider.slider);

    setupSlider(releaseSlider);
    releaseAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.apvts, "release", releaseSlider.slider);

    // ---- PORTAMENTO (Row 3) ----
    setupSlider(volumeSlider);
    volumeAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.apvts, "volume", volumeSlider.slider);

    setupSlider(portaTimeSlider);
    portaTimeAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.apvts, "glideTime", portaTimeSlider.slider);

    setupXSSlider(portaModeSlider, portaModeCombo, "portaMode", 3);

    // ---- ARPEGGIO (Row 2) ----
    auto setupArpBtn = [this](Mono101ToggleButton& btn, int idx) {
        btn.setClickingTogglesState(false); // we handle toggle manually
        btn.onClick = [this, idx] { arpButtonClicked(idx); };
        addAndMakeVisible(btn);
    };
    setupArpBtn(arpUpBtn, 0);
    setupArpBtn(arpDownBtn, 1);
    setupArpBtn(arpUDBtn, 2);

    // ---- PATCH BANK (Row 2) ----
    pbSaveBtn.onClick = [this] {
        auto aw = std::make_shared<juce::AlertWindow>("Save Patch", "Enter patch name:",
                                                       juce::MessageBoxIconType::NoIcon);
        aw->addTextEditor("name", "", "Name:");
        aw->addButton("OK", 1);
        aw->addButton("Cancel", 0);
        aw->enterModalState(true, juce::ModalCallbackFunction::create(
            [this, aw](int result) {
                if (result == 1)
                {
                    auto name = aw->getTextEditorContents("name");
                    if (name.isNotEmpty())
                    {
                        auto patch = mono101::PatchBank::captureFromAPVTS(name, processor.apvts);
                        processor.patchBank.savePatch(patch);
                        populatePatchDropdown();
                    }
                }
            }), true);
    };
    addAndMakeVisible(pbSaveBtn);

    pbLoadBtn.onClick = [this] {
        auto selected = pbSelect.getText();
        if (selected.isEmpty()) return;
        auto* patch = processor.patchBank.findPatch(selected);
        if (patch)
            mono101::PatchBank::applyToAPVTS(*patch, processor.apvts);
    };
    addAndMakeVisible(pbLoadBtn);

    addAndMakeVisible(pbSelect);
    populatePatchDropdown();

    pbImportBtn.onClick = [this] {
        fileChooser = std::make_unique<juce::FileChooser>(
            "Import Patch Bank", juce::File{}, "*.json");
        fileChooser->launchAsync(juce::FileBrowserComponent::openMode
                                 | juce::FileBrowserComponent::canSelectFiles,
            [this](const juce::FileChooser& fc) {
                auto file = fc.getResult();
                if (file.existsAsFile())
                {
                    auto json = file.loadFileAsString();
                    if (processor.patchBank.importFromJSON(json))
                        populatePatchDropdown();
                }
            });
    };
    addAndMakeVisible(pbImportBtn);

    pbExportBtn.onClick = [this] {
        fileChooser = std::make_unique<juce::FileChooser>(
            "Export Patch Bank", juce::File{}, "*.json");
        fileChooser->launchAsync(juce::FileBrowserComponent::saveMode,
            [this](const juce::FileChooser& fc) {
                auto file = fc.getResult();
                if (file != juce::File{})
                    file.replaceWithText(processor.patchBank.exportToJSON());
            });
    };
    addAndMakeVisible(pbExportBtn);
}

Mono101Editor::~Mono101Editor()
{
    setLookAndFeel(nullptr);
}

void Mono101Editor::setupSlider(Mono101Slider& s, bool xs)
{
    s.isXS = xs;
    addAndMakeVisible(s);
}

void Mono101Editor::setupXSSlider(Mono101Slider& s, juce::ComboBox& combo,
                                  const juce::String& paramId, int numItems)
{
    s.isXS = true;
    s.slider.setRange(0, numItems - 1, 1);
    s.slider.setSliderStyle(juce::Slider::LinearVertical);
    s.slider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    addAndMakeVisible(s);

    // Build combo items from parameter choices
    auto* param = processor.apvts.getParameter(paramId);
    if (auto* choiceParam = dynamic_cast<juce::AudioParameterChoice*>(param))
    {
        combo.addItemList(choiceParam->choices, 1);
    }
    else
    {
        for (int i = 0; i < numItems; ++i)
            combo.addItem(juce::String(i), i + 1);
    }
    addChildComponent(combo);

    // Bidirectional sync: slider <-> combo
    s.slider.onValueChange = [&s, &combo] {
        int idx = (int)s.slider.getValue();
        if (combo.getSelectedItemIndex() != idx)
            combo.setSelectedItemIndex(idx, juce::sendNotificationSync);
    };

    combo.onChange = [&s, &combo] {
        int idx = combo.getSelectedItemIndex();
        if ((int)s.slider.getValue() != idx)
            s.slider.setValue(idx, juce::dontSendNotification);
    };

    // Create combo attachment to APVTS
    if (paramId == "pwMode")
        pwModeAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
            processor.apvts, paramId, combo);
    else if (paramId == "subMode")
        subModeAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
            processor.apvts, paramId, combo);
    else if (paramId == "envMode")
        envModeAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
            processor.apvts, paramId, combo);
    else if (paramId == "portaMode")
        portaModeAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
            processor.apvts, paramId, combo);

    // Initial sync
    int idx = (int)*processor.apvts.getRawParameterValue(paramId);
    s.slider.setValue(idx, juce::dontSendNotification);
}

void Mono101Editor::lfoShapeButtonClicked(int index)
{
    lfoShapeCombo.setSelectedItemIndex(index, juce::sendNotificationSync);
}

void Mono101Editor::vcaModeButtonClicked(int index)
{
    vcaModeCombo.setSelectedItemIndex(index, juce::sendNotificationSync);
}

void Mono101Editor::arpButtonClicked(int patternIndex)
{
    Mono101ToggleButton* btns[] = { &arpUpBtn, &arpDownBtn, &arpUDBtn };

    // If clicking the already-lit button, turn arp off
    if (btns[patternIndex]->getToggleState())
    {
        btns[patternIndex]->setToggleState(false, juce::dontSendNotification);
        processor.arpEnabled.store(false);
        processor.arp.stop();
        return;
    }

    // Turn off others, turn on this one
    for (int i = 0; i < 3; ++i)
        btns[i]->setToggleState(i == patternIndex, juce::dontSendNotification);

    // Set pattern on APVTS (which processor reads each block)
    if (auto* param = processor.apvts.getParameter("arpPattern"))
        param->setValueNotifyingHost(param->convertTo0to1((float)patternIndex));

    processor.arpEnabled.store(true);
}

void Mono101Editor::populatePatchDropdown()
{
    pbSelect.clear(juce::dontSendNotification);
    int id = 1;
    // Factory
    for (auto& name : processor.patchBank.getFactoryNames())
        pbSelect.addItem(name, id++);
    // Separator
    pbSelect.addSeparator();
    // Custom
    for (auto& name : processor.patchBank.getCustomNames())
        pbSelect.addItem(name, id++);
}

//==============================================================================
juce::Rectangle<int> Mono101Editor::layoutSection(int x, int y, int w, int h,
                                                  const juce::String& title,
                                                  juce::Colour titleColour)
{
    sections.push_back({ { x, y, w, h }, title, titleColour });
    return { x, y, w, h };
}

void Mono101Editor::paintBrandBar(juce::Graphics& g, juce::Rectangle<int> area)
{
    // Dark background
    auto grad = juce::ColourGradient(juce::Colour(0xff1a1a1c), 0, (float)area.getY(),
                                      juce::Colour(0xff141416), 0, (float)area.getBottom(), false);
    g.setGradientFill(grad);
    g.fillRoundedRectangle(area.toFloat(), 5.0f);
    // Only round top corners - overdraw bottom
    g.fillRect(area.getX(), area.getY() + 5, area.getWidth(), area.getHeight() - 5);

    // Accent stripe at bottom
    g.setColour(juce::Colour(Mono101LookAndFeel::kAccentRed));
    g.fillRect(area.getX(), area.getBottom() - 4, area.getWidth(), 4);

    // Mono-101 text
    g.setFont(juce::Font("Oswald", 26.0f, juce::Font::plain));
    g.setColour(juce::Colour(Mono101LookAndFeel::kLabel));
    g.drawText("Mono-", area.getX() + 22, area.getY() - 1, 70, area.getHeight() - 4,
               juce::Justification::centredLeft, false);
    g.setFont(juce::Font("Oswald", 26.0f, juce::Font::bold));
    g.setColour(juce::Colours::white);
    g.drawText("101", area.getX() + 87, area.getY() - 1, 50, area.getHeight() - 4,
               juce::Justification::centredLeft, false);

    // Synthesizer tagline
    g.setFont(juce::Font("Barlow Condensed", 9.0f, juce::Font::plain));
    g.setColour(juce::Colour(Mono101LookAndFeel::kLabelDim));
    g.drawText("SYNTHESIZER", area.getX() + 145, area.getY(), 100, area.getHeight() - 4,
               juce::Justification::centredLeft, false);

    // Power LED
    float ledX = (float)(area.getRight() - 40);
    float ledY = (float)(area.getCentreY() - 6);
    g.setFont(juce::Font("Barlow Condensed", 8.0f, juce::Font::plain));
    g.setColour(juce::Colour(Mono101LookAndFeel::kLabelDim));
    g.drawText("POWER", (int)ledX - 40, area.getY(), 38, area.getHeight() - 4,
               juce::Justification::centredRight, false);
    // LED on
    g.setColour(juce::Colour(Mono101LookAndFeel::kLedOn));
    g.fillEllipse(ledX, ledY, 9.0f, 9.0f);
    g.setColour(juce::Colour(Mono101LookAndFeel::kLedOn).withAlpha(0.35f));
    g.fillEllipse(ledX - 3, ledY - 3, 15.0f, 15.0f);
}

void Mono101Editor::paintSections(juce::Graphics& g)
{
    for (auto& sec : sections)
    {
        auto b = sec.bounds.toFloat();

        // Background gradient
        auto bgGrad = juce::ColourGradient(juce::Colour(Mono101LookAndFeel::kPanelRaised),
                                            b.getX(), b.getY(),
                                            juce::Colour(Mono101LookAndFeel::kPanel),
                                            b.getX(), b.getBottom(), false);
        g.setGradientFill(bgGrad);
        g.fillRoundedRectangle(b, 3.0f);

        // Subtle inner highlight
        g.setColour(juce::Colours::white.withAlpha(0.04f));
        g.drawHorizontalLine((int)b.getY() + 1, b.getX() + 1, b.getRight() - 1);

        // Border
        g.setColour(juce::Colour(Mono101LookAndFeel::kSectionEdge));
        g.drawRoundedRectangle(b, 3.0f, 1.0f);

        // Title (fieldset legend style)
        if (sec.title.isNotEmpty())
        {
            g.setFont(juce::Font("Barlow Condensed", 8.5f, juce::Font::bold));
            int titleW = g.getCurrentFont().getStringWidth(sec.title) + 12;
            int titleX = sec.bounds.getCentreX() - titleW / 2;
            int titleY = sec.bounds.getY() - 7;

            // Background behind title (erase border)
            g.setColour(juce::Colour(Mono101LookAndFeel::kPanelRaised));
            g.fillRect(titleX, titleY, titleW, 14);

            // Title text
            g.setColour(sec.titleColour);
            g.drawText(sec.title, titleX, titleY, titleW, 14,
                       juce::Justification::centred, false);
        }
    }
}

//==============================================================================
void Mono101Editor::paint(juce::Graphics& g)
{
    // Main background
    g.fillAll(juce::Colour(0xff0e0e10));

    // Synth body background
    auto body = getLocalBounds().reduced(8, 4);
    g.setColour(juce::Colour(Mono101LookAndFeel::kPanel));
    g.fillRoundedRectangle(body.toFloat(), 8.0f);

    // Outer shadow/border
    g.setColour(juce::Colour(0xff111111));
    g.drawRoundedRectangle(body.toFloat(), 8.0f, 1.5f);
    g.setColour(juce::Colour(0xff444444));
    g.drawRoundedRectangle(body.toFloat().reduced(1.5f), 7.0f, 0.5f);

    // Brand bar
    paintBrandBar(g, { body.getX(), body.getY(), body.getWidth(), 46 });

    // Sections
    paintSections(g);

    // Divider lines between sections in VCO (between octave and P.Width, etc.)
    // These are painted as subtle vertical separators
}

void Mono101Editor::resized()
{
    sections.clear();

    const int bodyX = 8;
    const int bodyY = 4;

    // Row 1: Main panel
    const int row1Y = bodyY + 46 + 16;    // after brand bar + padding
    const int secH = 140;                  // section height
    const int sliderW = 28;               // width per slider column
    // const int sliderH = 90;
    const int xsW = 18;                   // xs switch width
    const int xsH = 32;                   // xs switch slider height
    const int secPad = 10;                // internal section padding
    const int secGap = 6;                 // gap between sections

    int x = bodyX + 12;

    // ---- TUNE section ----
    {
        int secW = sliderW + secPad * 2;
        layoutSection(x, row1Y, secW, secH, "TUNE", juce::Colour(Mono101LookAndFeel::kLabel));
        fineTuneSlider.setBounds(x + secPad, row1Y + 14, sliderW, secH - 22);
        x += secW + secGap;
    }

    // ---- LFO section ----
    {
        int btnW = 32;
        int btnH = 16;
        int secW = sliderW + 10 + btnW + secPad * 2;
        layoutSection(x, row1Y, secW, secH, "LFO", juce::Colour(Mono101LookAndFeel::kLedBlue));

        lfoRateSlider.setBounds(x + secPad, row1Y + 14, sliderW, secH - 22);

        // Stack LFO buttons vertically
        int btnX = x + secPad + sliderW + 6;
        int btnY = row1Y + 16;
        lfoSinBtn.setBounds(btnX, btnY, btnW, btnH); btnY += btnH + 2;
        lfoTriBtn.setBounds(btnX, btnY, btnW, btnH); btnY += btnH + 2;
        lfoSqrBtn.setBounds(btnX, btnY, btnW, btnH); btnY += btnH + 2;
        lfoSawBtn.setBounds(btnX, btnY, btnW, btnH); btnY += btnH + 2;
        lfoShBtn.setBounds(btnX, btnY, btnW, btnH);

        x += secW + secGap;
    }

    // ---- VCO section ----
    {
        int secW = sliderW * 3 + xsW + sliderW + secPad * 2 + 16; // Mod, Oct, PW, PMode(xs), PWM
        layoutSection(x, row1Y, secW, secH, "VCO", juce::Colour(Mono101LookAndFeel::kAccentRed));

        int sx = x + secPad;
        int slY = row1Y + 14;
        int fullH = secH - 22;

        vcoModSlider.setBounds(sx, slY, sliderW, fullH); sx += sliderW + 3;
        octaveSlider.setBounds(sx, slY, sliderW, fullH); sx += sliderW + 3;
        pWidthSlider.setBounds(sx, slY, sliderW, fullH); sx += sliderW + 3;

        // XS switch (centered vertically)
        // xs switch centered vertically within section
        pwModeSlider.setBounds(sx, slY, xsW + 4, fullH); sx += xsW + 7;

        pwmSlider.setBounds(sx, slY, sliderW, fullH);

        x += secW + secGap;
    }

    // ---- SOURCE MIXER section ----
    {
        int secW = sliderW * 3 + xsW + sliderW + secPad * 2 + 16;
        layoutSection(x, row1Y, secW, secH, "SOURCE MIXER", juce::Colour(Mono101LookAndFeel::kLabel));

        int sx = x + secPad;
        int slY = row1Y + 14;
        int fullH = secH - 22;

        pulseSlider.setBounds(sx, slY, sliderW, fullH); sx += sliderW + 3;
        sawSlider.setBounds(sx, slY, sliderW, fullH); sx += sliderW + 3;
        subSlider.setBounds(sx, slY, sliderW, fullH); sx += sliderW + 3;

        subModeSlider.setBounds(sx, slY, xsW + 4, fullH); sx += xsW + 7;

        noiseSlider.setBounds(sx, slY, sliderW, fullH);

        x += secW + secGap;
    }

    // ---- VCF section ----
    {
        int secW = sliderW * 5 + secPad * 2 + 12;
        layoutSection(x, row1Y, secW, secH, "VCF", juce::Colour(Mono101LookAndFeel::kAccentRed));

        int sx = x + secPad;
        int slY = row1Y + 14;
        int fullH = secH - 22;

        cutoffSlider.setBounds(sx, slY, sliderW, fullH); sx += sliderW + 3;
        resoSlider.setBounds(sx, slY, sliderW, fullH); sx += sliderW + 3;
        envModSlider.setBounds(sx, slY, sliderW, fullH); sx += sliderW + 3;
        lfoModSlider.setBounds(sx, slY, sliderW, fullH); sx += sliderW + 3;
        keyTrkSlider.setBounds(sx, slY, sliderW, fullH);

        x += secW + secGap;
    }

    // ---- VCA section ----
    {
        int btnW = 38;
        int btnH = 22;
        int secW = btnW + secPad * 2 + 4;
        layoutSection(x, row1Y, secW, secH, "VCA", juce::Colour(Mono101LookAndFeel::kAccentRed));

        int btnX = x + secPad + 2;
        int btnY = row1Y + 30;
        vcaEnvBtn.setBounds(btnX, btnY, btnW, btnH);
        btnY += btnH + 8;
        vcaGateBtn.setBounds(btnX, btnY, btnW, btnH);

        x += secW + secGap;
    }

    // ---- ENVELOPE section ----
    {
        int secW = xsW + 4 + sliderW * 4 + secPad * 2 + 16;
        layoutSection(x, row1Y, secW, secH, "ENVELOPE", juce::Colour(0xffccaa22));

        int sx = x + secPad;
        int slY = row1Y + 14;
        int fullH = secH - 22;

        envModeSlider.setBounds(sx, slY, xsW + 4, fullH); sx += xsW + 7;

        attackSlider.setBounds(sx, slY, sliderW, fullH); sx += sliderW + 3;
        decaySlider.setBounds(sx, slY, sliderW, fullH); sx += sliderW + 3;
        sustainSlider.setBounds(sx, slY, sliderW, fullH); sx += sliderW + 3;
        releaseSlider.setBounds(sx, slY, sliderW, fullH);

        x += secW + secGap;
    }

    // ---- Row 2: ARPEGGIO + PATCH BANK ----
    const int row2Y = row1Y + secH + 10;
    const int row2H = 42;
    int row2X = bodyX + 12;

    // Arpeggio section
    {
        int arpBtnW = 40;
        int arpBtnH = 22;
        int arpSecW = arpBtnW * 3 + 8 + secPad * 2;
        layoutSection(row2X, row2Y, arpSecW, row2H, "ARPEGGIO", juce::Colour(Mono101LookAndFeel::kLabel));

        int bx = row2X + secPad;
        int by = row2Y + 12;
        arpUpBtn.setBounds(bx, by, arpBtnW, arpBtnH);   bx += arpBtnW + 4;
        arpDownBtn.setBounds(bx, by, arpBtnW, arpBtnH); bx += arpBtnW + 4;
        arpUDBtn.setBounds(bx, by, arpBtnW, arpBtnH);

        row2X += arpSecW + secGap;
    }

    // Patch Bank section
    {
        int pbBtnW = 44;
        int pbBtnH = 22;
        int pbSelW = 140;
        int pbSecW = pbBtnW * 4 + pbSelW + 24 + secPad * 2;
        layoutSection(row2X, row2Y, pbSecW, row2H, "PATCH BANK", juce::Colour(Mono101LookAndFeel::kLabel));

        int bx = row2X + secPad;
        int by = row2Y + 12;
        pbSaveBtn.setBounds(bx, by, pbBtnW, pbBtnH);   bx += pbBtnW + 4;
        pbLoadBtn.setBounds(bx, by, pbBtnW, pbBtnH);   bx += pbBtnW + 8;
        pbSelect.setBounds(bx, by, pbSelW, pbBtnH);    bx += pbSelW + 8;
        pbImportBtn.setBounds(bx, by, pbBtnW, pbBtnH); bx += pbBtnW + 4;
        pbExportBtn.setBounds(bx, by, pbBtnW, pbBtnH);

        row2X += pbSecW + secGap;
    }

    // ---- Row 3: PORTAMENTO strip ----
    const int row3Y = row2Y + row2H + 10;
    const int row3H = secH;
    x = bodyX + 12;

    {
        int secW = sliderW * 2 + xsW + 4 + secPad * 2 + 12;
        layoutSection(x, row3Y, secW, row3H, "PORTAMENTO", juce::Colour(Mono101LookAndFeel::kLabel));

        int sx = x + secPad;
        int slY = row3Y + 14;
        int fullH = row3H - 22;

        volumeSlider.setBounds(sx, slY, sliderW, fullH); sx += sliderW + 3;
        portaTimeSlider.setBounds(sx, slY, sliderW, fullH); sx += sliderW + 3;
        portaModeSlider.setBounds(sx, slY, xsW + 4, fullH);
    }

    // Resize window to fit all content
    int totalW = juce::jmax((int)row2X, x) + 20;
    int totalH = row3Y + row3H + 16;
    if (totalW > getWidth() || totalH > getHeight())
        setSize(juce::jmax(totalW, getWidth()), juce::jmax(totalH, getHeight()));
}


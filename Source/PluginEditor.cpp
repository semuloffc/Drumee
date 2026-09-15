#include "PluginEditor.h"

DrumeeAudioProcessorEditor::DrumeeAudioProcessorEditor(DrumeeAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    setLookAndFeel(&lookAndFeel);

    setResizable(false, false);
    setSize(960, 540);

    titleLabel.setText("DRUMEE", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(22.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, DrumeeColours::textPrimary);
    addAndMakeVisible(titleLabel);

    presetBox.setTextWhenNothingSelected("Init Preset");
    presetBox.onChange = [this]
    {
        auto name = presetBox.getText();
        if (name.isNotEmpty())
            processor.presetManager.loadPreset(name);
    };
    addAndMakeVisible(presetBox);

    saveButton.onClick = [this] { savePresetDialog(); };
    addAndMakeVisible(saveButton);

    newButton.onClick = [this]
    {
        for (auto& step : processor.sequencer.pattern)
            for (int t = 0; t < kNumTracks; ++t)
                step.active[t] = false;
    };
    addAndMakeVisible(newButton);

    for (auto* label : { &sectionTiming, &sectionPitch, &sectionChaos })
    {
        label->setJustificationType(juce::Justification::centredLeft);
        label->setFont(juce::Font(12.0f, juce::Font::bold));
        label->setColour(juce::Label::textColourId, DrumeeColours::textSecondary);
        addAndMakeVisible(label);
    }

    for (auto& info : getAllParamInfo())
    {
        auto encoder = std::make_unique<Encoder>(processor.apvts, info);
        addAndMakeVisible(*encoder);
        encoders.push_back(std::move(encoder));
    }

    visualizer = std::make_unique<StepSequencerVisualizer>(processor.sequencer);
    addAndMakeVisible(*visualizer);

    for (int i = 0; i < kNumTracks; ++i)
    {
        auto slot = std::make_unique<SampleSlotComponent>(i, processor.tracks[(size_t) i]);
        slot->onLoadRequested = [this](int trackIndex) { loadSample(trackIndex); };
        addAndMakeVisible(*slot);
        sampleSlots.push_back(std::move(slot));
    }

    refreshPresetList();
}

DrumeeAudioProcessorEditor::~DrumeeAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void DrumeeAudioProcessorEditor::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    juce::ColourGradient gradient(DrumeeColours::surface2.brighter(0.05f), bounds.getX(), bounds.getY(),
                                   DrumeeColours::background.darker(0.35f), bounds.getX(), bounds.getBottom(), false);
    gradient.addColour(0.45, DrumeeColours::background);
    g.setGradientFill(gradient);
    g.fillAll();

    juce::Random glassRandom(1234);
    g.setColour(juce::Colours::white.withAlpha(0.015f));
    for (int i = 0; i < 40; ++i)
    {
        float rx = glassRandom.nextFloat() * bounds.getWidth();
        float ry = glassRandom.nextFloat() * bounds.getHeight() * 0.6f;
        float r = 40.0f + glassRandom.nextFloat() * 90.0f;
        g.fillEllipse(rx - r * 0.5f, ry - r * 0.5f, r, r);
    }

    g.setColour(DrumeeColours::surface1.withAlpha(0.35f));
    g.fillRoundedRectangle(bounds.removeFromTop(56.0f).reduced(8.0f), 12.0f);
}

void DrumeeAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced(16);

    auto topBar = bounds.removeFromTop(48);
    titleLabel.setBounds(topBar.removeFromLeft(160));
    newButton.setBounds(topBar.removeFromRight(60).reduced(2));
    topBar.removeFromRight(6);
    saveButton.setBounds(topBar.removeFromRight(70).reduced(2));
    topBar.removeFromRight(6);
    presetBox.setBounds(topBar.removeFromRight(180).reduced(2));

    bounds.removeFromTop(10);

    auto bottomBar = bounds.removeFromBottom(74);
    int slotWidth = bottomBar.getWidth() / kNumTracks;
    for (auto& slot : sampleSlots)
    {
        slot->setBounds(bottomBar.removeFromLeft(slotWidth).reduced(4));
    }

    bounds.removeFromBottom(10);

    auto leftColumn = bounds.removeFromLeft(170);
    auto rightColumn = bounds.removeFromRight(220);
    bounds.removeFromLeft(10);
    bounds.removeFromRight(10);

    std::vector<Encoder*> timingEncoders, pitchEncoders, chaosEncoders;
    auto& infoArray = getAllParamInfo();
    for (size_t i = 0; i < encoders.size(); ++i)
    {
        switch (infoArray[i].group)
        {
            case AccentGroup::timing:       timingEncoders.push_back(encoders[i].get()); break;
            case AccentGroup::pitchSound:   pitchEncoders.push_back(encoders[i].get()); break;
            case AccentGroup::ratchetChaos: chaosEncoders.push_back(encoders[i].get()); break;
        }
    }

    sectionTiming.setBounds(leftColumn.removeFromTop(20));
    leftColumn.removeFromTop(4);
    for (auto* enc : timingEncoders)
    {
        enc->setBounds(leftColumn.removeFromTop(110));
        leftColumn.removeFromTop(8);
    }

    sectionPitch.setBounds(rightColumn.removeFromTop(20));
    rightColumn.removeFromTop(4);
    auto pitchGrid = rightColumn.removeFromTop(220);
    int pitchColWidth = pitchGrid.getWidth() / 2;
    int pitchRowHeight = pitchGrid.getHeight() / 2;
    for (size_t i = 0; i < pitchEncoders.size(); ++i)
    {
        int row = (int) i / 2;
        int col = (int) i % 2;
        juce::Rectangle<int> cell(pitchGrid.getX() + col * pitchColWidth,
                                   pitchGrid.getY() + row * pitchRowHeight,
                                   pitchColWidth, pitchRowHeight);
        pitchEncoders[i]->setBounds(cell.reduced(4));
    }

    rightColumn.removeFromTop(6);
    sectionChaos.setBounds(rightColumn.removeFromTop(20));
    rightColumn.removeFromTop(4);
    int chaosColWidth = rightColumn.getWidth() / 2;
    for (auto* enc : chaosEncoders)
        enc->setBounds(rightColumn.removeFromLeft(chaosColWidth).reduced(4));

    visualizer->setBounds(bounds);
}

void DrumeeAudioProcessorEditor::refreshPresetList()
{
    presetBox.clear();
    auto names = processor.presetManager.getAllPresetNames();
    int id = 1;
    for (auto& name : names)
        presetBox.addItem(name, id++);
}

void DrumeeAudioProcessorEditor::savePresetDialog()
{
    auto* alert = new juce::AlertWindow("Save Preset", "Enter preset name:", juce::AlertWindow::NoIcon);
    alert->addTextEditor("name", processor.presetManager.getCurrentPresetName());
    alert->addButton("Save", 1, juce::KeyPress(juce::KeyPress::returnKey));
    alert->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

    alert->enterModalState(true, juce::ModalCallbackFunction::create([this, alert](int result)
    {
        if (result == 1)
        {
            auto name = alert->getTextEditorContents("name");
            processor.presetManager.savePreset(name);
            refreshPresetList();
        }
        delete alert;
    }));
}

void DrumeeAudioProcessorEditor::loadSample(int trackIndex)
{
    fileChooser = std::make_unique<juce::FileChooser>("Select a sample",
                                                        juce::File::getSpecialLocation(juce::File::userHomeDirectory),
                                                        "*.wav;*.aiff;*.mp3;*.flac");

    auto flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;

    fileChooser->launchAsync(flags, [this, trackIndex](const juce::FileChooser& chooser)
    {
        auto file = chooser.getResult();
        if (file.existsAsFile())
        {
            processor.loadSampleForTrack(trackIndex, file);
            if (trackIndex >= 0 && trackIndex < (int) sampleSlots.size())
                sampleSlots[(size_t) trackIndex]->refresh();
        }
    });
}

#include "GUI.h"

DrumeeLookAndFeel::DrumeeLookAndFeel()
{
    setColour(juce::ResizableWindow::backgroundColourId, DrumeeColours::background);
    setColour(juce::Label::textColourId, DrumeeColours::textPrimary);
    setColour(juce::TextButton::buttonColourId, DrumeeColours::surface1);
    setColour(juce::TextButton::textColourOffId, DrumeeColours::textPrimary);
    setColour(juce::ComboBox::backgroundColourId, DrumeeColours::surface1);
    setColour(juce::ComboBox::textColourId, DrumeeColours::textPrimary);
    setColour(juce::ComboBox::outlineColourId, DrumeeColours::secondary);
    setColour(juce::PopupMenu::backgroundColourId, DrumeeColours::surface2);
    setColour(juce::PopupMenu::textColourId, DrumeeColours::textPrimary);
}

void DrumeeLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                          float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                                          juce::Slider& slider)
{
    auto bounds = juce::Rectangle<float>((float) x, (float) y, (float) width, (float) height).reduced(6.0f);
    auto centre = bounds.getCentre();
    float radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
    float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    juce::Colour fillColour = slider.findColour(juce::Slider::rotarySliderFillColourId);

    g.setColour(DrumeeColours::surface2);
    juce::Path track;
    track.addCentredArc(centre.x, centre.y, radius, radius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.strokePath(track, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    juce::Path valueArc;
    valueArc.addCentredArc(centre.x, centre.y, radius, radius, 0.0f, rotaryStartAngle, angle, true);
    g.setColour(fillColour);
    g.strokePath(valueArc, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    float knobRadius = radius * 0.62f;
    g.setColour(DrumeeColours::surface1);
    g.fillEllipse(centre.x - knobRadius, centre.y - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f);
    g.setColour(fillColour.withAlpha(0.35f));
    g.drawEllipse(centre.x - knobRadius, centre.y - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f, 1.2f);

    juce::Point<float> tip(centre.x + std::sin(angle) * knobRadius * 0.82f,
                            centre.y - std::cos(angle) * knobRadius * 0.82f);
    g.setColour(fillColour);
    g.drawLine({ centre, tip }, 2.6f);
    g.fillEllipse(tip.x - 2.5f, tip.y - 2.5f, 5.0f, 5.0f);
}

void DrumeeLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour&,
                                              bool isHighlighted, bool isDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced(1.0f);
    juce::Colour base = DrumeeColours::surface1;
    if (isDown)
        base = DrumeeColours::surface2;
    else if (isHighlighted)
        base = base.brighter(0.08f);

    g.setColour(base);
    g.fillRoundedRectangle(bounds, 8.0f);
    g.setColour(DrumeeColours::secondary.withAlpha(0.35f));
    g.drawRoundedRectangle(bounds, 8.0f, 1.0f);
}

juce::Font DrumeeLookAndFeel::getLabelFont(juce::Label& label)
{
    return juce::Font(label.getFont().getHeight(), juce::Font::plain);
}

Encoder::Encoder(juce::AudioProcessorValueTreeState& state, const ParamInfo& info)
{
    accentColour = DrumeeColours::forGroup(info.group);

    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 16);
    slider.setColour(juce::Slider::rotarySliderFillColourId, accentColour);
    slider.setColour(juce::Slider::rotarySliderOutlineColourId, DrumeeColours::surface2);
    slider.setColour(juce::Slider::textBoxTextColourId, DrumeeColours::textSecondary);
    slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(slider);

    nameLabel.setText(info.label, juce::dontSendNotification);
    nameLabel.setJustificationType(juce::Justification::centred);
    nameLabel.setColour(juce::Label::textColourId, DrumeeColours::textPrimary);
    nameLabel.setFont(juce::Font(13.0f, juce::Font::bold));
    addAndMakeVisible(nameLabel);

    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(state, info.id, slider);
}

void Encoder::resized()
{
    auto bounds = getLocalBounds();
    nameLabel.setBounds(bounds.removeFromTop(18));
    slider.setBounds(bounds);
}

void Encoder::paint(juce::Graphics&) {}

StepSequencerVisualizer::StepSequencerVisualizer(Sequencer& sequencerToUse) : sequencer(sequencerToUse)
{
    startTimerHz(30);
}

StepSequencerVisualizer::~StepSequencerVisualizer() { stopTimer(); }

juce::Rectangle<float> StepSequencerVisualizer::getCellBounds(int track, int step) const
{
    auto area = getLocalBounds().toFloat().reduced(14.0f);
    float rowHeight = area.getHeight() / (float) kNumTracks;
    float colWidth = area.getWidth() / (float) kNumSteps;
    return { area.getX() + (float) step * colWidth, area.getY() + (float) track * rowHeight, colWidth, rowHeight };
}

void StepSequencerVisualizer::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    g.setColour(DrumeeColours::surface1.withAlpha(0.55f));
    g.fillRoundedRectangle(bounds, 16.0f);
    g.setColour(DrumeeColours::secondary.withAlpha(0.18f));
    g.drawRoundedRectangle(bounds.reduced(0.5f), 16.0f, 1.0f);

    int playStep = sequencer.currentStep.load();
    static const juce::Colour trackColours[kNumTracks] = {
        DrumeeColours::accent1, DrumeeColours::accent3, DrumeeColours::accent2,
        DrumeeColours::accent1, DrumeeColours::accent3
    };

    for (int step = 0; step < kNumSteps; ++step)
    {
        bool isPlayColumn = (step == playStep);

        if (isPlayColumn)
        {
            auto colBounds = getCellBounds(0, step);
            colBounds.setHeight(getLocalBounds().toFloat().getHeight() - 28.0f);
            g.setColour(DrumeeColours::accent3.withAlpha(0.10f));
            g.fillRoundedRectangle(colBounds, 6.0f);
        }

        for (int track = 0; track < kNumTracks; ++track)
        {
            auto cell = getCellBounds(track, step).reduced(3.0f);
            bool active = sequencer.pattern[(size_t) step].active[track];
            float vel = sequencer.pattern[(size_t) step].velocity[track];

            juce::Colour cellColour = active ? trackColours[track] : DrumeeColours::surface2;
            float alpha = active ? juce::jmap(vel, 0.4f, 1.0f) : 0.5f;

            if (isPlayColumn && active)
            {
                g.setColour(cellColour.withAlpha(0.9f));
                g.fillEllipse(cell.expanded(2.0f));
            }

            g.setColour(cellColour.withAlpha(alpha));
            float cornerSize = juce::jmin(cell.getWidth(), cell.getHeight()) * 0.5f;
            g.fillRoundedRectangle(cell, cornerSize);

            if (! active)
            {
                g.setColour(DrumeeColours::secondary.withAlpha(0.25f));
                g.drawRoundedRectangle(cell, cornerSize, 1.0f);
            }
        }
    }
}

void StepSequencerVisualizer::mouseDown(const juce::MouseEvent& event)
{
    auto area = getLocalBounds().toFloat().reduced(14.0f);
    if (! area.contains(event.position))
        return;

    float rowHeight = area.getHeight() / (float) kNumTracks;
    float colWidth = area.getWidth() / (float) kNumSteps;

    int track = (int) ((event.position.y - area.getY()) / rowHeight);
    int step = (int) ((event.position.x - area.getX()) / colWidth);

    track = juce::jlimit(0, kNumTracks - 1, track);
    step = juce::jlimit(0, kNumSteps - 1, step);

    bool& active = sequencer.pattern[(size_t) step].active[track];
    active = ! active;
    if (active)
        sequencer.pattern[(size_t) step].velocity[track] = 0.85f;

    repaint();
}

void StepSequencerVisualizer::timerCallback()
{
    int step = sequencer.currentStep.load();
    if (step != lastPaintedStep)
    {
        lastPaintedStep = step;
        repaint();
    }
}

SampleSlotComponent::SampleSlotComponent(int trackIndex, SampleTrack& trackToUse)
    : index(trackIndex), track(trackToUse)
{
    nameLabel.setJustificationType(juce::Justification::centredLeft);
    nameLabel.setFont(juce::Font(14.0f, juce::Font::bold));
    nameLabel.setColour(juce::Label::textColourId, DrumeeColours::textPrimary);
    addAndMakeVisible(nameLabel);

    statusLabel.setJustificationType(juce::Justification::centredLeft);
    statusLabel.setFont(juce::Font(11.0f, juce::Font::plain));
    statusLabel.setColour(juce::Label::textColourId, DrumeeColours::textMuted);
    addAndMakeVisible(statusLabel);

    loadButton.setButtonText("Load");
    loadButton.onClick = [this] { if (onLoadRequested) onLoadRequested(index); };
    addAndMakeVisible(loadButton);

    refresh();
}

void SampleSlotComponent::resized()
{
    auto bounds = getLocalBounds().reduced(8);
    loadButton.setBounds(bounds.removeFromRight(64).reduced(0, 8));
    bounds.removeFromRight(6);
    nameLabel.setBounds(bounds.removeFromTop(bounds.getHeight() / 2));
    statusLabel.setBounds(bounds);
}

void SampleSlotComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    static const juce::Colour accents[3] = { DrumeeColours::accent1, DrumeeColours::accent2, DrumeeColours::accent3 };
    juce::Colour accent = accents[index % 3];

    g.setColour(isDragHover ? DrumeeColours::surface2.brighter(0.1f) : DrumeeColours::surface1.withAlpha(0.7f));
    g.fillRoundedRectangle(bounds, 10.0f);
    g.setColour((isDragHover ? accent : accent.withAlpha(0.55f)));
    g.drawRoundedRectangle(bounds.reduced(0.75f), 10.0f, isDragHover ? 2.0f : 1.4f);
}

void SampleSlotComponent::refresh()
{
    nameLabel.setText(track.name, juce::dontSendNotification);
    statusLabel.setText(track.loaded ? track.sourceFile.getFileName() : "Empty slot",
                         juce::dontSendNotification);
}

bool SampleSlotComponent::isAcceptableFile(const juce::File& file) const
{
    static const juce::StringArray extensions { ".wav", ".wave", ".aif", ".aiff", ".flac", ".ogg", ".mp3" };
    return extensions.contains(file.getFileExtension().toLowerCase());
}

bool SampleSlotComponent::isInterestedInFileDrag(const juce::StringArray& files)
{
    for (auto& path : files)
        if (isAcceptableFile(juce::File(path)))
            return true;
    return false;
}

void SampleSlotComponent::fileDragEnter(const juce::StringArray&, int, int)
{
    isDragHover = true;
    repaint();
}

void SampleSlotComponent::fileDragExit(const juce::StringArray&)
{
    isDragHover = false;
    repaint();
}

void SampleSlotComponent::filesDropped(const juce::StringArray& files, int, int)
{
    isDragHover = false;

    for (auto& path : files)
    {
        juce::File file(path);
        if (isAcceptableFile(file) && file.existsAsFile())
        {
            if (onFileDropped)
                onFileDropped(index, file);
            break;
        }
    }

    repaint();
}

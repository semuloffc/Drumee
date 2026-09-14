#pragma once
#include <JuceHeader.h>
#include "DSP.h"
#include "Parameters.h"

namespace DrumeeColours
{
    static const juce::Colour background   { 0xFF1B1E24 };
    static const juce::Colour surface1     { 0xFF2C323B };
    static const juce::Colour surface2     { 0xFF34384A };
    static const juce::Colour secondary    { 0xFFC4C8D4 };

    static const juce::Colour accent1      { 0xFFD89A8A };
    static const juce::Colour accent2      { 0xFFA3B19B };
    static const juce::Colour accent3      { 0xFFE5BB94 };

    static const juce::Colour textPrimary  { 0xFFF6F4EF };
    static const juce::Colour textSecondary{ 0xFFC4C8D4 };
    static const juce::Colour textMuted    { 0xFF788194 };
    static const juce::Colour textInverse  { 0xFF1B1E24 };

    inline juce::Colour forGroup(AccentGroup group)
    {
        switch (group)
        {
            case AccentGroup::timing:       return accent1;
            case AccentGroup::pitchSound:   return accent3;
            case AccentGroup::ratchetChaos: return accent2;
        }
        return accent1;
    }
}

class DrumeeLookAndFeel : public juce::LookAndFeel_V4
{
public:
    DrumeeLookAndFeel();

    void drawRotarySlider(juce::Graphics&, int x, int y, int width, int height,
                           float sliderPosProportional, float rotaryStartAngle,
                           float rotaryEndAngle, juce::Slider&) override;

    void drawButtonBackground(juce::Graphics&, juce::Button&, const juce::Colour& backgroundColour,
                               bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

    juce::Font getLabelFont(juce::Label&) override;
};

class Encoder : public juce::Component
{
public:
    Encoder(juce::AudioProcessorValueTreeState& state, const ParamInfo& info);

    void resized() override;
    void paint(juce::Graphics&) override;

private:
    juce::Slider slider;
    juce::Label nameLabel;
    juce::Colour accentColour;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
};

class StepSequencerVisualizer : public juce::Component, private juce::Timer
{
public:
    explicit StepSequencerVisualizer(Sequencer& sequencerToUse);
    ~StepSequencerVisualizer() override;

    void paint(juce::Graphics&) override;
    void mouseDown(const juce::MouseEvent&) override;

private:
    void timerCallback() override;
    juce::Rectangle<float> getCellBounds(int track, int step) const;

    Sequencer& sequencer;
    int lastPaintedStep = -1;
};

class SampleSlotComponent : public juce::Component
{
public:
    SampleSlotComponent(int trackIndex, SampleTrack& trackToUse);

    void resized() override;
    void paint(juce::Graphics&) override;
    void refresh();

    std::function<void(int)> onLoadRequested;

private:
    int index;
    SampleTrack& track;
    juce::TextButton loadButton;
    juce::Label nameLabel;
    juce::Label statusLabel;
};

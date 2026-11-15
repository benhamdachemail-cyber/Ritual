#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include <array>

class RitualAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                         private juce::Timer
{
public:
    explicit RitualAudioProcessorEditor(RitualAudioProcessor&);
    ~RitualAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    struct RitualLookAndFeel : juce::LookAndFeel_V4
    {
        void drawRotarySlider(juce::Graphics&, int, int, int, int, float,
                              float, float, juce::Slider&) override;
    } knobLook;
    void timerCallback() override { repaint(juce::Rectangle<int>(155, 115, 380, 270)); }
    RitualAudioProcessor& processor;
    std::array<juce::Slider, 6> sliders;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>, 6> attachments;
    std::array<juce::String, 6> labels {{ "REVERSE", "HALF TIME", "TRIPLET",
                                         "PHASE SHIFT", "FLUX", "APERTURE" }};
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RitualAudioProcessorEditor)
};

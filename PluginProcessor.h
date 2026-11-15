#pragma once
#include <JuceHeader.h>
#include <array>
#include <atomic>
#include <vector>

class RitualAudioProcessor final : public juce::AudioProcessor
{
public:
    RitualAudioProcessor();
    ~RitualAudioProcessor() override = default;
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "Ritual"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 2.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;

    juce::AudioProcessorValueTreeState parameters;
    std::atomic<float> level { 0.0f };
    static juce::AudioProcessorValueTreeState::ParameterLayout makeLayout();

private:
    static constexpr int channels = 2;
    std::array<std::vector<float>, channels> history;
    std::array<float, channels> lowpass { 0, 0 };
    std::array<float, channels> phaseMemory { 0, 0 };
    int ringSize = 0, writeIndex = 0;
    double sr = 44100.0, clock = 0.0, lastBpm = 120.0;
    float previousLevel = 0.0f;
    float readHistory(int channel, float delaySamples) const noexcept;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RitualAudioProcessor)
};

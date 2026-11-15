#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

namespace
{
    constexpr float pi = juce::MathConstants<float>::pi;
    float equalPower(float a, float b, float amount)
    {
        amount = juce::jlimit(0.0f, 1.0f, amount);
        return a * std::cos(amount * pi * 0.5f) + b * std::sin(amount * pi * 0.5f);
    }
}

RitualAudioProcessor::RitualAudioProcessor()
    : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                                      .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "RitualState", makeLayout())
{}

juce::AudioProcessorValueTreeState::ParameterLayout RitualAudioProcessor::makeLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout result;
    for (const auto& p : std::array<std::pair<const char*, const char*>, 6> {{
         {"reverse", "Reverse"}, {"halftime", "Half Time"}, {"triplet", "Triplet"},
{"phase", "Phase Shift"}, {"flux", "Flux"}, {"aperture", "Aperture"}}})
    {
        result.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID { p.first, 1 }, p.second,
            juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.0f));
    }
    return result;
}

void RitualAudioProcessor::prepareToPlay(double sampleRate, int)
{
    sr = juce::jmax(8000.0, sampleRate);
    ringSize = (int) std::ceil(sr * 6.0) + 8;
    for (auto& channel : history)
        channel.assign((size_t) ringSize, 0.0f);
    lowpass.fill(0.0f);
    phaseMemory.fill(0.0f);
    clock = 0.0;
    writeIndex = 0;
    previousLevel = 0.0f;
    level.store(0.0f);
}

void RitualAudioProcessor::releaseResources() {}

float RitualAudioProcessor::readHistory(int channel, float delaySamples) const noexcept
{
    if (ringSize < 8) return 0.0f;
    delaySamples = juce::jlimit(1.0f, (float) ringSize - 3.0f, delaySamples);
    float position = (float) writeIndex - delaySamples;
    while (position < 0.0f) position += (float) ringSize;
    const int a = (int) position % ringSize;
    const int b = (a + 1) % ringSize;
    const float f = position - std::floor(position);
    const auto& h = history[(size_t) channel];
    return h[(size_t) a] * (1.0f - f) + h[(size_t) b] * f;
}

void RitualAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int count = buffer.getNumSamples();
    const int nCh = juce::jmin(channels, buffer.getNumChannels());
    if (nCh == 0 || ringSize < 8) return;
    for (int ch = nCh; ch < buffer.getNumChannels(); ++ch) buffer.clear(ch, 0, count);
    const float reverse = parameters.getRawParameterValue("reverse")->load();
    const float half = parameters.getRawParameterValue("halftime")->load();
    const float triplet = parameters.getRawParameterValue("triplet")->load();
    const float phase = parameters.getRawParameterValue("phase")->load();
    const float flux = parameters.getRawParameterValue("flux")->load();
    const float aperture = parameters.getRawParameterValue("aperture")->load();

    // DAW tempo: triplet subdivisions follow FL Studio's tempo whenever supplied.
    double bpm = lastBpm;
    if (auto* playHead = getPlayHead())
    {
        if (auto position = playHead->getPosition())
            if (auto tempo = position->getBpm())
                if (std::isfinite(*tempo) && *tempo >= 20.0 && *tempo <= 400.0) bpm = *tempo;
    }
    lastBpm = bpm;
    const double beat = sr * 60.0 / bpm;
    const float grainSamples = (float) juce::jlimit(sr * 0.10, sr * 0.65, beat);
    const float rate = 1.0f - 0.5f * half - 2.0f * reverse;
    const float temporalBlend = juce::jlimit(0.0f, 1.0f, juce::jmax(half, reverse));
    const float tripletSize = (float) (beat / 3.0);
    const float phaseLfoHz = 0.12f + 2.5f * flux;
    float maxLevel = 0.0f;

    for (int i = 0; i < count; ++i)
    {
        const double t = clock + (double) i;
        const float positionA = (float) std::fmod(t, grainSamples);
        const float positionB = (float) std::fmod(t + grainSamples * 0.5, grainSamples);
        const float wA = std::sin(pi * positionA / grainSamples);
        const float wB = std::sin(pi * positionB / grainSamples);
        const float norm = 1.0f / juce::jmax(0.0001f, wA + wB);
        const float stutterPhase = (float) std::fmod(t, (double) tripletSize) / tripletSize;
        const float stutterEnvelope = 0.15f + 0.85f * (1.0f - juce::jlimit(0.0f, 1.0f, (stutterPhase - 0.68f) / 0.32f));
        const float modulation = std::sin((float) (2.0 * juce::MathConstants<double>::pi * phaseLfoHz * t / sr));

        for (int ch = 0; ch < nCh; ++ch)
        {
            const float input = buffer.getSample(ch, i);
            history[(size_t) ch][(size_t) writeIndex] = input;
            const float baseline = grainSamples * 2.3f;
            const float delayA = baseline + (1.0f - rate) * positionA;
            const float delayB = baseline + (1.0f - rate) * positionB;
            const float a = readHistory(ch, delayA);
            const float b = readHistory(ch, delayB);
            float wet = (a * wA + b * wB) * norm;
            float result = equalPower(input, wet, temporalBlend);

            // Triplet: tempo-synced buffer repeat with a short musical gate.
            const float stutter = readHistory(ch, (float) (tripletSize * 1.10) +
                stutterPhase * tripletSize * (0.70f + 0.25f * flux));
            result = equalPower(result, stutter * stutterEnvelope, triplet);

            // Phase Shift: moving, short stereo-offset comb; Flux changes its rate/depth.
            const float side = ch == 0 ? 1.0f : -1.0f;
            const float modDelay = (float) (sr * (0.0025 + 0.004 * flux * (1.0 + modulation * side)));
            const float phaser = result - 0.55f * readHistory(ch, modDelay);
            phaseMemory[(size_t) ch] = phaseMemory[(size_t) ch] * 0.90f + phaser * 0.10f;
            result = equalPower(result, phaseMemory[(size_t) ch], phase);

            // Aperture: from warm/dark to open/airy with tasteful saturation.
            const float cutoff = 650.0f + 14500.0f * aperture;
            const float alpha = 1.0f - std::exp(-2.0f * pi * cutoff / (float) sr);
            lowpass[(size_t) ch] += alpha * (result - lowpass[(size_t) ch]);
            const float textured = std::tanh(lowpass[(size_t) ch] * (1.0f + 0.25f * aperture));
            result = equalPower(result, textured, aperture * 0.75f);
            buffer.setSample(ch, i, result);
            maxLevel = juce::jmax(maxLevel, std::abs(result));
        }
        writeIndex = (writeIndex + 1) % ringSize;
    }
    clock += (double) count;
    previousLevel = 0.75f * previousLevel + 0.25f * maxLevel;
    level.store(previousLevel, std::memory_order_relaxed);
}

void RitualAudioProcessor::getStateInformation(juce::MemoryBlock& data)
{
    if (auto xml = parameters.copyState().createXml()) copyXmlToBinary(*xml, data);
}
void RitualAudioProcessor::setStateInformation(const void* data, int size)
{
    if (auto xml = getXmlFromBinary(data, size))
        if (xml->hasTagName(parameters.state.getType()))
            parameters.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RitualAudioProcessor();
    }

juce::AudioProcessorEditor* RitualAudioProcessor::createEditor()
{
    return new RitualAudioProcessorEditor(*this);
}


#include "PluginEditor.h"
#include <cmath>

namespace
{
    const juce::Colour shell(0xffe8aaa7);
    const juce::Colour pale(0xfff6c5b8);
    const juce::Colour ink(0xff162b32);
    const juce::Colour screen(0xff10252c);
    const juce::Colour cyan(0xff34e2d1);
    const juce::Colour pink(0xfff18bca);
}

void RitualAudioProcessorEditor::RitualLookAndFeel::drawRotarySlider(
    juce::Graphics& g, int x, int y, int width, int height, float position,
    float startAngle, float endAngle, juce::Slider&)
{
    const auto bounds = juce::Rectangle<float>((float)x, (float)y, (float)width, (float)height);
    const auto centre = bounds.getCentre();
    const float radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.34f;
    const auto ring = juce::Rectangle<float>(centre.x-radius-6, centre.y-radius-6,
                                             (radius+6)*2, (radius+6)*2);
    g.setColour(juce::Colours::black.withAlpha(0.13f));
    g.fillEllipse(ring.translated(4, 5));
    g.setColour(juce::Colour(0xffa96f72));
    g.fillEllipse(ring);
    g.setColour(pale);
    g.drawEllipse(ring.reduced(1.5f), 1.2f);
    const auto face = ring.reduced(5.0f);
    juce::ColourGradient metal(juce::Colour(0xff66757b), face.getX(), face.getY(),
                               juce::Colour(0xff0b1c23), face.getRight(), face.getBottom(), false);
    metal.addColour(0.47, juce::Colour(0xff38444b));
    g.setGradientFill(metal);
    g.fillEllipse(face);
    g.setColour(juce::Colour(0xff90a5a5).withAlpha(0.55f));
    g.drawEllipse(face.reduced(1), 1.5f);
    const float angle = startAngle + (endAngle-startAngle)*position;
    const juce::Point<float> marker(centre.x + radius*0.64f*std::sin(angle),
                                     centre.y - radius*0.64f*std::cos(angle));
    g.setColour(cyan);
    g.drawLine(centre.x, centre.y, marker.x, marker.y, 3.0f);
    g.setColour(ink.withAlpha(0.55f));
    juce::Path arc;
    arc.addCentredArc(centre.x, centre.y, radius+10, radius+10, 0,
                        startAngle, endAngle, true);
    g.strokePath(arc, juce::PathStrokeType(1.6f));
    juce::Path progress;
    progress.addCentredArc(centre.x, centre.y, radius+10, radius+10, 0,
                             startAngle, angle, true);
    g.setColour(cyan);
    g.strokePath(progress, juce::PathStrokeType(3.0f));
}

RitualAudioProcessorEditor::RitualAudioProcessorEditor(RitualAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    const std::array<const char*, 6> ids {{"reverse", "halftime", "triplet", "phase", "flux", "aperture"}};
    for (size_t i = 0; i < sliders.size(); ++i)
    {
        auto& s = sliders[i];
        s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        s.setRotaryParameters(juce::MathConstants<float>::pi*1.25f,
                              juce::MathConstants<float>::pi*2.75f, true);
        s.setLookAndFeel(&knobLook);
        s.setDoubleClickReturnValue(true, 0.0);
        addAndMakeVisible(s);
        attachments[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            processor.parameters, ids[i], s);
    }
    setSize(690, 790);
    setResizable(false, false);
    startTimerHz(30);
}

RitualAudioProcessorEditor::~RitualAudioProcessorEditor()
{
    stopTimer();
    for (auto& slider : sliders) slider.setLookAndFeel(nullptr);
}

void RitualAudioProcessorEditor::resized()
{
    // Three large lower-face knobs, three complementary upper-side knobs.
    const std::array<juce::Rectangle<int>, 6> spots {{
        { 91, 490, 135, 135 }, { 278, 490, 135, 135 }, { 465, 490, 135, 135 },
        { 28, 184, 136, 136 }, { 526, 184, 136, 136 }, { 278, 638, 135, 135 }
    }};
    for (size_t i=0; i<spots.size(); ++i) sliders[i].setBounds(spots[i]);
}

void RitualAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff679c9b));
    auto body = juce::Rectangle<float>(14, 13, 662, 764);
    g.setColour(juce::Colours::black.withAlpha(0.22f));
    g.fillRoundedRectangle(body.translated(4, 5), 29);
    juce::ColourGradient bodyGradient(pale, 14, 30, shell, 670, 760, false);
    g.setGradientFill(bodyGradient);
    g.fillRoundedRectangle(body, 25);
    g.setColour(juce::Colour(0xffad7977));
    g.drawRoundedRectangle(body.reduced(3.0f), 22, 2);
    g.setColour(juce::Colour(0xffffdbca).withAlpha(0.7f));
    g.drawRoundedRectangle(body.reduced(7.0f), 19, 1);

    g.setColour(ink);
    g.setFont(juce::Font(juce::FontOptions(13.0f).withStyle("bold")));
    g.drawFittedText("R I T U A L", 0, 42, 690, 20, juce::Justification::centred, 1);
    g.setFont(juce::Font(juce::FontOptions(9.0f)));
    g.drawFittedText("AUDIO TRANSFORMATION DEVICE  /  V 1.0", 0, 64, 690, 20, juce::Justification::centred, 1);

    const auto outerScreen = juce::Rectangle<float>(167, 115, 356, 283);
    g.setColour(juce::Colour(0xffaa777a));
    g.fillRoundedRectangle(outerScreen.expanded(6), 20);
    g.setColour(pale);
    g.drawRoundedRectangle(outerScreen.expanded(4), 18, 2);
    g.setColour(screen);
    g.fillRoundedRectangle(outerScreen, 14);
    g.setColour(cyan.withAlpha(0.32f));
    g.drawRoundedRectangle(outerScreen.reduced(5), 11, 0.8f);

    g.setColour(pink);
    g.setFont(juce::Font(juce::FontOptions(12.0f).withStyle("bold")));
    g.drawText("R I T U A L", 197, 137, 260, 20, juce::Justification::centred);
    g.setColour(cyan.withAlpha(0.85f));
    g.setFont(juce::Font(juce::FontOptions(9.0f)));
    g.drawText("REVERSE    /    TIME    /    SPACE", 190, 161, 310, 18, juce::Justification::centred);

    const auto loudness = processor.level.load(std::memory_order_relaxed);
    const float signal = juce::jlimit(0.0f, 1.0f, loudness * 2.5f);
    g.setColour(cyan.withAlpha(0.16f));
    for (int j=0; j<4; ++j) g.drawHorizontalLine(205+j*33, 202.0f, 488.0f);
    juce::Path waveform;
    const int bars = 69;
    for (int b=0; b<bars; ++b)
    {
        const float x = 198.0f + (float)b*4.26f;
        const float envelope = 0.10f + 0.90f*std::pow(std::abs(std::sin((float)b*0.23f)), 6.0f);
        const float bulge = 0.35f + 0.65f*std::exp(-std::pow(((float)b-34.0f)/9.5f, 2.0f));
        const float amplitude = (6.0f + signal*61.0f)*envelope*bulge;
        waveform.startNewSubPath(x, 263-amplitude);
        waveform.lineTo(x, 263+amplitude);
    }
    g.setColour(cyan);
    g.strokePath(waveform, juce::PathStrokeType(2.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    juce::Path crescent;
    crescent.addCentredArc(345, 264, 116, 93, 0, -2.4f, 2.4f, true);
    g.setColour(cyan.withAlpha(0.9f));
    g.strokePath(crescent, juce::PathStrokeType(13.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour(pink.withAlpha(0.85f));
    juce::Path highArc;
    highArc.addCentredArc(345, 264, 116, 93, 0, -2.25f, -0.6f, true);
    g.strokePath(highArc, juce::PathStrokeType(3.0f));

    // Black inset strip beneath the screen, inspired by the supplied reference.
    g.setColour(screen);
    g.fillRoundedRectangle(76, 419, 538, 48, 13);
    g.setColour(cyan.withAlpha(0.65f));
    g.drawText("[  REVERSE  ]     [  HALF TIME  ]     [  TRIPLET  ]", 91, 434, 508, 20, juce::Justification::centred);

    g.setColour(ink);
    g.setFont(juce::Font(juce::FontOptions(11.5f).withStyle("bold")));
    const std::array<juce::Rectangle<int>, 6> texts {{
        { 100, 614, 116, 19 }, { 288, 614, 115, 19 }, { 476, 614, 115, 19 },
        { 42, 320, 108, 20 }, { 542, 320, 108, 20 }, { 292, 743, 108, 19 }
    }};
    for (size_t i=0; i<texts.size(); ++i)
        g.drawFittedText(labels[i], texts[i], juce::Justification::centred, 1);

    g.setFont(juce::Font(juce::FontOptions(9.5f)));
    g.drawText("TEMPORAL ENGINE", 65, 475, 560, 16, juce::Justification::centred);
    g.drawText("SPACE / TEXTURE", 60, 346, 100, 17, juce::Justification::centred);
    g.drawText("MODULATION", 534, 346, 112, 17, juce::Justification::centred);
    g.setColour(ink.withAlpha(0.50f));
    g.drawText("RITUAL AUDIO   /   MADE FOR AFTER DARK", 154, 760, 382, 14, juce::Justification::centred);
}

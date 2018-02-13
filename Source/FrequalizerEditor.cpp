/*
  ==============================================================================

    This is the Frequalizer UI editor implementation

  ==============================================================================
*/

#include "FrequalizerProcessor.h"
#include "FrequalizerEditor.h"


//==============================================================================
FrequalizerAudioProcessorEditor::FrequalizerAudioProcessorEditor (FrequalizerAudioProcessor& p)
  : AudioProcessorEditor (&p), processor (p),
    output (Slider::RotaryHorizontalVerticalDrag, Slider::TextBoxBelow)
{

    for (int i=0; i < FrequalizerAudioProcessor::numBands; ++i) {
        auto* bandEditor = bandEditors.add (new BandEditor (i, processor));
        addAndMakeVisible (bandEditor);
    }

    frame.setText (TRANS ("Output"));
    frame.setTextLabelPosition (Justification::centred);
    addAndMakeVisible (frame);
    addAndMakeVisible (output);
    attachments.add (new AudioProcessorValueTreeState::SliderAttachment (processor.getPluginState(), FrequalizerAudioProcessor::paramOutput, output));

    setResizable (true, false);
    setSize (840, 500);
    processor.addChangeListener (this);
}

FrequalizerAudioProcessorEditor::~FrequalizerAudioProcessorEditor()
{
    processor.removeChangeListener (this);
}

//==============================================================================
void FrequalizerAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));

    g.setColour (Colours::white);
    g.setFont (15.0f);

    auto bounds = getLocalBounds();
    for (int i=0; i < FrequalizerAudioProcessor::numBands; ++i) {
        auto* band = bandEditors.getUnchecked (i);
        g.setColour (processor.getBandColour (i));
        g.strokePath (band->frequencyResponse, PathStrokeType (1.0));
    }
    g.setColour (Colours::silver);
    g.strokePath (frequencyResponse, PathStrokeType (1.0));

    Image logo = ImageCache::getFromMemory (FFAudioData::LogoFF_png, FFAudioData::LogoFF_pngSize);
    g.drawImageWithin (logo, branding.getX(), branding.getY(), branding.getWidth(), branding.getHeight(),
                       RectanglePlacement (RectanglePlacement::fillDestination));
}

void FrequalizerAudioProcessorEditor::resized()
{
    plotFrame = getLocalBounds().reduced (3, 3);

    auto bandSpace = plotFrame.removeFromBottom (getHeight() / 2);
    float width = static_cast<float> (bandSpace.getWidth()) / (bandEditors.size() + 1);
    for (auto* band : bandEditors)
        band->setBounds (bandSpace.removeFromLeft (width));

    frame.setBounds (bandSpace.removeFromTop (bandSpace.getHeight() / 2.0));
    output.setBounds (frame.getBounds().reduced (8));

    plotFrame.reduce (3, 3);
    branding = bandSpace.reduced (5);

    for (int i=0; i < bandEditors.size(); ++i) {
        auto* band = bandEditors.getUnchecked (i);
        band->frequencyResponse.clear();
        processor.createFrequencyPlot (band->frequencyResponse, i, plotFrame);
    }
    frequencyResponse.clear();
    processor.createFrequencyPlot (frequencyResponse, plotFrame);
}

void FrequalizerAudioProcessorEditor::changeListenerCallback (ChangeBroadcaster* sender)
{
    ignoreUnused (sender);
    for (int i=0; i < bandEditors.size(); ++i) {
        auto* band = bandEditors.getUnchecked (i);
        band->updateControls (processor.getFilterType (i));
        band->frequencyResponse.clear();
        processor.createFrequencyPlot (band->frequencyResponse, i, plotFrame);
    }
    frequencyResponse.clear();
    processor.createFrequencyPlot (frequencyResponse, plotFrame);
    repaint();
}


//==============================================================================
FrequalizerAudioProcessorEditor::BandEditor::BandEditor (const int i, FrequalizerAudioProcessor& processor)
  : index (i),
    frequency (Slider::RotaryHorizontalVerticalDrag, Slider::TextBoxBelow),
    quality   (Slider::RotaryHorizontalVerticalDrag, Slider::TextBoxBelow),
    gain      (Slider::RotaryHorizontalVerticalDrag, Slider::TextBoxBelow)
{
    frame.setText (processor.getBandName (index));
    frame.setTextLabelPosition (Justification::centred);
    frame.setColour (GroupComponent::textColourId, processor.getBandColour (index));
    frame.setColour (GroupComponent::outlineColourId, processor.getBandColour (index));
    addAndMakeVisible (frame);

    for (int i=0; i < FrequalizerAudioProcessor::LastFilterID; ++i)
        filterType.addItem (FrequalizerAudioProcessor::getFilterTypeName (static_cast<FrequalizerAudioProcessor::FilterType> (i)), i + 1);

    addAndMakeVisible (filterType);
    boxAttachments.add (new AudioProcessorValueTreeState::ComboBoxAttachment (processor.getPluginState(), processor.getTypeParamName (index), filterType));

    addAndMakeVisible (frequency);
    attachments.add (new AudioProcessorValueTreeState::SliderAttachment (processor.getPluginState(), processor.getFrequencyParamName (index), frequency));
    frequency.setSkewFactorFromMidPoint (1000.0);

    addAndMakeVisible (quality);
    attachments.add (new AudioProcessorValueTreeState::SliderAttachment (processor.getPluginState(), processor.getQualityParamName (index), quality));
    quality.setSkewFactorFromMidPoint (1.0);

    addAndMakeVisible (gain);
    attachments.add (new AudioProcessorValueTreeState::SliderAttachment (processor.getPluginState(), processor.getGainParamName (index), gain));
    gain.setSkewFactorFromMidPoint (1.0);

    updateControls (processor.getFilterType (index));
}

void FrequalizerAudioProcessorEditor::BandEditor::resized ()
{
    auto bounds = getLocalBounds();
    frame.setBounds (bounds);

    bounds.reduce (10, 20);

    filterType.setBounds (bounds.removeFromTop (20));

    frequency.setBounds (bounds.removeFromBottom (bounds.getHeight() * 2 / 3));
    quality.setBounds (bounds.removeFromLeft (bounds.getWidth() / 2));
    gain.setBounds (bounds);
}

void FrequalizerAudioProcessorEditor::BandEditor::updateControls (FrequalizerAudioProcessor::FilterType type)
{
    switch (type) {
        case FrequalizerAudioProcessor::LowPass:
            frequency.setEnabled (true); quality.setEnabled (false); gain.setEnabled (false);
            break;
        case FrequalizerAudioProcessor::LowPass1st:
            frequency.setEnabled (true); quality.setEnabled (false); gain.setEnabled (false);
            break;
        case FrequalizerAudioProcessor::LowShelf:
            frequency.setEnabled (true); quality.setEnabled (false); gain.setEnabled (true);
            break;
        case FrequalizerAudioProcessor::BandPass:
            frequency.setEnabled (true); quality.setEnabled (true); gain.setEnabled (false);
            break;
        case FrequalizerAudioProcessor::AllPass:
            frequency.setEnabled (true); quality.setEnabled (false); gain.setEnabled (false);
            break;
        case FrequalizerAudioProcessor::AllPass1st:
            frequency.setEnabled (true); quality.setEnabled (false); gain.setEnabled (false);
            break;
        case FrequalizerAudioProcessor::Notch:
            frequency.setEnabled (true); quality.setEnabled (true); gain.setEnabled (false);
            break;
        case FrequalizerAudioProcessor::Peak:
            frequency.setEnabled (true); quality.setEnabled (true); gain.setEnabled (true);
            break;
        case FrequalizerAudioProcessor::HighShelf:
            frequency.setEnabled (true); quality.setEnabled (true); gain.setEnabled (true);
            break;
        case FrequalizerAudioProcessor::HighPass1st:
            frequency.setEnabled (true); quality.setEnabled (false); gain.setEnabled (false);
            break;
        case FrequalizerAudioProcessor::HighPass:
            frequency.setEnabled (true); quality.setEnabled (true); gain.setEnabled (false);
            break;
        default:
            frequency.setEnabled (true);
            quality.setEnabled (true);
            gain.setEnabled (true);
            break;
    }
}

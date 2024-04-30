#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent() {
    addAndMakeVisible(openButton);
    openButton.setButtonText("Open...");
    openButton.onClick = [this] {openButtonClicked();};

    addAndMakeVisible(clearButton);
    clearButton.setButtonText("Clear");
    clearButton.onClick = [this] {clearButtonClicked();};

    // Make sure you set the size of the component after
    // you add any child components.
    setSize (300, 200);

    formatManager.registerBasicFormats();
}

MainComponent::~MainComponent() {
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate) {
    
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) {
    auto numInputChannels = fileBuffer.getNumChannels();
    auto numOutputChannels = bufferToFill.buffer->getNumChannels();

    auto outputSamplesRemaining = bufferToFill.numSamples;
    auto outputSamplesOffset = bufferToFill.startSample;

    while (outputSamplesRemaining > 0) {
        auto bufferSamplesRemaining = fileBuffer.getNumSamples() - position;
        auto samplesThisTime = juce::jmin(outputSamplesRemaining, bufferSamplesRemaining);

        for (auto channel = 0; channel < numOutputChannels; ++channel) {
            bufferToFill.buffer->copyFrom(channel, outputSamplesOffset, fileBuffer, channel % numInputChannels, position, samplesThisTime);
        }

        outputSamplesRemaining -= samplesThisTime;
        outputSamplesOffset += samplesThisTime;
        position += samplesThisTime;

        if (position == fileBuffer.getNumSamples()) {
            position = 0;
        }
    }
}

void MainComponent::releaseResources() {
    // reset the buffer
    fileBuffer.setSize(0, 0);
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g) {
    // fill black
    g.fillAll(juce::Colours::black);
}

void MainComponent::resized() {
    openButton.setBounds(10, 10, getWidth() - 20, 20);
    clearButton.setBounds(10, 40, getWidth() - 20, 20);
}

void MainComponent::openButtonClicked() {
    shutdownAudio();

    chooser = std::make_unique<juce::FileChooser>("Select a Wave file shorter than 2 seconds to play...", juce::File{}, "*.wav");
    auto chooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;

    chooser->launchAsync(chooserFlags, [this](const juce::FileChooser& fc) {
        auto file = fc.getResult();

        if (file == juce::File{}) {
            return;
        }

        std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));

        if (reader.get() != nullptr) {
            auto duration = (float)reader->lengthInSamples / reader->sampleRate;

            if (duration < 2) {
                fileBuffer.setSize((int)reader->lengthInSamples, (int)reader->lengthInSamples);
                reader->read(&fileBuffer, 0, (int)reader->lengthInSamples, 0, true, true);
                position = 0;
                setAudioChannels(0, (int)reader->numChannels);
            } else {
                // handle the error that the file is 2 seconds or longer
            }
        }
    });
}

void MainComponent::clearButtonClicked() {
    shutdownAudio();
}

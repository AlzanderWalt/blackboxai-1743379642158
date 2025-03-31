#include "AudioUtils.h"
#include "Logger.h"

float AudioUtils::dbToGain(float db) {
    return std::pow(10.0f, db * 0.05f);
}

float AudioUtils::gainToDb(float gain) {
    return 20.0f * std::log10(std::max(gain, DENORMAL_PREVENTION));
}

float AudioUtils::velocityToGain(int velocity) {
    return velocity / 127.0f;
}

float AudioUtils::calculatePeakLevel(const float* data, int numSamples) {
    float peak = 0.0f;
    for (int i = 0; i < numSamples; ++i) {
        peak = std::max(peak, std::abs(data[i]));
    }
    return peak;
}

float AudioUtils::calculateRMSLevel(const float* data, int numSamples) {
    float sum = 0.0f;
    for (int i = 0; i < numSamples; ++i) {
        sum += data[i] * data[i];
    }
    return std::sqrt(sum / numSamples);
}

void AudioUtils::calculateLevels(const float* data, int numSamples,
                               float& peak, float& rms) {
    peak = 0.0f;
    float sumSquares = 0.0f;
    
    for (int i = 0; i < numSamples; ++i) {
        const float sample = std::abs(data[i]);
        peak = std::max(peak, sample);
        sumSquares += sample * sample;
    }
    
    rms = std::sqrt(sumSquares / numSamples);
}

void AudioUtils::applyGain(juce::AudioBuffer<float>& buffer, float gain) {
    buffer.applyGain(gain);
}

void AudioUtils::applyGainRamp(juce::AudioBuffer<float>& buffer,
                              int startSample, int numSamples,
                              float startGain, float endGain) {
    const float gainStep = (endGain - startGain) / numSamples;
    
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        float* data = buffer.getWritePointer(channel, startSample);
        float currentGain = startGain;
        
        for (int i = 0; i < numSamples; ++i) {
            data[i] *= currentGain;
            currentGain += gainStep;
        }
    }
}

void AudioUtils::copyWithGain(const juce::AudioBuffer<float>& source,
                            juce::AudioBuffer<float>& destination,
                            float gain) {
    const int numChannels = std::min(source.getNumChannels(),
                                   destination.getNumChannels());
    const int numSamples = std::min(source.getNumSamples(),
                                  destination.getNumSamples());
    
    for (int channel = 0; channel < numChannels; ++channel) {
        destination.copyFrom(channel, 0, source, channel, 0, numSamples);
    }
    
    if (gain != 1.0f) {
        destination.applyGain(gain);
    }
}

void AudioUtils::mixBuffers(const juce::AudioBuffer<float>& source,
                          juce::AudioBuffer<float>& destination,
                          float gain) {
    const int numChannels = std::min(source.getNumChannels(),
                                   destination.getNumChannels());
    const int numSamples = std::min(source.getNumSamples(),
                                  destination.getNumSamples());
    
    for (int channel = 0; channel < numChannels; ++channel) {
        destination.addFrom(channel, 0, source, channel, 0, numSamples, gain);
    }
}

float AudioUtils::panToGain(float pan, bool leftChannel) {
    // Constant power panning
    const float angle = (leftChannel ? (1.0f - pan) : (1.0f + pan)) * HALF_PI * 0.5f;
    return std::cos(angle);
}

juce::Array<float> AudioUtils::calculatePanLaw(int numSteps) {
    juce::Array<float> law;
    law.resize(numSteps);
    
    for (int i = 0; i < numSteps; ++i) {
        const float pan = (i / static_cast<float>(numSteps - 1)) * 2.0f - 1.0f;
        law.set(i, panToGain(pan, true));
    }
    
    return law;
}

void AudioUtils::resampleBuffer(const juce::AudioBuffer<float>& source,
                              double sourceSampleRate,
                              juce::AudioBuffer<float>& destination,
                              double targetSampleRate) {
    const double ratio = targetSampleRate / sourceSampleRate;
    
    // Simple linear interpolation for now
    // TODO: Implement better resampling algorithm
    for (int channel = 0; channel < destination.getNumChannels(); ++channel) {
        const float* srcData = source.getReadPointer(channel);
        float* dstData = destination.getWritePointer(channel);
        
        for (int i = 0; i < destination.getNumSamples(); ++i) {
            const double srcPos = i / ratio;
            const int srcIndex = static_cast<int>(srcPos);
            const float alpha = static_cast<float>(srcPos - srcIndex);
            
            if (srcIndex + 1 < source.getNumSamples()) {
                dstData[i] = srcData[srcIndex] * (1.0f - alpha) +
                            srcData[srcIndex + 1] * alpha;
            } else {
                dstData[i] = srcData[srcIndex];
            }
        }
    }
}

void AudioUtils::floatToInt16(const float* source, int16_t* destination,
                             int numSamples, float gain) {
    for (int i = 0; i < numSamples; ++i) {
        const float sample = source[i] * gain;
        destination[i] = static_cast<int16_t>(juce::jlimit(-32768.0f, 32767.0f,
                                                          sample * 32768.0f));
    }
}

void AudioUtils::floatToInt24(const float* source, uint8_t* destination,
                             int numSamples, float gain) {
    for (int i = 0; i < numSamples; ++i) {
        const float sample = source[i] * gain;
        const int32_t value = static_cast<int32_t>(juce::jlimit(-8388608.0f, 8388607.0f,
                                                                sample * 8388608.0f));
        int32ToInt24Bytes(value, destination + i * 3);
    }
}

void AudioUtils::floatToInt32(const float* source, int32_t* destination,
                             int numSamples, float gain) {
    for (int i = 0; i < numSamples; ++i) {
        const float sample = source[i] * gain;
        destination[i] = float32ToInt32(sample);
    }
}

void AudioUtils::int16ToFloat(const int16_t* source, float* destination,
                             int numSamples) {
    const float scale = 1.0f / 32768.0f;
    for (int i = 0; i < numSamples; ++i) {
        destination[i] = source[i] * scale;
    }
}

void AudioUtils::int24ToFloat(const uint8_t* source, float* destination,
                             int numSamples) {
    const float scale = 1.0f / 8388608.0f;
    for (int i = 0; i < numSamples; ++i) {
        const int32_t value = int24BytesToInt32(source + i * 3);
        destination[i] = value * scale;
    }
}

void AudioUtils::int32ToFloat(const int32_t* source, float* destination,
                             int numSamples) {
    for (int i = 0; i < numSamples; ++i) {
        destination[i] = int32ToFloat32(source[i]);
    }
}

void AudioUtils::applyTriangularDither(float* data, int numSamples,
                                     int bitDepth) {
    const float quantizationStep = std::pow(2.0f, static_cast<float>(-bitDepth));
    
    for (int i = 0; i < numSamples; ++i) {
        data[i] += generateTriangularDither() * quantizationStep;
        data[i] = quantize(data[i], bitDepth);
    }
}

void AudioUtils::applyNoiseShaping(float* data, int numSamples,
                                  int bitDepth) {
    // Simple 2nd order noise shaping
    static float error1 = 0.0f;
    static float error2 = 0.0f;
    
    const float quantizationStep = std::pow(2.0f, static_cast<float>(-bitDepth));
    
    for (int i = 0; i < numSamples; ++i) {
        const float input = data[i] + error1 * 1.5f - error2 * 0.5f;
        const float quantized = quantize(input, bitDepth);
        const float error = input - quantized;
        
        error2 = error1;
        error1 = error;
        data[i] = quantized;
    }
}

void AudioUtils::removeDCOffset(float* data, int numSamples) {
    // Calculate mean
    float sum = 0.0f;
    for (int i = 0; i < numSamples; ++i) {
        sum += data[i];
    }
    const float mean = sum / numSamples;
    
    // Remove DC offset
    for (int i = 0; i < numSamples; ++i) {
        data[i] -= mean;
    }
}

void AudioUtils::removeDCOffset(juce::AudioBuffer<float>& buffer) {
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        removeDCOffset(buffer.getWritePointer(channel),
                      buffer.getNumSamples());
    }
}

float AudioUtils::getNormalizationGain(const juce::AudioBuffer<float>& buffer,
                                     float targetLevel) {
    float maxLevel = 0.0f;
    
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        maxLevel = std::max(maxLevel,
                          calculatePeakLevel(buffer.getReadPointer(channel),
                                          buffer.getNumSamples()));
    }
    
    return maxLevel > 0.0f ? targetLevel / maxLevel : 1.0f;
}

void AudioUtils::normalizeBuffer(juce::AudioBuffer<float>& buffer,
                               float targetLevel) {
    const float gain = getNormalizationGain(buffer, targetLevel);
    buffer.applyGain(gain);
}

void AudioUtils::applyFadeIn(float* data, int numSamples,
                            FadeShape shape) {
    for (int i = 0; i < numSamples; ++i) {
        const float position = static_cast<float>(i) / numSamples;
        data[i] *= calculateFadeGain(position, shape);
    }
}

void AudioUtils::applyFadeOut(float* data, int numSamples,
                             FadeShape shape) {
    for (int i = 0; i < numSamples; ++i) {
        const float position = static_cast<float>(i) / numSamples;
        data[i] *= calculateFadeGain(1.0f - position, shape);
    }
}

void AudioUtils::applyCrossfade(float* data1, float* data2,
                               int numSamples,
                               FadeShape shape) {
    for (int i = 0; i < numSamples; ++i) {
        const float position = static_cast<float>(i) / numSamples;
        const float gain1 = calculateFadeGain(1.0f - position, shape);
        const float gain2 = calculateFadeGain(position, shape);
        
        const float sample = data1[i] * gain1 + data2[i] * gain2;
        data1[i] = sample;
    }
}

int AudioUtils::findNextZeroCrossing(const float* data, int startSample,
                                   int numSamples) {
    for (int i = startSample + 1; i < numSamples; ++i) {
        if (data[i - 1] <= 0.0f && data[i] > 0.0f) {
            return i;
        }
    }
    return -1;
}

int AudioUtils::findPreviousZeroCrossing(const float* data, int startSample,
                                       int numSamples) {
    for (int i = startSample - 1; i > 0; --i) {
        if (data[i - 1] <= 0.0f && data[i] > 0.0f) {
            return i;
        }
    }
    return -1;
}

float AudioUtils::midiNoteToFrequency(int noteNumber) {
    return 440.0f * std::pow(2.0f, (noteNumber - 69) / 12.0f);
}

int AudioUtils::frequencyToMidiNote(float frequency) {
    return static_cast<int>(std::round(69 + 12 * std::log2(frequency / 440.0f)));
}

float AudioUtils::midiNoteToFrequency(float noteNumber) {
    return 440.0f * std::pow(2.0f, (noteNumber - 69.0f) / 12.0f);
}

double AudioUtils::beatsToSeconds(double beats, double tempo) {
    return (beats * 60.0) / tempo;
}

double AudioUtils::secondsToBeats(double seconds, double tempo) {
    return (seconds * tempo) / 60.0;
}

int64_t AudioUtils::beatsToSamples(double beats, double tempo,
                                  double sampleRate) {
    return static_cast<int64_t>(beatsToSeconds(beats, tempo) * sampleRate);
}

double AudioUtils::samplesToBeats(int64_t samples, double tempo,
                                double sampleRate) {
    return secondsToBeats(samples / sampleRate, tempo);
}

// Private utility functions

int32_t AudioUtils::float32ToInt32(float sample) {
    const float scaled = sample * 2147483648.0f;
    return static_cast<int32_t>(juce::jlimit(-2147483648.0f, 2147483647.0f, scaled));
}

float AudioUtils::int32ToFloat32(int32_t sample) {
    return sample / 2147483648.0f;
}

void AudioUtils::int32ToInt24Bytes(int32_t source, uint8_t* destination) {
    destination[0] = static_cast<uint8_t>(source & 0xFF);
    destination[1] = static_cast<uint8_t>((source >> 8) & 0xFF);
    destination[2] = static_cast<uint8_t>((source >> 16) & 0xFF);
}

int32_t AudioUtils::int24BytesToInt32(const uint8_t* source) {
    int32_t result = (source[2] << 16) | (source[1] << 8) | source[0];
    if (result & 0x800000) {
        result |= 0xFF000000;
    }
    return result;
}

float AudioUtils::generateTriangularDither() {
    // Generate two uniform random numbers between -1 and 1
    const float r1 = (static_cast<float>(rand()) / RAND_MAX) * 2.0f - 1.0f;
    const float r2 = (static_cast<float>(rand()) / RAND_MAX) * 2.0f - 1.0f;
    return (r1 + r2) * 0.5f;
}

float AudioUtils::quantize(float sample, int bitDepth) {
    const float scale = std::pow(2.0f, static_cast<float>(bitDepth - 1));
    return std::round(sample * scale) / scale;
}

float AudioUtils::calculateFadeGain(float position, FadeShape shape) {
    switch (shape) {
        case FadeShape::Linear:
            return position;
            
        case FadeShape::QuarterSine:
            return std::sin(position * HALF_PI);
            
        case FadeShape::HalfSine:
            return (1.0f - std::cos(position * PI)) * 0.5f;
            
        case FadeShape::Logarithmic:
            return std::pow(position, 2.0f);
            
        case FadeShape::Exponential:
            return std::sqrt(position);
            
        case FadeShape::SCurve:
            return (1.0f - std::cos(position * PI)) * 0.5f;
            
        default:
            return position;
    }
}
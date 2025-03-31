#pragma once
#include <JuceHeader.h>

class AudioUtils {
public:
    // Level conversion
    static float dbToGain(float db);
    static float gainToDb(float gain);
    static float velocityToGain(int velocity);
    
    // Peak and RMS measurement
    static float calculatePeakLevel(const float* data, int numSamples);
    static float calculateRMSLevel(const float* data, int numSamples);
    static void calculateLevels(const float* data, int numSamples,
                              float& peak, float& rms);
    
    // Buffer operations
    static void applyGain(juce::AudioBuffer<float>& buffer, float gain);
    static void applyGainRamp(juce::AudioBuffer<float>& buffer,
                             int startSample, int numSamples,
                             float startGain, float endGain);
    
    static void copyWithGain(const juce::AudioBuffer<float>& source,
                           juce::AudioBuffer<float>& destination,
                           float gain = 1.0f);
                           
    static void mixBuffers(const juce::AudioBuffer<float>& source,
                         juce::AudioBuffer<float>& destination,
                         float gain = 1.0f);
    
    // Pan law
    static float panToGain(float pan, bool leftChannel);
    static juce::Array<float> calculatePanLaw(int numSteps);
    
    // Sample rate conversion
    static void resampleBuffer(const juce::AudioBuffer<float>& source,
                             double sourceSampleRate,
                             juce::AudioBuffer<float>& destination,
                             double targetSampleRate);
    
    // Format conversion
    static void floatToInt16(const float* source, int16_t* destination,
                            int numSamples, float gain = 1.0f);
    static void floatToInt24(const float* source, uint8_t* destination,
                            int numSamples, float gain = 1.0f);
    static void floatToInt32(const float* source, int32_t* destination,
                            int numSamples, float gain = 1.0f);
    
    static void int16ToFloat(const int16_t* source, float* destination,
                            int numSamples);
    static void int24ToFloat(const uint8_t* source, float* destination,
                            int numSamples);
    static void int32ToFloat(const int32_t* source, float* destination,
                            int numSamples);
    
    // Dithering
    static void applyTriangularDither(float* data, int numSamples,
                                    int bitDepth);
    static void applyNoiseShaping(float* data, int numSamples,
                                 int bitDepth);
    
    // DC offset removal
    static void removeDCOffset(float* data, int numSamples);
    static void removeDCOffset(juce::AudioBuffer<float>& buffer);
    
    // Normalization
    static float getNormalizationGain(const juce::AudioBuffer<float>& buffer,
                                    float targetLevel = 1.0f);
    static void normalizeBuffer(juce::AudioBuffer<float>& buffer,
                              float targetLevel = 1.0f);
    
    // Fades
    enum class FadeShape {
        Linear,
        QuarterSine,
        HalfSine,
        Logarithmic,
        Exponential,
        SCurve
    };
    
    static void applyFadeIn(float* data, int numSamples,
                           FadeShape shape = FadeShape::Linear);
    static void applyFadeOut(float* data, int numSamples,
                            FadeShape shape = FadeShape::Linear);
    static void applyCrossfade(float* data1, float* data2,
                              int numSamples,
                              FadeShape shape = FadeShape::Linear);
    
    // Zero crossing detection
    static int findNextZeroCrossing(const float* data, int startSample,
                                  int numSamples);
    static int findPreviousZeroCrossing(const float* data, int startSample,
                                      int numSamples);
    
    // MIDI note frequency conversion
    static float midiNoteToFrequency(int noteNumber);
    static int frequencyToMidiNote(float frequency);
    static float midiNoteToFrequency(float noteNumber);  // For pitch bend
    
    // Time and tempo conversion
    static double beatsToSeconds(double beats, double tempo);
    static double secondsToBeats(double seconds, double tempo);
    static int64_t beatsToSamples(double beats, double tempo,
                                 double sampleRate);
    static double samplesToBeats(int64_t samples, double tempo,
                               double sampleRate);

private:
    // Utility functions for format conversion
    static int32_t float32ToInt32(float sample);
    static float int32ToFloat32(int32_t sample);
    static void int32ToInt24Bytes(int32_t source, uint8_t* destination);
    static int32_t int24BytesToInt32(const uint8_t* source);
    
    // Dithering utilities
    static float generateTriangularDither();
    static float quantize(float sample, int bitDepth);
    
    // Fade shape calculations
    static float calculateFadeGain(float position, FadeShape shape);
    
    // Make constructor private to prevent instantiation
    AudioUtils() = delete;
    
    // Constants
    static constexpr float PI = 3.14159265358979323846f;
    static constexpr float HALF_PI = PI * 0.5f;
    static constexpr float TWO_PI = PI * 2.0f;
    static constexpr float LOG_10 = 2.30258509299404568402f;
    static constexpr float LOG_2 = 0.69314718055994530942f;
    
    static constexpr float MIN_GAIN_DB = -144.0f;
    static constexpr float MAX_GAIN_DB = 24.0f;
    static constexpr float MIN_FREQUENCY = 20.0f;
    static constexpr float MAX_FREQUENCY = 20000.0f;
    
    static constexpr float DENORMAL_PREVENTION = 1e-15f;
};
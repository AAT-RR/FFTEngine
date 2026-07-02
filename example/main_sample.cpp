/**
 * @file main_sample.cpp
 * @brief Example showcasing how to dynamically load and invoke the Audio Frequency Analysis Engine DLL
 */

#include <iostream>
#include <vector>
#include <cmath>
#include <windows.h>
#include <iomanip>

// Function pointer mapping matching the exported C interface signature
typedef void (*ProcessAudioFunc)(
    const double*, int, int, int, int, int, double, int, double, double*, double*, double*
);

int main() {
    std::cout << "=== Audio Analysis Engine DLL Client Demo ===" << std::endl;

    // 1. Explicitly load the DLL library at runtime
    HMODULE hEngine = LoadLibraryA("FFTEngine.dll");
    if (!hEngine) {
        std::cerr << "[Error] Failed to load FFTEngine.dll. Ensure the file is in the workspace path." << std::endl;
        return -1;
    }

    // 2. Map the procedure address
    ProcessAudioFunc process_audio = (ProcessAudioFunc)GetProcAddress(hEngine, "process_audio_aat_final");
    if (!process_audio) {
        std::cerr << "[Error] Function 'process_audio_aat_final' could not be resolved from DLL." << std::endl;
        FreeLibrary(hEngine);
        return -1;
    }

    // 3. Configure processing constants
    const int sample_rate = 48000;
    const int nfft = 2048;
    const int hop = 512;
    const int total_samples = sample_rate * 2; // 2 seconds of mock audio

    // 4. Generate mock audio input (a mixture of 1kHz and 5kHz sine waves)
    std::vector<double> mock_audio(total_samples);
    for (int i = 0; i < total_samples; ++i) {
        double t = (double)i / sample_rate;
        mock_audio[i] = 0.6 * std::sin(2.0 * 3.1415926535 * 1000.0 * t) + 
                        0.4 * std::sin(2.0 * 3.1415926535 * 5000.0 * t);
    }

    // 5. Allocate buffers for frequency spectrum outputs
    const int out_bins = nfft / 2 + 1;
    std::vector<double> frequencies(out_bins);
    std::vector<double> raw_db(out_bins);
    std::vector<double> smoothed_db(out_bins);

    // 6. Execute processing pipeline through the DLL
    std::cout << "-> Executing audio processing via library binding..." << std::endl;
    
    int window_type = 1;     // Hamming window
    double smooth_oct = 12.0; // 1/12 Fractional Octave Smoothing
    int enable_a_weight = 1; // Apply A-Weighting filtering
    double manual_gain = 0.0; // 0 dB calibration gain

    process_audio(
        mock_audio.data(), total_samples, sample_rate, nfft, hop, window_type,
        smooth_oct, enable_a_weight, manual_gain,
        frequencies.data(), raw_db.data(), smoothed_db.data()
    );

    std::cout << "-> Processing finished successfully. Sample results layout:" << std::endl;
    std::cout << std::setw(15) << "Freq (Hz)" 
              << std::setw(15) << "Raw (dB)" 
              << std::setw(15) << "Smoothed+A (dB)" << std::endl;
    std::cout << "--------------------------------------------------------" << std::endl;

    // Display sample frequency bin outputs across the spectrum band
    int points_to_show[] = { 0, 10, 42, 43, 213, 214, 500 };
    for (int idx : points_to_show) {
        if (idx < out_bins) {
            std::cout << std::setw(15) << std::fixed << std::setprecision(2) << frequencies[idx]
                      << std::setw(15) << raw_db[idx]
                      << std::setw(15) << smoothed_db[idx] << std::endl;
        }
    }

    // 7. Unload modules and free system resources
    FreeLibrary(hEngine);
    std::cout << "=== Resource deallocated. Execution terminated ===" << std::endl;
    return 0;
}
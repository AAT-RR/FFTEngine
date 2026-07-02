/**
 * @file FFTEngine.h
 * @brief Industrial Audio & Acoustic Frequency Analysis Engine DLL Interface
 */

#ifndef FFT_ENGINE_H
#define FFT_ENGINE_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef BUILDING_DLL
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT __declspec(dllimport)
#endif

/**
 * @brief Core Audio Signal Processing Pipeline
 * @param audio_in        Pointer to the input time-domain audio buffer (mono continuous data).
 * @param total_len       Total length of the input audio buffer (number of samples).
 * @param fs              Sampling rate of the audio (e.g., 44100, 48000).
 * @param nfft            FFT window size (must be a power of 2, e.g., 1024, 2048, 65536).
 * @param hop             Hop size / Frame stride between consecutive windows (e.g., nfft / 4).
 * @param win_type        Window function type: 0=Hann, 1=Hamming, 2=Blackman, 4=Blackman-Harris, others=Rectangular.
 * @param smooth_frac     Fractional octave smoothing factor (e.g., 12.0 for 1/12 octave). Disabled if < 1.0.
 * @param mode            Weighting filter mode: 1 = Enable A-Weighting, 0 = Linear/None.
 * @param gain_db         Calibration offset or manual gain adjustment in decibels (dB).
 * @param freqs_out       [Output] Array to store center frequencies of FFT bins. Size: (nfft / 2) + 1.
 * @param raw_db_out      [Output] Array to store raw spectrum in dB. Size: (nfft / 2) + 1.
 * @param smooth_db_out   [Output] Array to store processed/smoothed spectrum in dB. Size: (nfft / 2) + 1.
 */
DLL_EXPORT void process_audio_aat_final(
    const double* audio_in, int total_len, int fs, int nfft, int hop, int win_type,
    double smooth_frac, int mode, double gain_db,
    double* freqs_out, double* raw_db_out, double* smooth_db_out
);

#ifdef __cplusplus
}
#endif

#endif // FFT_ENGINE_H
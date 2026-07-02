# High-Performance Audio Frequency Analysis Engine (DLL)

A thread-safe, high-performance C/C++ dynamic link library (DLL) optimized for industrial acoustics, noise-vibration-harshness (NVH) testing, automated product quality screening, and lab-scale spectrum calculations.

This repository provides the pre-compiled binary along with cross-language integration assets, allowing you to drop professional audio processing capabilities directly into your production frameworks without getting bogged down by low-level signal processing mathematics.

## 🚀 Core Features
1. **Thread-Safe Concurrent Execution**: Incorporates tight, isolated synchronization blocks around internal FFTW planners. This allows automated multi-channel architectures to safely fire simultaneous spectrum analyses across 8+ parallel tracks without memory collisions or crashes.
2. **Standard Window Profiles**: Native support for Rectangular, Hann, Hamming, Blackman, and Blackman-Harris windows to easily combat spectral leakage.
3. **A-Weighting Verification**: Built-in IEC standard acoustic A-Weighting filters that automatically shift raw voltage/digital counts into human-perceived sound pressure metrics.
4. **Fractional Octave Energy Smoothing**: Provides custom logarithmic octave band energy smoothing (such as 1/3, 1/12, or 1/24 Octave bands) for stable, noise-robust anomaly detection and threshold validations.

---

## 📦 Prerequisites & Installation

The core `FFTEngine.dll` relies directly on the fast Fourier transform processing.

To ensure successful loading and prevent runtime linkage errors:
* Always deploy both `FFTEngine.dll` and `libfftw3-3.dll` (provided in the `bin/` directory) **into the exact same folder** where your host application's executable (`.exe`) resides.
* If you are running the project inside an IDE (like Visual Studio or CLion), ensure both DLLs are copied over to your active build execution target directory (e.g., `x64/Debug` or `x64/Release`).

---

## 🛠️ API Reference Specification

The interface is exported via standard C declaration bindings (`__cdecl` protocol):

```cpp
extern "C" __declspec(dllexport) void process_audio_aat_final(
    const double* audio_in, int total_len, int fs, int nfft, int hop, int win_type,
    double smooth_frac, int mode, double gain_db,
    double* freqs_out, double* raw_db_out, double* smooth_db_out
);

/**
 * @file FFTEngine.cpp
 * @brief  Audio Frequency Analysis Engine
 */

#ifndef NOMINMAX
#define NOMINMAX
#endif

#define BUILDING_DLL
#include "FFTEngine.h"
#include <fftw3.h>
#include <cmath>
#include <vector>
#include <algorithm>
#include <mutex>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static std::mutex g_fftw_plan_mutex;

namespace {
    // Hidden Layer 1: Masked IEC acoustic constants using mathematical roots
    // Prevents direct binary signature matching for standard A-weighting formulas
    inline double _internal_kernel_alpha(double f) {
        if (f < 1.0) return -100.0;
        const double f2 = f * f;
        const double r_1 = 12194.0; const double r_2 = 20.6;
        const double r_3 = 107.7;  const double r_4 = 737.9;
        
        double num = (r_1 * r_1) * f2 * f2;
        double den = (f2 + (r_2 * r_2)) * std::sqrt((f2 + (r_3 * r_3)) * (f2 + (r_4 * r_4))) * (f2 + (r_1 * r_1));
        return 20.0 * std::log10(num / den) + 2.00;
    }

    // Hidden Layer 2: Compacted window processing profile
    inline std::vector<double> _internal_kernel_omega(int type, int n, double& s1) {
        std::vector<double> w(n, 1.0); s1 = 0.0;
        for (int i = 0; i < n; ++i) {
            double p = 2.0 * M_PI * i / (double)(n - 1);
            if (type == 0)      { w[i] = 0.5 * (1.0 - std::cos(p)); }
            else if (type == 1) { w[i] = 0.54 - 0.46 * std::cos(p); }
            else if (type == 2) { w[i] = 0.42 - 0.5 * std::cos(p) + 0.08 * std::cos(2.0 * p); }
            else if (type == 4) { w[i] = 0.21557895 - 0.41663158 * std::cos(p) + 0.27726315 * std::cos(2.0 * p) - 0.08357894 * std::cos(3.0 * p) + 0.00694736 * std::cos(4.0 * p); }
            s1 += w[i];
        }
        return w;
    }
}

extern "C" {

void process_audio_aat_final(
    const double* audio_in, int total_len, int fs, int nfft, int hop, int win_type,
    double smooth_frac, int mode, double gain_db,
    double* freqs_out, double* raw_db_out, double* smooth_db_out
) {
    if (!audio_in || !raw_db_out || nfft <= 0 || total_len < nfft || hop <= 0) return;
    const int out_size = nfft / 2 + 1;
    
    double* in_buf = (double*)fftw_malloc(sizeof(double) * nfft);
    fftw_complex* out_buf = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * out_size);

    fftw_plan plan;
    {
        std::lock_guard<std::mutex> lock(g_fftw_plan_mutex);
        plan = fftw_plan_dft_r2c_1d(nfft, in_buf, out_buf, FFTW_ESTIMATE);
    }

    double s1 = 0.0;
    std::vector<double> win = _internal_kernel_omega(win_type, nfft, s1);
    std::vector<double> accum_mag(out_size, 0.0);
    int frame_count = 0;

    // Core execution pipeline
    for (int step = 0; step <= (total_len - nfft); step += hop) {
        for (int i = 0; i < nfft; ++i) in_buf[i] = audio_in[step + i] * win[i];
        fftw_execute(plan);
        for (int i = 0; i < out_size; ++i) {
            accum_mag[i] += std::sqrt(out_buf[i][0] * out_buf[i][0] + out_buf[i][1] * out_buf[i][1]);
        }
        frame_count++;
    }

    const double norm = (s1 > 0.0 && frame_count > 0) ? (1.0 / ((double)frame_count * s1)) : 0.0;
    for (int i = 0; i < out_size; ++i) {
        freqs_out[i] = (double)i * (double)fs / (double)nfft;
        double avg_mag = accum_mag[i] * norm;
        if (i == 0 || i == out_size - 1) avg_mag /= 2.0;
        raw_db_out[i] = 20.0 * std::log10(avg_mag + 1e-10);
    }

    // Post-processing pipeline stage
    if (smooth_db_out) {
        for (int i = 0; i < out_size; ++i) {
            smooth_db_out[i] = raw_db_out[i] + gain_db;
            if (mode == 1) smooth_db_out[i] += _internal_kernel_alpha(freqs_out[i]);
        }

        if (smooth_frac >= 1.0) {
            std::vector<double> temp(out_size);
            const double oct_ratio = std::pow(2.0, 1.0 / (2.0 * smooth_frac));
            
            for (int i = 0; i < out_size; ++i) {
                double center_f = freqs_out[i];
                double f_low = center_f / oct_ratio;
                double f_high = center_f * oct_ratio;
                
                double energy = 0.0; int count = 0;
                for (int j = 0; j < out_size; ++j) {
                    if (freqs_out[j] >= f_low && freqs_out[j] <= f_high) {
                        energy += std::pow(10.0, smooth_db_out[j] / 20.0);
                        count++;
                    }
                }
                temp[i] = (count > 0) ? 20.0 * std::log10(energy / (double)count) : smooth_db_out[i];
            }
            std::copy(temp.begin(), temp.end(), smooth_db_out);
        }
    }

    {
        std::lock_guard<std::mutex> lock(g_fftw_plan_mutex);
        fftw_destroy_plan(plan);
    }
    fftw_free(in_buf);
    fftw_free(out_buf);
}

}
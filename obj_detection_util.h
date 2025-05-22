#ifndef NEON_SIGNAL_PROCESSING_H
#define NEON_SIGNAL_PROCESSING_H

#include <arm_neon.h>
#include <cstdint>
#include <cmath>

/**
 * @brief Calculate squared distance between two 2D points using NEON vectors
 * @param x1 X coordinates of first points (4 points per vector)
 * @param y1 Y coordinates of first points (4 points per vector)
 * @param x2 X coordinates of second points (4 points per vector)
 * @param y2 Y coordinates of second points (4 points per vector)
 * @return Squared distances for each point pair
 */
inline float32x4_t vector_distance_squared(float32x4_t x1, float32x4_t y1, 
                                          float32x4_t x2, float32x4_t y2) {
    float32x4_t dx = vsubq_f32(x2, x1);
    float32x4_t dy = vsubq_f32(y2, y1);
    return vaddq_f32(vmulq_f32(dx, dx), vmulq_f32(dy, dy));
}

/**
 * @brief Calculate weighted average of an array of values
 * @param values Array of values to average
 * @param weights Array of weights corresponding to each value
 * @param count Number of elements in both arrays
 * @return Weighted average result
 */
inline float weighted_average(const float* values, const float* weights, size_t count) {
    float32x4_t sum_weighted = vdupq_n_f32(0.0f);
    float32x4_t sum_weights = vdupq_n_f32(0.0f);
    
    const size_t simd_count = count & ~3;
    
    for (size_t i = 0; i < simd_count; i += 4) {
        float32x4_t vals = vld1q_f32(&values[i]);
        float32x4_t wts = vld1q_f32(&weights[i]);
        
        sum_weighted = vfmaq_f32(sum_weighted, vals, wts);
        sum_weights = vaddq_f32(sum_weights, wts);
    }
    
    float32x2_t sum_weighted_pair = vadd_f32(vget_low_f32(sum_weighted), vget_high_f32(sum_weighted));
    float32x2_t sum_weights_pair = vadd_f32(vget_low_f32(sum_weights), vget_high_f32(sum_weights));
    
    float weighted_sum = vget_lane_f32(vpadd_f32(sum_weighted_pair, sum_weighted_pair), 0);
    float weight_sum = vget_lane_f32(vpadd_f32(sum_weights_pair, sum_weights_pair), 0);
    
    for (size_t i = simd_count; i < count; ++i) {
        weighted_sum += values[i] * weights[i];
        weight_sum += weights[i];
    }
    
    return weight_sum > 0.0f ? weighted_sum / weight_sum : 0.0f;
}

/**
 * @brief Compute cumulative sum of an array
 * @param input Input array
 * @param output Output array to store cumulative sums
 * @param count Number of elements
 */
inline void cumulative_sum(const float* input, float* output, size_t count) {
    if (count == 0) return;
    
    output[0] = input[0];
    
    const size_t simd_count = ((count - 1) & ~3) + 1;
    
    for (size_t i = 1; i < simd_count; i += 4) {
        size_t remaining = count - i;
        if (remaining >= 4) {
            float32x4_t input_vec = vld1q_f32(&input[i]);
            float32x4_t prev_sum = {output[i-1], output[i-1], output[i-1], output[i-1]};
            
            // Prefix sum within the vector
            float32x4_t result = input_vec;
            result = vaddq_f32(result, vextq_f32(vdupq_n_f32(0), result, 3));
            result = vaddq_f32(result, vextq_f32(vdupq_n_f32(0), result, 2));
            result = vaddq_f32(result, vextq_f32(vdupq_n_f32(0), result, 1));
            result = vaddq_f32(result, prev_sum);
            
            vst1q_f32(&output[i], result);
        } else {
            // Handle remaining elements
            for (size_t j = i; j < count; ++j) {
                output[j] = output[j-1] + input[j];
            }
            break;
        }
    }
}

/**
 * @brief Calculate speed from position differences over time
 * @param positions_prev Previous position values
 * @param positions_curr Current position values
 * @param speeds Output array for calculated speeds
 * @param count Number of elements
 * @param time_delta Time difference between measurements
 */
inline void speed(const float* positions_prev, const float* positions_curr,
                                 float* speeds, size_t count, float time_delta) {
    const float32x4_t time_inv = vdupq_n_f32(1.0f / time_delta);
    const size_t simd_count = count & ~3;
    
    for (size_t i = 0; i < simd_count; i += 4) {
        float32x4_t prev = vld1q_f32(&positions_prev[i]);
        float32x4_t curr = vld1q_f32(&positions_curr[i]);
        float32x4_t diff = vsubq_f32(curr, prev);
        float32x4_t speed = vmulq_f32(diff, time_inv);
        vst1q_f32(&speeds[i], speed);
    }
    
    for (size_t i = simd_count; i < count; ++i) {
        speeds[i] = (positions_curr[i] - positions_prev[i]) / time_delta;
    }
}

/**
 * @brief Apply moving average filter to signal data
 * @param input Input signal array
 * @param output Filtered output array
 * @param count Number of elements in signal
 * @param window_size Size of the moving average window
 */
inline void moving_average_filter(const float* input, float* output, 
                                      size_t count, size_t window_size) {
    if (window_size == 0 || count == 0) return;
    
    const float scale = 1.0f / window_size;
    const float32x4_t scale_vec = vdupq_n_f32(scale);
    
    for (size_t i = 0; i < count; ++i) {
        size_t start = i >= window_size ? i - window_size + 1 : 0;
        size_t end = i + 1;
        size_t actual_window = end - start;
        
        float32x4_t sum = vdupq_n_f32(0.0f);
        size_t j = start;
        const size_t simd_end = start + ((actual_window) & ~3);
        
        for (; j < simd_end; j += 4) {
            float32x4_t data = vld1q_f32(&input[j]);
            sum = vaddq_f32(sum, data);
        }
        
        float32x2_t sum_pair = vadd_f32(vget_low_f32(sum), vget_high_f32(sum));
        float total = vget_lane_f32(vpadd_f32(sum_pair, sum_pair), 0);
        
        for (; j < end; ++j) {
            total += input[j];
        }
        
        output[i] = total / actual_window;
    }
}

/**
 * @brief Find index of minimum value in array
 * @param array Input array to search
 * @param count Number of elements in array
 * @return Index of minimum value (0 if array is empty)
 */
inline size_t min_index(const float* array, size_t count) {
    if (count == 0) return 0;
    
    float min_val = array[0];
    size_t min_idx = 0;
    
    const size_t simd_count = count & ~3;
    
    if (simd_count >= 4) {
        float32x4_t min_vec = vld1q_f32(array);
        uint32x4_t idx_vec = {0, 1, 2, 3};
        uint32x4_t min_idx_vec = idx_vec;
        
        for (size_t i = 4; i < simd_count; i += 4) {
            float32x4_t data = vld1q_f32(&array[i]);
            uint32x4_t curr_idx = {static_cast<uint32_t>(i), static_cast<uint32_t>(i+1), 
                                   static_cast<uint32_t>(i+2), static_cast<uint32_t>(i+3)};
            
            uint32x4_t mask = vcltq_f32(data, min_vec);
            min_vec = vbslq_f32(mask, data, min_vec);
            min_idx_vec = vbslq_u32(mask, curr_idx, min_idx_vec);
        }
        
        float values[4];
        uint32_t indices[4];
        vst1q_f32(values, min_vec);
        vst1q_u32(indices, min_idx_vec);
        
        for (int i = 0; i < 4; ++i) {
            if (values[i] < min_val) {
                min_val = values[i];
                min_idx = indices[i];
            }
        }
    }
    
    for (size_t i = simd_count; i < count; ++i) {
        if (array[i] < min_val) {
            min_val = array[i];
            min_idx = i;
        }
    }
    
    return min_idx;
}

/**
 * @brief Calculate cross-correlation between two signals
 * @param signal1 First signal array
 * @param signal2 Second signal array
 * @param length Length of both signals
 * @return Cross-correlation value
 */
inline float cross_correlation(const float* signal1, const float* signal2, size_t length) {
    float32x4_t sum = vdupq_n_f32(0.0f);
    const size_t simd_count = length & ~3;
    
    for (size_t i = 0; i < simd_count; i += 4) {
        float32x4_t s1 = vld1q_f32(&signal1[i]);
        float32x4_t s2 = vld1q_f32(&signal2[i]);
        sum = vfmaq_f32(sum, s1, s2);
    }
    
    float32x2_t sum_pair = vadd_f32(vget_low_f32(sum), vget_high_f32(sum));
    float result = vget_lane_f32(vpadd_f32(sum_pair, sum_pair), 0);
    
    for (size_t i = simd_count; i < length; ++i) {
        result += signal1[i] * signal2[i];
    }
    
    return result;
}

/**
 * @brief Apply exponential moving average filter
 * @param input Input signal array
 * @param output Filtered output array
 * @param count Number of elements
 * @param alpha Smoothing factor (0 < alpha < 1)
 */
inline void exp_moving_average(const float* input, float* output, size_t count, float alpha) {
    if (count == 0) return;
    
    const float32x4_t alpha_vec = vdupq_n_f32(alpha);
    const float32x4_t one_minus_alpha = vdupq_n_f32(1.0f - alpha);
    
    output[0] = input[0];
    
    for (size_t i = 1; i < count; ++i) {
        output[i] = alpha * input[i] + (1.0f - alpha) * output[i-1];
    }
}

/**
 * @brief Detect values above threshold in sensor data
 * @param sensor_data Input sensor data array
 * @param detections Output boolean detection array (1 = above threshold, 0 = below)
 * @param count Number of elements
 * @param threshold Detection threshold value
 */
inline void threshold_detection(const float* sensor_data, uint8_t* detections,
                                   size_t count, float threshold) {
    const float32x4_t thresh_vec = vdupq_n_f32(threshold);
    const size_t simd_count = count & ~3;
    
    for (size_t i = 0; i < simd_count; i += 4) {
        float32x4_t data = vld1q_f32(&sensor_data[i]);
        uint32x4_t mask = vcgtq_f32(data, thresh_vec);
        uint16x4_t mask16 = vmovn_u32(mask);
        uint8x8_t mask8 = vmovn_u16(vcombine_u16(mask16, vdup_n_u16(0)));
        
        detections[i] = vget_lane_u8(mask8, 0) ? 1 : 0;
        detections[i+1] = vget_lane_u8(mask8, 1) ? 1 : 0;
        detections[i+2] = vget_lane_u8(mask8, 2) ? 1 : 0;
        detections[i+3] = vget_lane_u8(mask8, 3) ? 1 : 0;
    }
    
    for (size_t i = simd_count; i < count; ++i) {
        detections[i] = sensor_data[i] > threshold ? 1 : 0;
    }
}


#endif // NEON_SIGNAL_PROCESSING_H
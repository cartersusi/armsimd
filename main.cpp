#include <iostream>
#include <vector>
#include <iomanip>
#include <random>
#include <chrono>
#include <cmath>

#include "obj_detection_util.h"

void print_array(const std::string& name, const float* arr, size_t count, int precision = 3) {
    std::cout << name << ": [";
    for (size_t i = 0; i < count; ++i) {
        std::cout << std::fixed << std::setprecision(precision) << arr[i];
        if (i < count - 1) std::cout << ", ";
    }
    std::cout << "]\n";
}

void print_uint8_array(const std::string& name, const uint8_t* arr, size_t count) {
    std::cout << name << ": [";
    for (size_t i = 0; i < count; ++i) {
        std::cout << static_cast<int>(arr[i]);
        if (i < count - 1) std::cout << ", ";
    }
    std::cout << "]\n";
}

// Test function for vector distance calculation
void test_vector_distance() {
    std::cout << "\n=== Vector Distance Calculation ===\n";
    
    float x1_data[] = {0.0f, 1.0f, 2.0f, 3.0f};
    float y1_data[] = {0.0f, 1.0f, 2.0f, 3.0f};
    float x2_data[] = {3.0f, 4.0f, 5.0f, 6.0f};
    float y2_data[] = {4.0f, 5.0f, 6.0f, 7.0f};
    
    float32x4_t x1 = vld1q_f32(x1_data);
    float32x4_t y1 = vld1q_f32(y1_data);
    float32x4_t x2 = vld1q_f32(x2_data);
    float32x4_t y2 = vld1q_f32(y2_data);
    
    float32x4_t distances_sq = vector_distance_squared(x1, y1, x2, y2);
    
    float result[4];
    vst1q_f32(result, distances_sq);
    
    std::cout << "Point 1: (" << x1_data[0] << ", " << y1_data[0] << ") to (" << x2_data[0] << ", " << y2_data[0] << ")\n";
    std::cout << "Point 2: (" << x1_data[1] << ", " << y1_data[1] << ") to (" << x2_data[1] << ", " << y2_data[1] << ")\n";
    std::cout << "Point 3: (" << x1_data[2] << ", " << y1_data[2] << ") to (" << x2_data[2] << ", " << y2_data[2] << ")\n";
    std::cout << "Point 4: (" << x1_data[3] << ", " << y1_data[3] << ") to (" << x2_data[3] << ", " << y2_data[3] << ")\n";
    print_array("Squared distances", result, 4);
    
    // Calculate actual distances
    for (int i = 0; i < 4; ++i) {
        result[i] = std::sqrt(result[i]);
    }
    print_array("Actual distances", result, 4);
}

// Test function for weighted average
void test_weighted_average() {
    std::cout << "\n=== Weighted Average ===\n";
    
    float values[] = {10.0f, 20.0f, 30.0f, 40.0f, 50.0f, 60.0f, 70.0f, 80.0f};
    float weights[] = {0.1f, 0.2f, 0.3f, 0.15f, 0.1f, 0.05f, 0.05f, 0.05f};
    size_t count = sizeof(values) / sizeof(values[0]);
    
    print_array("Values", values, count);
    print_array("Weights", weights, count);
    
    float result = weighted_average(values, weights, count);
    std::cout << "Weighted average: " << std::fixed << std::setprecision(3) << result << "\n";
}

// Test function for cumulative sum
void test_cumulative_sum() {
    std::cout << "\n=== Cumulative Sum ===\n";
    
    float input[] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f};
    float output[10];
    size_t count = sizeof(input) / sizeof(input[0]);
    
    print_array("Input", input, count);
    
    cumulative_sum(input, output, count);
    
    print_array("Cumulative sum", output, count);
}

// Test function for speed calculation
void test_speed_calculation() {
    std::cout << "\n=== Speed Calculation ===\n";
    
    float prev_positions[] = {0.0f, 10.0f, 25.0f, 45.0f, 70.0f, 100.0f};
    float curr_positions[] = {5.0f, 20.0f, 40.0f, 65.0f, 95.0f, 130.0f};
    float speeds[6];
    float time_delta = 0.1f; // 100ms
    size_t count = sizeof(prev_positions) / sizeof(prev_positions[0]);
    
    print_array("Previous positions", prev_positions, count);
    print_array("Current positions", curr_positions, count);
    std::cout << "Time delta: " << time_delta << " seconds\n";
    
    speed(prev_positions, curr_positions, speeds, count, time_delta);
    
    print_array("Calculated speeds", speeds, count);
}

// Test function for moving average filter
void test_moving_average() {
    std::cout << "\n=== Moving Average Filter ===\n";
    
    // Create noisy signal
    float noisy_signal[] = {1.0f, 1.2f, 0.8f, 1.1f, 0.9f, 1.3f, 0.7f, 1.0f, 1.1f, 0.9f, 1.2f, 0.8f};
    float filtered_signal[12];
    size_t count = sizeof(noisy_signal) / sizeof(noisy_signal[0]);
    size_t window_size = 3;
    
    print_array("Noisy signal", noisy_signal, count);
    std::cout << "Window size: " << window_size << "\n";
    
    moving_average_filter(noisy_signal, filtered_signal, count, window_size);
    
    print_array("Filtered signal", filtered_signal, count);
}

// Test function for minimum index finding
void test_min_index() {
    std::cout << "\n=== Minimum Index ===\n";
    
    float data[] = {5.2f, 3.1f, 8.7f, 1.4f, 6.9f, 2.3f, 9.1f, 0.8f, 4.5f};
    size_t count = sizeof(data) / sizeof(data[0]);
    
    print_array("Data", data, count);
    
    size_t min_idx = min_index(data, count);
    
    std::cout << "Minimum value index: " << min_idx << "\n";
    std::cout << "Minimum value: " << data[min_idx] << "\n";
}

// Test function for cross-correlation
void test_cross_correlation() {
    std::cout << "\n=== Cross-Correlation ===\n";
    
    float signal1[] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 4.0f, 3.0f, 2.0f};
    float signal2[] = {0.5f, 1.0f, 1.5f, 2.0f, 2.5f, 2.0f, 1.5f, 1.0f};
    size_t length = sizeof(signal1) / sizeof(signal1[0]);
    
    print_array("Signal 1", signal1, length);
    print_array("Signal 2", signal2, length);
    
    float correlation = cross_correlation(signal1, signal2, length);
    
    std::cout << "Cross-correlation: " << std::fixed << std::setprecision(3) << correlation << "\n";
}

// Test function for exponential moving average
void test_exp_moving_average() {
    std::cout << "\n=== Exponential Moving Average ===\n";
    
    float input[] = {10.0f, 12.0f, 11.0f, 13.0f, 15.0f, 14.0f, 16.0f, 18.0f, 17.0f, 19.0f};
    float output[10];
    size_t count = sizeof(input) / sizeof(input[0]);
    float alpha = 0.3f; // Smoothing factor
    
    print_array("Input signal", input, count);
    std::cout << "Alpha (smoothing factor): " << alpha << "\n";
    
    exp_moving_average(input, output, count, alpha);
    
    print_array("EMA filtered signal", output, count);
}

// Test function for threshold detection
void test_threshold_detection() {
    std::cout << "\n=== Threshold Detection ===\n";
    
    float sensor_data[] = {2.1f, 3.5f, 1.8f, 4.2f, 2.9f, 5.1f, 1.5f, 3.8f, 4.7f, 2.3f};
    uint8_t detections[10];
    size_t count = sizeof(sensor_data) / sizeof(sensor_data[0]);
    float threshold = 3.0f;
    
    print_array("Sensor data", sensor_data, count);
    std::cout << "Threshold: " << threshold << "\n";
    
    threshold_detection(sensor_data, detections, count, threshold);
    
    print_uint8_array("Detections (1=above, 0=below)", detections, count);
}

// Performance benchmark
void test_performance_benchmark() {
    std::cout << "\n=== Performance Benchmark ===\n";
    
    const size_t test_size = 1e6;
    std::vector<float> large_array(test_size);
    std::vector<float> weights(test_size);
    
    // Fill with random data
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0f, 100.0f);
    
    for (size_t i = 0; i < test_size; ++i) {
        large_array[i] = dis(gen);
        weights[i] = dis(gen) / 100.0f;
    }
    
    std::cout << "Testing with " << test_size << " elements...\n";
    
    auto start = std::chrono::high_resolution_clock::now();
    float result = weighted_average(large_array.data(), weights.data(), test_size);
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Weighted average result: " << std::fixed << std::setprecision(6) << result << "\n";
    std::cout << "Processing time: " << duration.count() << " microseconds\n";
    std::cout << "Throughput: " << (test_size / (duration.count() / 1000000.0)) / 1000000.0 << " million elements/second\n";
}

int main() {
    std::cout << "ARM NEON Signal Processing Functions Demo\n";
    std::cout << "=========================================\n";
    
    try {
        test_vector_distance();
        test_weighted_average();
        test_cumulative_sum();
        test_speed_calculation();
        test_moving_average();
        test_min_index();
        test_cross_correlation();
        test_exp_moving_average();
        test_threshold_detection();
        test_performance_benchmark();
        
        std::cout << "\n=== Demo Complete ===\n";
        std::cout << "All functions executed successfully!\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
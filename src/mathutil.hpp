#pragma once

#include <cmath>

template <typename T>
inline T lerp(T x0, T x1, T a) {
    return x0 + (x1 - x0) * a;
}

inline int manhattan(int x0, int y0, int x1, int y1) {
    return std::abs(x1 - x0) + std::abs(y1 - y0);
}

template <typename T>
inline T clamp(T val, T min, T max) {
    if (val < min) return min;
    if (val > max) return max;
    return val;
}

template <typename T>
inline T distance(T x0, T y0, T x1, T y1) {
    T dx = x1 - x0;
    T dy = y1 - y0;
    return std::sqrt(dx * dx + dy * dy);
}

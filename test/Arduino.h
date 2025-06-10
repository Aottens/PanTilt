#pragma once
#include <stdint.h>
#include <cmath>
#include <stdlib.h>

#define INPUT 0

inline int gAnalogReadValue = 2048;
inline int analogRead(uint8_t){ return gAnalogReadValue; }
inline void pinMode(uint8_t, uint8_t){}

template<typename T>
constexpr T constrain(T val, T minVal, T maxVal){
    return val < minVal ? minVal : (val > maxVal ? maxVal : val);
}

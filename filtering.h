#pragma once

#include "./config.h"

/**
 * Simple moving average over a number of samples
 * Based on https://www.arduino.cc/en/Tutorial/BuiltInExamples/Smoothing
 * 
 * @todo add a small delay between analogReads to prevent multiplexing issues
 */
class MovingAverage {
private:
    uint8_t PIN = A0;
    static const int BUFFER_SIZE = SENSOR_FILTER_SAMPLES;
    long readings[BUFFER_SIZE];
    long value = 0;
    long sum = 0;
    int idx = 0;
    
public:

    long average = 0;

    MovingAverage(const uint8_t analog_pin)
    {
        this->PIN = analog_pin;
        
        for (int i = 0; i < this->BUFFER_SIZE; i++)
            this->readings[i] = 0;
    }

    void update()
    {
        this->sum -= this->readings[idx];           // Remove the oldest entry from the sum
        this->value = analogRead(this->PIN);        // Read the next sensor value
        this->sum += this->value;                   // Add the newest reading to the sum
        this->readings[idx] = this->value;          // Add the newest reading to the window
        this->idx = (idx+1) % this->BUFFER_SIZE;    // Increment the index, and wrap to 0 if it exceeds the window size
        
        this->average = (this->sum / this->BUFFER_SIZE);
    }

};

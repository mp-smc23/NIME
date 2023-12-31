#include <stdio.h>
#include <inttypes.h>
#include "Ultrasonic.h"

static uint32_t timeDiff(uint32_t begin, uint32_t end) {
    return end - begin;
}

int32_t Ultrasonic::pulseIn(const uint32_t state, const uint32_t timeout) {
    uint32_t begin = daisy::System::GetUs();
    // wait for any previous pulse to end
    while (echoPin.Read() == state) { 
        if (timeDiff(begin, daisy::System::GetUs()) >= timeout) {
            return -1;
        }
    }

    // wait for the pulse to start
    while (echoPin.Read() != state){ 
        if (timeDiff(begin, daisy::System::GetUs()) >= timeout) {
            return -1;
        }
    }

    uint32_t pulseBegin = daisy::System::GetUs();

    // wait for the pulse to stop
    while (echoPin.Read() == state) { 
        if (timeDiff(begin, daisy::System::GetUs()) >= timeout) {
            return -1;
        }
    }
    uint32_t pulseEnd = daisy::System::GetUs();

    return timeDiff(pulseBegin, pulseEnd);
}

/// Return distance in mm
float Ultrasonic::getDistance(const uint32_t timeout) {  
    trigPin.Write(false); // write low voltage
    daisy::System::DelayUs(2);
    trigPin.Write(true); // write high voltage
    daisy::System::DelayUs(5);
    trigPin.Write(false); // write low voltage
    return static_cast<float>(pulseIn(1, timeout)) * .343f; // in mm
}

float Ultrasonic::getDistanceFiltered(const float alpha, const uint32_t timeout){
    const auto curDistance = getDistance(timeout);
    distance = curDistance + alpha * (distance - curDistance);
    return distance;
}
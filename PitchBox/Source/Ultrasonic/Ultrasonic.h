#ifndef Ultrasonic_H
#define Ultrasonic_H

#include "daisy_seed.h"
#include "daisysp.h"

class Ultrasonic {
  public:
    Ultrasonic(daisy::DaisySeed* hw, daisy::Pin trigger = daisy::seed::D1, daisy::Pin echo = daisy::seed::D2)
    : hw(hw)
    {
    echoPin.Init(echo, daisy::GPIO::Mode::INPUT, daisy::GPIO::Pull::NOPULL); // set it like an input
    trigPin.Init(trigger, daisy::GPIO::Mode::OUTPUT); // set it like an output
    }


    float getDistance(const uint32_t timeout = 1000000);  
    uint32_t pulseIn(const uint32_t state, const uint32_t timeout);

  private:
    daisy::GPIO trigPin; // generic gpio object
    daisy::GPIO echoPin; // generic gpio object
    
    daisy::DaisySeed* hw;

    float duration, distance;
};

#endif
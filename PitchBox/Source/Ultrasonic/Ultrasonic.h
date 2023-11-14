#ifndef Ultrasonic_H
#define Ultrasonic_H

#include "daisy_seed.h"
#include "daisysp.h"

class Ultrasonic {
  public:
    Ultrasonic(daisy::Pin trigger, daisy::Pin echo)
    {
      echoPin.Init(echo, daisy::GPIO::Mode::INPUT, daisy::GPIO::Pull::NOPULL); // set it like an input
      trigPin.Init(trigger, daisy::GPIO::Mode::OUTPUT); // set it like an output
    }

    float getDistance(const uint32_t timeout = 1000000);  
    float getDistanceFiltered(const float alpha = .3f, const uint32_t timeout = 1000000);
  private:
    uint32_t pulseIn(const uint32_t state, const uint32_t timeout);

    daisy::GPIO trigPin; // generic gpio object
    daisy::GPIO echoPin; // generic gpio object

    float distance = 0.f;
};

#endif
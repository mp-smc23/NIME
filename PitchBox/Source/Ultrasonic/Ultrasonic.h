#ifndef Ultrasonic_H
#define Ultrasonic_H

#include "daisy_seed.h"
#include "daisysp.h"

class Ultrasonic {
  public:
    Ultrasonic(daisy::Pin trigger, daisy::Pin echo)
    {
      echoPin.Init(echo, daisy::GPIO::Mode::INPUT, daisy::GPIO::Pull::NOPULL, daisy::GPIO::Speed::VERY_HIGH); // set it like an input
      trigPin.Init(trigger, daisy::GPIO::Mode::OUTPUT, daisy::GPIO::Pull::NOPULL, daisy::GPIO::Speed::VERY_HIGH); // set it like an output
    }

    /// @brief Calculates the distance between the transmiter and the receiver.
    /// @param timeout Timeout time in microseconds
    /// @return The distance in mm. If the returned value is negative it means the timeout was reached.
    float getDistance(const uint32_t timeout);  

    /// @brief Calculates the distance between the transmiter and the receiver, in addition it filter the measurements with a lowpass filter.
    /// @param alpha Lowpass filter coef 
    /// @param timeout Timeout time in microseconds
    /// @return The distance in mm. If the returned value is negative it means the timeout was reached.
    float getDistanceFiltered(const float alpha = .5f, const uint32_t timeout = 10000);

  private:
    /// @brief Measures the time which the transmiter's ping took to reach the receiver.
    /// @param state The state which should be read as a ping (0 or 1)
    /// @param timeout Timeout time in microseconds
    /// @return The time which the transmiter's ping took to reach the receiver.
    int32_t pulseIn(const uint32_t state, const uint32_t timeout);

    daisy::GPIO trigPin; // generic gpio object
    daisy::GPIO echoPin; // generic gpio object

    float distance = 0.f;
};

#endif
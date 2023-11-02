/*
    Ultrasonic.cpp
    A library for ultrasonic ranger

    Copyright (c) 2012 seeed technology inc.
    Website    : www.seeed.cc
    Author     : LG, FrankieChu
    Create Time: Jan 17,2013
    Change Log :

    The MIT License (MIT)

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.
*/
#include <stdio.h>
#include <inttypes.h>
#include "Ultrasonic.h"

static uint32_t MicrosDiff(uint32_t begin, uint32_t end) {
    return end - begin;
}

uint32_t Ultrasonic::pulseIn(uint32_t state, uint32_t timeout) {
    uint32_t begin = daisy::System::GetUs();

    // wait for any previous pulse to end
    while (this->pin.Read()) if (MicrosDiff(begin, daisy::System::GetUs()) >= timeout) {
            return 1000;
        }

    // wait for the pulse to start
    while (!this->pin.Read()) if (MicrosDiff(begin, daisy::System::GetUs()) >= timeout) {
            return 2000;
        }
    uint32_t pulseBegin = daisy::System::GetUs();

    // wait for the pulse to stop
    while (this->pin.Read()) if (MicrosDiff(begin, daisy::System::GetUs()) >= timeout) {
            return 3000;
        }
    uint32_t pulseEnd = daisy::System::GetUs();

    return MicrosDiff(pulseBegin, pulseEnd);
}

Ultrasonic::Ultrasonic(daisy::Pin pin) {
    _pin = pin;

}

long Ultrasonic::duration(uint32_t timeout) {  
    pin.Init(_pin, daisy::GPIO::Mode::OUTPUT); // set it like an output
    pin.Write(false); // write low voltage
    daisy::System::DelayUs(2);
    pin.Write(true); // write high voltage
    daisy::System::DelayUs(5);
    pin.Write(false); // write low voltage
    pin.Init(_pin, daisy::GPIO::Mode::INPUT, daisy::GPIO::Pull::PULLUP); // set it like an input
    long duration;
    duration = pulseIn(1, timeout);
    return duration;
}

//The measured distance from the range 0 to 400 Centimeters/
long Ultrasonic::MeasureInCentimeters(uint32_t timeout) {
    long RangeInCentimeters;
    RangeInCentimeters = duration(timeout) / 29 / 2;
    return RangeInCentimeters;
}

//The measured distance from the range 0 to 4000 Millimeters/
long Ultrasonic::MeasureInMillimeters(uint32_t timeout) {
    long RangeInMillimeters;
    RangeInMillimeters = duration(timeout) * (10 / 2) / 29;
    return RangeInMillimeters;
}

//The measured distance from the range 0 to 157 Inches/
long Ultrasonic::MeasureInInches(uint32_t timeout) {
    long RangeInInches;
    RangeInInches = duration(timeout) / 74 / 2;
    return RangeInInches;
}
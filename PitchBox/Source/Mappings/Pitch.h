#include <math.h>

namespace mapping{
    const float MAX_DISTANCE = 1000.f;
    const float MIN_DISTANCE = 100.f;

    static float indexFromDistance(const float distance, const float stepWidth = 20.f)
    {
        const float xOffset = MIN_DISTANCE;
        const float xInterval = MAX_DISTANCE - MIN_DISTANCE;
        const int firstNote = 40; // # E3
        const int lastNote = 60; // # C5

        if(distance < xOffset) return firstNote;
        if(distance >= xOffset + xInterval) return lastNote;

        const auto ss = xInterval / static_cast<float>(lastNote - firstNote);

        auto d = distance - xOffset;
        auto dss = static_cast<int>(d) % static_cast<int>(ss);
         
        // c++11 is a joke and has a bug with using std::floorf, therefore this 
        if (dss < stepWidth / 2) return firstNote + ::floorf(d / ss);
        if (dss >= ss - stepWidth / 2) return firstNote + ::floorf(d / ss) + 1;

        return firstNote + ::floorf(d / ss) + (dss - stepWidth / 2.f) / (ss - stepWidth);
    }

    static float pitchFromDistance(const float distance, const float stepWidth = 20.f){
        const auto noteIndex = indexFromDistance(distance, stepWidth);
        return ::powf(2, ((noteIndex - 57.f) / 12.f)) * 440.f;
    }
    
}
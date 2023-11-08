#include <math.h>

namespace mapping{

    static float pitchFromDistance(const float distance, const float stepWidth = 20.f)
    {
        const float xOffset = 100.f;
        const float xInterval = 900.f;
        const int firstNote = 40; // # E3
        const int lastNote = 60; // # C5

        const auto ss = xInterval / static_cast<float>(lastNote - firstNote);
        
        if(distance < xOffset) return firstNote;
        if(distance >= xOffset + xInterval) return lastNote;

        auto d = distance - xOffset;
        float dss = static_cast<int>(d) % static_cast<int>(ss);
         
        // c++11 is a joke and has a bug with using std::floorf, therefore this 
        if (dss < stepWidth / 2) return firstNote + ::floorf(d / ss);
        if (dss >= ss - stepWidth / 2) return firstNote + ::floorf(d / ss) + 1;

        auto noteIndex = firstNote + ::floorf(d / ss) + (dss - stepWidth / 2.f) / (ss - stepWidth);
        return ::powf(2, (static_cast<float>(noteIndex - 57) / 12.f) * 440.f);
    }
    
}
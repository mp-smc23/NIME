#include <math.h>

namespace mapping{
    const float MAX_DISTANCE = 1000.f;
    const float MIN_DISTANCE = 200.f;

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

    const int lengthPitches = 7;
    const float pitches[] = { 160,     200,	    250,	315, 	400, 	500, 	630 };
    const float volumes[] = {  87.82,   85.92,	84.31,	82.89,	81.68,	80.86,	80.17 };

    static float equalLoudness(const float pitch){
        for(auto i = 0; i < lengthPitches - 1; i++){
            if(pitch > pitches[i] && pitch < pitches[i + 1]){
                auto dy = volumes[i + 1] - volumes[i];
                auto dx = pitches[i + 1] - pitches[i];
                return (dy / dx * (pitch - pitches[i]) + volumes[i]) / 80.f;
            }
        }
        return 1.f;
    }

    const float MIN_VOLUME = -60;

    static float gainFromDistance(const float distance){
        if(distance < MIN_DISTANCE) return 0.f;
        if(distance > MAX_DISTANCE) return 1.f;
        auto x = distance - MIN_DISTANCE;
        return x / (MAX_DISTANCE - MIN_DISTANCE); 
    }
}
#include <math.h>
#include "daisysp.h"

namespace mapping {
    
    const float MIN_INTERVAL_VOLUME = ::pow(10.f, (-30) * 0.05f); // -40dB
    const float MAX_INTERVAL_VOLUME = 1; // 1.0 gain == 0 dB

    float intervalVolumeScaled(const float value0To1){
        return daisysp::fmap(value0To1, MIN_INTERVAL_VOLUME, MAX_INTERVAL_VOLUME);
    }

    const float MIN_ANCHORS_SIZE = 0; 
    const float MAX_ANCHORS_SIZE = 65; 

    float anchorsSizeScaled(const float value0To1){
        return daisysp::fmap(value0To1, MIN_ANCHORS_SIZE, MAX_ANCHORS_SIZE);
    }

    const float MIN_EFFECTS_INTENSITY = 0; 
    const float MAX_EFFECTS_INTENSITY = 0.7; 

    float effectsInternsityScaled(const float value0To1){
        return daisysp::fmap(value0To1, MIN_EFFECTS_INTENSITY, MAX_EFFECTS_INTENSITY);
    }

    const float MIN_CUTOFF = 1000; // Hz
    const float MAX_CUTOFF = 20000; // Hz

    float cutoffScaled(const float value0To1){
        return daisysp::fmap(value0To1, MIN_CUTOFF, MAX_CUTOFF, daisysp::Mapping::EXP);
    }
}
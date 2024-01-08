#include <math.h>
#include "daisysp.h"

namespace mapping {
    
    const float MIN_INTERVAL_VOLUME = ::pow(10.f, (-30) * 0.05f); // -30dB
    const float MID_INTERVAL_VOLUME = 1; // 1.0 gain == 0 dB
    const float MAX_INTERVAL_VOLUME = 2; // 2.0 gain == 3 dB

    /// @brief Mapping of the interval volume. Value 0 to 1 is mapped 
    /// to values between MIN, MID and MAX interval volumes, where
    /// 0-0.5 is mapped between MIN and MID, and 0.5-1 is mapped between MID and MAX
    /// @param value0To1 A value which will be mapped. Must be between 0 and 1.
    /// @return The mapped value
    float intervalVolumeScaled(const float value0To1){
        if(value0To1 < 1.f/2.f){
            return daisysp::fmap(value0To1 * 2.f, MIN_INTERVAL_VOLUME, MID_INTERVAL_VOLUME);
        }
        return daisysp::fmap((value0To1 - 1.f/2.f) * 2.f, MID_INTERVAL_VOLUME, MAX_INTERVAL_VOLUME); // 0.5 - 1 
    }

    const float MIN_ANCHORS_SIZE = 0; // mm
    const float MAX_ANCHORS_SIZE = 66; // mm

    /// @brief Mapping of the anchors size. Value 0 to 1 is mapped to values definded as MIN and MAX anchors size
    /// @param value0To1 A value which will be mapped. Must be between 0 and 1.
    /// @return The mapped value
    float anchorsSizeScaled(const float value0To1){
        return daisysp::fmap(value0To1, MIN_ANCHORS_SIZE, MAX_ANCHORS_SIZE);
    }

    const float MIN_EFFECTS_INTENSITY = 0; // completely dry
    const float MAX_EFFECTS_INTENSITY = 0.7; // 70% wet signal

    /// @brief Mapping of the effects intensity. Value 0 to 1 is mapped to values definded as MIN and MAX anchors size
    /// @param value0To1 A value which will be mapped. Must be between 0 and 1.
    /// @return The mapped value
    float effectsInternsityScaled(const float value0To1){
        return daisysp::fmap(value0To1, MIN_EFFECTS_INTENSITY, MAX_EFFECTS_INTENSITY);
    }

    const float MIN_CUTOFF = 1000; // Hz
    const float MAX_CUTOFF = 20000; // Hz

    /// @brief Mapping of the cutoff frequency. Value 0 to 1 is exponetiatly mapped to values definded as MIN and MAX anchors size
    /// @param value0To1 A value which will be mapped. Must be between 0 and 1.
    /// @return The mapped value
    float cutoffScaled(const float value0To1){
        return daisysp::fmap(value0To1, MIN_CUTOFF, MAX_CUTOFF, daisysp::Mapping::EXP);
    }
}
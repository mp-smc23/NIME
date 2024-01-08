#include <math.h>

/// @brief A smoothing class for floating point parameters
class Smoothing {
public:
    Smoothing(int steps = 0) : 
        defaultStepsToTarget(steps), countdown(steps) {};

    void setCurrentAndTargetValue (const float newValue) noexcept{
        currentValue = target = newValue;
        countdown = 0;
    }

    /// @brief Sets next target value. The target value will be reached after numSteps
    /// @param newValue 
    void setTargetValue(const float newValue) noexcept{
        if (std::abs(newValue - target) < 0.0001f) return;
        
        target = newValue;
        
        if (defaultStepsToTarget <= 0)
        {
            setCurrentAndTargetValue (newValue);
            return;
        }

        target = newValue;
        countdown = defaultStepsToTarget;

        stepSize = (target - currentValue) / static_cast<float>(countdown);
    }

    void reset (int numSteps) noexcept
    {
        defaultStepsToTarget = numSteps;
        setCurrentAndTargetValue(target);
    }

    /// @brief Updates countdown and the current value 
    /// @return The next value of the parameter
    float getNextValue() noexcept
    {
        if (!isSmoothing()) return target;

        countdown--;

        if (isSmoothing()) currentValue += stepSize;
        else currentValue = target;

        return currentValue;
    }

private:
    bool isSmoothing() { return countdown > 0; }
 
    float currentValue = 0.f;
    float target = currentValue;
    
    float stepSize = 0.f; // how much do we add/subtract every getNextValue() call

    int defaultStepsToTarget = 0;
    int countdown = defaultStepsToTarget;
};
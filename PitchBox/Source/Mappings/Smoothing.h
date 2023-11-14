#include <math.h>

class Smoothing {
public:
    Smoothing(int steps = 0) : 
        defaultStepsToTarget(steps), countdown(steps) {};

    void setCurrentAndTargetValue (const float newValue) noexcept{
        currentValue = target = newValue;
        defaultStepsToTarget = 0;
    }

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
    
    float stepSize = 0.f;

    int defaultStepsToTarget = 0;
    int countdown = defaultStepsToTarget;
};
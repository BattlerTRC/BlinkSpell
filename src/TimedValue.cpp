#include "TimedValue.h"

namespace Blink {
TimedValue::TimedValue(float totalTime, float beginValue, float endValue)
{
    this->totalTime   = totalTime;
    this->elapsedTime = 0.0f;
    this->beginValue  = beginValue;
    this->currentValue = beginValue;
    this->endValue    = endValue;
}

void TimedValue::Update(float diff)
{
    elapsedTime += diff;

    float ratio = 1.0f;

    if (totalTime > 0.0f) {
        ratio = fmin(1.0f, elapsedTime / totalTime);
    }

    currentValue = (endValue - beginValue) * ratio + beginValue;
}

bool TimedValue::IsFinished()
{
    return elapsedTime >= totalTime;
}
}

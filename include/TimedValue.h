#pragma once

namespace Blink {
class TimedValue {
  public:
    TimedValue(float totalTime, float beginValue, float endValue);

    float currentValue;
    float elapsedTime;

    bool IsFinished();
    void Update(float diff);

  private:
    float totalTime;
    float beginValue;
    float endValue;
};
}

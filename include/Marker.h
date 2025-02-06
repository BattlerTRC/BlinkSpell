#pragma once

namespace Blink {

class MarkerData {
  public:
    MarkerData(std::string markerNif, float markerScale, float fadeInTime, float fadeOutTime);
    bool triedToLoadMarker = false;
    std::string markerNif;
    float markerScale;
    float fadeInTime;
    float fadeOutTime;
    RE::NiAVObject* object;
    float currentFade  = 0.0f;
    float wantFade     = 0.0f;
    int wantState      = 0;
    bool didLoadMarker = false;
    bool firstUpdate = true;

    bool IsInWorld();
    void UpdateFade(float diff);
    void SetCurrentFade(float now);
    void Free();
    void AddToWorld();
    void RemoveFromWorld();
    bool LoadFromModel();
    void UpdateObject(float totalTime);
    bool Update(float diff, float totalTime);
};
}

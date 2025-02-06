#pragma once
#include "RayCast.h"

namespace Blink {
// Credits to mwilsnd for SmoothCam's code, and SkyrimThiago for Object Manipulation Overhaul's code.
class RayCollector {
  public:
    struct HitResult {
        RE::NiPoint3 normal;
        float hitFraction;
        const RE::hkpCdBody* body;

        RE::NiAVObject* getAVObject() const;
    };

  public:
    RayCollector();
    ~RayCollector() = default;

    virtual void AddRayHit(const RE::hkpCdBody& body, const RE::hkpShapeRayCastCollectorOutput& hitInfo);

    const std::vector<HitResult>& GetHits();
    void Reset();

  private:
    float earlyOutHitFraction { 1.0f }; // 08
    std::uint32_t pad0C {};
    RE::hkpWorldRayCastOutput rayHit; // 10

    std::vector<HitResult> hits {};
};
}

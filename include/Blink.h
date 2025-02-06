#pragma once

#include "Marker.h"
#include "RayCast.h"
#include "TimedValue.h"
#include "Utility.h"

namespace Blink {

void InitState();

class BlinkState {
  public:
    void Initialize();
    void Update(float diff, float totalTime);
    void FireSpell(bool fromSpell);
    void Reset();
    float GetDistortionEffect();
    RE::BSTimer* timer;

  private:
    enum NormalTypes {
        Sideways,
        Up,
        Down,
        Diagonal
    };

    enum TimedValueTypes {
        TeleportDoneRatio,
        Max
    };

    enum InternalStates {
        None,
        Aiming,
        Fire,
        Teleporting,
    };

    RE::PlayerCharacter* playerPointer;

    std::unique_ptr<char[]> allocatedMemory;
    RE::NiPoint3* currentTeleportPoint;
    RE::NiPoint3* sourceTeleportPoint;
    RE::NiPoint3* sourceMovePoint;
    RE::NiPoint3* targetTeleportPoint;
    RE::NiPoint3* targetMarkerPoint;
    RE::NiPoint3* aimVectorPoint;
    RE::NiPoint3* aimVectorPointDoubled;
    RE::NiTransform* thirdPersonTempTransform;
    std::vector<std::vector<TimedValue>> timedValues { static_cast<int>(TimedValueTypes::Max) };
    InternalStates state = InternalStates::None;

    bool lastFireIsFromSpell            = false;
    const float maxTargetDistort        = 0.6f;
    const float distortionDurationAfter = 1.3f;
    float currentTeleportTime           = 0.0f;
    float currentDistortionTime         = 1000.0f;
    float currentMaxDistort             = 0.0f;
    int hotkeyState                     = 0;

    std::vector<MarkerData> markers = {};

    bool CheckCosts(RE::Actor* player, bool notify, bool take);
    RE::NiPoint3 GetCollisionPointFromCamera(RE::NiNode* cameraNode, RE::Actor* caster, std::vector<RE::NiAVObject*> ignore);
    void UpdateAiming(float totalTime, RE::Actor* player);
    bool CalculatePositionFromCollision(RE::NiPoint3 colPos, RE::NiPoint3 colNormal, RE::NiPoint3& teleportPos, RE::NiPoint3& markerPos, std::vector<RE::NiAVObject*> ignore, RE::Actor* caster, bool allowWallClimb);
    NormalTypes CalculateNormalType(RE::NiPoint3 normal);
    void UpdateHotkey();
    bool CheckCollidableObject(RE::NiAVObject* obj, RE::hkpCollidable* havokObj, std::vector<RE::NiAVObject*> ignore);
};

extern BlinkState g_BlinkState;
extern long lastUpdateTime;
extern RayCast rayCast;
}

#pragma once

namespace Blink {
enum class CollisionLayer {
    kUnidentified         = RE::COL_LAYER::kUnidentified,
    kStatic               = RE::COL_LAYER::kStatic,
    kAnimStatic           = RE::COL_LAYER::kAnimStatic,
    kTransparent          = RE::COL_LAYER::kTransparent,
    kClutter              = RE::COL_LAYER::kClutter,
    kWeapon               = RE::COL_LAYER::kWeapon,
    kProjectile           = RE::COL_LAYER::kProjectile,
    kSpell                = RE::COL_LAYER::kSpell,
    kBiped                = RE::COL_LAYER::kBiped,
    kTrees                = RE::COL_LAYER::kTrees,
    kProps                = RE::COL_LAYER::kProps,
    kWater                = RE::COL_LAYER::kWater,
    kTrigger              = RE::COL_LAYER::kTrigger,
    kTerrain              = RE::COL_LAYER::kTerrain,
    kTrap                 = RE::COL_LAYER::kTrap,
    kNonCollidable        = RE::COL_LAYER::kNonCollidable,
    kCloudTrap            = RE::COL_LAYER::kCloudTrap,
    kGround               = RE::COL_LAYER::kGround,
    kPortal               = RE::COL_LAYER::kPortal,
    kDebrisSmall          = RE::COL_LAYER::kDebrisSmall,
    kDebrisLarge          = RE::COL_LAYER::kDebrisLarge,
    kAcousticSpace        = RE::COL_LAYER::kAcousticSpace,
    kActorZone            = RE::COL_LAYER::kActorZone,
    kProjectileZone       = RE::COL_LAYER::kProjectileZone,
    kGasTrap              = RE::COL_LAYER::kGasTrap,
    kShellCasting         = RE::COL_LAYER::kShellCasting,
    kTransparentWall      = RE::COL_LAYER::kTransparentWall,
    kInvisibleWall        = RE::COL_LAYER::kInvisibleWall,
    kTransparentSmallAnim = RE::COL_LAYER::kTransparentSmallAnim,
    kClutterLarge         = RE::COL_LAYER::kClutterLarge,
    kCharController       = RE::COL_LAYER::kCharController,
    kStairHelper          = RE::COL_LAYER::kStairHelper,
    kDeadBip              = RE::COL_LAYER::kDeadBip,
    kBipedNoCC            = RE::COL_LAYER::kBipedNoCC,
    kAvoidBox             = RE::COL_LAYER::kAvoidBox,
    kCollisionBox         = RE::COL_LAYER::kCollisionBox,
    kCameraSphere         = RE::COL_LAYER::kCameraSphere,
    kDoorDetection        = RE::COL_LAYER::kDoorDetection,
    kConeProjectile       = RE::COL_LAYER::kConeProjectile,
    kCamera               = RE::COL_LAYER::kCamera,
    kItemPicker           = RE::COL_LAYER::kItemPicker,
    kLOS                  = RE::COL_LAYER::kLOS,
    kPathingPick          = RE::COL_LAYER::kPathingPick,
    kUnused0              = RE::COL_LAYER::kUnused0,
    kUnused1              = RE::COL_LAYER::kUnused1,
    kSpellExplosion       = RE::COL_LAYER::kSpellExplosion,
    kDroppingPick         = RE::COL_LAYER::kDroppingPick,

    kLivingAndDeadActors = 52
};

class RayCast {
  public:
    struct RayCastResult {
        RE::NiPoint3 normal;
        RE::NiPoint3 pos;
        RE::NiAVObject* object;
        const RE::hkpCollidable* body;

        bool operator==(const RayCastResult& other) const
        {
            return normal == other.normal && pos == other.pos && object == other.object;
        }

        bool operator!=(const RayCastResult& other) const
        {
            return !(*this == other);
        }
    };
    RayCast();
    static std::uint64_t raycastMask;
    static void SetupRaycastMask(const std::vector<CollisionLayer> layers);
    static std::vector<RayCastResult> DoRayCast(RE::Actor* caster, RE::NiPoint3 from, RE::NiPoint3 to);
    static bool CheckRay(RE::NiPoint3 from, RE::NiPoint3 to, std::vector<RE::NiAVObject*> ignore, RE::Actor* caster);
    static float QuickRay(RE::NiPoint3 from, RE::NiPoint3 to, std::vector<RE::NiAVObject*> ignore, RE::Actor* caster);
    static RayCastResult GetBestResult(RE::NiPoint3 from, std::vector<RayCastResult> ls, std::vector<RE::NiAVObject*> ignore, bool any);
};
}

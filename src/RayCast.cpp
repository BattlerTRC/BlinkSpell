#include "RayCast.h"
#include "RayCollector.h"
#include "Utility.h"

namespace Blink {

std::uint64_t RayCast::raycastMask = 0;

RayCast::RayCast()
{
    std::vector<CollisionLayer> layers = {
        CollisionLayer::kUnidentified,
        CollisionLayer::kStatic,
        CollisionLayer::kAnimStatic,
        CollisionLayer::kTransparent,
        CollisionLayer::kClutter,
        CollisionLayer::kWeapon,
        CollisionLayer::kProjectile,
        CollisionLayer::kTrees,
        CollisionLayer::kProps,
        CollisionLayer::kWater,
        CollisionLayer::kTerrain,
        CollisionLayer::kGround,
        CollisionLayer::kDebrisLarge,
        CollisionLayer::kTransparentWall,
        CollisionLayer::kTransparentSmallAnim,
        CollisionLayer::kInvisibleWall,
        CollisionLayer::kCharController,
        CollisionLayer::kStairHelper,
        CollisionLayer::kBipedNoCC,
        CollisionLayer::kCollisionBox,
        CollisionLayer::kLivingAndDeadActors
    };

    SetupRaycastMask(layers);
};

void RayCast::SetupRaycastMask(const std::vector<CollisionLayer> layers)
{
    std::uint64_t m = 0;
    for (const auto& l : layers) {
        std::uint64_t fl = static_cast<std::uint64_t>(1) << static_cast<int>(l);
        m |= fl;
    }
    raycastMask = m;
};

std::vector<RayCast::RayCastResult> RayCast::DoRayCast(RE::Actor* caster, RE::NiPoint3 from, RE::NiPoint3 to)
{
    auto havokWorldScale = RE::bhkWorld::GetWorldScale();
    RE::bhkPickData pickData;
    pickData.rayInput.from = from * havokWorldScale;
    pickData.rayInput.to   = to * havokWorldScale;

    uint32_t collisionFilterInfo = 0;
    caster->GetCollisionFilterInfo(collisionFilterInfo);
    pickData.rayInput.filterInfo = (static_cast<uint32_t>(collisionFilterInfo >> 16) << 16) | static_cast<uint32_t>(RE::COL_LAYER::kItemPicker);
    auto collector               = RayCollector();
    collector.Reset();
    pickData.rayHitCollectorA8 = reinterpret_cast<RE::hkpClosestRayHitCollector*>(&collector);

    if (!caster->GetParentCell()) {
        return {};
    }
    auto world = caster->GetParentCell()->GetbhkWorld();

    if (world) {
        world->PickObject(pickData);
    }

    std::vector<RayCast::RayCastResult> results;
    for (auto& hit : collector.GetHits()) {
        const auto hitPos = from + (to - from) * hit.hitFraction;
        auto obj          = hit.getAVObject();
        results.push_back(RayCastResult { .normal = hit.normal, .pos = hitPos, .object = obj, .body = static_cast<const RE::hkpCollidable*>(hit.body) });
    }
    return results;
}

bool RayCast::CheckRay(RE::NiPoint3 from, RE::NiPoint3 to, std::vector<RE::NiAVObject*> ignore, RE::Actor* caster)
{
    auto ray = DoRayCast(caster, from, to);
    return GetBestResult(from, ray, ignore, true) == RayCastResult {};
}
float RayCast::QuickRay(RE::NiPoint3 from, RE::NiPoint3 to, std::vector<RE::NiAVObject*> ignore, RE::Actor* caster)
{
    auto ray  = DoRayCast(caster, from, to);
    auto best = GetBestResult(from, ray, ignore, false);
    if (best == RayCastResult {}) {
        return -1.0f;
    }
    return Utility::Distance(best.pos, from);
}
RayCast::RayCastResult RayCast::GetBestResult(RE::NiPoint3 from, std::vector<RayCastResult> ls, std::vector<RE::NiAVObject*> ignore, bool any)
{
    std::vector<std::pair<RayCastResult, float>> all {};
    if (!ls.empty()) {
        for (auto& x : ls) {
            if (any) {
                if (Utility::checkCollidableObject(x.object, x.body, ignore, raycastMask)) {
                    return x;
                }
                continue;
            }

            auto other = x.pos;
            float dx   = other[0] - from[0];
            float dy   = other[1] - from[1];
            float dz   = other[2] - from[2];
            float d    = dx * dx + dy * dy + dz * dz;
            all.push_back(std::pair<RayCastResult, float> { x, d });
        }
    }

    if (all.size() > 1) {
        std::sort(all.begin(), all.end(), [](const std::pair<RayCastResult, float>& a, const std::pair<RayCastResult, float>& b) {
            return a.second < b.second;
        });
    }

    for (auto& t : all) {
        auto r = t.first;
        if (Utility::checkCollidableObject(r.object, r.body, ignore, raycastMask)) {
            return r;
        }
    }
    return {};
}
}

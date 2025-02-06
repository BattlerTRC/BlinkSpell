#include "Blink.h"
#include "Raycast.h"
#include "Settings.h"
#include "Utility.h"

namespace Blink {

BlinkState g_BlinkState;
long lastUpdateTime = 0;
RayCast rayCast;

void InitState()
{
    g_BlinkState.Initialize();
}

void BlinkState::Reset()
{
    currentDistortionTime = 1000.0f;
    state                 = InternalStates::None;
    hotkeyState           = 0;

    if (markers.size()) {
        for (auto& m : markers) {
            m.wantState = -1;
        }
    }

    if (!timedValues.empty()) {
        for (int i = 0; i < static_cast<int>(TimedValueTypes::Max); i++) {
            if (i < static_cast<int>(timedValues.size()) && !timedValues[i].empty()) {
                timedValues[i].clear();
            }
        }
    }
}

void BlinkState::FireSpell(bool fromSpell)
{
    if (state != InternalStates::Aiming) {
        return;
    }

    lastFireIsFromSpell = fromSpell;
    state               = InternalStates::Fire;
}

float BlinkState::GetDistortionEffect()
{
    if (currentDistortionTime >= distortionDurationAfter) {
        return 0.0f;
    }

    float mult = Settings::screenDistortion;
    if (mult == 0.0f) {
        return 0.0f;
    }

    if (currentDistortionTime < 0.0f) {
        float timeTotal = currentTeleportTime;
        float curTime   = timeTotal + currentDistortionTime;
        if (timeTotal <= 0.0f) {
            return currentMaxDistort * mult;
        }
        float ratio = curTime / timeTotal;
        return Utility::Curve(ratio, true) * currentMaxDistort * mult;
    }

    float ratioDone = currentDistortionTime / distortionDurationAfter;
    return currentMaxDistort * mult * Utility::Curve(ratioDone, false);
}

void BlinkState::UpdateHotkey()
{
    bool isPressed = Settings::hotkey > 0 && Utility::IsKeyPressed(Settings::hotkey);
    bool isAbort   = Settings::abortHotkey > 0 && Utility::IsKeyPressed(Settings::abortHotkey);

    if (hotkeyState > 0) {
        if (isAbort) {
            hotkeyState = -1;
        } else if (!isPressed) {
            FireSpell(false);
            hotkeyState = 0;
        }
        return;
    }

    if (hotkeyState < 0) {
        if (!isPressed) {
            hotkeyState = 0;
        }
        return;
    }

    if (!isPressed || isAbort) {
        return;
    }

    hotkeyState = 1;
}

bool BlinkState::CheckCosts(RE::Actor* player, bool notify, bool take)
{
    float magickaCost  = Settings::magickaCost;
    float staminaCost  = Settings::staminaCost;
    float recoveryTime = Settings::recoveryTime;

    if (magickaCost > 0.0f && !lastFireIsFromSpell) {
        float has = player->AsActorValueOwner()->GetActorValue(RE::ActorValue::kMagicka);
        if (has < magickaCost) {
            if (notify) {
                Utility::ShowLowMagicka();
            }
            return false;
        }
    }

    if (staminaCost > 0.0f) {
        float has = player->AsActorValueOwner()->GetActorValue(RE::ActorValue::kStamina);
        if (has < staminaCost) {
            if (notify) {
                Utility::ShowLowStamina();
            }
            return false;
        }
    }

    if (recoveryTime > 0.0f) {
        float has = player->GetVoiceRecoveryTime();
        if (has > 0.0f) {
            if (notify) {
                Utility::ShowRecoveryTime();
            }
            return false;
        }
    }

    if (take) {
        if (magickaCost != 0.0f && !lastFireIsFromSpell) {
            player->AsActorValueOwner()->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kMagicka, -magickaCost);
        }
        if (staminaCost != 0.0f) {
            player->AsActorValueOwner()->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kStamina, -staminaCost);
        }
        if (recoveryTime != 0.0f) {
            player->GetHighProcess()->voiceRecoveryTime = recoveryTime;
        }
    }

    return true;
}

void BlinkState::Update(float diff, float totalTime)
{
    auto ui    = RE::UI::GetSingleton();
    auto spell = RE::TESDataHandler::GetSingleton()->LookupForm<RE::SpellItem>(Settings::spellFormId, Settings::spellFormFile);
    if (!ui || ui->GameIsPaused() || !playerPointer) {
        Reset();
        return;
    }

    UpdateHotkey();

    if (currentDistortionTime < distortionDurationAfter) {
        currentDistortionTime += diff;
    }

    if (Settings::autoLearnSpell && spell && !playerPointer->HasSpell(spell)) {
        playerPointer->AddSpell(spell);
    }
    for (int i = (int)markers.size() - 1; i >= 0; i--) {
        if (&markers[i] == nullptr) {
            markers.erase(markers.begin() + i);
            continue;
        }

        if (!markers[i].Update(diff, totalTime)) {
            markers.erase(markers.begin() + i);
        }
    }
    auto castState = Utility::GetCurrentCastingState(playerPointer, spell, hotkeyState);
    switch (state) {
    case InternalStates::None: {
        if (castState == RE::MagicCaster::State::kReady) {
            if (!CheckCosts(playerPointer, true, false)) {
                if (spell) {
                    Utility::InterruptCast(playerPointer, spell);
                }
                if (hotkeyState != 0) {
                    hotkeyState = -1;
                }
                return;
            }

            state = InternalStates::Aiming;

            MarkerData m = MarkerData(
                Settings::markerNif,
                Settings::markerScale,
                Settings::markerFadeInTime,
                Settings::markerFadeOutTime);

            m.wantState = 1;
            markers.push_back(std::move(m));
            UpdateAiming(totalTime, playerPointer);
        };
    } break;
    case InternalStates::Aiming: {
        if (castState != RE::MagicCaster::State::kReady && castState != RE::MagicCaster::State::kUnk04) {
            state = InternalStates::None;
            for (auto& m : markers) {
                m.wantState = -1;
            }
        } else {
            if (!CheckCosts(playerPointer, true, false)) {
                state = InternalStates::None;
                for (auto& m : markers) {
                    m.wantState = -1;
                }
                if (spell) {
                    Utility::InterruptCast(playerPointer, spell);
                }
                if (hotkeyState != 0) {
                    hotkeyState = -1;
                }
                return;
            }
            UpdateAiming(totalTime, playerPointer);
        }
    } break;
    case InternalStates::Fire: {
        for (auto& m : markers) {
            m.wantState = -1;
        }
        if (!CheckCosts(playerPointer, true, true)) {
            state = InternalStates::None;
            if (hotkeyState != 0) {
                hotkeyState = -1;
            }
            return;
        }

        auto dataHandler = RE::TESDataHandler::GetSingleton();
        auto imod        = dataHandler->LookupForm<RE::TESImageSpaceModifier>(Settings::imodFormId, Settings::imodFormFile);
        Utility::ApplyIMod(imod);
        Utility::PlaySound(Settings::soundFormId, playerPointer);

        float distance        = targetTeleportPoint->GetDistance(*sourceTeleportPoint);
        float speed           = Settings::teleportSpeed;
        currentDistortionTime = 0.0f;
        currentTeleportTime   = 0.0f;
        float maxDistance     = fmax(100.0f, Settings::maxDistance);
        currentMaxDistort     = maxTargetDistort * (distance / maxDistance);

        if (speed > 0.0f) {
            if (speed < 1.0f) {
                speed = 1.0f;
            }
            float time = distance / speed;
            if (time > 0.0f) {
                timedValues[static_cast<std::size_t>(TimedValueTypes::TeleportDoneRatio)].emplace_back(time, 0, 1);
                currentDistortionTime = -time;
                currentTeleportTime   = maxDistance / speed;
            }
        }
        state = InternalStates::Teleporting;
    } break;
    case InternalStates::Teleporting: {
        auto& tm    = timedValues.at(static_cast<std::size_t>(TimedValueTypes::TeleportDoneRatio));
        float ratio = 1.0f;

        if (!tm.empty() && tm.size() != 0) {
            tm[0].Update(diff);
            ratio = tm[0].currentValue;
            if (tm[0].IsFinished()) {
                timedValues[static_cast<size_t>(TimedValueTypes::TeleportDoneRatio)].clear();
            }
        }

        currentTeleportPoint->x = (targetTeleportPoint->x - sourceTeleportPoint->x) * ratio + sourceTeleportPoint->x;
        currentTeleportPoint->y = (targetTeleportPoint->y - sourceTeleportPoint->y) * ratio + sourceTeleportPoint->y;
        currentTeleportPoint->z = (targetTeleportPoint->z - sourceTeleportPoint->z) * ratio + sourceTeleportPoint->z;

        playerPointer->SetPosition(*currentTeleportPoint, true);

        if (ratio >= 1.0f) {
            state = InternalStates::None;
        }
    } break;
    default:
        break;
    }
}

RE::NiPoint3 BlinkState::GetCollisionPointFromCamera(RE::NiNode* cameraNode, RE::Actor* caster, std::vector<RE::NiAVObject*> ignore)
{
    RE::NiPoint3 from;
    RE::NiPoint3 to;

    auto nodePos = cameraNode->world.translate;
    from.x       = nodePos.x;
    from.y       = nodePos.y;
    from.z       = nodePos.z;

    Utility::Translate(cameraNode->world, *aimVectorPointDoubled, *targetTeleportPoint);

    to.x = targetTeleportPoint->x;
    to.y = targetTeleportPoint->y;
    to.z = targetTeleportPoint->z;

    auto ray  = rayCast.DoRayCast(caster, from, to);
    auto best = rayCast.GetBestResult(from, ray, ignore, false);

    if (best == RayCast::RayCastResult {}) {
        return to;
    }
    return best.pos;
}

void BlinkState::UpdateAiming(float totalTime, RE::Actor* player)
{
    MarkerData* currentMarker;
    if (markers.size()) {
        currentMarker = &markers[markers.size() - 1];
    }
    if (!currentMarker || !currentMarker->object || currentMarker->wantState != 0) {
        return;
    }

    auto camera     = RE::PlayerCamera::GetSingleton();
    auto cameraNode = camera->cameraRoot.get();

    if (!cameraNode) {
        return;
    }

    if (!player->IsPlayerRef()) {
        return;
    }

    std::vector<RE::NiAVObject*> playerNode(2);
    playerNode[0] = player->Get3D1(false);
    playerNode[1] = player->Get3D1(true);

    if (!playerNode[0] || !playerNode[1]) {
        return;
    }

    *sourceMovePoint = player->GetPosition();

    auto cameraState = camera->currentState.get();
    if (cameraState && cameraState->id == RE::CameraState::kFirstPerson) {
        auto worldTransform = cameraNode->world;

        *sourceTeleportPoint = worldTransform.translate;

        *targetTeleportPoint = worldTransform * *aimVectorPoint;
    } else {
        auto headNode = player->GetNodeByName("NPC Head [Head]");
        if (headNode) {
            *sourceTeleportPoint = headNode->world.translate;
        } else {
            auto playerPos         = player->GetPosition();
            sourceTeleportPoint->x = playerPos.x;
            sourceTeleportPoint->y = playerPos.y;
            sourceTeleportPoint->z = playerPos.z + Settings::playerRadius;
        }

        auto cameraCol            = GetCollisionPointFromCamera(cameraNode, player, playerNode);
        *thirdPersonTempTransform = cameraNode->world;

        targetTeleportPoint->x = cameraCol[0];
        targetTeleportPoint->y = cameraCol[1];
        targetTeleportPoint->z = cameraCol[2];

        Utility::LookAt(thirdPersonTempTransform, targetTeleportPoint);
        Utility::Translate(*thirdPersonTempTransform, *aimVectorPoint, *targetTeleportPoint);
    }

    bool allowWallClimb = true;
    {
        RE::NiPoint3 from = { sourceTeleportPoint->x, sourceTeleportPoint->y, sourceTeleportPoint->z };
        RE::NiPoint3 to   = { targetTeleportPoint->x, targetTeleportPoint->y, targetTeleportPoint->z };

        auto ray = rayCast.DoRayCast(player, from, to);

        RE::NiPoint3 collisionPosition;
        RE::NiPoint3 collisionNormal;

        auto best = rayCast.GetBestResult(from, ray, playerNode, false);

        if (best == RayCast::RayCastResult {}) {
            collisionPosition = to;
            RE::NiPoint3 revNormal;
            float rx = to[0] - from[0];
            float ry = to[1] - from[1];
            float rz = to[2] - from[2];
            float rd = static_cast<float>(sqrt(rx * rx + ry * ry + rz * rz));
            if (rd > 0.0f) {
                rx /= rd;
                ry /= rd;
                rz /= rd;
            }
            revNormal[0]    = rx * -1.0f;
            revNormal[1]    = ry * -1.0f;
            revNormal[2]    = rz * -1.0f;
            collisionNormal = revNormal;
            allowWallClimb  = false;
        } else {
            collisionPosition = best.pos;
            collisionNormal   = best.normal;

            float nlen = collisionNormal.Length();
            if (nlen > 0.0f) {
                collisionNormal[0] /= nlen;
                collisionNormal[1] /= nlen;
                collisionNormal[2] /= nlen;
            }

            if (!Settings::allowLedgeClimbNpc) {
                auto obj = best.object;
                if (obj) {
                    auto layer = static_cast<Utility::CollisionLayer>(obj->GetCollisionLayer());
                    switch (layer) {
                    case Utility::CollisionLayer::kCharController:
                    case Utility::CollisionLayer::kBipedNoCC:
                    case Utility::CollisionLayer::kLivingAndDeadActors:
                        allowWallClimb = false;
                        break;
                    default:
                        break;
                    }
                }
            }
        }

        float fullDist = Utility::Distance(collisionPosition, from);
        float ratioInc = 1.0f;

        if (fullDist > 0.0f) {
            ratioInc = fmax(10.0f, Settings::teleportIncrementalCheck) / fullDist;
        }
        ratioInc = fmax(0.02f, ratioInc);

        float ratioNow = 1.0f;
        RE::NiPoint3 checkPos;
        bool ok = false;

        while (ratioNow > 0.0f) {
            for (int i = 0; i < 3; i++) {
                checkPos[i] = (collisionPosition[i] - from[i]) * ratioNow + from[i];
            }

            RE::NiPoint3 tpPos;
            RE::NiPoint3 mrPos;

            if (CalculatePositionFromCollision(checkPos, collisionNormal, tpPos, mrPos, playerNode, player, allowWallClimb)) {
                *targetTeleportPoint = tpPos;
                *targetMarkerPoint   = mrPos;

                ok = true;
                break;
            }
            ratioNow -= ratioInc;
        }

        if (!ok) {
            targetTeleportPoint = sourceMovePoint;

            targetMarkerPoint->x = sourceMovePoint->x;
            targetMarkerPoint->y = sourceMovePoint->y;
            targetMarkerPoint->z = sourceMovePoint->z + Settings::playerRadius;
        }
    }
    currentMarker->object->local.translate = *targetMarkerPoint;

    currentMarker->UpdateObject(totalTime);
}

bool BlinkState::CalculatePositionFromCollision(RE::NiPoint3 colPos, RE::NiPoint3 colNormal, RE::NiPoint3& teleportPos, RE::NiPoint3& markerPos, std::vector<RE::NiAVObject*> ignore, RE::Actor* caster, bool allowWallClimb)
{
    auto ntype       = CalculateNormalType(colNormal);
    float tpush      = 1.0f;
    bool canMoveDown = true;
    switch (ntype) {
    case NormalTypes::Up: {
        canMoveDown = false;
    } break;
    case NormalTypes::Down:
        break;
    case NormalTypes::Sideways: {
        tpush             = Settings::playerRadius;
        auto pushed       = Utility::PushAway(colPos, colNormal, Settings::playerRadius * 2.0f + 2.0f);
        auto pushedSource = Utility::PushAway(colPos, colNormal, 0.5f);
        if (!rayCast.CheckRay(pushedSource, pushed, ignore, caster)) {
            return false;
        }
    } break;
    case NormalTypes::Diagonal:
        break;
    default:
        break;
    }
    auto tpos = Utility::PushAway(colPos, colNormal, tpush);

    bool skipSnap   = false;
    bool skipHeight = false;
    if (allowWallClimb) {
        float wallClimb = Settings::maxWallClimbHeight;
        if (ntype == NormalTypes::Sideways && wallClimb > 0.0f) {
            auto srcPos = tpos;
            auto dstPos = srcPos;
            dstPos[2] += wallClimb;

            float dist    = rayCast.QuickRay(srcPos, dstPos, ignore, caster);
            float pheight = Settings::playerRadius * 3.0f;
            if (dist < 0.0f || dist > pheight) {
                auto colNormalRev = Utility::Invert(colNormal);

                if (dist < 0.0f) {
                    srcPos[2] += wallClimb;
                } else {
                    srcPos[2] += (dist - 5.0f);
                }

                float width = Settings::wallClimbWidth;
                dstPos      = Utility::PushAway(srcPos, colNormalRev, width);
                if (rayCast.CheckRay(srcPos, dstPos, ignore, caster)) {
                    srcPos = Utility::PushAway(srcPos, colNormalRev, width - Settings::playerRadius);
                    dstPos = srcPos;
                    dstPos[2] -= wallClimb;

                    float dist2 = rayCast.QuickRay(srcPos, dstPos, ignore, caster);
                    if (dist2 >= 0.0f && (srcPos[2] - dist2) > (tpos[2] - 40.0f)) {
                        srcPos[2] -= (dist2 - 1.0f);

                        // Make sure there's enough height for player to exist there.
                        bool climb = true;

                        {
                            dstPos = srcPos;
                            dstPos[2] += pheight;

                            float dist3 = rayCast.QuickRay(srcPos, dstPos, ignore, caster);
                            if (dist3 >= 0.0f) {
                                float moveDown = pheight - dist3;
                                dstPos[2]      = srcPos[2] - moveDown;
                                if (!rayCast.CheckRay(srcPos, dstPos, ignore, caster)) {
                                    climb = false;
                                } else {
                                    srcPos[2] -= moveDown;
                                }
                            }

                            if (climb) {
                                tpos       = srcPos;
                                skipSnap   = true;
                                skipHeight = true;
                            }
                        }
                    }
                }
            }
        }
    }

    // Make sure there's enough height for player to exist there.
    if (!skipHeight) {
        float pheight = Settings::playerRadius * 3.0f;
        auto srcPos   = tpos;
        auto dstPos   = srcPos;
        dstPos[2] += pheight;

        float dist = rayCast.QuickRay(srcPos, dstPos, ignore, caster);
        if (dist >= 0.0f) {
            if (!canMoveDown)
                return false;

            float moveDown = pheight - dist;
            dstPos[2]      = srcPos[2] - moveDown;
            if (!rayCast.CheckRay(srcPos, dstPos, ignore, caster))
                return false;

            tpos[2] -= moveDown;
        }
    }

    // Try to snap to ground

    float maxSnap = Settings::maxSnapToGroundDistance;
    if (!skipSnap && maxSnap > 0.0f && canMoveDown) {
        auto srcPos = tpos;
        auto dstPos = srcPos;
        dstPos[2] -= maxSnap;

        float dist = rayCast.QuickRay(srcPos, dstPos, ignore, caster);
        if (dist >= 0.0f) {
            float reduce = dist - 1.0f;
            tpos[2] -= reduce;
        }
    }

    teleportPos       = tpos;
    RE::NiPoint3 mpos = tpos;
    mpos[2] += Settings::playerRadius;
    markerPos = mpos;
    return true;
}

BlinkState::NormalTypes BlinkState::CalculateNormalType(RE::NiPoint3 normal)
{
    RE::NiPoint2 two = { fmax(normal[0], normal[1]), normal[2] };
    float len        = sqrtf(two[0] * two[0] + two[1] * two[1]);
    if (len > 0.0f) {
        two[0] /= len;
        two[1] /= len;
    }

    if (abs(two[0]) == 1.0f) {
        return NormalTypes::Sideways;
    }
    if (abs(two[1]) == 1.0f) {
        return two[1] > 0.0f ? NormalTypes::Up : NormalTypes::Down;
    }

    double angle = atan2(two[1], two[0]) * 180.0 / std::numbers::pi;
    if (angle < 0.0) {
        angle += 180.0;
    }
    if (angle <= 30.0 || angle >= 150.0) {
        return NormalTypes::Sideways;
    }
    if (angle <= 60.0 || angle >= 120.0) {
        return NormalTypes::Diagonal;
    }
    return two[1] >= 0.0f ? NormalTypes::Up : NormalTypes::Down;
}

void BlinkState::Initialize()
{
    allocatedMemory = std::make_unique<char[]>(0x110);

    targetMarkerPoint        = reinterpret_cast<RE::NiPoint3*>(allocatedMemory.get());
    targetTeleportPoint      = reinterpret_cast<RE::NiPoint3*>(allocatedMemory.get() + 0x10);
    aimVectorPoint           = reinterpret_cast<RE::NiPoint3*>(allocatedMemory.get() + 0x20);
    sourceTeleportPoint      = reinterpret_cast<RE::NiPoint3*>(allocatedMemory.get() + 0x30);
    currentTeleportPoint     = reinterpret_cast<RE::NiPoint3*>(allocatedMemory.get() + 0x40);
    aimVectorPointDoubled    = reinterpret_cast<RE::NiPoint3*>(allocatedMemory.get() + 0x50);
    thirdPersonTempTransform = reinterpret_cast<RE::NiTransform*>(allocatedMemory.get() + 0x60);
    sourceMovePoint          = reinterpret_cast<RE::NiPoint3*>(allocatedMemory.get() + 0x100);

    aimVectorPoint->x = 0.0f;
    aimVectorPoint->y = fmax(100.0f, fmin(8000.0f, Settings::maxDistance));
    aimVectorPoint->z = 0.0f;

    aimVectorPointDoubled->x = 0.0f;
    aimVectorPointDoubled->y = 2000.0f + fmax(100.0f, fmin(8000.0f, Settings::maxDistance));
    aimVectorPointDoubled->x = 0.0f;

    timer = RE::BSTimer::GetSingleton();
    timedValues.resize(static_cast<int>(TimedValueTypes::Max));
    playerPointer = RE::PlayerCharacter::GetSingleton();
}
}

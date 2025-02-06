#pragma once

class Utility {
  public:
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

    static float Length(const RE::NiPoint3 vec)
    {
        float dx = vec[0];
        float dy = vec[1];
        float dz = vec[2];
        return std::sqrt(dx * dx + dy * dy + dz * dz);
    }

    static float Distance(const RE::NiPoint3 a, const RE::NiPoint3 b)
    {
        RE::NiPoint3 c = {
            b[0] - a[0],
            b[1] - a[1],
            b[2] - a[2]
        };

        return Length(c);
    }

    static float Curve(float ratio, bool rising)
    {
        ratio      = std::max(0.0f, std::min(1.0f, ratio));
        double amt = 0.3 / (ratio + 0.25) - 0.25;
        amt        = std::max(0.0, std::min(1.0, amt));
        if (rising) {
            amt = 1.0 - amt;
        }
        return static_cast<float>(amt);
    }

    static RE::NiPoint3 PushAway(RE::NiPoint3 pos, RE::NiPoint3 normal, float amount)
    {
        RE::NiPoint3 result;
        for (int i = 0; i < 3; ++i) {
            result[i] = pos[i] + normal[i] * amount;
        }
        return result;
    }

    static void ApplyIMod(RE::TESImageSpaceModifier* iModForm)
    {
        if (!iModForm) {
            return;
        }

        auto iModInstance = RE::ImageSpaceModifierInstanceForm::Trigger(iModForm, 1.0f, nullptr);

        iModInstance->Apply();
    };

    static void ShowLowMagicka()
    {
        RE::HUDMenu::FlashMeter(RE::ActorValue::kMagicka);

        auto msg = RE::MagicSystem::GetCannotCastString(RE::MagicSystem::CannotCastReason::kMagicka);

        RE::DebugNotification(msg, nullptr);
    }

    static void ShowLowStamina()
    {
        RE::HUDMenu::FlashMeter(RE::ActorValue::kStamina);
    }

    static void ShowRecoveryTime()
    {
        RE::HUDMenu::FlashMeter(RE::ActorValue::kVoiceRate);
    }

    static void PlaySound(RE::FormID sound, RE::TESObjectREFR* ref)
    {
        auto audioManager = RE::BSAudioManager::GetSingleton();

        auto descriptor = RE::TESForm::LookupByID<RE::BGSSoundDescriptorForm>(sound);

        if (!descriptor->Is(RE::FormType::SoundRecord)) {
            return;
        }

        RE::BSSoundHandle handle;
        handle.soundID            = static_cast<uint32_t>(-1);
        handle.assumeSuccess      = false;
        *(uint32_t*)&handle.state = 0;
        audioManager->BuildSoundDataFromDescriptor(handle, descriptor, 16);

        if (handle.SetPosition({ ref->data.location.x, ref->data.location.y, ref->data.location.z })) {
            handle.SetObjectToFollow(ref->Get3D());
            handle.Play();
        }
    }

    static void InterruptCast(RE::Actor* player, RE::MagicItem* spell)
    {
        if (!player || !spell) {
            return;
        }

        for (int i = 0; i <= 2; i++) {
            auto castingSource = static_cast<RE::MagicSystem::CastingSource>(i);
            auto caster        = player->GetMagicCaster(castingSource);
            if (!caster) {
                continue;
            }

            auto item = caster->currentSpell;
            if (!item || item != spell) {
                continue;
            }

            player->InterruptCast(true);
            return;
        }
    }

    static RE::MagicCaster::State GetCurrentCastingState(RE::Actor* player, RE::MagicItem* spell, int hotkeyState)
    {
        if (!player) {
            return RE::MagicCaster::State::kNone;
        }
        if (hotkeyState > 0) {
            return RE::MagicCaster::State::kReady;
        }

        if (!spell) {
            return RE::MagicCaster::State::kNone;
        }

        for (int i = 0; i <= 2; i++) {
            auto castingSource = static_cast<RE::MagicSystem::CastingSource>(i);
            auto caster        = player->GetMagicCaster(castingSource);
            if (!caster) {
                continue;
            }

            auto item = caster->currentSpell;
            if (!item || item != spell) {
                continue;
            }

            return caster->state.get();
        }

        return RE::MagicCaster::State::kNone;
    }

    static void LookAt(RE::NiTransform* transform, const RE::NiPoint3* target)
    {
        RE::NiPoint3 direction = *target - transform->translate;
        direction.Unitize();

        RE::NiPoint3 up = { 0.0f, 0.0f, 1.0f };

        RE::NiPoint3 right = up.Cross(direction);
        right.Unitize();

        up = direction.Cross(right);
        up.Unitize();

        transform->rotate.entry[0][0] = -right.x;
        transform->rotate.entry[1][0] = -right.y;
        transform->rotate.entry[2][0] = -right.z;

        transform->rotate.entry[0][1] = direction.x;
        transform->rotate.entry[1][1] = direction.y;
        transform->rotate.entry[2][1] = direction.z;

        transform->rotate.entry[0][2] = up.x;
        transform->rotate.entry[1][2] = up.y;
        transform->rotate.entry[2][2] = up.z;
    }

    static RE::NiPoint3 Invert(RE::NiPoint3 point)
    {
        std::array<float, 3> r = { point.x, point.y, point.z };
        for (int i = 0; i < r.size(); i++) {
            r[i] *= -1.0f;
        }
        return RE::NiPoint3 { r[0], r[1], r[2] };
    }

    static bool IsKeyPressed(std::uint32_t keyCode)
    {
        auto inputManager = RE::BSInputDeviceManager::GetSingleton();
        if (!inputManager) {
            return false;
        }
        auto keyboard = inputManager->GetKeyboard();
        if (keyboard) {
            auto keyState = keyboard->curState[keyCode];
            auto pressed  = (keyState & 0x80) != 0;
            if (pressed) {
                return true;
            }
        }

        auto mouse = inputManager->GetMouse();
        if (mouse && keyCode > SKSE::InputMap::kMacro_MouseButtonOffset && keyCode < SKSE::InputMap::kGamepadButtonOffset_A) {
            auto mouseButtonState = mouse->dInputNextState.rgbButtons[keyCode - SKSE::InputMap::kMacro_MouseButtonOffset];
            if ((mouseButtonState & 0x80) != 0) {
                return true;
            }
        }

        auto gamepad = inputManager->GetGamepad();
        if (gamepad && gamepad->IsPressed(SKSE::InputMap::GamepadKeycodeToMask(keyCode))) {
            return true;
        }

        return false;
    };

    static void Translate(const RE::NiTransform& transform, const RE::NiPoint3& amount, RE::NiPoint3& result)
    {
        const auto& rotation = transform.rotate.entry;

        result.x = rotation[0][0] * amount.x + rotation[0][1] * amount.y + rotation[0][2] * amount.z + transform.translate.x;
        result.y = rotation[1][0] * amount.x + rotation[1][1] * amount.y + rotation[1][2] * amount.z + transform.translate.y;
        result.z = rotation[2][0] * amount.x + rotation[2][1] * amount.y + rotation[2][2] * amount.z + transform.translate.z;
    };
    static bool checkCollidableObject(RE::NiAVObject* obj, const RE::hkpCollidable* havokObj, std::vector<RE::NiAVObject*> ignore, uint64_t raycastMask)
    {
        if (havokObj) {
            std::uint32_t flags = static_cast<uint32_t>(havokObj->GetCollisionLayer()) & 0x7F;
            std::uint64_t mask  = static_cast<std::uint64_t>(1) << flags;
            if ((raycastMask & mask) == 0) {
                return false;
            }
        }

        if (!obj) {
            return true;
        }

        if (!ignore.empty()) {
            for (const auto& ignoredObj : ignore) {
                if (ignoredObj && ignoredObj == obj) {
                    return false;
                }
            }
        }

        return true;
    }
};

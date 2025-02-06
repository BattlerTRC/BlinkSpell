#include "Hooks.h"
#include "Blink.h"

namespace Hooks {
void Install() noexcept
{
    REL::Relocation main_update { RELOCATION_ID(35565, 36564), REL::Relocate(0x748, 0xc26, 0x7ee) };
    stl::write_thunk_call<MainUpdate>(main_update);
    logger::info("Installed main update hook.");
    logger::info("");

    REL::Relocation<std::uintptr_t> tescamera_update { RELOCATION_ID(49852, 50784), REL::Relocate(0x1A6, 0x1A6) }; // 84AB90, 876700
    stl::write_thunk_call<TESCamera_Update>(tescamera_update);
    logger::info("Installed camera update hook.");
    logger::info("");
}

i32 MainUpdate::Thunk() noexcept
{
    if (!&Blink::g_BlinkState) {
        return func();
    }
    long now  = Blink::g_BlinkState.timer->unk24;
    long diff = 0;
    if (Blink::lastUpdateTime) {
        diff = now - Blink::lastUpdateTime;
    }
    Blink::lastUpdateTime = now;
    if (&Blink::g_BlinkState != nullptr) {
        Blink::g_BlinkState.Update(static_cast<float>(diff) / 1000.0f, static_cast<float>(now) / 1000.0f);
    }
    return func();
}

void TESCamera_Update::Thunk(RE::TESCamera* a_camera) noexcept
{
    func(a_camera);
    auto state = &Blink::g_BlinkState;
    if (!state) {
        return;
    }
    float amt = state->GetDistortionEffect();
    if (amt == 0.0f) {
        return;
    }
    auto cameraBase = a_camera;
    if (!a_camera) {
        return;
    }

    auto playerCamera = static_cast<RE::PlayerCamera*>(cameraBase);
    if (!playerCamera) {
        return;
    }

    auto node = playerCamera->cameraRoot.get();
    if (!node) {
        return;
    }

    if (amt > 0.0f) {
        node->local.rotate.entry[2][2] *= 1.0f + amt;
    } else {
        node->local.rotate.entry[2][2] /= 1.0f - amt;
    }
    RE::NiUpdateData updateData;
    updateData.time  = 0.0f;
    updateData.flags = RE::NiUpdateData::Flag::kNone;
    node->Update(updateData);
}
} // namespace Hooks

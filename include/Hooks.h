#pragma once

namespace Hooks {

void Install() noexcept;

class MainUpdate {
  public:
    static i32 Thunk() noexcept;
    inline static REL::Relocation<decltype(&Thunk)> func;
};

class TESCamera_Update {
  public:
    static void Thunk(RE::TESCamera* a_camera) noexcept;

    static inline REL::Relocation<decltype(&Thunk)> func;
};
} // namespace Hooks

#pragma once

#include <string>

#include "SimpleIni.h"

class Settings {
  public:
    static void LoadSettings() noexcept;

    inline static bool debugLogging {};

    // Other settings
    inline static float maxDistance;
    inline static std::string markerNif;
    inline static float markerScale;
    inline static uint32_t markerFormId;
    inline static std::string markerFormFile;
    inline static float teleportSpeed;
    inline static float teleportIncrementalCheck;
    inline static float playerRadius;
    inline static bool autoLearnSpell;
    inline static float screenDistortion;
    inline static uint32_t spellFormId;
    inline static std::string spellFormFile;
    inline static float maxSnapToGroundDistance;
    inline static float maxWallClimbHeight;
    inline static float wallClimbWidth;
    inline static int hotkey;
    inline static int abortHotkey;
    inline static uint32_t soundFormId;
    inline static std::string soundFormFile;
    inline static uint32_t imodFormId;
    inline static std::string imodFormFile;
    inline static float magickaCost;
    inline static float staminaCost;
    inline static float recoveryTime;
    inline static bool allowLedgeClimbNpc;
    inline static float markerFadeInTime;
    inline static float markerFadeOutTime;
};

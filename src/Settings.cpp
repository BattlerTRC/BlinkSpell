#include "Settings.h"

void Settings::LoadSettings() noexcept
{
    logger::info("Loading settings");

    CSimpleIniA ini;
    ini.SetUnicode();
    ini.LoadFile(R"(.\Data\SKSE\Plugins\BlinkSpell.ini)");

    debugLogging = ini.GetBoolValue("Log", "Debug");

    if (debugLogging) {
        spdlog::set_level(spdlog::level::debug);
        logger::debug("Debug logging enabled");
    }

    maxDistance              = static_cast<float>(ini.GetDoubleValue("Settings", "MaxDistance", 3000.0f));
    markerNif                = ini.GetValue("Settings", "MarkerNif", "Magic\\MAGINVGenericSpellArt.nif");
    markerScale              = static_cast<float>(ini.GetDoubleValue("Settings", "MarkerScale", 1.5f));
    markerFormId             = static_cast<uint32_t>(ini.GetLongValue("Settings", "MarkerFormId", 0xD8C));
    markerFormFile           = ini.GetValue("Settings", "MarkerFormFile", "BlinkSpell.esp");
    teleportSpeed            = static_cast<float>(ini.GetDoubleValue("Settings", "TeleportSpeed", 10000.0f));
    teleportIncrementalCheck = static_cast<float>(ini.GetDoubleValue("Settings", "TeleportIncrementalCheck", 50.0f));
    playerRadius             = static_cast<float>(ini.GetDoubleValue("Settings", "PlayerRadius", 60.0f));
    autoLearnSpell           = ini.GetBoolValue("Settings", "AutoLearnSpell", true);
    screenDistortion         = static_cast<float>(ini.GetDoubleValue("Settings", "ScreenDistortion", 1.0f));
    spellFormId              = static_cast<uint32_t>(ini.GetLongValue("Settings", "SpellFormId", 0xD63));
    spellFormFile            = ini.GetValue("Settings", "SpellFormFile", "BlinkSpell.esp");
    maxSnapToGroundDistance  = static_cast<float>(ini.GetDoubleValue("Settings", "MaxSnapToGroundDistance", 150.0f));
    maxWallClimbHeight       = static_cast<float>(ini.GetDoubleValue("Settings", "MaxWallClimbHeight", 300.0f));
    wallClimbWidth           = static_cast<float>(ini.GetDoubleValue("Settings", "WallClimbWidth", 200.0f));
    hotkey                   = static_cast<int>(ini.GetLongValue("Settings", "Hotkey", 0));
    abortHotkey              = static_cast<int>(ini.GetLongValue("Settings", "AbortHotkey", 0));
    soundFormId              = static_cast<uint32_t>(ini.GetLongValue("Settings", "SoundFormId", 0x3F37C));
    soundFormFile            = ini.GetValue("Settings", "SoundFormFile", "Skyrim.esm");
    imodFormId               = static_cast<uint32_t>(ini.GetLongValue("Settings", "IModFormId", 0x800));
    imodFormFile             = ini.GetValue("Settings", "IModFormFile", "BlinkSpell.esp");
    magickaCost              = static_cast<float>(ini.GetDoubleValue("Settings", "MagickaCost", 0.0f));
    staminaCost              = static_cast<float>(ini.GetDoubleValue("Settings", "StaminaCost", 0.0f));
    recoveryTime             = static_cast<float>(ini.GetDoubleValue("Settings", "RecoveryTime", 0.0f));
    allowLedgeClimbNpc       = ini.GetBoolValue("Settings", "AllowLedgeClimbNPC", false);
    markerFadeInTime         = static_cast<float>(ini.GetDoubleValue("Settings", "MarkerFadeInTime", 0.1f));
    markerFadeOutTime        = static_cast<float>(ini.GetDoubleValue("Settings", "MarkerFadeOutTime", 0.3f));

    logger::info("Loaded settings");
    logger::info("");
};

#include "Marker.h"
#include "Settings.h"

namespace Blink {
MarkerData::MarkerData(std::string markerNif, float markerScale, float fadeInTime, float fadeOutTime)
{
    this->triedToLoadMarker = false;
    this->object            = nullptr;
    this->markerNif         = markerNif;
    this->markerScale       = markerScale;
    this->fadeInTime        = fadeInTime;
    this->fadeOutTime       = fadeOutTime;
    this->currentFade       = 0.0f;
    this->wantFade          = 0.0f;
    this->wantState         = 0;
    this->didLoadMarker     = false;
}
bool MarkerData::IsInWorld()
{
    return object && object->parent;
}

void MarkerData::UpdateFade(float diff)
{
    if (diff <= 0.0f || currentFade == wantFade) {
        return;
    }

    if (currentFade < wantFade) {
        float time = fadeInTime;
        if (time <= 0.0f) {
            currentFade = wantFade;
            return;
        }

        float change = diff / time;
        currentFade += change;
        if (currentFade > wantFade) {
            currentFade = wantFade;
        }
    } else {
        float time = fadeOutTime;
        if (time <= 0.0f) {
            currentFade = wantFade;
            return;
        }
        float change = diff / time;
        currentFade -= change;
        if (currentFade < wantFade) {
            currentFade = wantFade;
        }
    }
}

void MarkerData::SetCurrentFade(float now)
{
    if (!object) {
        return;
    }

    auto m = object->AsFadeNode();
    if (m) {
        m->GetRuntimeData().currentFade = now;
        m->GetRuntimeData().unk140      = now;
    }
}

void MarkerData::Free()
{
    if (object) {
        RemoveFromWorld();
        object->DecRefCount();
        object = nullptr;
    }
}

bool MarkerData::LoadFromModel()
{
    RE::NiPointer<RE::NiNode> loadedModel;
    constexpr RE::BSModelDB::DBTraits::ArgsType args {};

    if (const auto error = RE::BSModelDB::Demand(markerNif.c_str(), loadedModel, args); error == RE::BSResource::ErrorCode::kNone) {
        RE::NiAVObject* loadedModelAsNiObject = loadedModel.get();

        if (loadedModel) {
            object = loadedModelAsNiObject ? loadedModelAsNiObject : loadedModel->Clone();
            object->IncRefCount();
            object->local.scale = markerScale;
            fadeInTime          = Settings::markerFadeInTime;
            fadeOutTime         = Settings::markerFadeOutTime;
            return true;
        }
    } else {
        logger::warn("Failed to load marker model: {} [{}]", markerNif, static_cast<int>(error));
    }
    return false;
}

void MarkerData::AddToWorld()
{
    if (!object) {
        return;
    }

    auto player = RE::PlayerCharacter::GetSingleton();
    if (!player) {
        return;
    }
    auto node = player->Get3D();
    if (!node || !node->parent) {
        return;
    }

    currentFade = 0.0f;
    wantFade    = 1.0f;
    SetCurrentFade(currentFade);
    node->parent->AttachChild(object);
}

void MarkerData::RemoveFromWorld()
{
    if (object) {
        object->parent->DetachChild(object);
    }
}

void MarkerData::UpdateObject(float totalTime)
{
    if (!object) {
        return;
    }

    RE::NiUpdateData updateData;
    updateData.time  = totalTime;
    updateData.flags = totalTime > 0.0f ? RE::NiUpdateData::Flag::kDirty : RE::NiUpdateData::Flag::kNone;

    object->Update(updateData);
}

bool MarkerData::Update(float diff, float totalTime)
{
    if (wantState > 0 && !triedToLoadMarker) {
        triedToLoadMarker = true;
        didLoadMarker     = LoadFromModel();
    }

    if (!didLoadMarker) {
        return true;
    }

    if (wantState > 0 && object) {
        wantState   = 0;
        currentFade = 0.0f;
        wantFade    = 1.0f;
        SetCurrentFade(0.0f);
        AddToWorld();
    } else if (wantState < 0) {
        wantFade = 0.0f;
    }

    if (object != nullptr) {
        UpdateFade(diff);
        SetCurrentFade(currentFade);
    }

    if (object && IsInWorld() && wantState < 0 && currentFade > 0.0f) {
        UpdateObject(totalTime);
    }

    if (wantState < 0 && currentFade <= 0.0f) {
        Free();
        return false;
    }

    return true;
}
}

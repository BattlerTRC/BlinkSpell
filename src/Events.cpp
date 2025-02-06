#include "Events.h"
#include "Blink.h"
#include "Settings.h"

namespace Events {
RE::BSEventNotifyControl SpellCastEvent::ProcessEvent(const RE::TESSpellCastEvent* a_event, RE::BSTEventSource<RE::TESSpellCastEvent>* a_eventSource) noexcept
{
    if (!a_event) {
        return RE::BSEventNotifyControl::kContinue;
    }

    auto state = &Blink::g_BlinkState;
    if (!state) {
        return RE::BSEventNotifyControl::kContinue;
    }

    auto item  = a_event->spell;
    auto spell = Settings::spellFormId;

    if (!item || !spell) {
        return RE::BSEventNotifyControl::kContinue;
    }

    auto dataHandler = RE::TESDataHandler::GetSingleton();
    bool spellEquals = false;

    auto form = dataHandler->LookupForm(Settings::spellFormId, Settings::spellFormFile);
    if (form->GetFormID() == item || form->GetLocalFormID() == item || form->GetRawFormID() == item) {
        spellEquals = true;
    }
    if (!spellEquals) {
        return RE::BSEventNotifyControl::kContinue;
    }
    auto casterBase = &a_event->object;
    if (!casterBase) {
        return RE::BSEventNotifyControl::kContinue;
    }

    auto casterActor = casterBase->get();
    if (!casterActor->IsPlayerRef()) {
        return RE::BSEventNotifyControl::kContinue;
    }

    state->FireSpell(true);

    return RE::BSEventNotifyControl::kContinue;
}
} // namespace Events

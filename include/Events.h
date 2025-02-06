#pragma once

namespace Events {
class SpellCastEvent final : public EventHandler<SpellCastEvent, RE::TESSpellCastEvent> {
  public:
    RE::BSEventNotifyControl ProcessEvent(const RE::TESSpellCastEvent* a_event, RE::BSTEventSource<RE::TESSpellCastEvent>* a_eventSource) noexcept override;
};
} // namespace Events

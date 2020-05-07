#include "bnDelayedAttack.h"

DelayedAttack::DelayedAttack(Spell * next, Battle::Tile::Highlight highlightMode, double seconds) : duration(seconds),
Spell(next->GetField(), next->GetTeam()) {
    HighlightTile(highlightMode);
    SetFloatShoe(true);
    DelayedAttack::next = next;
}

DelayedAttack::~DelayedAttack()
{
}

void DelayedAttack::OnUpdate(float elapsed)
{
    duration -= elapsed;

    if (duration <= 0 && next) {
        GetField()->AddEntity(*next, *GetTile());
        Delete();
        next = nullptr;
    }
}

void DelayedAttack::Attack(Character * _entity)
{
}

void DelayedAttack::OnDelete()
{
  Remove();
}
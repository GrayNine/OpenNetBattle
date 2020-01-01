#include "bnDefenseIndestructable.h"
#include "bnEntity.h"
#include "bnField.h"
#include "bnSpell.h"
#include "bnHitbox.h"
#include "bnGuardHit.h"

DefenseIndestructable::DefenseIndestructable(bool breakCollidingObjectOnHit) : breakCollidingObjectOnHit(breakCollidingObjectOnHit), DefenseRule(Priority(1))
{
}

DefenseIndestructable::~DefenseIndestructable()
{
}

const bool DefenseIndestructable::Blocks(Spell * in, Character* owner)
{
  // Only drop gaurd effect as a response to attacks that can do impact damage > 0
  if (in->GetHitboxProperties().damage > 0 && (in->GetHitboxProperties().flags & Hit::impact) != 0) {
    owner->GetField()->AddEntity(*new GuardHit(owner->GetField(), owner, true), *owner->GetTile());
  }

  owner->GetField()->AddEntity(*new Hitbox(owner->GetField(), owner->GetTeam(), 0), *owner->GetTile());

  if (breakCollidingObjectOnHit) in->Delete();

  return true; // Guard blocks - this prevents an attack to passthrough
}

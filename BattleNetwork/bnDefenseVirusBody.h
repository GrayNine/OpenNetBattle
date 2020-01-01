#pragma once
#include "bnDefenseRule.h"

class DefenseVirusBody : public DefenseRule {
public:
  DefenseVirusBody();
  virtual ~DefenseVirusBody();
  virtual Hit::Properties& FilterStatuses(Hit::Properties& statuses);
  virtual const bool Blocks(Spell* in, Character* owner) { /* does nothing */ return false; }
};
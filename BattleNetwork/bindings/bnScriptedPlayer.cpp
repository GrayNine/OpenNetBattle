#ifdef BN_MOD_SUPPORT
#include "bnScriptedPlayer.h"
#include "../bnCardAction.h"

ScriptedPlayer::ScriptedPlayer(sol::state& script) : 
  script(script),
  Player()
{
  chargeEffect.setPosition(0, -30.0f);


  SetLayer(0);
  SetTeam(Team::red);

  script["battle_init"](this);

  animationComponent->Reload();
  CreateMoveAnimHash();
}

void ScriptedPlayer::SetChargePosition(const float x, const float y)
{
  chargeEffect.setPosition({ x,y });
}

void ScriptedPlayer::SetFullyChargeColor(const sf::Color& color)
{
  chargeEffect.SetFullyChargedColor(color);
}

void ScriptedPlayer::SetHeight(const float height)
{
  this->height = height;
}

void ScriptedPlayer::SetAnimation(const std::string& path)
{
  animationComponent->SetPath(path);
}

const float ScriptedPlayer::GetHeight() const
{
  return height;
}

Animation& ScriptedPlayer::GetAnimationObject()
{
  return animationComponent->GetAnimationObject();
}

Battle::Tile* ScriptedPlayer::GetCurrentTile() const
{
  return GetTile();
}

CardAction * ScriptedPlayer::OnExecuteSpecialAction()
{
  Character& character = *this;
  sol::object obj = script["execute_special_attack"](character);

  if (obj.valid()) {
    auto& ptr = obj.as<std::unique_ptr<CardAction>&>();
    return ptr.release();
  }

  return nullptr;
}

CardAction* ScriptedPlayer::OnExecuteBusterAction()
{
  Character& character = *this;
  sol::object obj = script["execute_buster_attack"](character);

  if (obj.valid()) {
    auto& ptr = obj.as<std::unique_ptr<CardAction>&>();
    return ptr.release();
  }

  return nullptr;
}

CardAction* ScriptedPlayer::OnExecuteChargedBusterAction()
{
  Character& character = *this;
  sol::object obj = script["execute_charged_attack"](character);

  if (obj.valid()) {
    auto& ptr = obj.as<std::unique_ptr<CardAction>&>();
    return ptr.release();
  }

  return nullptr;
}

#endif
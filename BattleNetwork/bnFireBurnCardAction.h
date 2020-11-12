#pragma once
#include "bnCardAction.h"
#include "bnAnimation.h"
#include "bnFireBurn.h"
#include <SFML/Graphics.hpp>

class SpriteProxyNode;
class Character;
class FireBurnCardAction : public CardAction {
private:
  sf::Sprite overlay;
  SpriteProxyNode* attachment;
  Animation attachmentAnim;
  FireBurn::Type type;
  int damage;
  bool crackTiles{ true };
public:
  FireBurnCardAction(Character* owner, FireBurn::Type type, int damage);
  ~FireBurnCardAction();
  void OnUpdate(float _elapsed);
  void OnAnimationEnd() override;
  void EndAction();
  void Execute();
  void CrackTiles(bool state);
};

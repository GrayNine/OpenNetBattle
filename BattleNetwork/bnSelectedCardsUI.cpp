#include <string>
#include <Swoosh/Ease.h>

#include "bnWebClientMananger.h"
#include "bnPlayer.h"
#include "bnField.h"
#include "bnSelectedCardsUI.h"
#include "bnResourceHandle.h"
#include "bnTextureResourceManager.h"
#include "bnInputManager.h"
#include "bnCurrentTime.h"
#include "bnCard.h"
#include "bnCardAction.h"
#include "battlescene/bnBattleSceneBase.h"

using std::to_string;

SelectedCardsUI::SelectedCardsUI(Player* _player) : 
  CardUsePublisher(), 
  UIComponent(_player), 
  player(_player), 
  text(Font::Style::thick),
  dmg(Font::Style::gradient_orange),
  multiplier(Font::Style::thick)
{
  cardCount = curr = 0;
  auto iconRect = sf::IntRect(0, 0, 14, 14);
  icon.setTextureRect(iconRect);
  icon.setScale(sf::Vector2f(2.f, 2.f));

  text.setScale(2.f, 2.f);
  dmg.setScale(2.f, 2.f);
  multiplier.setScale(2.f, 2.f);

  frame.setTexture(LOAD_TEXTURE(CHIP_FRAME));
  frame.setScale(sf::Vector2f(2.f, 2.f));

  interpolTimeFlat = interpolTimeDest = 0;
  interpolDur = sf::seconds(0.2f);

  firstFrame = true; // first time drawn, update positions
  spread = false;
}

SelectedCardsUI::~SelectedCardsUI() {
}

void SelectedCardsUI::draw(sf::RenderTarget & target, sf::RenderStates states) const {
  if (this->IsHidden()) return;

  const auto orange = sf::Color(225, 140, 0);

  text.SetString("");
  dmg.SetString("");
  multiplier.SetString("");

  if (player) {
    int cardOrder = 0;

    // i = curr so we only see the cards that are left
    for (int i = curr; i < cardCount; i++) {
      // The first card appears in front
      // But the trick is that we start at i which is a remainder of the whole list
      // We first draw the last card in the list down to i
      int drawOrderIndex = cardCount - i + curr - 1;

      if (spread) {
        // If we spread the cards, this algorithm is simple:
        // x = (i_drawOrderIndex - curr ) * size of card) - spacing
        // y = Place -121 screen pixels above the user
        sf::Vector2f flat = player->getPosition() + sf::Vector2f(((drawOrderIndex - curr) * 32.0f) - 4.f, -player->GetHeight() - 20.f);

        // We want to smoothly move from above the head to the spread position
        double alpha = swoosh::ease::linear(interpolTimeFlat, (double)interpolDur.asSeconds(), 1.0);

        // interpolate
        icon.setPosition(((float)alpha*flat) + ((float)(1.0-alpha)*icon.getPosition()));

        interpolTimeDest = 0;
        interpolTimeFlat += elapsed;
      }
      else {
        // If stacked, the algorithm makes a jagged pattern that goes up and to the left:
        // x = ( ( i - curr ) * spacing ) - spacing
        // y = -121.f - (i - curr) * (-spacing )
        sf::Vector2f dest = player->getPosition() + sf::Vector2f(((i - curr) * 4.0f) - 4.f, -player->GetHeight() - 20.f - (i - curr) * -4.0f);

        // We want to smoothly move from the spread position to the stacked position
        double alpha = swoosh::ease::linear(interpolTimeDest, (double)interpolDur.asSeconds(), 1.0);

        if (alpha >= 1.0f) {
          if (firstFrame) {
            firstFrame = false;
          }
        }
        // interpolate
        icon.setPosition(((float)alpha*dest) + ((float)(1.0 - alpha)*icon.getPosition()));

        interpolTimeDest += elapsed;

        interpolTimeFlat = 0;
      }

      if (!firstFrame) {
        // The black border needs to sit 1 pixel outside of the icon
        // 1px * 2 (scale) = 2px
        frame.setPosition(icon.getPosition());
        frame.setPosition(frame.getPosition() - sf::Vector2f(2.f, 2.f));
        target.draw(frame);

        // Grab the ID of the card and draw that icon from the spritesheet
        icon.setTexture(WEBCLIENT.GetIconForCard(selectedCards[drawOrderIndex]->GetUUID()));

        target.draw(icon);
      }
    }

    if (!player->CanAttack() && player->IsMoving()) return;

    // If we have a valid card, update and draw the data
    if (cardCount > 0 && curr < cardCount && selectedCards[curr]) {

      // Text sits at the bottom-left of the screen
      text.SetString(selectedCards[curr]->GetShortName());
      text.setOrigin(0, 0);
      text.setPosition(3.0f, 290.0f);

      // Text sits at the bottom-left of the screen
      int unmodDamage = selectedCards[curr]->GetUnmoddedProps().damage;
      int delta = selectedCards[curr]->GetDamage() - unmodDamage;
      sf::String dmgText = std::to_string(unmodDamage);

      if (delta != 0) {
        dmgText = dmgText + sf::String("+") + sf::String(std::to_string(delta));
      }

      // attacks that normally show no damage will show if the modifer adds damage
      if (delta != 0 || unmodDamage != 0) {
        dmg.SetString(dmgText);
        dmg.setOrigin(0, 0);
        dmg.setPosition((text.GetLocalBounds().width*text.getScale().x) + 10.f, 290.f);
      }

      if (multiplierValue != 1 && unmodDamage != 0) {
        // add "x N" where N is the multiplier
        std::string multStr = "x" + std::to_string(multiplierValue);
        multiplier.SetString(multStr);
        multiplier.setOrigin(0, 0);
        multiplier.setPosition(dmg.getPosition().x + (dmg.GetLocalBounds().width*dmg.getScale().x) + 3.0f, 290.0f);
      }
    }
  }

  // shadow beneath
  auto textPos = text.getPosition();
  text.SetColor(sf::Color::Black);
  text.setPosition(textPos.x + 2.f, textPos.y + 2.f);
  target.draw(text);

  // font on top
  text.setPosition(textPos);
  text.SetColor(sf::Color::White);
  target.draw(text);

  // our number font has shadow baked in
  target.draw(dmg);

  // shadow
  auto multiPos = multiplier.getPosition();
  multiplier.SetColor(sf::Color::Black);
  multiplier.setPosition(multiPos.x + 2.f, multiPos.y + 2.f);
  target.draw(multiplier);

  // font on top
  multiplier.setPosition(multiPos);
  multiplier.SetColor(sf::Color::White);
  target.draw(multiplier);

  UIComponent::draw(target, states);
};

void SelectedCardsUI::OnUpdate(double _elapsed) {
  if (Input().Has(InputEvents::held_option)) {
    spread = true;
  }
  else {
    spread = false;
  }

  elapsed = _elapsed;
}

void SelectedCardsUI::LoadCards(Battle::Card ** incoming, int size) {
  selectedCards = incoming;
  cardCount = size;
  curr = 0;
}

void SelectedCardsUI::UseNextCard() {
  const auto actions = player->AsyncActionList();
  bool hasNextCard = curr < cardCount;
  bool canUseCard = true;

  // We could be using an ability, just make sure one of these actions are not from a card
  // Cards cannot be used if another card is still active
  for (const CardAction* action : actions) {
    canUseCard = canUseCard && action->GetLockoutGroup() != CardAction::LockoutGroup::card;
  }

  canUseCard = canUseCard && hasNextCard;

  if (!canUseCard) {
    return;
  }

  auto card = selectedCards[curr];

  card->MultiplyDamage(multiplierValue);
  multiplierValue = 1; // reset 

  // add a peek event to the action queue
  player->AddAction(PeekCardEvent{ this }, ActionOrder::voluntary);

}

void SelectedCardsUI::Broadcast(const Battle::Card& card, Character& user)
{
  curr++;
  CardUsePublisher::Broadcast(card, user, CurrentTime::AsMilli());
}

std::optional<std::reference_wrapper<const Battle::Card>> SelectedCardsUI::Peek()
{
  if (cardCount > 0) {
    return { std::ref(*selectedCards[curr]) };
  }

  return {};
}

void SelectedCardsUI::Inject(BattleSceneBase& scene) {
  scene.Inject(*this);
}

void SelectedCardsUI::SetMultiplier(unsigned mult)
{
  multiplierValue = mult;
}

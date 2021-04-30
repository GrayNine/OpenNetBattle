#pragma once

#include <Swoosh/Activity.h>
#include <Swoosh/Ease.h>
#include <Swoosh/Timer.h>
#include <time.h>
#include <typeinfo>
#include <SFML/Graphics.hpp>
#include <Poco/Net/DatagramSocket.h>
#include <Poco/Buffer.h>

#include "../../bnMobHealthUI.h"
#include "../../bnCounterHitListener.h"
#include "../../bnCharacterDeleteListener.h"
#include "../../bnBackground.h"
#include "../../bnLanBackground.h"
#include "../../bnGraveyardBackground.h"
#include "../../bnVirusBackground.h"
#include "../../bnCamera.h"
#include "../../bnInputManager.h"
#include "../../bnCardSelectionCust.h"
#include "../../bnCardFolder.h"
#include "../../bnShaderResourceManager.h"
#include "../../bnPA.h"
#include "../../bnDrawWindow.h"
#include "../../bnSceneNode.h"
#include "../../bnBattleResults.h"
#include "../../battlescene/bnBattleSceneBase.h"
#include "../../bnMob.h"
#include "../../bnField.h"
#include "../../bnPlayer.h"
#include "../../bnSelectedCardsUI.h"
#include "../../bnPlayerCardUseListener.h"
#include "../../bnCounterHitListener.h"
#include "../../bnCharacterDeleteListener.h"
#include "../../bnNaviRegistration.h"
#include "../../battlescene/States/bnCharacterTransformBattleState.h"

#include "../bnNetPlayFlags.h"
#include "../bnNetPlayConfig.h"
#include "../bnNetPlaySignals.h"

using sf::RenderWindow;
using sf::VideoMode;
using sf::Clock;
using sf::Event;

struct CombatBattleState;
struct TimeFreezeBattleState;

class Mob;
class Player;
class PlayerHealthUI;
class NetworkCardUseListener; 

struct NetworkBattleSceneProps {
  BattleSceneBaseProps base;
  NetPlayConfig& netconfig;
};

class NetworkBattleScene final : public BattleSceneBase {
private:
  friend class NetworkCardUseListener;
  friend class PlayerInputReplicator;

  bool handshakeComplete{ false }; //!< Establish a connection with remote player
  bool isClientReady{ false }; //!< Signal when the client is ready to begin the round
  bool resync{ true }; //!< Try to restablish the handshake with your opponent
  SelectedNavi selectedNavi; //!< the type of navi we selected
  NetworkCardUseListener* networkCardUseListener{ nullptr };
  SelectedCardsUI* remoteCardUsePublisher{ nullptr };
  PlayerCardUseListener* remoteCardUseListener{ nullptr };
  Battle::Card** remoteHand{ nullptr }; // THIS IS HACKING TO GET IT TO WORK PLS REMOVE LATER
  NetPlayFlags remoteState; //!< remote state flags to ensure stability
  Poco::Net::SocketAddress remoteAddress; //!< them
  Player *remotePlayer{ nullptr }; //!< their player pawn
  Mob* mob{ nullptr }; //!< Our managed mob structure for PVP
  std::vector<Player*> players; //!< Track all players
  std::vector<std::shared_ptr<TrackedFormData>> trackedForms;
  CombatBattleState* combatPtr{ nullptr };
  TimeFreezeBattleState* timeFreezePtr{ nullptr };

  void sendHandshakeSignal(); // sent until we recieve a handshake
  void sendShootSignal();
  void sendUseSpecialSignal();
  void sendChargeSignal(const bool);
  void sendConnectSignal(const SelectedNavi navi);
  void sendReadySignal();
  void sendChangedFormSignal(const int form);
  void sendMoveSignal(const Direction dir);
  void sendHPSignal(const int hp);
  void sendTileCoordSignal(const int x, const int y);
  void sendChipUseSignal(const std::string& used);
  void sendRequestedCardSelectSignal(); 
  void sendLoserSignal(); // if we die, let them know

  void recieveHandshakeSignal();
  void recieveShootSignal();
  void recieveUseSpecialSignal();
  void recieveChargeSignal(const Poco::Buffer<char>&);
  void recieveConnectSignal(const Poco::Buffer<char>&);
  void recieveReadySignal();
  void recieveChangedFormSignal(const Poco::Buffer<char>&);
  void recieveMoveSignal(const Poco::Buffer<char>&);
  void recieveHPSignal(const Poco::Buffer<char>&);
  void recieveTileCoordSignal(const Poco::Buffer<char>&);
  void recieveChipUseSignal(const Poco::Buffer<char>&);
  void recieveLoserSignal(); // if they die, update our state flags
  void recieveRequestedCardSelectSignal(); // if the remote opens card select, we should be too
  void processIncomingPackets();

public:
  using BattleSceneBase::ProcessNewestComponents;

  void OnHit(Character& victim, const Hit::Properties& props) override final;
  void onUpdate(double elapsed) override final;
  void onDraw(sf::RenderTexture& surface) override final;
  void onExit() override;
  void onEnter() override;
  void onResume() override;
  void onEnd() override;

  void Inject(PlayerInputReplicator& pub);

  const NetPlayFlags& GetRemoteStateFlags();


  /**
   * @brief Construct scene with selected player, generated mob data, and the folder to use
   */
  NetworkBattleScene(swoosh::ActivityController&, const NetworkBattleSceneProps& props);
  
  /**
   * @brief Clears all nodes and components
   */
  ~NetworkBattleScene();

};

class NetworkCardUseListener final : public CardUseListener {
  NetworkBattleScene& nbs;
  Player& client;

public:
  NetworkCardUseListener(NetworkBattleScene& nbs, Player& client)
    : nbs(nbs), client(client), CardUseListener() {

  }

  void OnCardUse(const Battle::Card& card, Character& user, long long timestamp) override {
    nbs.sendChipUseSignal(card.GetUUID());
  }
};
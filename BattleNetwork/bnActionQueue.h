#pragma once
#include "frame_time_t.h"
#include <vector>
#include <iostream>
#include <limits>
#include <functional>
#include <memory>
#include <map>
#include <any>

enum class ActionOrder : short {
  immediate = 0,
  combo,
  traps,
  involuntary,
  voluntary
};

enum class ActionTypes : short {
  none = 0, // indicates empty queue
  special,
  buster,
  peek_card,
  card,
  movement
};

enum class ActionDiscardOp : short {
  until_resolve = 0, // stay in queue until resolved
  until_eof          // stay in queue until End Of Frame
};

namespace Battle {
  class Tile; // forward decl
}

template<typename T>
struct Queue {
  std::vector<T> list;
};

class ActionQueue {
public:
  struct Index {
    ActionTypes type{};
    ActionOrder order{};
    ActionDiscardOp discardOp{};
    size_t index{}; // index of the Queue<> in the ActionQueue::types hash
    bool processing{}; // whether or not this action is being processed
  };

  enum class ExecutionType : short {
    reserve = 0,
    process,
    interrupt
  };

  enum class CleanupType: short {
    allow_interrupts = 0,
    no_interrupts,
    clear_and_reset
  };

private:
  friend std::ostream& operator<<(std::ostream& os, const ActionQueue::Index& index);
  friend std::ostream& operator<<(std::ostream& os, const ActionQueue& queue);

  bool clearFilters{ false };
  bool toggleInterval{ false };
  std::map<ActionTypes, std::function<void(const ExecutionType&)>> handlers;
  std::map<ActionTypes, std::function<void(size_t)>> poppers;
  std::map<std::string, ActionTypes> type2Action;
  std::map<ActionTypes, std::any> types;
  std::map<ActionTypes, ActionDiscardOp> discardFilters;
  std::map<ActionOrder, ActionOrder> priorityFilters;
  std::vector<Index> indices;
  std::function<void()> actionableCallback;

  ActionOrder ApplyPriorityFilter(const ActionOrder& in);
  Index ApplyDiscardFilter(const Index& in);
  bool IsProcessing(const Index& in);

public:
  ~ActionQueue();

  ActionTypes TopType();
  bool IsEmpty() const;
  void CreatePriorityFilter(const ActionOrder& target, const ActionOrder& newOrder);
  void CreateDiscardFilter(const ActionTypes& type, const ActionDiscardOp& newOp);
  void ClearFilters();
  void Sort();
  void Process();
  void Pop();
  void ClearQueue(CleanupType cleanup);
  void SetActionableCallback(const std::function<void()>& callback);

  template<typename T>
  struct NoDeleter {
    void operator()(const T&) {};
  };

  template<typename Key, typename DeleterFunc = NoDeleter<Key>, typename Func>
  void RegisterType(ActionTypes type, const Func& func);

  template<typename Y>
  void Add(const Y& in, ActionOrder priority, ActionDiscardOp discard);
};

template<typename Key, typename DeleterFunc, typename Func>
void ActionQueue::RegisterType(ActionTypes type, const Func& func) {
  if (type == ActionTypes::none) return;

  types.insert(std::make_pair(type, std::make_shared<Queue<Key>>()));
  type2Action.insert(std::make_pair(typeid(Key).name(), type));

  auto invoker = [=](const ExecutionType& exec) {
    // ExecutionType::reserve
    auto queue = std::any_cast<std::shared_ptr<Queue<Key>>>(types[type]);
    Index& idx = indices[0];
    idx.processing = true;

    if (exec >= ExecutionType::process) {
      func(queue->list[idx.index], exec);
    }
  };

  handlers.insert(std::make_pair(type, invoker));

  auto popper = [=](size_t index) {
    auto queue = std::any_cast<std::shared_ptr<Queue<Key>>>(types[type]);

    if (index < queue->list.size()) {
      DeleterFunc deleter{};
      deleter.operator()(queue->list[index]);

      // update the index positions referring to this queue...
      for (auto& idx : indices) {
        size_t new_index = (idx.index==0)? 0 : idx.index-1;
        if (idx.type == type && idx.index >= index) {
          idx.index = new_index;
        }
      }

      queue->list.erase(queue->list.begin() + index);
    }
  };

  poppers.insert(std::make_pair(type, popper));
}

template<typename Y>
void ActionQueue::Add(const Y& in, ActionOrder priority, ActionDiscardOp discard) {
  try {
    ActionTypes key = type2Action[typeid(Y).name()];
    auto queue = std::any_cast<std::shared_ptr<Queue<Y>>>(types[key]);

    if (queue) {
      queue->list.push_back(in);
      size_t sz = queue->list.size();
      size_t index = sz == 0? 0 : sz-1;
      indices.push_back(Index{ key, priority, discard, index });
      Sort();
    }
  }
  catch (std::bad_any_cast&) {
    std::cout << "Type " << typeid(Y).name() << " not registered" << std::endl;
  }
}

inline std::ostream& operator<<(std::ostream& os, const ActionQueue::Index& index) {
  std::string type;

  switch (index.type) {
  case ActionTypes::none:
    type = "none";
    break;
  case ActionTypes::special:
    type = "special";
    break;
  case ActionTypes::buster:
    type = "buster";
    break;
  case ActionTypes::peek_card:
    type = "peek_card";
    break;
  case ActionTypes::card:
    type = "card";
    break;
  case ActionTypes::movement:
    type = "movement";
    break;
  }
  os << "(" << type << ", " << index.index << ")";
  return os;
}

inline std::ostream& operator<<(std::ostream& os, const ActionQueue& queue) {
  os << "[";
  for (auto& i : queue.indices) {
    os << i << ", ";
  }
  os << "]";

  return os;
}


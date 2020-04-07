#pragma once
#include "Engine/Function.hpp"

#include <utility>
#include "Dialog/DialogManager.hpp"

namespace ng {
class _GotoFunction : public Function {
public:
  explicit _GotoFunction(DialogVisitor &dialogVisitor, std::string name)
      : _dialogVisitor(dialogVisitor), _name(std::move(name)) {
  }

  bool isElapsed() override { return _done; }

  void operator()(const sf::Time &) override {
    if (_done)
      return;
    _dialogVisitor.getDialogManager().selectLabel(_name);
    _done = true;
  }

private:
  DialogVisitor &_dialogVisitor;
  std::string _name;
  bool _done{false};
};
} // namespace ng
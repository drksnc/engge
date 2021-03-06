#pragma once
#include <squirrel.h>
#include "engge/Scripting/ScriptExecute.hpp"

namespace ng {
class DefaultScriptExecute final : public ScriptExecute {
public:
  explicit DefaultScriptExecute(HSQUIRRELVM vm) : _vm(vm) {}

public:
  void execute(const std::string &code) override {
    sq_resetobject(&_result);
    auto top = sq_gettop(_vm);
    // compile
    sq_pushroottable(_vm);
    if (SQ_FAILED(sq_compilebuffer(_vm, code.data(), code.size(), _SC("_DefaultScriptExecute"), SQTrue))) {
      error("Error executing code {}", code);
      return;
    }
    sq_push(_vm, -2);
    // call
    if (SQ_FAILED(sq_call(_vm, 1, SQTrue, SQTrue))) {
      error("Error calling code {}", code);
      return;
    }
    sq_getstackobj(_vm, -1, &_result);
    sq_settop(_vm, top);
  }

  bool executeCondition(const std::string &code) override {
    std::string c;
    c.append("return ");
    c.append(code);

    execute(c);
    if (_result._type == OT_BOOL) {
      trace("{} returns {}", code, sq_objtobool(&_result));
      return sq_objtobool(&_result);
    }

    if (_result._type == OT_INTEGER) {
      trace("{} return {}", code, sq_objtointeger(&_result));
      return sq_objtointeger(&_result) != 0;
    }

    error("Error getting result {}", code);
    return false;
  }

  std::string executeDollar(const std::string &code) override {
    std::string c;
    c.append("return ");
    c.append(code);

    execute(c);
    // get the result
    if (_result._type != OT_STRING) {
      error("Error getting result {}", code);
      return "";
    }
    trace("{} returns {}", code, sq_objtostring(&_result));
    return sq_objtostring(&_result);
  }

  SoundDefinition *getSoundDefinition(const std::string &name) override {
    auto top = sq_gettop(_vm);
    sq_pushroottable(_vm);
    sq_pushstring(_vm, name.data(), -1);
    sq_get(_vm, -2);

    auto *pSound = EntityManager::getSoundDefinition(_vm, -1);
    sq_settop(_vm, top);
    return pSound;
  }

private:
  HSQUIRRELVM _vm{};
  HSQOBJECT _result{};
};
}

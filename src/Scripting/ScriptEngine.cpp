#include <cstdarg>
#include <squirrel.h>
#include "../../extlibs/squirrel/squirrel/sqpcheader.h"
#include "../../extlibs/squirrel/squirrel/sqvm.h"
#include "../../extlibs/squirrel/squirrel/sqstring.h"
#include <sqstdio.h>
#include <sqstdaux.h>
#include <sqstdstring.h>
#include <sqstdmath.h>
#include "engge/Entities/Entity.hpp"
#include "engge/System/Locator.hpp"
#include "engge/System/Logger.hpp"
#include "engge/Engine/ExCommandConstants.hpp"
#include "engge/Room/Room.hpp"
#include "engge/Scripting/ScriptEngine.hpp"
#include "engge/Audio/SoundDefinition.hpp"
#include "engge/Scripting/VerbExecute.hpp"
#include "ScriptEngine.inl"
#include "SystemPack.hpp"
#include "GeneralPack.hpp"
#include "ObjectPack.hpp"
#include "ActorPack.hpp"
#include "RoomPack.hpp"
#include "SoundPack.hpp"
#include "DefaultScriptExecute.hpp"
#include "DefaultVerbExecute.hpp"
#include "BnutPass.hpp"
#include "engge/Input/InputConstants.hpp"
#include "engge/Engine/InputStateConstants.hpp"

#ifdef SQUNICODE
#define scvprintf vfwprintf
#else
#define scvprintf vfprintf
#endif

namespace ng {
template<typename TConstant>
void ScriptEngine::registerConstants(std::initializer_list<std::tuple<const SQChar *, TConstant>> list) {
  for (auto t : list) {
    sq_pushconsttable(_vm);
    sq_pushstring(_vm, std::get<0>(t), -1);
    push(_vm, std::get<1>(t));
    sq_newslot(_vm, -3, SQTrue);
    sq_pop(_vm, 1);
  }
}

template<class T>
void ScriptEngine::pushObject(HSQUIRRELVM v, T *pObject) {
  sq_newtable(v);
  sq_pushstring(v, _SC("_id"), -1);
  sq_pushinteger(v, pObject ? pObject->getId() : 0);
  sq_newslot(v, -3, SQFalse);
}

template<typename TPack>
void ScriptEngine::registerPack() {
  auto pack = std::make_unique<TPack>();
  auto pPack = (Pack *) pack.get();
  pPack->registerPack();
  _packs.push_back(std::move(pack));
}

enum class Platform : int {
  Mac = 1,
  Win = 2,
  Linux = 3,
  Xbox = 4,
  IOS = 5,
  Android = 6,
  Switch = 7,
  PS4 = 8,
};

static Platform _getPlatform() {
#ifdef __APPLE__
#ifdef TARGET_OS_MAC
  return Platform::Mac;
#elif TARGET_OS_IPHONE
  return Platform::IOS;
#endif
#elif _WIN32
  return Platform::Win;
#elif defined __linux__ && !defined __ANDROID__
  return Platform::Linux;
// TODO: XBOX
//     return Platform::Xbox;
#elif __ANDROID__
  return Platform::Android;
// TODO: SWITCH
//    return Platform::Switch;
// TODO: PS4__
// TODO: return Platform::PS4;
#endif
}

ScriptEngine::ScriptEngine() {
  _vm = sq_open(1024 * 2);
  sq_setcompilererrorhandler(_vm, errorHandler);
  sq_newclosure(_vm, aux_printerror, 0);
  sq_seterrorhandler(_vm);
  sq_setprintfunc(_vm, printfunc, errorfunc); // sets the print function

  sq_pushroottable(_vm);
  sqstd_register_mathlib(_vm);
  sqstd_register_stringlib(_vm);
  sq_pushstring(_vm, _SC("PLATFORM"), -1);
  sq_pushinteger(_vm, (SQInteger) _getPlatform());
  sq_newslot(_vm, -3, SQFalse);
  registerConstants<int>({
                             {"ALL", ObjectStateConstants::ALL},
                             {"HERE", ObjectStateConstants::HERE},
                             {"GONE", ObjectStateConstants::GONE},
                             {"OFF", ObjectStateConstants::OFF},
                             {"ON", ObjectStateConstants::ON},
                             {"FULL", ObjectStateConstants::FULL},
                             {"EMPTY", ObjectStateConstants::EMPTY},
                             {"OPEN", ObjectStateConstants::OPEN},
                             {"CLOSED", ObjectStateConstants::CLOSED},
                             {"FALSE", 0},
                             {"TRUE", 1},
                             {"MOUSE", 1},
                             {"CONTROLLER", 2},
                             {"DIRECTDRIVE", 3},
                             {"TOUCH", 4},
                             {"REMOTE", 5},
                             {"FADE_IN", 0},
                             {"FADE_OUT", 1},
                             {"FADE_WOBBLE", 2},
                             {"FADE_WOBBLE_TO_SEPIA", 3},
                             {"FACE_FRONT", DirectionConstants::FACE_FRONT},
                             {"FACE_BACK", DirectionConstants::FACE_BACK},
                             {"FACE_LEFT", DirectionConstants::FACE_LEFT},
                             {"FACE_RIGHT", DirectionConstants::FACE_RIGHT},
                             {"FACE_FLIP", DirectionConstants::FACE_FLIP},
                             {"DIR_FRONT", DirectionConstants::FACE_FRONT},
                             {"DIR_BACK", DirectionConstants::FACE_BACK},
                             {"DIR_LEFT", DirectionConstants::FACE_LEFT},
                             {"DIR_RIGHT", DirectionConstants::FACE_RIGHT},
                             {"LINEAR", 0},
                             {"EASE_IN", 1},
                             {"EASE_INOUT", 2},
                             {"EASE_OUT", 3},
                             {"SLOW_EASE_IN", 4},
                             {"SLOW_EASE_OUT", 5},
                             {"LOOPING", 0x100},
                             {"SWING", 0X200},
                             {"ALIGN_LEFT", 0x10000000},
                             {"ALIGN_CENTER", 0x20000000},
                             {"ALIGN_RIGHT", 0x40000000},
                             {"ALIGN_TOP", 0x80000000},
                             {"ALIGN_BOTTOM", 0x1000000},
                             {"LESS_SPACING", 0x2000000},
                             {"EX_ALLOW_SAVEGAMES", ExCommandConstants::EX_ALLOW_SAVEGAMES},
                             {"EX_POP_CHARACTER_SELECTION", ExCommandConstants::EX_POP_CHARACTER_SELECTION},
                             {"EX_CAMERA_TRACKING", ExCommandConstants::EX_CAMERA_TRACKING},
                             {"EX_BUTTON_HOVER_SOUND", ExCommandConstants::EX_BUTTON_HOVER_SOUND},
                             {"EX_RESTART", ExCommandConstants::EX_RESTART},
                             {"EX_IDLE_TIME", ExCommandConstants::EX_IDLE_TIME},
                             {"EX_AUTOSAVE", ExCommandConstants::EX_AUTOSAVE},
                             {"EX_AUTOSAVE_STATE", ExCommandConstants::EX_AUTOSAVE_STATE},
                             {"EX_DISABLE_SAVESYSTEM", ExCommandConstants::EX_DISABLE_SAVESYSTEM},
                             {"EX_SHOW_OPTIONS", ExCommandConstants::EX_SHOW_OPTIONS},
                             {"EX_OPTIONS_MUSIC", ExCommandConstants::EX_OPTIONS_MUSIC},
                             {"EX_FORCE_TALKIE_TEXT", ExCommandConstants::EX_FORCE_TALKIE_TEXT},
                             {"GRASS_BACKANDFORTH", 0x00},
                             {"EFFECT_NONE", 0x00},
                             {"DOOR", 0x40},
                             {"DOOR_LEFT", 0x140},
                             {"DOOR_RIGHT", 0x240},
                             {"DOOR_BACK", 0x440},
                             {"DOOR_FRONT", 0x840},
                             {"FAR_LOOK", 0x8},
                             {"USE_WITH", ObjectFlagConstants::USE_WITH},
                             {"USE_ON", ObjectFlagConstants::USE_ON},
                             {"USE_IN", ObjectFlagConstants::USE_IN},
                             {"GIVEABLE", ObjectFlagConstants::GIVEABLE},
                             {"TALKABLE", ObjectFlagConstants::TALKABLE},
                             {"IMMEDIATE", ObjectFlagConstants::IMMEDIATE},
                             {"FEMALE", ObjectFlagConstants::FEMALE},
                             {"MALE", ObjectFlagConstants::MALE},
                             {"PERSON", ObjectFlagConstants::PERSON},
                             {"REACH_HIGH", ObjectFlagConstants::REACH_HIGH},
                             {"REACH_MED", ObjectFlagConstants::REACH_MED},
                             {"REACH_LOW", ObjectFlagConstants::REACH_LOW},
                             {"REACH_NONE", ObjectFlagConstants::REACH_NONE},
                             {"VERB_CLOSE", VerbConstants::VERB_CLOSE},
                             {"VERB_GIVE", VerbConstants::VERB_GIVE},
                             {"VERB_LOOKAT", VerbConstants::VERB_LOOKAT},
                             {"VERB_OPEN", VerbConstants::VERB_OPEN},
                             {"VERB_PICKUP", VerbConstants::VERB_PICKUP},
                             {"VERB_PULL", VerbConstants::VERB_PULL},
                             {"VERB_PUSH", VerbConstants::VERB_PUSH},
                             {"VERB_TALKTO", VerbConstants::VERB_TALKTO},
                             {"VERB_USE", VerbConstants::VERB_USE},
                             {"VERB_WALKTO", VerbConstants::VERB_WALKTO},
                             {"VERB_DIALOG", VerbConstants::VERB_DIALOG},
                             {"VERBFLAG_INSTANT", 1},
                             {"NO", 0},
                             {"YES", 1},
                             {"UNSELECTABLE", 0},
                             {"SELECTABLE", 1},
                             {"TEMP_UNSELECTABLE", 2},
                             {"TEMP_SELECTABLE", 3},
                             {"MAC", 1},
                             {"WIN", 2},
                             {"LINUX", 3},
                             {"XBOX", 4},
                             {"IOS", 5},
                             {"ANDROID", 6},
                             {"SWITCH", 7},
                             {"PS4", 8},
                             {"EFFECT_NONE", RoomEffectConstants::EFFECT_NONE},
                             {"EFFECT_SEPIA", RoomEffectConstants::EFFECT_SEPIA},
                             {"EFFECT_EGA", RoomEffectConstants::EFFECT_EGA},
                             {"EFFECT_VHS", RoomEffectConstants::EFFECT_VHS},
                             {"EFFECT_GHOST", RoomEffectConstants::EFFECT_GHOST},
                             {"EFFECT_BLACKANDWHITE", RoomEffectConstants::EFFECT_BLACKANDWHITE},
                             {"KEY_UP", static_cast<int>(InputConstants::KEY_UP)},
                             {"KEY_RIGHT", static_cast<int>(InputConstants::KEY_RIGHT)},
                             {"KEY_DOWN", static_cast<int>(InputConstants::KEY_DOWN)},
                             {"KEY_LEFT", static_cast<int>(InputConstants::KEY_LEFT)},
                             {"KEY_PAD1", static_cast<int>(InputConstants::KEY_PAD1)},
                             {"KEY_PAD2", static_cast<int>(InputConstants::KEY_PAD2)},
                             {"KEY_PAD3", static_cast<int>(InputConstants::KEY_PAD3)},
                             {"KEY_PAD4", static_cast<int>(InputConstants::KEY_PAD4)},
                             {"KEY_PAD5", static_cast<int>(InputConstants::KEY_PAD5)},
                             {"KEY_PAD6", static_cast<int>(InputConstants::KEY_PAD6)},
                             {"KEY_PAD7", static_cast<int>(InputConstants::KEY_PAD7)},
                             {"KEY_PAD8", static_cast<int>(InputConstants::KEY_PAD8)},
                             {"KEY_PAD9", static_cast<int>(InputConstants::KEY_PAD9)},
                             {"BUTTON_A", static_cast<int>(InputConstants::BUTTON_A)},
                             {"BUTTON_B", static_cast<int>(InputConstants::BUTTON_B)},
                             {"BUTTON_X", static_cast<int>(InputConstants::BUTTON_X)},
                             {"BUTTON_Y", static_cast<int>(InputConstants::BUTTON_Y)},
                             {"BUTTON_START", static_cast<int>(InputConstants::BUTTON_START)},
                             {"BUTTON_BACK", static_cast<int>(InputConstants::BUTTON_BACK)},
                             {"UI_INPUT_ON", InputStateConstants::UI_INPUT_ON},
                             {"UI_INPUT_OFF", InputStateConstants::UI_INPUT_OFF},
                             {"UI_VERBS_ON", InputStateConstants::UI_VERBS_ON},
                             {"UI_VERBS_OFF", InputStateConstants::UI_VERBS_OFF},
                             {"UI_CURSOR_ON", InputStateConstants::UI_CURSOR_ON},
                             {"UI_CURSOR_OFF", InputStateConstants::UI_CURSOR_OFF},
                             {"UI_HUDOBJECTS_ON", InputStateConstants::UI_HUDOBJECTS_ON},
                             {"UI_HUDOBJECTS_OFF", InputStateConstants::UI_HUDOBJECTS_OFF},
                             {"WAITING_FOR_CHOICE", 2},
                         });
}

ScriptEngine::~ScriptEngine() {
  sq_close(_vm);
}

void ScriptEngine::setEngine(Engine &engine) {
  g_pEngine = &engine;
  auto pVerbExecute = std::make_unique<DefaultVerbExecute>(engine);
  engine.setVerbExecute(std::move(pVerbExecute));
  auto pScriptExecute = std::make_unique<DefaultScriptExecute>(_vm);
  engine.setScriptExecute(std::move(pScriptExecute));

  registerPack<ActorPack>();
  registerPack<GeneralPack>();
  registerPack<ObjectPack>();
  registerPack<RoomPack>();
  registerPack<SoundPack>();
  registerPack<SystemPack>();
}

Engine &ScriptEngine::getEngine() { return *g_pEngine; }

SQInteger ScriptEngine::aux_printerror(HSQUIRRELVM v) {
  auto pf = sq_geterrorfunc(v);
  if (!pf)
    return 0;

  if (sq_gettop(v) < 1)
    return 0;

  const SQChar *error = nullptr;
  if (SQ_FAILED(sq_getstring(v, 2, &error))) {
    error = "unknown";
  }
  pf(v, _SC("\nAn error occured in the script: %s\n"), error);
  sqstd_printcallstack(v);

  return 0;
}

void ScriptEngine::errorHandler(HSQUIRRELVM v, const SQChar *desc, const SQChar *source, SQInteger line,
                                SQInteger column) {
  std::ostringstream os;
  os << desc << ' ' << source << '(' << line << ',' << column << ')';
  for (auto &callback : _errorCallbacks) {
    callback(v, os.str().data());
  }
  error(os.str());
}

void ScriptEngine::errorfunc(HSQUIRRELVM v, const SQChar *s, ...) {
  SQChar buf[1024];
  va_list vl;
  va_start(vl, s);
  vsprintf(buf, s, vl);
  va_end(vl);

  for (auto &callback : _errorCallbacks) {
    callback(v, buf);
  }
  error(buf);
}

void ScriptEngine::printfunc(HSQUIRRELVM v, const SQChar *s, ...) {
  std::vector<SQChar> buf(1024 * 1024);
  va_list vl;
  va_start(vl, s);
  vsnprintf(buf.data(), buf.size(), s, vl);
  va_end(vl);

  for (auto &callback : _printCallbacks) {
    callback(v, buf.data());
  }
  trace(buf.data());
}

void ScriptEngine::registerGlobalFunction(SQFUNCTION f, const SQChar *functionName, SQInteger nparamscheck,
                                          const SQChar *typemask) {
  sq_pushroottable(_vm);
  sq_pushstring(_vm, functionName, -1);
  sq_newclosure(_vm, f, 0); // create a new function
  sq_setparamscheck(_vm, nparamscheck, typemask);
  sq_setnativeclosurename(_vm, -1, functionName);
  sq_newslot(_vm, -3, SQFalse);
  sq_pop(_vm, 1); // pops the root table
}

void ScriptEngine::executeScript(const std::string &name) {
  if (SQ_FAILED(sqstd_dofile(_vm, name.c_str(), SQFalse, SQTrue))) {
    error("failed to execute {}", name);
    sq_getlasterror(_vm);
    aux_printerror(_vm);
    return;
  }
}

void ScriptEngine::executeNutScript(const std::string &name) {
  std::vector<char> code;

  std::ifstream is(name);
  if (is.is_open()) {
    trace("Load local file file {}", name);
    is.seekg(-1, std::ios::end);
    auto len = (size_t) is.tellg();
    is.seekg(0, std::ios::beg);
    code.resize(len + 1);
    is.read(code.data(), len);
  } else {
    auto entryName = std::regex_replace(name, std::regex("\\.nut"), ".bnut");
    code = Locator<EngineSettings>::get().readBuffer(entryName);

    // decode bnut
    int cursor = static_cast<int>(code.size() - 1) & 0xff;
    for (char &i : code) {
      i ^= _bnutPass[cursor];
      cursor = (cursor + 1) % 4096;
    }
  }

#if 0
  std::ofstream o;
  o.open(name);
  o.write(code.data(), code.size());
  o.close();
#endif
  auto top = sq_gettop(_vm);
  sq_pushroottable(_vm);
  if (SQ_FAILED(sq_compilebuffer(_vm, code.data(), code.size() - 1, _SC(name.data()), SQTrue))) {
    error("Error compiling {}", name);
    return;
  }
  sq_push(_vm, -2);
  // call
  if (SQ_FAILED(sq_call(_vm, 1, SQFalse, SQTrue))) {
    error("Error calling {}", name);
    sqstd_printcallstack(_vm);
    return;
  }
  sq_settop(_vm, top);
}

bool ScriptEngine::rawCall(const char *name) {
  sq_pushroottable(_vm);
  sq_pushstring(_vm, _SC(name), -1);
  if (SQ_FAILED(sq_rawget(_vm, -2))) {
    sq_pop(_vm, 1);
    trace("can't find {} function", name);
    return false;
  }
  sq_remove(_vm, -2);

  sq_pushroottable(_vm);
  if (SQ_FAILED(sq_call(_vm, 1, SQFalse, SQTrue))) {
    sqstd_printcallstack(_vm);
    sq_pop(_vm, 1);
    error("function {} call failed", name);
    return false;
  }
  sq_pop(_vm, 1);
  return true;
}

bool ScriptEngine::call(const char *name) {
  sq_pushroottable(_vm);
  sq_pushstring(_vm, _SC(name), -1);
  if (SQ_FAILED(sq_get(_vm, -2))) {
    sq_pop(_vm, 1);
    trace("can't find {} function", name);
    return false;
  }
  sq_remove(_vm, -2);

  sq_pushroottable(_vm);
  if (SQ_FAILED(sq_call(_vm, 1, SQFalse, SQTrue))) {
    sqstd_printcallstack(_vm);
    sq_pop(_vm, 1);
    error("function {} call failed", name);
    return false;
  }
  sq_pop(_vm, 1);
  return true;
}

SQObjectPtr ScriptEngine::toSquirrel(const std::string &value) {
  SQObjectPtr string = SQString::Create(_ss(getVm()), value.c_str());
  return string;
}

Engine *ScriptEngine::g_pEngine = nullptr;

} // namespace ng

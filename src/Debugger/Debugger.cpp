#include <iostream>
#include <memory>
#include <regex>
#include <sstream>
#include <utility>
#include "squirrel.h"
#include "nlohmann/json.hpp"
#include "Engine.h"
#include "EngineSettings.h"
#include "ScriptEngine.h"
#include "Debugger.h"
#include "_ByteBuffer.h"
#include "_DebugSession.h"

namespace ng
{
static Debugger *_pDebugger = nullptr;
static HSQUIRRELVM _v;
static SQInteger _line;

Debugger::Debugger(Engine& engine, ScriptEngine &scriptEngine)
    : _scriptEngine(scriptEngine)
{
    _pDebugger = this;
    auto vm = engine.getVm();
    sq_setnativedebughook(vm, debugHook);
    sq_enabledebuginfo(vm, SQTrue);
}

void Debugger::add(DebuggerListener *listener)
{
    _listeners.emplace_back(listener);
}

void Debugger::remove(DebuggerListener *listener)
{
    auto it = std::find(_listeners.begin(), _listeners.end(), listener);
    if (it != _listeners.end())
    {
        _listeners.erase(it);
    }
}

void Debugger::pause()
{
    _pause = true;
}

void Debugger::resume()
{
    _pause = false;
    _mutex.unlock();
}

void Debugger::step()
{
    _pause = true;
    _mutex.unlock();
}

const std::vector<Breakpoint> &Debugger::getBreakpoints() const
{
    return _breakpoints;
}

void Debugger::addBreakpoint(std::string path, SQInteger line)
{
    auto pos = path.find_last_of("/");
    std::string name;
    if (pos != std::string::npos)
    {
        name = path.substr(pos + 1, path.size() - pos - 1);
    }
    else
    {
        name = path;
    }
    _breakpoints.emplace_back(Breakpoint{name, path, line});
}

void Debugger::removeBreakpoint(const Breakpoint &breakpoint)
{
    auto it = std::find_if(_breakpoints.cbegin(), _breakpoints.cend(), [&breakpoint](const Breakpoint &bp) {
        return bp.name == breakpoint.name && bp.line == breakpoint.line;
    });
    if (it == _breakpoints.cend())
        return;
    _breakpoints.erase(it);
}

void Debugger::execute(std::string code)
{
    auto top = sq_gettop(_v);
    sq_pushroottable(_v);
    if (SQ_FAILED(sq_compilebuffer(_v, code.data(), code.size() - 1, _SC("debug"), SQTrue)))
    {
        std::cerr << "Error compiling debug" << std::endl;
        return;
    }
    sq_push(_v, -2);
    // call
    if (SQ_FAILED(sq_call(_v, 1, SQFalse, SQTrue)))
    {
        std::cerr << "Error calling debug code" << std::endl;
        return;
    }
    sq_settop(_v, top);
}

void Debugger::getStackTrace(std::vector<StackFrame> &stackFrames)
{
    if (!_vm)
        return;

    SQStackInfos infos;
    SQInteger i = 0;
    while (SQ_SUCCEEDED(sq_stackinfos(_v, i++, &infos)))
    {
        StackFrame frame;
        if (strcmp(infos.source, "NATIVE") != 0)
        {
            frame.source = infos.source;
        }
        frame.line = infos.line;
        if (infos.funcname)
            frame.functionName = infos.funcname;
        stackFrames.push_back(frame);
    }
}

std::string Debugger::getSource(std::string path)
{
    auto name = path.substr(2);
    std::vector<char> buffer;
    _scriptEngine.getSource(name, buffer);

    std::string content;
    std::copy_n(buffer.begin(), buffer.size() - 1, std::back_inserter(content));
    return content;
}

void Debugger::debugHook(HSQUIRRELVM v,
                         SQInteger type, const SQChar *sourcename,
                         SQInteger line, const SQChar *funcname)
{
    _v = v;
    _line = line;
    std::string srcfile = sourcename ? sourcename : "unknown";
    if (type == 'l')
    {
        std::optional<Breakpoint> pBreakpoint;
        for (const auto &breakpoint : _pDebugger->_breakpoints)
        {
            if (breakpoint.name == srcfile && breakpoint.line == line)
            {
                pBreakpoint = breakpoint;
                break;
            }
        }

        if (pBreakpoint.has_value())
        {
            _pDebugger->_mutex.lock();
            _pDebugger->_vm = v;

            for (const auto &listener : _pDebugger->_listeners)
            {
                listener->onBreakpointHit(pBreakpoint.value());
            }
        }
    }
}
} // namespace ng
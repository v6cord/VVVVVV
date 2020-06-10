#include "LuaScript.h"
#include "Script.h"
#include <iostream>
#include <utility>

lua_script::lua_script(std::string name, size_t start, size_t end) {
    for (size_t i = start; i < end; ++i) {
        text += script.customscript[i];
        text += "\n";
    }

    lua.open_libraries(sol::lib::base);
    thread = sol::thread::create(lua);
    sol::function func = lua.load(text).get<sol::function>();
    sol::state_view thread_state = thread.state();
    coroutine = sol::coroutine(thread_state, func);
}

bool lua_script::run() {
    coroutine();
    return false;
}

void lua_script::load(std::string name, size_t start, size_t end) {
    lua_script scr(name, start, end);
    script.lua_scripts.push_back(std::move(scr));
}

#include "LuaScript.h"
#include "Script.h"
#include <iostream>

lua_script::lua_script(std::string name, size_t start, size_t end) {
    for (size_t i = start; i < end; ++i) {
        text += script.customscript[i];
        text += "\n";
    }

    lua.open_libraries(sol::lib::base);
}

void lua_script::run() {
    lua.script(text);
}

void lua_script::load(std::string name, size_t start, size_t end) {
    lua_script scr(name, start, end);
    scr.run(); // TODO: dont do this
}

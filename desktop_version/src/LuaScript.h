#ifndef LUASCRIPT_H
#define LUASCRIPT_H

#define SOL_ALL_SAFETIES_ON 1

#include <string>
#include <sol.hpp>

class lua_script {
    sol::state lua;
    sol::thread thread;
    sol::coroutine coroutine;
    std::string text;

    void add_functions();

public:
    unsigned int delay = 0;
    bool endtext = false;

    lua_script(std::string name, std::vector<std::string> contents);
    bool run();
    static void load(std::string name, std::vector<std::string> contents);

    lua_script(lua_script&& x) = delete;
};

#endif

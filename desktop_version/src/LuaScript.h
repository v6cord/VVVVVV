#ifndef LUASCRIPT_H
#define LUASCRIPT_H

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

    lua_script(std::string name, size_t start, size_t end);
    bool run();
    static void load(std::string name, size_t start, size_t end);
};

#endif

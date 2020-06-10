#ifndef LUASCRIPT_H
#define LUASCRIPT_H

#include <string>
#include <sol.hpp>

class lua_script {
    sol::state lua;
    std::string text;

public:
    lua_script(std::string name, size_t start, size_t end);
    void run();
    static void load(std::string name, size_t start, size_t end);
};

#endif

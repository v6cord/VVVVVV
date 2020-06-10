#ifndef LUASCRIPT_H
#define LUASCRIPT_H

#include <string>
#include <sol.hpp>

class lua_script {
    sol::state state;

public:
    static void load(std::string script);
};

#endif

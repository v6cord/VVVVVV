#include "LuaScript.h"
#include "Script.h"
#include "Utilities.h"
#include <iostream>
#include <utility>
#include <stdexcept>

class lua_yielding {
public:
    virtual void handle(lua_script& s) = 0;
};

class lua_delay : public lua_yielding {
    unsigned int delay;

public:
    lua_delay(unsigned int time) : delay(time) {}

    void handle(lua_script& s) override {
        s.delay = delay;
    }
};

lua_script::lua_script(std::string name, size_t start, size_t end) {
    for (size_t i = start; i < end; ++i) {
        text += script.customscript[i];
        text += "\n";
    }

    lua.open_libraries(sol::lib::base, sol::lib::string, sol::lib::math, sol::lib::table);
    add_functions();

    sol::protected_function::set_default_handler(lua["error_handler"]);

    thread = sol::thread::create(lua);
    sol::function func = lua.load(text).get<sol::function>();
    sol::state_view thread_state = thread.state();
    coroutine = sol::coroutine(thread_state, func);
}

void lua_script::add_functions() {
    lua.set_function("error_handler", [](std::string msg) {
        script_exception ex(msg, true);
        std::cerr << ex.what() << std::endl;
        throw ex;
    });

    lua["delay"] = sol::yielding([](unsigned int delay) {
        return lua_delay(delay);
    });
}

bool lua_script::run() {
    if (delay > 0) {
        --delay;
        return true;
    }

    auto res = coroutine();
    if (res.status() == sol::call_status::yielded) {
        lua_yielding& command = res.get<lua_yielding&>();
        command.handle(*this);
        return true;
    } else {
        return false;
    }
}

void lua_script::load(std::string name, size_t start, size_t end) {
    lua_script scr(name, start, end);
    script.lua_scripts.push_back(std::move(scr));
}

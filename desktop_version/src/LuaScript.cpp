#include "Graphics.h"
#include "KeyPoll.h"
#include "Game.h"
#include "LuaScript.h"
#include "Script.h"
#include "Utilities.h"
#include <iostream>
#include <utility>
#include <stdexcept>

struct textbox {
    std::string text;
    int x = 0;
    int y = 0;

    textbox(std::string t) : text(t) {}
    textbox() = default;

    void show() {
        std::stringstream ss(text);
        std::string line;
        std::getline(ss, line, '\n');
        graphics.createtextbox(line, x, y, 174, 174, 174);
        while (std::getline(ss, line, '\n')) {
            graphics.addline(line);
        }

        graphics.textboxadjust();
        graphics.textboxactive();

        game.advancetext = true;
        game.hascontrol = false;
        game.pausescript = true;
        if (key.isDown(90) || key.isDown(32) || key.isDown(86) ||
            key.isDown(KEYBOARD_UP) || key.isDown(KEYBOARD_DOWN)) {
            game.jumpheld = true;
        }
    }

    void say() {
        // TODO: squeak
        show();
    }
};

lua_script::lua_script(std::string name, std::vector<std::string> contents) {
    for (auto& line : contents) {
        text += line;
        text += "\n";
    }

    lua.open_libraries(sol::lib::base, sol::lib::string, sol::lib::math, sol::lib::table);
    add_functions();

    sol::protected_function::set_default_handler(lua["error_handler"]);

    thread = sol::thread::create(lua);
    sol::protected_function func = lua.load(text, name).get<sol::protected_function>();
    sol::state_view thread_state = thread.state();
    coroutine = sol::coroutine(thread_state, func);
}

void lua_script::add_functions() {
    lua.set_function("error_handler", [](std::string msg) {
        script_exception ex(msg, true);
        std::cerr << ex.what() << std::endl;
        throw ex;
    });

    lua["delay"] = sol::yielding([this](unsigned int time) {
        delay = time;
    });

    sol::usertype<textbox> t = lua.new_usertype<textbox>("textbox",
            sol::constructors<textbox(), textbox(std::string)>());
    t["text"] = &textbox::text;
    t["x"] = &textbox::x;
    t["y"] = &textbox::y;
    t["show"] = sol::yielding([this](textbox t) {
        t.show();
        endtext = true;
    });
    t["say"] = sol::yielding([this](sol::object obj) {
        textbox t;
        if (obj.is<textbox>()) {
            t = obj.as<textbox>();
        } else {
            t = textbox(obj.as<std::string>());
        }
        t.say();
        endtext = true;
    });
}

bool lua_script::run() {
    if (delay > 0) {
        --delay;
        return true;
    } else if (game.pausescript) {
        return true;
    }

    if (endtext) {
        graphics.textboxremove();
        game.hascontrol = true;
        game.advancetext = false;
        endtext = false;
    }

    sol::protected_function_result res = coroutine();
    return res.status() == sol::call_status::yielded;
}

void lua_script::load(std::string name, std::vector<std::string> contents) {
    script.lua_scripts.emplace_back(name, contents);
}

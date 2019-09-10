#pragma once

#ifndef KENGINE_MAX_LUA_SCRIPT_PATH
# define KENGINE_MAX_LUA_SCRIPT_PATH 64
#endif

#ifndef KENGINE_MAX_LUA_SCRIPTS
# define KENGINE_MAX_LUA_SCRIPTS 8
#endif

#include "string.hpp"
#include "vector.hpp"
#include "lua/sol.hpp"
#include "reflection/Reflectible.hpp"

namespace kengine {
    class LuaComponent : public putils::Reflectible<LuaComponent> {
	public:
		static constexpr char stringName[] = "LuaComponentString";
		using script = putils::string<KENGINE_MAX_LUA_SCRIPT_PATH, stringName>;
		static constexpr char vectorName[] = "LuaComponentVector";
		using script_vector = putils::vector<script, KENGINE_MAX_LUA_SCRIPTS>;

    public:
        LuaComponent(const script_vector & scripts = {}) : _scripts(scripts) {}

		LuaComponent(const LuaComponent &) = default;
		LuaComponent & operator=(const LuaComponent &) = default;
		LuaComponent(LuaComponent &&) = default;
		LuaComponent & operator=(LuaComponent &&) = default;

    public:
        sol::table meta;

    public:
        void attachScript(const char * file) noexcept { _scripts.push_back(file); }

        void removeScript(const char * file) noexcept {
            _scripts.erase(std::find(_scripts.begin(), _scripts.end(), file));
        }

    public:
        const script_vector & getScripts() const noexcept { return _scripts; }

    private:
        script_vector _scripts;

        /*
         * Reflectible
         */
    public:
        pmeta_get_class_name(LuaComponent);
        pmeta_get_attributes(
                pmeta_reflectible_attribute_private(&LuaComponent::_scripts),
                pmeta_reflectible_attribute(&LuaComponent::meta)
        );
        pmeta_get_methods(
                pmeta_reflectible_attribute(&LuaComponent::attachScript),
                pmeta_reflectible_attribute(&LuaComponent::removeScript)
        );
        pmeta_get_parents();
    };
}
#pragma once

namespace kengine { class EntityManager; }

#include "systems/LuaSystem.hpp"
#include "systems/PySystem.hpp"

#include "helpers/RegisterComponentFunctions.hpp"

template<typename F>
static void registerFunction(kengine::EntityManager & em, const char * name, F && func) {
	kengine::lua::registerFunction(em, name, func);
	kengine::python::registerFunction(em, name, func);
}

template<typename ...Types>
static void registerTypes(kengine::EntityManager & em) {
	putils::for_each_type<Types...>([&](auto && t) {
		using T = putils_wrapped_type(t);
		kengine::lua::registerType<T>(em);
		kengine::python::registerType<T>(em);
	});
}

template<typename ... Comps>
static void registerComponents(kengine::EntityManager & em) {
	registerTypes<Comps...>(em);

	kengine::registerComponentsFunctions<Comps...>(em);
}

#pragma once

#include <string>
#include "EntityManager.hpp"

#include "systems/LuaSystem.hpp"
#include "systems/PySystem.hpp"

#include "helpers/RegisterComponentFunctions.hpp"
#include "helpers/RegisterComponentEditor.hpp"
#include "helpers/RegisterComponentMatcher.hpp"
#include "helpers/RegisterComponentJSONLoader.hpp"

#include "components/LuaComponent.hpp"
#include "Point.hpp"

template<typename F>
static void registerFunction(kengine::EntityManager & em, const std::string & name, F && func) {
	em.getSystem<kengine::LuaSystem>().registerFunction(name, func);
	em.getSystem<kengine::PySystem>().registerFunction(name, func);
}

template<typename ...Types>
static void registerTypes(kengine::EntityManager & em) {
	em.registerTypes<kengine::LuaSystem, Types...>();
	em.registerTypes<kengine::PySystem, Types...>();
}

template<typename ... Comps>
static void registerComponents(kengine::EntityManager & em) {
	registerTypes<Comps...>(em);

	kengine::registerComponentsFunctions<Comps...>(em);
	kengine::registerComponentEditors<Comps...>(em);
	kengine::registerComponentMatchers<Comps...>(em);
	kengine::registerComponentJSONLoaders<Comps...>(em);
}

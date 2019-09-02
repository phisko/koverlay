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
#include "components/GraphicsComponent.hpp"
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

void registerComponents1(kengine::EntityManager & em);
void registerComponents2(kengine::EntityManager & em);
void registerComponents3(kengine::EntityManager & em);

static void registerTypes(kengine::EntityManager & em) {
	registerComponents1(em);
	registerComponents2(em);
	registerComponents3(em);

	registerTypes<
		kengine::LuaComponent::script, kengine::LuaComponent::script_vector,
		putils::Rect3f, putils::Point3f,
		kengine::GraphicsComponent::Layer
	>(em);

	if constexpr (!std::is_same<kengine::LuaComponent::script, kengine::PyComponent::script>::value)
		registerTypes<kengine::PyComponent::script>(em);

	if constexpr (!std::is_same<kengine::LuaComponent::script_vector, kengine::PyComponent::script_vector>::value)
		registerTypes<kengine::PyComponent::script_vector>(em);

	// registerFunction(em, "ImGui", std::function<void(const std::string &, const std::function<void()> &)>(
	// 	[&em](const std::string & name, const std::function<void()> & func) { em.createOrAttach<kengine::ImGuiComponent>(name, func); }
	// ));

	// registerFunction(em, "AdjustableInt", std::function<int (const std::string &, int)>(
	// 	[&em](const std::string & name, int val) { return em.createOrAttach<kengine::AdjustableComponent>(name, name, val).getComponent<kengine::AdjustableComponent>().i; }
	// ));
	// registerFunction(em, "AdjustableString", std::function<std::string & (const std::string &, const std::string &)>(
	// 	[&em](const std::string & name, const std::string & val) { return std::ref(em.createOrAttach<kengine::AdjustableComponent>(name, name, val).getComponent<kengine::AdjustableComponent>().s); }
	// ));
	// registerFunction(em, "AdjustableFloat", std::function<float (const std::string &, float)>(
	// 	[&em](const std::string & name, float val) { return em.createOrAttach<kengine::AdjustableComponent>(name, name, val).getComponent<kengine::AdjustableComponent>().d; }
	// ));
	// registerFunction(em, "AdjustableBool", std::function<bool (const std::string &, bool)>(
	// 	[&em](const std::string & name, bool val) { return em.createOrAttach<kengine::AdjustableComponent>(name, name, val).getComponent<kengine::AdjustableComponent>().b; }
	// ));
}

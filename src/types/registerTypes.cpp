#include "registerTypes.hpp"

#define REGISTER_FUNC_DECL(COMP) void register##COMP##Component(kengine::EntityManager & em);

REGISTER_FUNC_DECL(Adjustable);
REGISTER_FUNC_DECL(ImGui);
REGISTER_FUNC_DECL(Lua);
REGISTER_FUNC_DECL(Py);

#define REGISTER_FUNC_NAME(COMP) register##COMP##Component

using RegisterFunc = void(*)(kengine::EntityManager &);
static const RegisterFunc funcs[] = {
	REGISTER_FUNC_NAME(Adjustable),
	REGISTER_FUNC_NAME(ImGui),
	REGISTER_FUNC_NAME(Lua),
	REGISTER_FUNC_NAME(Py),
};

void registerTypes(kengine::EntityManager & em) {
	registerTypes<
		putils::Color, putils::NormalizedColor
	>(em);

	for (const auto f : funcs)
		f(em);
}

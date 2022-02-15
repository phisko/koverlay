#include "helpers/registerTypeHelper.hpp"
#include "data/LuaComponent.hpp"

namespace types{
	void registerkengineLuaComponent() noexcept {
		kengine::registerComponents<kengine::LuaComponent>();

	}
}
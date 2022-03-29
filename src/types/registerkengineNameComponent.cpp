#include "helpers/registerTypeHelper.hpp"
#include "data/NameComponent.hpp"
#include "helpers/logHelper.hpp"

namespace types{
	void registerkengineNameComponent() noexcept {
		kengine_log(Log, "Init/registerTypes", "Registering 'kengine::NameComponent'");
		kengine::registerComponents<kengine::NameComponent>();

	}
}
#include "helpers/registerTypeHelper.hpp"
#include "data/SelectedComponent.hpp"
#include "helpers/logHelper.hpp"

namespace types{
	void registerkengineSelectedComponent() noexcept {
		kengine_log(Log, "Init/registerTypes", "Registering 'kengine::SelectedComponent'");
		kengine::registerComponents<kengine::SelectedComponent>();

	}
}
#pragma once

#ifndef PI
# define PI 3.14159265359f
#endif

#include "ISystem.hpp"
#include "EntityManager.hpp"

#pragma warning(disable : 4250)

namespace kengine {
    template<typename CRTP, typename ...DataPackets>
    class System : public ISystem, public putils::Module<CRTP, DataPackets...> {
    public:
		System(EntityManager & em) : Module(&em) {
			detail::components = &em.__getComponentMap();
		}

		System() = delete;

    public:
        pmeta::type_index getType() const noexcept final {
            static_assert(std::is_base_of<System, CRTP>::value,
                          "System's first template parameter should be inheriting class");
            return pmeta::type<CRTP>::index;
        }
    };
}
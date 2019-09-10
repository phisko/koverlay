#pragma once

#include "sol.hpp"
#include "traits.hpp"
#include "function.hpp"

namespace sol {
	namespace detail {
		template <typename Signature, size_t MaxSize>
		struct lua_type_of<putils::function<Signature, MaxSize>> : std::integral_constant<type, type::function> {};
	}

	namespace stack {
		template <typename Signature, size_t MaxSize>
		struct pusher<putils::function<Signature, MaxSize>> {
			static int push(lua_State* L, const putils::function<Signature, MaxSize>& fx) {
				return pusher<function_sig<Signature>>{}.push(L, fx);
			}

			static int push(lua_State* L, putils::function<Signature, MaxSize>&& fx) {
				return pusher<function_sig<Signature>>{}.push(L, std::move(fx));
			}
		};
	}

	namespace stack {
		template <typename Signature, size_t MaxSize>
		struct getter<putils::function<Signature, MaxSize>> {
			typedef meta::bind_traits<Signature> fx_t;
			typedef typename fx_t::args_list args_lists;
			typedef meta::tuple_types<typename fx_t::return_type> return_types;

			template <typename... Args, typename... Ret>
			static putils::function<Signature, MaxSize> get_std_func(types<Ret...>, types<Args...>, lua_State* L, int index) {
				unsafe_function f(L, index);
				auto fx = [f = std::move(f)](Args && ... args)->meta::return_type_t<Ret...> {
					return f.call<Ret...>(std::forward<Args>(args)...);
				};
				return std::move(fx);
			}

			template <typename... FxArgs>
			static putils::function<Signature, MaxSize> get_std_func(types<void>, types<FxArgs...>, lua_State* L, int index) {
				unsafe_function f(L, index);
				auto fx = [f = std::move(f)](FxArgs&&... args) -> void {
					f(std::forward<FxArgs>(args)...);
				};
				return std::move(fx);
			}

			template <typename... FxArgs>
			static putils::function<Signature, MaxSize> get_std_func(types<>, types<FxArgs...> t, lua_State* L, int index) {
				return get_std_func(types<void>(), t, L, index);
			}

			static putils::function<Signature, MaxSize> get(lua_State* L, int index, record& tracking) {
				tracking.last = 1;
				tracking.used += 1;
				type t = type_of(L, index);
				if (t == type::none || t == type::lua_nil) {
					return nullptr;
				}
				return get_std_func(return_types(), args_lists(), L, index);
			}
		};
	}

}

namespace putils {
	namespace lua {
		template<typename T>
		void registerType(sol::state & state) {
			static_assert(putils::has_member_get_class_name<T>());

			if constexpr (putils::has_member_get_attributes<T>() && putils::has_member_get_methods<T>()) {
				const auto t = std::tuple_cat(
					std::make_tuple(T::get_class_name()),
					T::get_attributes().getFlatKeyValues(),
					T::get_methods().getFlatKeyValues()
				);
				std::apply(
					[&state](auto && ...params) { state.new_usertype<T>(FWD(params)...); },
					t
				);
			}
			else if constexpr (putils::has_member_get_attributes<T>()) {
				const auto t = std::tuple_cat(
					std::make_tuple(T::get_class_name()),
					T::get_attributes().getFlatKeyValues()
				);
				std::apply(
					[&state](auto && ...params) { state.new_usertype<T>(FWD(params)...); },
					t
				);
			}
			else if constexpr (putils::has_member_get_methods<T>()) {
				const auto t = std::tuple_cat(
					std::make_tuple(T::get_class_name()),
					T::get_methods().getFlatKeyValues()
				);
				std::apply(
					[&state](auto && ...params) { state.new_usertype<T>(FWD(params)...); },
					t
				);
			}
			else {
				const auto t = std::tuple_cat(std::make_tuple(T::get_class_name()));
				std::apply(
					[&state](auto && ...params) { state.new_usertype<T>(FWD(params)...); },
					t
				);
			}

            if constexpr (putils::is_streamable<std::ostream, T>::value) {
                state[T::get_class_name()][sol::meta_function::to_string] =
                        [](const T & obj) { return putils::toString(obj); };
            }
        }
    }
}
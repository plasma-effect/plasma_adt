#pragma once

#include<boost/optional.hpp>
#include<boost/variant.hpp>

#include<type_traits>
#include<memory>
#include<array>

namespace generic_adt
{
	template<class T>struct type_tag
	{
		typedef T type;
	};
	template<class T>constexpr type_tag<T> make_type_tag(T const&)
	{
		return type_tag<T>{};
	}
	template<class T>constexpr type_tag<T> make_type_tag()
	{
		return type_tag<T>{};
	}

	namespace type_traits
	{
		namespace detail
		{
			template<bool I, bool... Is>struct conjunction_i :conjunction_i<Is...> {};
			template<bool... Is>struct conjunction_i<false, Is...> :std::false_type {};
			template<>struct conjunction_i<true> :std::true_type {};

			template<bool I, bool... Is>struct disjunction_i :disjunction_i<Is...> {};
			template<bool... Is>struct disjunction_i<true, Is...> :std::true_type {};
			template<>struct disjunction_i<false> :std::false_type {};


		}
		template<class...Ts>struct conjunction :detail::conjunction_i<(Ts::value)...> {};
		template<class...Ts>struct disjunction :detail::disjunction_i<(Ts::value)...> {};

	}

	namespace detail
	{

		template<class T, class Tag>struct have_tag :std::false_type {};
		template<template<class>class Temp, class Tag, class T0>struct have_tag<Temp<T0>, Tag> :
			type_traits::disjunction<have_tag<T0, Tag>> {};
		template<template<class, class>class Temp, class Tag, class T0, class T1>struct have_tag<Temp<T0, T1>, Tag> :
			type_traits::disjunction<have_tag<T0, Tag>, have_tag<T1, Tag>> {};
		template<template<class, class, class>class Temp, class Tag, class T0, class T1, class T2>struct have_tag<Temp<T0, T1, T2>, Tag> :
			type_traits::disjunction<have_tag<T0, Tag>, have_tag<T1, Tag>, have_tag<T2, Tag>> {};
		template<template<class, class, class, class>class Temp, class Tag, class T0, class T1, class T2, class T3>struct have_tag<Temp<T0, T1, T2, T3>, Tag> :
			type_traits::disjunction<have_tag<T0, Tag>, have_tag<T1, Tag>, have_tag<T2, Tag>, have_tag<T3, Tag>> {};
		template<template<class, class, class, class, class>class Temp, class Tag, class T0, class T1, class T2, class T3, class T4>struct have_tag<Temp<T0, T1, T2, T3, T4>, Tag> :
			type_traits::disjunction<have_tag<T0, Tag>, have_tag<T1, Tag>, have_tag<T2, Tag>, have_tag<T3, Tag>, have_tag<T4, Tag>> {};
		template<template<class, class, class, class, class, class>class Temp, class Tag, class T0, class T1, class T2, class T3, class T4, class T5>struct have_tag<Temp<T0, T1, T2, T3, T4, T5>, Tag> :
			type_traits::disjunction<have_tag<T0, Tag>, have_tag<T1, Tag>, have_tag<T2, Tag>, have_tag<T3, Tag>, have_tag<T4, Tag>, have_tag<T5, Tag>> {};

		template<template<class...>class Temp, class Tag, class... Ts>struct have_tag<Temp<Ts...>, Tag> :
			type_traits::disjunction<have_tag<Ts, Tag>...> {};
		template<class Tag>struct have_tag<Tag, Tag> :std::true_type {};

		template<class T, class From, class To>struct trans_type
		{
			typedef T type;
		};
		template<template<class>class Temp, class From, class To, class T0>struct trans_type<Temp<T0>, From, To>
		{
			typedef Temp<typename trans_type<T0, From, To>::type> type;
		};
		template<template<class, class>class Temp, class From, class To, class T0, class T1>struct trans_type<Temp<T0, T1>, From, To>
		{
			typedef Temp<typename trans_type<T0, From, To>::type, typename trans_type<T1, From, To>::type> type;
		};
		template<template<class, class, class>class Temp, class From, class To, class T0, class T1, class T2>struct trans_type<Temp<T0, T1, T2>, From, To>
		{
			typedef Temp<typename trans_type<T0, From, To>::type, typename trans_type<T1, From, To>::type, typename trans_type<T2, From, To>::type> type;
		};
		template<template<class, class, class, class>class Temp, class From, class To, class T0, class T1, class T2, class T3>struct trans_type<Temp<T0, T1, T2, T3>, From, To>
		{
			typedef Temp<typename trans_type<T0, From, To>::type, typename trans_type<T1, From, To>::type, typename trans_type<T2, From, To>::type, typename trans_type<T3, From, To>::type> type;
		};
		template<template<class, class, class, class, class>class Temp, class From, class To, class T0, class T1, class T2, class T3, class T4>struct trans_type<Temp<T0, T1, T2, T3, T4>, From, To>
		{
			typedef Temp<typename trans_type<T0, From, To>::type, typename trans_type<T1, From, To>::type, typename trans_type<T2, From, To>::type, typename trans_type<T3, From, To>::type, typename trans_type<T4, From, To>::type> type;
		};
		template<template<class, class, class, class, class, class>class Temp, class From, class To, class T0, class T1, class T2, class T3, class T4, class T5>struct trans_type<Temp<T0, T1, T2, T3, T4, T5>, From, To>
		{
			typedef Temp<typename trans_type<T0, From, To>::type, typename trans_type<T1, From, To>::type, typename trans_type<T2, From, To>::type, typename trans_type<T3, From, To>::type, typename trans_type<T4, From, To>::type, typename trans_type<T5, From, To>::type> type;
		};

		template<template<class...>class Temp, class From, class To, class... Ts>struct trans_type<Temp<Ts...>, From, To>
		{
			typedef Temp<typename trans_type<Ts, From, To>::type...> type;
		};
		template<class From, class To>struct trans_type<From, From, To>
		{
			typedef To type;
		};

		template<std::size_t Id, class T>struct id_type
		{
			T value;
		};
		template<std::size_t Id, class T>constexpr id_type<Id,
			typename std::remove_const<
			typename std::remove_reference<T>::type>::type> make_id_type(T&& v)
		{
			return id_type<Id,
				typename std::remove_const<
				typename std::remove_reference<T>::type>::type>{std::forward<T>(v)};
		}
		template<std::size_t Id, class T>T& get_id_value(id_type<Id, T>& v)
		{
			return v.value;
		}
		template<std::size_t Id, class T>constexpr T const& get_id_value(id_type<Id, T> const& v)
		{
			return v.value;
		}
		template<std::size_t Id, class T>constexpr T&& get_id_value(id_type<Id, T>&& v)
		{
			return std::move(v.value);
		}
	}

	namespace pattern_match
	{
		struct pattern_match_tag {};
		namespace detail 
		{
			template<std::size_t Id, class T>struct id_type
			{
				typedef T type;
				T value;
				static constexpr std::size_t id = Id;
			};
			template<std::size_t Id, class T>auto make_tuple(T&& v)
			{
				return std::make_tuple(id_type<Id, typename std::remove_const<typename std::remove_reference<T>::type>::type>{std::forward<T>(v)});
			}
			template<std::size_t Id, class T, std::size_t I = 0>auto id_get(T const& v, std::enable_if_t<std::tuple_element_t<I, T>::id == Id>* = nullptr)
			{
				return std::get<I>(v).value;
			}
			template<std::size_t Id, class T, std::size_t I = 0>auto id_get(T const& v, std::enable_if_t<std::tuple_element_t<I, T>::id != Id>* = nullptr)
			{
				return id_get<Id, T, I + 1>(v);
			}
			template<std::size_t Id, class T, std::size_t I = 0, class = void>struct id_element
			{
				typedef typename id_element<Id, T, I + 1>::type type;
			};
			template<std::size_t Id, class T, std::size_t I>struct id_element<Id, T, I, std::enable_if_t<std::tuple_element_t<I, T>::id == Id>>
			{
				typedef typename std::tuple_element_t<I, T>::type type;
			};
			template<class T>auto optional_cat(T const&, boost::none_t)
			{
				return boost::none;
			}
			template<class T>auto optional_cat(boost::none_t, T const&)
			{
				return boost::none;
			}
			template<class T>auto optional_cat(T const& v)
			{
				return v;
			}
			template<class T, class U>auto optional_cat(T const& x, U const& y)
				->decltype(boost::make_optional(std::tuple_cat(*x, *y)))
			{
				if (x&&y)
				{
					return boost::make_optional(std::tuple_cat(*x, *y));
				}
				return boost::none;
			}
			template<class T, class... Ts>auto optional_cat(T const& v, Ts const&... vs)
			{
				return optional_cat(v, optional_cat(vs...));
			}

			template<class IndexSequence>struct id_tuple_apply_i;
			template<std::size_t... Is>struct id_tuple_apply_i<std::index_sequence<Is...>>
			{
				template<class T, class Func, class... Ts>static auto run(T const& t, Func const& func, Ts&&... args)
				{
					return func(id_get<Is>(t)..., std::forward<Ts>(args)...);
				}
			};
			template<class Tuple, class Func, class... Ts>auto id_tuple_apply(Tuple const& t, Func const& func, Ts&&...args)
			{
				return id_tuple_apply_i<std::make_index_sequence<std::tuple_size<Tuple>::value>>::run(t, func, std::forward<Ts>(args)...);
			}

			template<class Func, class Args, class IndexSequence>struct lambda_t;
			template<class Func, class... Args, std::size_t... Is>struct lambda_t<Func, std::tuple<Args...>, std::index_sequence<Is...>>
			{
				Func func;
				std::tuple<Args...> arg;
				template<class... Ts>decltype(auto) operator()(Ts&&... args)const
				{
					return func(std::get<Is>(arg)..., std::forward<Ts>(args)...);
				}
			};
			template<class Func, class... Args>auto make_lambda(Func func, Args... args)
			{
				return lambda_t<Func, std::tuple<Args...>, std::make_index_sequence<sizeof...(Args)>>{func, std::make_tuple(args...)};
			}
		}
	}

	namespace place_holder
	{
		namespace detail
		{
			template<int V, int I, int... Is>struct value_t :value_t<10 * V + I, Is...> {};
			template<int V, int I>struct value_t<V, I> :std::integral_constant<int, 10 * V + I> {};
		}
		template<std::size_t I>struct argument_place_t :pattern_match::pattern_match_tag
		{
			struct place_holder_t
			{
				template<class T>auto operator()(T const& v)const
				{
					return boost::make_optional(pattern_match::detail::make_tuple<I>(v));
				}
			};
			place_holder_t operator()(...)const
			{
				return place_holder_t{};
			}

			template<class Func>auto operator<=(Func&& func)
			{
				return std::make_pair(*this, std::forward<Func>(func));
			}
		};
		template<char... Cs>auto operator"" _()
		{
			return argument_place_t<detail::value_t<0, (Cs - '0')...>::value>{};
		}

		struct ignore_place_t :pattern_match::pattern_match_tag
		{
			struct place_holder_t
			{
				auto operator()(...)const
				{
					return boost::make_optional(std::tuple<>{});
				}
			};
			auto operator()(...)const
			{
				return place_holder_t{};
			}
		};
		constexpr ignore_place_t i_{};
	}

	template<class... Ts>struct tuple {};
	template<std::size_t Id, class Tuple>struct tuple_element;
	template<std::size_t Id, class... Ts>struct tuple_element<Id, tuple<Ts...>>
	{
	private:
		template<std::size_t Id, class T, class... Ts>struct impl :impl<Id - 1, Ts...>
		{

		};
		template<class T, class... Ts>struct impl<0, T, Ts...>
		{
			typedef T type;
		};
	public:
		typedef typename impl<Id, Ts...>::type type;
	};
	struct generic_tag {};

	namespace detail
	{
		template<class Target, class Base,class Pointer, class Generic>
		struct trans_tuple
		{
			typedef std::tuple<
				typename detail::trans_type<
				typename detail::trans_type<
				Target, generic_tag, Generic>::type,
				Base, Pointer>::type> type;
		};
		template<class Base, class Pointer, class Generic, class... Ts>
		struct trans_tuple<tuple<Ts...>, Base, Pointer, Generic>
		{
			typedef std::tuple<
				typename detail::trans_type<
				typename detail::trans_type<
				Ts, generic_tag, Generic>::type,
				Base, Pointer>::type...> type;
		};
		template<class Base, class Pointer, class Generic>
		struct trans_tuple<void, Base, Pointer, Generic>
		{
			typedef std::tuple<>;
		};
	}
}
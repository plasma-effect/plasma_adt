#pragma once


// Copyright plasma-effect 2015
// Distributed under the Boost Software License, Version 1.0.
// (See http://www.boost.org/LICENSE_1_0.txt)

#include<boost/variant.hpp>
#include<boost/optional.hpp>

#include<utility>
#include<tuple>
#include<type_traits>
#include<vector>
#include<functional>

namespace plasma_adt
{
	template<class... Ts>struct tuple{};

	
	namespace pattern_match
	{
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
			template<std::size_t Id, class T, std::size_t I = 0,class = void>struct id_element
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

			template<class Func,class Recur, class DataType>struct lambda
			{
				Func const& func;
				Recur const& recur;
				template<class... Ts>auto operator()(Ts&&...args)const
				{
					return func(recur, DataType::make_derived(std::forward<Ts>(args))...);
				}
			};

			template<class Return, class DataType, class Pattern, class Function, class... Args>
			struct pattern_function_t
			{
				Pattern pat;
				Function func;
				boost::optional<Return> operator()(DataType const& v, Args const&... args)const
				{
					auto t = pat.variable_instance(v);
					return t ? boost::make_optional(id_tuple_apply(*t, func, args...)) : boost::none;
				}
			};
			template<class Return,class Recur, class DataType, class Pattern, class Function, class... Args>
			struct recursion_function_t
			{
				Pattern pat;
				Function func;
				
				boost::optional<Return> operator()(Recur const& recur, DataType const& v, Args const&... args)const
				{
					auto t = pat.variable_instance(v);
					return t ? boost::make_optional(id_tuple_apply(*t, lambda<Function, Recur, DataType>{func, recur}, args...)) : boost::none;
				}
			};

			template<class Return, class DataType,class Pattern,class Function, class... Args>
			std::function<boost::optional<Return>(DataType, Args...)> make_pattern_function(
			std::pair<Pattern, Function> const& set)
			{
				return pattern_function_t<Return, DataType, Pattern, Function, Args...>{set.first, set.second};
			}

			template<class Return,class Recur, class DataType, class Pattern, class Function, class... Args>
			std::function<boost::optional<Return>(Recur, DataType, Args...)> make_recursion_function(
				std::pair<Pattern, Function> const& set)
			{
				return std::function<boost::optional<Return>(Recur, DataType, Args...)>(recursion_function_t<Return, Recur, DataType, Pattern, Function, Args...>{set.first, set.second});
			}
		}
		struct pattern_match_tag {};


		template<class Return,class DataType,class... Args>struct pattern_match_t
		{
			std::vector<std::function<boost::optional<Return>(DataType, Args...)>> patterns;
			Return operator()(DataType const& v, Args const&...args)const
			{
				for (auto const& pat : patterns)
				{
					auto ret = pat(v, args...);
					if (ret)
						return *ret;
				}
				throw std::domain_error("pattern match exception: domain error");
			}
			template<class Pattern,class Function>
			pattern_match_t<Return, DataType, Args...>& operator|(std::pair<Pattern, Function>const& set)
			{
				patterns.push_back(detail::make_pattern_function<Return, DataType, Pattern, Function, Args...>(set));
				return *this;
			}
		};
		template<class Return, class DataType, class... Args>struct recursion_match_t
		{
			std::vector<std::function<boost::optional<Return>(std::reference_wrapper<
				const recursion_match_t<Return,DataType,Args...>>,DataType, Args...)>> patterns;
			Return operator()(DataType const& v, Args const&...args)const
			{
				for (auto const& pat : patterns)
				{
					auto ret = pat(std::cref(*this), v, args...);
					if (ret)
						return *ret;
				}
				throw std::domain_error("pattern match exception: domain error");
			}
			template<class Pattern, class Function>
			recursion_match_t<Return, DataType, Args...>& operator|(std::pair<Pattern, Function>const& set)
			{
				patterns.push_back(
					detail::make_recursion_function<
					Return,
					std::reference_wrapper<const recursion_match_t<Return,DataType,Args...>>, 
					DataType,
					Pattern,
					Function,
					Args...>(set));
				return *this;
			}
		};
		template<class Return, class DataType, class... Args>struct memoization_recursion_t
		{
			std::vector<std::function<boost::optional<Return>(std::reference_wrapper<
				const memoization_recursion_t<Return, DataType, Args... >> , DataType, Args...) >> patterns;
			mutable std::vector<std::pair<std::tuple<DataType, Args...>, Return>> memo;
			Return operator()(DataType const& v, Args const&...args)const
			{
				auto t = std::make_tuple(v, args...);
				for (auto const& m : memo)
				{
					if (m.first == t)
					{
						return m.second;
					}
				}

				for (auto const& pat : patterns)
				{
					auto ret = pat(std::cref(*this), v, args...);
					if (ret)
					{
						memo.push_back(std::make_pair(t, *ret));
						return *ret;
					}
				}
				throw std::domain_error("pattern match exception: domain error");
			}
			template<class Pattern, class Function>
			memoization_recursion_t<Return, DataType, Args...>& operator|(std::pair<Pattern, Function>const& set)
			{
				patterns.push_back(
					detail::make_recursion_function<
					Return,
					std::reference_wrapper<const memoization_recursion_t<Return, DataType, Args...>>,
					DataType,
					Pattern,
					Function,
					Args...>(set));
				return *this;
			}
		};
		template<class Return,class DataType,class... Args>pattern_match_t<Return, DataType, Args...> pattern_match()
		{
			return pattern_match_t<Return, DataType, Args...>{};
		}
		template<class Return,class DataType ,class... Args>recursion_match_t<Return, DataType, Args...> recursion_match()
		{
			return recursion_match_t<Return, DataType, Args...>{};
		}
		template<class Return, class DataType, class... Args>memoization_recursion_t<Return, DataType, Args...> memoization_recursion()
		{
			return memoization_recursion_t<Return, DataType, Args...>{};
		}

	}

	namespace place_holder
	{
		namespace detail
		{
			template<int V, int I, int... Is>struct value_t :value_t<10 * V + I, Is...> {};
			template<int V, int I>struct value_t<V, I> :std::integral_constant<int, 10 * V + I> {};
		}
		template<int I>struct place_holder_t :pattern_match::pattern_match_tag
		{
			template<class T>auto variable_instance(T const& v)const
			{
				return boost::make_optional(std::make_tuple(pattern_match::detail::id_type<I, T>{v}));
			}
			template<class Func>auto operator<=(Func const& func)const
			{
				return std::make_pair(*this, func);
			}
		};
		template<char... Cs>constexpr auto operator"" _()
		{
			return place_holder_t<detail::value_t<0, (Cs - '0')...>::value>{};
		}
	}

	template<class Derived, class... Types>struct data_type_base
	{
		template<class>struct container_base;
		typedef container_base<std::make_index_sequence<sizeof...(Types)>> container_type;
		
		template<class T,class=void>struct tuple_helper
		{
			typedef T type;
		};
		template<class T>struct tuple_helper<T, std::enable_if_t<std::is_same<T, Derived>::value>>
		{
			typedef std::shared_ptr<container_type> type;
		};
		static auto value_helper(std::shared_ptr<container_type> const& v)
		{
			return Derived(data_type_base<Derived, Types...>(v));
		}
		template<class T>static auto value_helper(T const& v)
		{
			return v;
		}

		static auto tuple_get(Derived const& v)
		{
			return v.ptr;
		}
		template<class T>static auto tuple_get(T const& v)
		{
			return v;
		}
		static auto make_derived(std::shared_ptr<container_type>const& ptr)
		{
			return Derived(data_type_base(ptr));
		}
		static auto make_derived(std::shared_ptr<container_type>&& ptr)
		{
			return Derived(data_type_base(ptr));
		}
		template<class T>static auto make_derived(T&& v)
		{
			return std::forward<T>(v);
		}

		static bool equal(std::shared_ptr<container_type>const& lhs, std::shared_ptr<container_type>const& rhs)
		{
			return *rhs == *lhs;
		}
		template<class T>static bool equal(T const& lhs, T const& rhs)
		{
			return lhs == rhs;
		}
		template<class Tuple, std::size_t... Is>static bool equal(Tuple const& lhs, Tuple const& rhs, std::index_sequence<Is...>)
		{
			auto init = { equal(std::get<Is>(lhs),std::get<Is>(rhs))... };
			return std::all_of(std::begin(init), std::end(init), [](auto f) {return f;});
		}

		template<std::size_t Id,class T>struct value_type
		{
			typedef std::tuple<T> constructor_tuple;

			std::tuple<T> value;
			value_type(T const& v) :value(v) {}
			value_type(const value_type&) = default;
			value_type(value_type&&) = default;
			value_type& operator=(const value_type&) = default;
			value_type& operator=(value_type&&) = default;
			~value_type() = default;

			bool operator==(value_type<Id, T>const& rhs)const
			{
				return value == rhs.value;
			}
		};
		template<std::size_t Id>struct value_type<Id, void>
		{
			typedef std::tuple<> constructor_tuple;

			value_type() = default;
			value_type(const value_type&) = default;
			value_type(value_type&&) = default;
			value_type& operator=(const value_type&) = default;
			value_type& operator=(value_type&&) = default;
			~value_type() = default;

			bool operator==(value_type<Id, void>const&)const
			{
				return true;
			}
		};
		template<std::size_t Id, class... Ts>struct value_type<Id, tuple<Ts...>>
		{
			typedef std::tuple<Ts...> constructor_tuple;
			typedef std::tuple<typename tuple_helper<Ts>::type...> value_t;
			
			value_t value;
			value_type(typename tuple_helper<Ts>::type const&... vs) :value(std::make_tuple(vs...)) {}
			value_type(const value_type&) = default;
			value_type(value_type&&) = default;
			value_type& operator=(const value_type&) = default;
			value_type& operator=(value_type&&) = default;
			~value_type() = default;

			bool operator==(value_type<Id, tuple<Ts...>>const& rhs)const
			{
				return equal(value, rhs.value, std::make_index_sequence<sizeof...(Ts)>());
			}
		};

		template<class>struct parameters_i;
		template<std::size_t... Is>struct parameters_i<std::index_sequence<Is...>>
		{
			typedef std::tuple<value_type<Is, Types>...> type;
		};
		typedef typename parameters_i<std::make_index_sequence<sizeof...(Types)>>::type parameters;
		template<std::size_t Id>using param = std::tuple_element_t<Id, parameters>;

		template<std::size_t... Is>struct container_base<std::index_sequence<Is...>>
		{
			boost::variant<value_type<Is, Types>...> value;
			template<class T>container_base(T const& v) :value(v) {}
			container_base(container_base const&) = default;
			container_base(container_base&&) = default;
			container_base& operator=(container_base const&) = default;
			container_base& operator=(container_base&&) = default;
			~container_base() = default;

			bool operator==(container_base<std::index_sequence<Is...>>const& rhs)const
			{
				return value == rhs.value;
			}
		};

		bool operator==(Derived const& rhs)const
		{
			return *ptr == *rhs.ptr;
		}

		std::shared_ptr<container_type> ptr;
		data_type_base(std::shared_ptr<container_type> p) :ptr(p) {}
		data_type_base(data_type_base const&) = default;
		data_type_base(data_type_base&&) = default;
		data_type_base& operator=(data_type_base const&) = default;
		data_type_base& operator=(data_type_base&&) = default;
		~data_type_base() = default;

		template<std::size_t,class>struct instance_t;
		template<std::size_t Id, class... Ts>struct instance_t<Id, std::tuple<Ts...>> :pattern_match::pattern_match_tag
		{
			Derived operator()(Ts const&... args)const
			{
				return Derived(data_type_base<Derived, Types...>(std::make_shared<container_type>(param<Id>(tuple_get(args)...))));
			}
		 	boost::optional<std::tuple<>> variable_instance(Derived const& v)const
			{
				return v.ptr->value.which() == Id ? boost::make_optional(std::make_tuple()) : boost::none;
			}
			template<class T>boost::optional<std::tuple<>> variable_instance(T const&)const
			{
				return boost::none;
			}
			template<class Function>auto operator<=(Function const& func)const
			{
				return std::make_pair(*this, func);
			}
			template<class... Us>struct pattern_t :pattern_match::pattern_match_tag
			{
				std::tuple<Us...> pattern;
				pattern_t(std::tuple<Us...>const& u) :pattern(u) {}
				pattern_t(pattern_t const&) = default;
				pattern_t(pattern_t&&) = default;
				pattern_t& operator=(pattern_t const&) = default;
				pattern_t& operator=(pattern_t&&) = default;
				~pattern_t() = default;

				template<std::size_t... Is>auto variable_instance_i(Derived const& v, std::index_sequence<Is...>)const
				{
					const auto& p = v.ptr->value;
					return p.which() == Id ?
						pattern_match::detail::optional_cat(make_derived(std::get<Is>(pattern).variable_instance(
							std::get<Is>(boost::get<param<Id>>(v.ptr->value).value)))...) :
						static_cast<decltype(pattern_match::detail::optional_cat(make_derived(std::get<Is>(pattern).variable_instance(
							std::get<Is>(boost::get<param<Id>>(v.ptr->value).value)))...))>(boost::none);
				}
				auto variable_instance(Derived const& v)const
					->decltype(variable_instance_i(v, std::make_index_sequence<sizeof...(Us)>()))
				{
					return variable_instance_i(v, std::make_index_sequence<sizeof...(Us)>());
				}
				template<class T>auto variable_instance(T const&)const
					->decltype(variable_instance_i(std::declval<Derived>(), std::make_index_sequence<sizeof...(Us)>()))
				{
					return boost::none;
				}
				template<class Func>auto operator<=(Func const& func)const
				{
					return std::make_pair(*this, func);
				}
			};
			template<class U,class... Us>auto operator()(U&& arg, Us&&... args)const
				->typename std::enable_if<
				std::is_base_of<pattern_match::pattern_match_tag,U>::value&&
				sizeof...(Us)+1 == std::tuple_size<typename param<Id>::constructor_tuple>::value, pattern_t<U,Us...>>::type
			{
				return pattern_t<U, Us...>{std::make_tuple(std::forward<U>(arg), std::forward<Us>(args)...)};
			}
			template<class U, class...Us>auto operator()(U&&, Us&&...)const
				->typename std::enable_if<
				std::is_base_of<pattern_match::pattern_match_tag, U>::value&&
				sizeof...(Us)+1 != std::tuple_size<typename param<Id>::constructor_tuple>::value,boost::none_t>::type
			{
				static_assert(sizeof...(Us)+1 == std::tuple_size<typename param<Id>::constructor_tuple>::value,
					"pattern match error: invalid parameter");
				return boost::none;
			}
		};
		template<std::size_t Id>static auto instance_function()
		{
			return instance_t<Id, typename param<Id>::constructor_tuple>{};
		}
	};

}
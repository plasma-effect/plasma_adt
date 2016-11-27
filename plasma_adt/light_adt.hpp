#pragma once

#ifndef _MSC_VER
#include<boost/optional.hpp>
#include<boost/variant.hpp>
#define PLASMA_ADT_BOOST_OT_STD boost
#define PLASMA_ADT_VARIANT_INDEX .which()
#else
#include<variant>
#include<optional>
#define PLASMA_ADT_BOOST_OT_STD std
#define PLASMA_ADT_VARIANT_INDEX .index()
#endif

#include<utility>
#include<typeindex>
#include<tuple>
#include<vector>
#include<memory>

namespace plasma_adt
{
	template<class... Ts>struct tuple
	{
		static constexpr std::size_t size = sizeof...(Ts);
	};
	namespace detail
	{
		template<std::size_t I, class, class... Ts>struct tuple_element_i
		{
			typedef typename tuple_element_i<I - 1, Ts...>::type type;
		};
		template<class T, class... Ts>struct tuple_element_i<0, T, Ts...>
		{
			typedef T type;
		};
	}
	template<std::size_t I, class Tuple>struct tuple_element;
	template<std::size_t I, class... Ts>struct tuple_element<I, tuple<Ts...>>
	{
		typedef typename detail::tuple_element_i<I, Ts...>::type type;
	};

	template<std::size_t ID, class T>struct id_type
	{
		T val;
		typedef T value_type;
		static constexpr auto id = ID;
	};

	namespace detail
	{
		template<class... Ts>class inside_tuple
		{
			std::tuple<Ts...> value;
		public:
			template<class... Us>constexpr inside_tuple(Us&&... args) :value(std::forward<Us>(args)...)
			{

			}
			template<std::size_t I>auto& get()
			{
				return std::get<I>(value);
			}
			template<std::size_t I>constexpr auto const& get()const
			{
				return std::get<I>(value);
			}
		};
		template<class... Ts>class inside_variant
		{
			PLASMA_ADT_BOOST_OT_STD::variant<Ts...> value;
		public:
			template<class T>inside_variant(T&& arg) :value(std::forward<T>(arg))
			{

			}
			template<class T>auto& get()
			{
				return PLASMA_ADT_BOOST_OT_STD::get<T>(value);
			}
			template<class T>auto const& get()const
			{
				return PLASMA_ADT_BOOST_OT_STD::get<T>(value);
			}
			auto which()const
			{
				return value PLASMA_ADT_VARIANT_INDEX;
			}
		};

		template<class... Ts>constexpr auto make_tuple(Ts&&... args)
		{
			return inside_tuple<std::remove_reference_t<Ts>...>(std::forward<Ts>(args)...);
		}
	}

	namespace detail
	{
		template<class DataType, class Index, class... Ts>struct data_type_base;
		template<class DataType, std::size_t... Is, class... Ts>struct data_type_base<DataType, std::index_sequence<Is...>, Ts...>
		{
			template<std::size_t ID, class T>struct instance_type;
			struct inside_type
			{
				template<class T, class = void>struct value_type
				{
					typedef T type;
				};
				template<class... Us>struct value_type<tuple<Us...>, void>
				{
					typedef detail::inside_tuple<typename value_type<Us>::type...> type;
				};
				template<class T>struct value_type<DataType, T>
				{
					struct return_type
					{
						std::shared_ptr<inside_type> val;
						return_type(DataType const& d) :val(d.ptr)
						{

						}
						return_type(DataType&& d) :val(std::move(d.ptr))
						{

						}
						operator data_type_base()const
						{
							return data_type_base{ val };
						}
						template<std::size_t ID, class U>auto const& get(data_type_base::instance_type<ID, U>const& ins)const
						{
							return val->get(ins);
						}
					};
					typedef return_type type;
				};
				detail::inside_variant<id_type<Is, typename value_type<Ts>::type>...> value;
				template<class T>inside_type(T&& v) :value(std::forward<T>(v))
				{

				}
				template<std::size_t ID, class T>auto get(instance_type<ID, T>const&)const
				{
					typedef decltype(value.template get<id_type<ID, typename instance_type<ID, T>::Inside>>().val) return_type;
					if (value.which() == ID)
					{
						return PLASMA_ADT_BOOST_OT_STD::make_optional(std::cref(value.template get<id_type<ID, typename instance_type<ID, T>::Inside>>().val));
					}
					return PLASMA_ADT_BOOST_OT_STD::optional<std::reference_wrapper<return_type const>>();
				}
			};
			template<class T, class = void>struct argument_type
			{
				typedef T type;
			};
			std::shared_ptr<inside_type> ptr;
			template<class T>auto get(T const& val)const
			{
				return ptr->get(val);
			}

			template<std::size_t ID, class T>struct instance_type
			{
				typedef typename inside_type::template value_type<T>::type Type;
				typedef typename argument_type<T>::type Arg;
				typedef typename inside_type::template value_type<T>::type Inside;

				auto operator()(Arg&& val)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,Type>{std::move(val)}) };
				}
				auto operator()(Arg const& val)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,Type>{val}) };
				}
			};
			template<std::size_t ID, class T0>struct instance_type<ID, tuple<T0>>
			{
				typedef typename inside_type::template value_type<T0>::type Type0;
				typedef typename argument_type<T0>::type Arg0;
				typedef detail::inside_tuple<Type0> Inside;
				auto operator()(Arg0&& arg0)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0>> {detail::inside_tuple<Type0>(std::move(arg0))}) };
				}
				auto operator()(Arg0 const& arg0)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0>> {detail::inside_tuple<Type0>(arg0)}) };
				}
			};
			template<std::size_t ID, class T0, class T1>struct instance_type<ID, tuple<T0, T1>>
			{
				typedef typename inside_type::template value_type<T0>::type Type0;
				typedef typename argument_type<T0>::type Arg0;
				typedef typename inside_type::template value_type<T1>::type Type1;
				typedef typename argument_type<T1>::type Arg1;
				typedef detail::inside_tuple<Type0, Type1> Inside;
				auto operator()(Arg0&& arg0, Arg1&& arg1)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1>> {detail::inside_tuple<Type0, Type1>(std::move(arg0), std::move(arg1))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1>> {detail::inside_tuple<Type0, Type1>(arg0, std::move(arg1))}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1>> {detail::inside_tuple<Type0, Type1>(std::move(arg0), arg1)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1>> {detail::inside_tuple<Type0, Type1>(arg0, arg1)}) };
				}
			};
			template<std::size_t ID, class T0, class T1, class T2>struct instance_type<ID, tuple<T0, T1, T2>>
			{
				typedef typename inside_type::template value_type<T0>::type Type0;
				typedef typename argument_type<T0>::type Arg0;
				typedef typename inside_type::template value_type<T1>::type Type1;
				typedef typename argument_type<T1>::type Arg1;
				typedef typename inside_type::template value_type<T2>::type Type2;
				typedef typename argument_type<T2>::type Arg2;
				typedef detail::inside_tuple<Type0, Type1, Type2> Inside;
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2>> {detail::inside_tuple<Type0, Type1, Type2>(std::move(arg0), std::move(arg1), std::move(arg2))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2&& arg2)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2>> {detail::inside_tuple<Type0, Type1, Type2>(arg0, std::move(arg1), std::move(arg2))}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2&& arg2)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2>> {detail::inside_tuple<Type0, Type1, Type2>(std::move(arg0), arg1, std::move(arg2))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2&& arg2)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2>> {detail::inside_tuple<Type0, Type1, Type2>(arg0, arg1, std::move(arg2))}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2 const& arg2)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2>> {detail::inside_tuple<Type0, Type1, Type2>(std::move(arg0), std::move(arg1), arg2)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2 const& arg2)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2>> {detail::inside_tuple<Type0, Type1, Type2>(arg0, std::move(arg1), arg2)}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2 const& arg2)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2>> {detail::inside_tuple<Type0, Type1, Type2>(std::move(arg0), arg1, arg2)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2 const& arg2)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2>> {detail::inside_tuple<Type0, Type1, Type2>(arg0, arg1, arg2)}) };
				}
			};
			template<std::size_t ID, class T0, class T1, class T2, class T3>struct instance_type<ID, tuple<T0, T1, T2, T3>>
			{
				typedef typename inside_type::template value_type<T0>::type Type0;
				typedef typename argument_type<T0>::type Arg0;
				typedef typename inside_type::template value_type<T1>::type Type1;
				typedef typename argument_type<T1>::type Arg1;
				typedef typename inside_type::template value_type<T2>::type Type2;
				typedef typename argument_type<T2>::type Arg2;
				typedef typename inside_type::template value_type<T3>::type Type3;
				typedef typename argument_type<T3>::type Arg3;
				typedef detail::inside_tuple<Type0, Type1, Type2, Type3> Inside;
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3>> {detail::inside_tuple<Type0, Type1, Type2, Type3>(std::move(arg0), std::move(arg1), std::move(arg2), std::move(arg3))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3>> {detail::inside_tuple<Type0, Type1, Type2, Type3>(arg0, std::move(arg1), std::move(arg2), std::move(arg3))}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3&& arg3)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3>> {detail::inside_tuple<Type0, Type1, Type2, Type3>(std::move(arg0), arg1, std::move(arg2), std::move(arg3))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3&& arg3)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3>> {detail::inside_tuple<Type0, Type1, Type2, Type3>(arg0, arg1, std::move(arg2), std::move(arg3))}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3&& arg3)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3>> {detail::inside_tuple<Type0, Type1, Type2, Type3>(std::move(arg0), std::move(arg1), arg2, std::move(arg3))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3&& arg3)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3>> {detail::inside_tuple<Type0, Type1, Type2, Type3>(arg0, std::move(arg1), arg2, std::move(arg3))}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3&& arg3)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3>> {detail::inside_tuple<Type0, Type1, Type2, Type3>(std::move(arg0), arg1, arg2, std::move(arg3))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3&& arg3)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3>> {detail::inside_tuple<Type0, Type1, Type2, Type3>(arg0, arg1, arg2, std::move(arg3))}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3 const& arg3)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3>> {detail::inside_tuple<Type0, Type1, Type2, Type3>(std::move(arg0), std::move(arg1), std::move(arg2), arg3)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2&& arg2, Arg3 const& arg3)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3>> {detail::inside_tuple<Type0, Type1, Type2, Type3>(arg0, std::move(arg1), std::move(arg2), arg3)}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3 const& arg3)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3>> {detail::inside_tuple<Type0, Type1, Type2, Type3>(std::move(arg0), arg1, std::move(arg2), arg3)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3 const& arg3)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3>> {detail::inside_tuple<Type0, Type1, Type2, Type3>(arg0, arg1, std::move(arg2), arg3)}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3 const& arg3)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3>> {detail::inside_tuple<Type0, Type1, Type2, Type3>(std::move(arg0), std::move(arg1), arg2, arg3)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3 const& arg3)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3>> {detail::inside_tuple<Type0, Type1, Type2, Type3>(arg0, std::move(arg1), arg2, arg3)}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3 const& arg3)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3>> {detail::inside_tuple<Type0, Type1, Type2, Type3>(std::move(arg0), arg1, arg2, arg3)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3 const& arg3)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3>> {detail::inside_tuple<Type0, Type1, Type2, Type3>(arg0, arg1, arg2, arg3)}) };
				}
			};
			template<std::size_t ID, class T0, class T1, class T2, class T3, class T4>struct instance_type<ID, tuple<T0, T1, T2, T3, T4>>
			{
				typedef typename inside_type::template value_type<T0>::type Type0;
				typedef typename argument_type<T0>::type Arg0;
				typedef typename inside_type::template value_type<T1>::type Type1;
				typedef typename argument_type<T1>::type Arg1;
				typedef typename inside_type::template value_type<T2>::type Type2;
				typedef typename argument_type<T2>::type Arg2;
				typedef typename inside_type::template value_type<T3>::type Type3;
				typedef typename argument_type<T3>::type Arg3;
				typedef typename inside_type::template value_type<T4>::type Type4;
				typedef typename argument_type<T4>::type Arg4;
				typedef detail::inside_tuple<Type0, Type1, Type2, Type3, Type4> Inside;
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4>(std::move(arg0), std::move(arg1), std::move(arg2), std::move(arg3), std::move(arg4))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4>(arg0, std::move(arg1), std::move(arg2), std::move(arg3), std::move(arg4))}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4>(std::move(arg0), arg1, std::move(arg2), std::move(arg3), std::move(arg4))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4>(arg0, arg1, std::move(arg2), std::move(arg3), std::move(arg4))}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4&& arg4)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4>(std::move(arg0), std::move(arg1), arg2, std::move(arg3), std::move(arg4))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4&& arg4)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4>(arg0, std::move(arg1), arg2, std::move(arg3), std::move(arg4))}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4&& arg4)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4>(std::move(arg0), arg1, arg2, std::move(arg3), std::move(arg4))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4&& arg4)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4>(arg0, arg1, arg2, std::move(arg3), std::move(arg4))}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4&& arg4)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4>(std::move(arg0), std::move(arg1), std::move(arg2), arg3, std::move(arg4))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4&& arg4)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4>(arg0, std::move(arg1), std::move(arg2), arg3, std::move(arg4))}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4&& arg4)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4>(std::move(arg0), arg1, std::move(arg2), arg3, std::move(arg4))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4&& arg4)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4>(arg0, arg1, std::move(arg2), arg3, std::move(arg4))}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4&& arg4)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4>(std::move(arg0), std::move(arg1), arg2, arg3, std::move(arg4))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4&& arg4)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4>(arg0, std::move(arg1), arg2, arg3, std::move(arg4))}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4&& arg4)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4>(std::move(arg0), arg1, arg2, arg3, std::move(arg4))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4&& arg4)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4>(arg0, arg1, arg2, arg3, std::move(arg4))}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4 const& arg4)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4>(std::move(arg0), std::move(arg1), std::move(arg2), std::move(arg3), arg4)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4 const& arg4)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4>(arg0, std::move(arg1), std::move(arg2), std::move(arg3), arg4)}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3&& arg3, Arg4 const& arg4)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4>(std::move(arg0), arg1, std::move(arg2), std::move(arg3), arg4)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3&& arg3, Arg4 const& arg4)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4>(arg0, arg1, std::move(arg2), std::move(arg3), arg4)}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4 const& arg4)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4>(std::move(arg0), std::move(arg1), arg2, std::move(arg3), arg4)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4 const& arg4)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4>(arg0, std::move(arg1), arg2, std::move(arg3), arg4)}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4 const& arg4)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4>(std::move(arg0), arg1, arg2, std::move(arg3), arg4)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4 const& arg4)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4>(arg0, arg1, arg2, std::move(arg3), arg4)}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4 const& arg4)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4>(std::move(arg0), std::move(arg1), std::move(arg2), arg3, arg4)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4 const& arg4)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4>(arg0, std::move(arg1), std::move(arg2), arg3, arg4)}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4 const& arg4)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4>(std::move(arg0), arg1, std::move(arg2), arg3, arg4)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4 const& arg4)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4>(arg0, arg1, std::move(arg2), arg3, arg4)}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4 const& arg4)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4>(std::move(arg0), std::move(arg1), arg2, arg3, arg4)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4 const& arg4)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4>(arg0, std::move(arg1), arg2, arg3, arg4)}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4 const& arg4)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4>(std::move(arg0), arg1, arg2, arg3, arg4)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4 const& arg4)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4>(arg0, arg1, arg2, arg3, arg4)}) };
				}
			};
			template<std::size_t ID, class T0, class T1, class T2, class T3, class T4, class T5>struct instance_type<ID, tuple<T0, T1, T2, T3, T4, T5>>
			{
				typedef typename inside_type::template value_type<T0>::type Type0;
				typedef typename argument_type<T0>::type Arg0;
				typedef typename inside_type::template value_type<T1>::type Type1;
				typedef typename argument_type<T1>::type Arg1;
				typedef typename inside_type::template value_type<T2>::type Type2;
				typedef typename argument_type<T2>::type Arg2;
				typedef typename inside_type::template value_type<T3>::type Type3;
				typedef typename argument_type<T3>::type Arg3;
				typedef typename inside_type::template value_type<T4>::type Type4;
				typedef typename argument_type<T4>::type Arg4;
				typedef typename inside_type::template value_type<T5>::type Type5;
				typedef typename argument_type<T5>::type Arg5;
				typedef detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5> Inside;
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>(std::move(arg0), std::move(arg1), std::move(arg2), std::move(arg3), std::move(arg4), std::move(arg5))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>(arg0, std::move(arg1), std::move(arg2), std::move(arg3), std::move(arg4), std::move(arg5))}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>(std::move(arg0), arg1, std::move(arg2), std::move(arg3), std::move(arg4), std::move(arg5))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>(arg0, arg1, std::move(arg2), std::move(arg3), std::move(arg4), std::move(arg5))}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>(std::move(arg0), std::move(arg1), arg2, std::move(arg3), std::move(arg4), std::move(arg5))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>(arg0, std::move(arg1), arg2, std::move(arg3), std::move(arg4), std::move(arg5))}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>(std::move(arg0), arg1, arg2, std::move(arg3), std::move(arg4), std::move(arg5))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>(arg0, arg1, arg2, std::move(arg3), std::move(arg4), std::move(arg5))}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5&& arg5)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>(std::move(arg0), std::move(arg1), std::move(arg2), arg3, std::move(arg4), std::move(arg5))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5&& arg5)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>(arg0, std::move(arg1), std::move(arg2), arg3, std::move(arg4), std::move(arg5))}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5&& arg5)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>(std::move(arg0), arg1, std::move(arg2), arg3, std::move(arg4), std::move(arg5))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5&& arg5)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>(arg0, arg1, std::move(arg2), arg3, std::move(arg4), std::move(arg5))}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5&& arg5)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>(std::move(arg0), std::move(arg1), arg2, arg3, std::move(arg4), std::move(arg5))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5&& arg5)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>(arg0, std::move(arg1), arg2, arg3, std::move(arg4), std::move(arg5))}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5&& arg5)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>(std::move(arg0), arg1, arg2, arg3, std::move(arg4), std::move(arg5))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5&& arg5)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>(arg0, arg1, arg2, arg3, std::move(arg4), std::move(arg5))}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5&& arg5)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>(std::move(arg0), std::move(arg1), std::move(arg2), std::move(arg3), arg4, std::move(arg5))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5&& arg5)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>(arg0, std::move(arg1), std::move(arg2), std::move(arg3), arg4, std::move(arg5))}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5&& arg5)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>(std::move(arg0), arg1, std::move(arg2), std::move(arg3), arg4, std::move(arg5))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5&& arg5)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>(arg0, arg1, std::move(arg2), std::move(arg3), arg4, std::move(arg5))}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5&& arg5)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>(std::move(arg0), std::move(arg1), arg2, std::move(arg3), arg4, std::move(arg5))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5&& arg5)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>(arg0, std::move(arg1), arg2, std::move(arg3), arg4, std::move(arg5))}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5&& arg5)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>(std::move(arg0), arg1, arg2, std::move(arg3), arg4, std::move(arg5))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5&& arg5)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>(arg0, arg1, arg2, std::move(arg3), arg4, std::move(arg5))}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5&& arg5)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>(std::move(arg0), std::move(arg1), std::move(arg2), arg3, arg4, std::move(arg5))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5&& arg5)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>(arg0, std::move(arg1), std::move(arg2), arg3, arg4, std::move(arg5))}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5&& arg5)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>(std::move(arg0), arg1, std::move(arg2), arg3, arg4, std::move(arg5))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5&& arg5)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>(arg0, arg1, std::move(arg2), arg3, arg4, std::move(arg5))}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5&& arg5)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>(std::move(arg0), std::move(arg1), arg2, arg3, arg4, std::move(arg5))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5&& arg5)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>(arg0, std::move(arg1), arg2, arg3, arg4, std::move(arg5))}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5&& arg5)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>(std::move(arg0), arg1, arg2, arg3, arg4, std::move(arg5))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5&& arg5)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>(arg0, arg1, arg2, arg3, arg4, std::move(arg5))}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5 const& arg5)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>(std::move(arg0), std::move(arg1), std::move(arg2), std::move(arg3), std::move(arg4), arg5)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5 const& arg5)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>(arg0, std::move(arg1), std::move(arg2), std::move(arg3), std::move(arg4), arg5)}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5 const& arg5)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>(std::move(arg0), arg1, std::move(arg2), std::move(arg3), std::move(arg4), arg5)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5 const& arg5)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>(arg0, arg1, std::move(arg2), std::move(arg3), std::move(arg4), arg5)}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4&& arg4, Arg5 const& arg5)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>(std::move(arg0), std::move(arg1), arg2, std::move(arg3), std::move(arg4), arg5)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4&& arg4, Arg5 const& arg5)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>(arg0, std::move(arg1), arg2, std::move(arg3), std::move(arg4), arg5)}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4&& arg4, Arg5 const& arg5)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>(std::move(arg0), arg1, arg2, std::move(arg3), std::move(arg4), arg5)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4&& arg4, Arg5 const& arg5)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>(arg0, arg1, arg2, std::move(arg3), std::move(arg4), arg5)}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5 const& arg5)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>(std::move(arg0), std::move(arg1), std::move(arg2), arg3, std::move(arg4), arg5)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5 const& arg5)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>(arg0, std::move(arg1), std::move(arg2), arg3, std::move(arg4), arg5)}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5 const& arg5)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>(std::move(arg0), arg1, std::move(arg2), arg3, std::move(arg4), arg5)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5 const& arg5)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>(arg0, arg1, std::move(arg2), arg3, std::move(arg4), arg5)}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5 const& arg5)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>(std::move(arg0), std::move(arg1), arg2, arg3, std::move(arg4), arg5)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5 const& arg5)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>(arg0, std::move(arg1), arg2, arg3, std::move(arg4), arg5)}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5 const& arg5)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>(std::move(arg0), arg1, arg2, arg3, std::move(arg4), arg5)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5 const& arg5)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>(arg0, arg1, arg2, arg3, std::move(arg4), arg5)}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5 const& arg5)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>(std::move(arg0), std::move(arg1), std::move(arg2), std::move(arg3), arg4, arg5)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5 const& arg5)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>(arg0, std::move(arg1), std::move(arg2), std::move(arg3), arg4, arg5)}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5 const& arg5)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>(std::move(arg0), arg1, std::move(arg2), std::move(arg3), arg4, arg5)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5 const& arg5)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>(arg0, arg1, std::move(arg2), std::move(arg3), arg4, arg5)}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5 const& arg5)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>(std::move(arg0), std::move(arg1), arg2, std::move(arg3), arg4, arg5)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5 const& arg5)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>(arg0, std::move(arg1), arg2, std::move(arg3), arg4, arg5)}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5 const& arg5)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>(std::move(arg0), arg1, arg2, std::move(arg3), arg4, arg5)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5 const& arg5)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>(arg0, arg1, arg2, std::move(arg3), arg4, arg5)}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5 const& arg5)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>(std::move(arg0), std::move(arg1), std::move(arg2), arg3, arg4, arg5)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5 const& arg5)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>(arg0, std::move(arg1), std::move(arg2), arg3, arg4, arg5)}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5 const& arg5)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>(std::move(arg0), arg1, std::move(arg2), arg3, arg4, arg5)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5 const& arg5)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>(arg0, arg1, std::move(arg2), arg3, arg4, arg5)}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5 const& arg5)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>(std::move(arg0), std::move(arg1), arg2, arg3, arg4, arg5)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5 const& arg5)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>(arg0, std::move(arg1), arg2, arg3, arg4, arg5)}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5 const& arg5)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>(std::move(arg0), arg1, arg2, arg3, arg4, arg5)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5 const& arg5)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5>(arg0, arg1, arg2, arg3, arg4, arg5)}) };
				}
			};
			template<std::size_t ID, class T0, class T1, class T2, class T3, class T4, class T5, class T6>struct instance_type<ID, tuple<T0, T1, T2, T3, T4, T5, T6>>
			{
				typedef typename inside_type::template value_type<T0>::type Type0;
				typedef typename argument_type<T0>::type Arg0;
				typedef typename inside_type::template value_type<T1>::type Type1;
				typedef typename argument_type<T1>::type Arg1;
				typedef typename inside_type::template value_type<T2>::type Type2;
				typedef typename argument_type<T2>::type Arg2;
				typedef typename inside_type::template value_type<T3>::type Type3;
				typedef typename argument_type<T3>::type Arg3;
				typedef typename inside_type::template value_type<T4>::type Type4;
				typedef typename argument_type<T4>::type Arg4;
				typedef typename inside_type::template value_type<T5>::type Type5;
				typedef typename argument_type<T5>::type Arg5;
				typedef typename inside_type::template value_type<T6>::type Type6;
				typedef typename argument_type<T6>::type Arg6;
				typedef detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6> Inside;
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5, Arg6&& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(std::move(arg0), std::move(arg1), std::move(arg2), std::move(arg3), std::move(arg4), std::move(arg5), std::move(arg6))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5, Arg6&& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(arg0, std::move(arg1), std::move(arg2), std::move(arg3), std::move(arg4), std::move(arg5), std::move(arg6))}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5, Arg6&& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(std::move(arg0), arg1, std::move(arg2), std::move(arg3), std::move(arg4), std::move(arg5), std::move(arg6))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5, Arg6&& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(arg0, arg1, std::move(arg2), std::move(arg3), std::move(arg4), std::move(arg5), std::move(arg6))}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5, Arg6&& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(std::move(arg0), std::move(arg1), arg2, std::move(arg3), std::move(arg4), std::move(arg5), std::move(arg6))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5, Arg6&& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(arg0, std::move(arg1), arg2, std::move(arg3), std::move(arg4), std::move(arg5), std::move(arg6))}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5, Arg6&& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(std::move(arg0), arg1, arg2, std::move(arg3), std::move(arg4), std::move(arg5), std::move(arg6))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5, Arg6&& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(arg0, arg1, arg2, std::move(arg3), std::move(arg4), std::move(arg5), std::move(arg6))}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5&& arg5, Arg6&& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(std::move(arg0), std::move(arg1), std::move(arg2), arg3, std::move(arg4), std::move(arg5), std::move(arg6))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5&& arg5, Arg6&& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(arg0, std::move(arg1), std::move(arg2), arg3, std::move(arg4), std::move(arg5), std::move(arg6))}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5&& arg5, Arg6&& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(std::move(arg0), arg1, std::move(arg2), arg3, std::move(arg4), std::move(arg5), std::move(arg6))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5&& arg5, Arg6&& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(arg0, arg1, std::move(arg2), arg3, std::move(arg4), std::move(arg5), std::move(arg6))}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5&& arg5, Arg6&& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(std::move(arg0), std::move(arg1), arg2, arg3, std::move(arg4), std::move(arg5), std::move(arg6))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5&& arg5, Arg6&& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(arg0, std::move(arg1), arg2, arg3, std::move(arg4), std::move(arg5), std::move(arg6))}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5&& arg5, Arg6&& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(std::move(arg0), arg1, arg2, arg3, std::move(arg4), std::move(arg5), std::move(arg6))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5&& arg5, Arg6&& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(arg0, arg1, arg2, arg3, std::move(arg4), std::move(arg5), std::move(arg6))}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6&& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(std::move(arg0), std::move(arg1), std::move(arg2), std::move(arg3), arg4, std::move(arg5), std::move(arg6))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6&& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(arg0, std::move(arg1), std::move(arg2), std::move(arg3), arg4, std::move(arg5), std::move(arg6))}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6&& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(std::move(arg0), arg1, std::move(arg2), std::move(arg3), arg4, std::move(arg5), std::move(arg6))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6&& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(arg0, arg1, std::move(arg2), std::move(arg3), arg4, std::move(arg5), std::move(arg6))}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6&& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(std::move(arg0), std::move(arg1), arg2, std::move(arg3), arg4, std::move(arg5), std::move(arg6))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6&& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(arg0, std::move(arg1), arg2, std::move(arg3), arg4, std::move(arg5), std::move(arg6))}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6&& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(std::move(arg0), arg1, arg2, std::move(arg3), arg4, std::move(arg5), std::move(arg6))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6&& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(arg0, arg1, arg2, std::move(arg3), arg4, std::move(arg5), std::move(arg6))}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6&& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(std::move(arg0), std::move(arg1), std::move(arg2), arg3, arg4, std::move(arg5), std::move(arg6))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6&& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(arg0, std::move(arg1), std::move(arg2), arg3, arg4, std::move(arg5), std::move(arg6))}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6&& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(std::move(arg0), arg1, std::move(arg2), arg3, arg4, std::move(arg5), std::move(arg6))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6&& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(arg0, arg1, std::move(arg2), arg3, arg4, std::move(arg5), std::move(arg6))}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6&& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(std::move(arg0), std::move(arg1), arg2, arg3, arg4, std::move(arg5), std::move(arg6))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6&& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(arg0, std::move(arg1), arg2, arg3, arg4, std::move(arg5), std::move(arg6))}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6&& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(std::move(arg0), arg1, arg2, arg3, arg4, std::move(arg5), std::move(arg6))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6&& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(arg0, arg1, arg2, arg3, arg4, std::move(arg5), std::move(arg6))}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6&& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(std::move(arg0), std::move(arg1), std::move(arg2), std::move(arg3), std::move(arg4), arg5, std::move(arg6))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6&& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(arg0, std::move(arg1), std::move(arg2), std::move(arg3), std::move(arg4), arg5, std::move(arg6))}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6&& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(std::move(arg0), arg1, std::move(arg2), std::move(arg3), std::move(arg4), arg5, std::move(arg6))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6&& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(arg0, arg1, std::move(arg2), std::move(arg3), std::move(arg4), arg5, std::move(arg6))}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6&& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(std::move(arg0), std::move(arg1), arg2, std::move(arg3), std::move(arg4), arg5, std::move(arg6))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6&& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(arg0, std::move(arg1), arg2, std::move(arg3), std::move(arg4), arg5, std::move(arg6))}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6&& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(std::move(arg0), arg1, arg2, std::move(arg3), std::move(arg4), arg5, std::move(arg6))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6&& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(arg0, arg1, arg2, std::move(arg3), std::move(arg4), arg5, std::move(arg6))}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6&& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(std::move(arg0), std::move(arg1), std::move(arg2), arg3, std::move(arg4), arg5, std::move(arg6))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6&& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(arg0, std::move(arg1), std::move(arg2), arg3, std::move(arg4), arg5, std::move(arg6))}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6&& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(std::move(arg0), arg1, std::move(arg2), arg3, std::move(arg4), arg5, std::move(arg6))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6&& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(arg0, arg1, std::move(arg2), arg3, std::move(arg4), arg5, std::move(arg6))}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6&& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(std::move(arg0), std::move(arg1), arg2, arg3, std::move(arg4), arg5, std::move(arg6))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6&& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(arg0, std::move(arg1), arg2, arg3, std::move(arg4), arg5, std::move(arg6))}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6&& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(std::move(arg0), arg1, arg2, arg3, std::move(arg4), arg5, std::move(arg6))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6&& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(arg0, arg1, arg2, arg3, std::move(arg4), arg5, std::move(arg6))}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6&& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(std::move(arg0), std::move(arg1), std::move(arg2), std::move(arg3), arg4, arg5, std::move(arg6))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6&& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(arg0, std::move(arg1), std::move(arg2), std::move(arg3), arg4, arg5, std::move(arg6))}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6&& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(std::move(arg0), arg1, std::move(arg2), std::move(arg3), arg4, arg5, std::move(arg6))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6&& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(arg0, arg1, std::move(arg2), std::move(arg3), arg4, arg5, std::move(arg6))}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6&& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(std::move(arg0), std::move(arg1), arg2, std::move(arg3), arg4, arg5, std::move(arg6))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6&& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(arg0, std::move(arg1), arg2, std::move(arg3), arg4, arg5, std::move(arg6))}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6&& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(std::move(arg0), arg1, arg2, std::move(arg3), arg4, arg5, std::move(arg6))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6&& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(arg0, arg1, arg2, std::move(arg3), arg4, arg5, std::move(arg6))}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6&& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(std::move(arg0), std::move(arg1), std::move(arg2), arg3, arg4, arg5, std::move(arg6))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6&& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(arg0, std::move(arg1), std::move(arg2), arg3, arg4, arg5, std::move(arg6))}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6&& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(std::move(arg0), arg1, std::move(arg2), arg3, arg4, arg5, std::move(arg6))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6&& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(arg0, arg1, std::move(arg2), arg3, arg4, arg5, std::move(arg6))}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6&& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(std::move(arg0), std::move(arg1), arg2, arg3, arg4, arg5, std::move(arg6))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6&& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(arg0, std::move(arg1), arg2, arg3, arg4, arg5, std::move(arg6))}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6&& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(std::move(arg0), arg1, arg2, arg3, arg4, arg5, std::move(arg6))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6&& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(arg0, arg1, arg2, arg3, arg4, arg5, std::move(arg6))}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5, Arg6 const& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(std::move(arg0), std::move(arg1), std::move(arg2), std::move(arg3), std::move(arg4), std::move(arg5), arg6)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5, Arg6 const& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(arg0, std::move(arg1), std::move(arg2), std::move(arg3), std::move(arg4), std::move(arg5), arg6)}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5, Arg6 const& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(std::move(arg0), arg1, std::move(arg2), std::move(arg3), std::move(arg4), std::move(arg5), arg6)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5, Arg6 const& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(arg0, arg1, std::move(arg2), std::move(arg3), std::move(arg4), std::move(arg5), arg6)}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5, Arg6 const& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(std::move(arg0), std::move(arg1), arg2, std::move(arg3), std::move(arg4), std::move(arg5), arg6)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5, Arg6 const& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(arg0, std::move(arg1), arg2, std::move(arg3), std::move(arg4), std::move(arg5), arg6)}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5, Arg6 const& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(std::move(arg0), arg1, arg2, std::move(arg3), std::move(arg4), std::move(arg5), arg6)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5, Arg6 const& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(arg0, arg1, arg2, std::move(arg3), std::move(arg4), std::move(arg5), arg6)}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5&& arg5, Arg6 const& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(std::move(arg0), std::move(arg1), std::move(arg2), arg3, std::move(arg4), std::move(arg5), arg6)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5&& arg5, Arg6 const& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(arg0, std::move(arg1), std::move(arg2), arg3, std::move(arg4), std::move(arg5), arg6)}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5&& arg5, Arg6 const& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(std::move(arg0), arg1, std::move(arg2), arg3, std::move(arg4), std::move(arg5), arg6)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5&& arg5, Arg6 const& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(arg0, arg1, std::move(arg2), arg3, std::move(arg4), std::move(arg5), arg6)}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5&& arg5, Arg6 const& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(std::move(arg0), std::move(arg1), arg2, arg3, std::move(arg4), std::move(arg5), arg6)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5&& arg5, Arg6 const& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(arg0, std::move(arg1), arg2, arg3, std::move(arg4), std::move(arg5), arg6)}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5&& arg5, Arg6 const& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(std::move(arg0), arg1, arg2, arg3, std::move(arg4), std::move(arg5), arg6)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5&& arg5, Arg6 const& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(arg0, arg1, arg2, arg3, std::move(arg4), std::move(arg5), arg6)}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6 const& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(std::move(arg0), std::move(arg1), std::move(arg2), std::move(arg3), arg4, std::move(arg5), arg6)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6 const& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(arg0, std::move(arg1), std::move(arg2), std::move(arg3), arg4, std::move(arg5), arg6)}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6 const& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(std::move(arg0), arg1, std::move(arg2), std::move(arg3), arg4, std::move(arg5), arg6)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6 const& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(arg0, arg1, std::move(arg2), std::move(arg3), arg4, std::move(arg5), arg6)}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6 const& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(std::move(arg0), std::move(arg1), arg2, std::move(arg3), arg4, std::move(arg5), arg6)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6 const& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(arg0, std::move(arg1), arg2, std::move(arg3), arg4, std::move(arg5), arg6)}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6 const& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(std::move(arg0), arg1, arg2, std::move(arg3), arg4, std::move(arg5), arg6)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6 const& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(arg0, arg1, arg2, std::move(arg3), arg4, std::move(arg5), arg6)}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6 const& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(std::move(arg0), std::move(arg1), std::move(arg2), arg3, arg4, std::move(arg5), arg6)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6 const& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(arg0, std::move(arg1), std::move(arg2), arg3, arg4, std::move(arg5), arg6)}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6 const& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(std::move(arg0), arg1, std::move(arg2), arg3, arg4, std::move(arg5), arg6)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6 const& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(arg0, arg1, std::move(arg2), arg3, arg4, std::move(arg5), arg6)}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6 const& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(std::move(arg0), std::move(arg1), arg2, arg3, arg4, std::move(arg5), arg6)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6 const& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(arg0, std::move(arg1), arg2, arg3, arg4, std::move(arg5), arg6)}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6 const& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(std::move(arg0), arg1, arg2, arg3, arg4, std::move(arg5), arg6)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6 const& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(arg0, arg1, arg2, arg3, arg4, std::move(arg5), arg6)}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6 const& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(std::move(arg0), std::move(arg1), std::move(arg2), std::move(arg3), std::move(arg4), arg5, arg6)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6 const& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(arg0, std::move(arg1), std::move(arg2), std::move(arg3), std::move(arg4), arg5, arg6)}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6 const& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(std::move(arg0), arg1, std::move(arg2), std::move(arg3), std::move(arg4), arg5, arg6)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6 const& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(arg0, arg1, std::move(arg2), std::move(arg3), std::move(arg4), arg5, arg6)}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6 const& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(std::move(arg0), std::move(arg1), arg2, std::move(arg3), std::move(arg4), arg5, arg6)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6 const& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(arg0, std::move(arg1), arg2, std::move(arg3), std::move(arg4), arg5, arg6)}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6 const& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(std::move(arg0), arg1, arg2, std::move(arg3), std::move(arg4), arg5, arg6)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6 const& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(arg0, arg1, arg2, std::move(arg3), std::move(arg4), arg5, arg6)}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6 const& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(std::move(arg0), std::move(arg1), std::move(arg2), arg3, std::move(arg4), arg5, arg6)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6 const& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(arg0, std::move(arg1), std::move(arg2), arg3, std::move(arg4), arg5, arg6)}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6 const& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(std::move(arg0), arg1, std::move(arg2), arg3, std::move(arg4), arg5, arg6)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6 const& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(arg0, arg1, std::move(arg2), arg3, std::move(arg4), arg5, arg6)}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6 const& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(std::move(arg0), std::move(arg1), arg2, arg3, std::move(arg4), arg5, arg6)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6 const& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(arg0, std::move(arg1), arg2, arg3, std::move(arg4), arg5, arg6)}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6 const& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(std::move(arg0), arg1, arg2, arg3, std::move(arg4), arg5, arg6)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6 const& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(arg0, arg1, arg2, arg3, std::move(arg4), arg5, arg6)}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6 const& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(std::move(arg0), std::move(arg1), std::move(arg2), std::move(arg3), arg4, arg5, arg6)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6 const& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(arg0, std::move(arg1), std::move(arg2), std::move(arg3), arg4, arg5, arg6)}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6 const& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(std::move(arg0), arg1, std::move(arg2), std::move(arg3), arg4, arg5, arg6)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6 const& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(arg0, arg1, std::move(arg2), std::move(arg3), arg4, arg5, arg6)}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6 const& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(std::move(arg0), std::move(arg1), arg2, std::move(arg3), arg4, arg5, arg6)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6 const& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(arg0, std::move(arg1), arg2, std::move(arg3), arg4, arg5, arg6)}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6 const& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(std::move(arg0), arg1, arg2, std::move(arg3), arg4, arg5, arg6)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6 const& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(arg0, arg1, arg2, std::move(arg3), arg4, arg5, arg6)}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6 const& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(std::move(arg0), std::move(arg1), std::move(arg2), arg3, arg4, arg5, arg6)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6 const& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(arg0, std::move(arg1), std::move(arg2), arg3, arg4, arg5, arg6)}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6 const& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(std::move(arg0), arg1, std::move(arg2), arg3, arg4, arg5, arg6)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6 const& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(arg0, arg1, std::move(arg2), arg3, arg4, arg5, arg6)}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6 const& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(std::move(arg0), std::move(arg1), arg2, arg3, arg4, arg5, arg6)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6 const& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(arg0, std::move(arg1), arg2, arg3, arg4, arg5, arg6)}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6 const& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(std::move(arg0), arg1, arg2, arg3, arg4, arg5, arg6)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6 const& arg6)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6>(arg0, arg1, arg2, arg3, arg4, arg5, arg6)}) };
				}
			};
			template<std::size_t ID, class T0, class T1, class T2, class T3, class T4, class T5, class T6, class T7>struct instance_type<ID, tuple<T0, T1, T2, T3, T4, T5, T6, T7>>
			{
				typedef typename inside_type::template value_type<T0>::type Type0;
				typedef typename argument_type<T0>::type Arg0;
				typedef typename inside_type::template value_type<T1>::type Type1;
				typedef typename argument_type<T1>::type Arg1;
				typedef typename inside_type::template value_type<T2>::type Type2;
				typedef typename argument_type<T2>::type Arg2;
				typedef typename inside_type::template value_type<T3>::type Type3;
				typedef typename argument_type<T3>::type Arg3;
				typedef typename inside_type::template value_type<T4>::type Type4;
				typedef typename argument_type<T4>::type Arg4;
				typedef typename inside_type::template value_type<T5>::type Type5;
				typedef typename argument_type<T5>::type Arg5;
				typedef typename inside_type::template value_type<T6>::type Type6;
				typedef typename argument_type<T6>::type Arg6;
				typedef typename inside_type::template value_type<T7>::type Type7;
				typedef typename argument_type<T7>::type Arg7;
				typedef detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7> Inside;
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5, Arg6&& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), std::move(arg1), std::move(arg2), std::move(arg3), std::move(arg4), std::move(arg5), std::move(arg6), std::move(arg7))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5, Arg6&& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, std::move(arg1), std::move(arg2), std::move(arg3), std::move(arg4), std::move(arg5), std::move(arg6), std::move(arg7))}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5, Arg6&& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), arg1, std::move(arg2), std::move(arg3), std::move(arg4), std::move(arg5), std::move(arg6), std::move(arg7))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5, Arg6&& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, arg1, std::move(arg2), std::move(arg3), std::move(arg4), std::move(arg5), std::move(arg6), std::move(arg7))}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5, Arg6&& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), std::move(arg1), arg2, std::move(arg3), std::move(arg4), std::move(arg5), std::move(arg6), std::move(arg7))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5, Arg6&& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, std::move(arg1), arg2, std::move(arg3), std::move(arg4), std::move(arg5), std::move(arg6), std::move(arg7))}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5, Arg6&& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), arg1, arg2, std::move(arg3), std::move(arg4), std::move(arg5), std::move(arg6), std::move(arg7))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5, Arg6&& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, arg1, arg2, std::move(arg3), std::move(arg4), std::move(arg5), std::move(arg6), std::move(arg7))}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5&& arg5, Arg6&& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), std::move(arg1), std::move(arg2), arg3, std::move(arg4), std::move(arg5), std::move(arg6), std::move(arg7))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5&& arg5, Arg6&& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, std::move(arg1), std::move(arg2), arg3, std::move(arg4), std::move(arg5), std::move(arg6), std::move(arg7))}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5&& arg5, Arg6&& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), arg1, std::move(arg2), arg3, std::move(arg4), std::move(arg5), std::move(arg6), std::move(arg7))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5&& arg5, Arg6&& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, arg1, std::move(arg2), arg3, std::move(arg4), std::move(arg5), std::move(arg6), std::move(arg7))}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5&& arg5, Arg6&& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), std::move(arg1), arg2, arg3, std::move(arg4), std::move(arg5), std::move(arg6), std::move(arg7))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5&& arg5, Arg6&& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, std::move(arg1), arg2, arg3, std::move(arg4), std::move(arg5), std::move(arg6), std::move(arg7))}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5&& arg5, Arg6&& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), arg1, arg2, arg3, std::move(arg4), std::move(arg5), std::move(arg6), std::move(arg7))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5&& arg5, Arg6&& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, arg1, arg2, arg3, std::move(arg4), std::move(arg5), std::move(arg6), std::move(arg7))}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6&& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), std::move(arg1), std::move(arg2), std::move(arg3), arg4, std::move(arg5), std::move(arg6), std::move(arg7))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6&& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, std::move(arg1), std::move(arg2), std::move(arg3), arg4, std::move(arg5), std::move(arg6), std::move(arg7))}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6&& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), arg1, std::move(arg2), std::move(arg3), arg4, std::move(arg5), std::move(arg6), std::move(arg7))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6&& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, arg1, std::move(arg2), std::move(arg3), arg4, std::move(arg5), std::move(arg6), std::move(arg7))}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6&& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), std::move(arg1), arg2, std::move(arg3), arg4, std::move(arg5), std::move(arg6), std::move(arg7))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6&& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, std::move(arg1), arg2, std::move(arg3), arg4, std::move(arg5), std::move(arg6), std::move(arg7))}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6&& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), arg1, arg2, std::move(arg3), arg4, std::move(arg5), std::move(arg6), std::move(arg7))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6&& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, arg1, arg2, std::move(arg3), arg4, std::move(arg5), std::move(arg6), std::move(arg7))}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6&& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), std::move(arg1), std::move(arg2), arg3, arg4, std::move(arg5), std::move(arg6), std::move(arg7))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6&& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, std::move(arg1), std::move(arg2), arg3, arg4, std::move(arg5), std::move(arg6), std::move(arg7))}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6&& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), arg1, std::move(arg2), arg3, arg4, std::move(arg5), std::move(arg6), std::move(arg7))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6&& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, arg1, std::move(arg2), arg3, arg4, std::move(arg5), std::move(arg6), std::move(arg7))}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6&& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), std::move(arg1), arg2, arg3, arg4, std::move(arg5), std::move(arg6), std::move(arg7))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6&& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, std::move(arg1), arg2, arg3, arg4, std::move(arg5), std::move(arg6), std::move(arg7))}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6&& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), arg1, arg2, arg3, arg4, std::move(arg5), std::move(arg6), std::move(arg7))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6&& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, arg1, arg2, arg3, arg4, std::move(arg5), std::move(arg6), std::move(arg7))}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6&& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), std::move(arg1), std::move(arg2), std::move(arg3), std::move(arg4), arg5, std::move(arg6), std::move(arg7))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6&& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, std::move(arg1), std::move(arg2), std::move(arg3), std::move(arg4), arg5, std::move(arg6), std::move(arg7))}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6&& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), arg1, std::move(arg2), std::move(arg3), std::move(arg4), arg5, std::move(arg6), std::move(arg7))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6&& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, arg1, std::move(arg2), std::move(arg3), std::move(arg4), arg5, std::move(arg6), std::move(arg7))}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6&& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), std::move(arg1), arg2, std::move(arg3), std::move(arg4), arg5, std::move(arg6), std::move(arg7))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6&& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, std::move(arg1), arg2, std::move(arg3), std::move(arg4), arg5, std::move(arg6), std::move(arg7))}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6&& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), arg1, arg2, std::move(arg3), std::move(arg4), arg5, std::move(arg6), std::move(arg7))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6&& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, arg1, arg2, std::move(arg3), std::move(arg4), arg5, std::move(arg6), std::move(arg7))}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6&& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), std::move(arg1), std::move(arg2), arg3, std::move(arg4), arg5, std::move(arg6), std::move(arg7))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6&& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, std::move(arg1), std::move(arg2), arg3, std::move(arg4), arg5, std::move(arg6), std::move(arg7))}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6&& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), arg1, std::move(arg2), arg3, std::move(arg4), arg5, std::move(arg6), std::move(arg7))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6&& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, arg1, std::move(arg2), arg3, std::move(arg4), arg5, std::move(arg6), std::move(arg7))}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6&& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), std::move(arg1), arg2, arg3, std::move(arg4), arg5, std::move(arg6), std::move(arg7))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6&& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, std::move(arg1), arg2, arg3, std::move(arg4), arg5, std::move(arg6), std::move(arg7))}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6&& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), arg1, arg2, arg3, std::move(arg4), arg5, std::move(arg6), std::move(arg7))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6&& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, arg1, arg2, arg3, std::move(arg4), arg5, std::move(arg6), std::move(arg7))}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6&& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), std::move(arg1), std::move(arg2), std::move(arg3), arg4, arg5, std::move(arg6), std::move(arg7))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6&& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, std::move(arg1), std::move(arg2), std::move(arg3), arg4, arg5, std::move(arg6), std::move(arg7))}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6&& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), arg1, std::move(arg2), std::move(arg3), arg4, arg5, std::move(arg6), std::move(arg7))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6&& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, arg1, std::move(arg2), std::move(arg3), arg4, arg5, std::move(arg6), std::move(arg7))}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6&& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), std::move(arg1), arg2, std::move(arg3), arg4, arg5, std::move(arg6), std::move(arg7))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6&& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, std::move(arg1), arg2, std::move(arg3), arg4, arg5, std::move(arg6), std::move(arg7))}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6&& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), arg1, arg2, std::move(arg3), arg4, arg5, std::move(arg6), std::move(arg7))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6&& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, arg1, arg2, std::move(arg3), arg4, arg5, std::move(arg6), std::move(arg7))}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6&& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), std::move(arg1), std::move(arg2), arg3, arg4, arg5, std::move(arg6), std::move(arg7))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6&& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, std::move(arg1), std::move(arg2), arg3, arg4, arg5, std::move(arg6), std::move(arg7))}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6&& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), arg1, std::move(arg2), arg3, arg4, arg5, std::move(arg6), std::move(arg7))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6&& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, arg1, std::move(arg2), arg3, arg4, arg5, std::move(arg6), std::move(arg7))}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6&& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), std::move(arg1), arg2, arg3, arg4, arg5, std::move(arg6), std::move(arg7))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6&& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, std::move(arg1), arg2, arg3, arg4, arg5, std::move(arg6), std::move(arg7))}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6&& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), arg1, arg2, arg3, arg4, arg5, std::move(arg6), std::move(arg7))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6&& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, arg1, arg2, arg3, arg4, arg5, std::move(arg6), std::move(arg7))}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5, Arg6 const& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), std::move(arg1), std::move(arg2), std::move(arg3), std::move(arg4), std::move(arg5), arg6, std::move(arg7))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5, Arg6 const& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, std::move(arg1), std::move(arg2), std::move(arg3), std::move(arg4), std::move(arg5), arg6, std::move(arg7))}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5, Arg6 const& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), arg1, std::move(arg2), std::move(arg3), std::move(arg4), std::move(arg5), arg6, std::move(arg7))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5, Arg6 const& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, arg1, std::move(arg2), std::move(arg3), std::move(arg4), std::move(arg5), arg6, std::move(arg7))}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5, Arg6 const& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), std::move(arg1), arg2, std::move(arg3), std::move(arg4), std::move(arg5), arg6, std::move(arg7))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5, Arg6 const& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, std::move(arg1), arg2, std::move(arg3), std::move(arg4), std::move(arg5), arg6, std::move(arg7))}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5, Arg6 const& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), arg1, arg2, std::move(arg3), std::move(arg4), std::move(arg5), arg6, std::move(arg7))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5, Arg6 const& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, arg1, arg2, std::move(arg3), std::move(arg4), std::move(arg5), arg6, std::move(arg7))}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5&& arg5, Arg6 const& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), std::move(arg1), std::move(arg2), arg3, std::move(arg4), std::move(arg5), arg6, std::move(arg7))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5&& arg5, Arg6 const& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, std::move(arg1), std::move(arg2), arg3, std::move(arg4), std::move(arg5), arg6, std::move(arg7))}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5&& arg5, Arg6 const& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), arg1, std::move(arg2), arg3, std::move(arg4), std::move(arg5), arg6, std::move(arg7))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5&& arg5, Arg6 const& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, arg1, std::move(arg2), arg3, std::move(arg4), std::move(arg5), arg6, std::move(arg7))}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5&& arg5, Arg6 const& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), std::move(arg1), arg2, arg3, std::move(arg4), std::move(arg5), arg6, std::move(arg7))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5&& arg5, Arg6 const& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, std::move(arg1), arg2, arg3, std::move(arg4), std::move(arg5), arg6, std::move(arg7))}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5&& arg5, Arg6 const& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), arg1, arg2, arg3, std::move(arg4), std::move(arg5), arg6, std::move(arg7))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5&& arg5, Arg6 const& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, arg1, arg2, arg3, std::move(arg4), std::move(arg5), arg6, std::move(arg7))}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6 const& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), std::move(arg1), std::move(arg2), std::move(arg3), arg4, std::move(arg5), arg6, std::move(arg7))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6 const& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, std::move(arg1), std::move(arg2), std::move(arg3), arg4, std::move(arg5), arg6, std::move(arg7))}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6 const& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), arg1, std::move(arg2), std::move(arg3), arg4, std::move(arg5), arg6, std::move(arg7))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6 const& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, arg1, std::move(arg2), std::move(arg3), arg4, std::move(arg5), arg6, std::move(arg7))}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6 const& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), std::move(arg1), arg2, std::move(arg3), arg4, std::move(arg5), arg6, std::move(arg7))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6 const& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, std::move(arg1), arg2, std::move(arg3), arg4, std::move(arg5), arg6, std::move(arg7))}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6 const& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), arg1, arg2, std::move(arg3), arg4, std::move(arg5), arg6, std::move(arg7))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6 const& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, arg1, arg2, std::move(arg3), arg4, std::move(arg5), arg6, std::move(arg7))}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6 const& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), std::move(arg1), std::move(arg2), arg3, arg4, std::move(arg5), arg6, std::move(arg7))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6 const& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, std::move(arg1), std::move(arg2), arg3, arg4, std::move(arg5), arg6, std::move(arg7))}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6 const& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), arg1, std::move(arg2), arg3, arg4, std::move(arg5), arg6, std::move(arg7))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6 const& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, arg1, std::move(arg2), arg3, arg4, std::move(arg5), arg6, std::move(arg7))}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6 const& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), std::move(arg1), arg2, arg3, arg4, std::move(arg5), arg6, std::move(arg7))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6 const& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, std::move(arg1), arg2, arg3, arg4, std::move(arg5), arg6, std::move(arg7))}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6 const& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), arg1, arg2, arg3, arg4, std::move(arg5), arg6, std::move(arg7))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6 const& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, arg1, arg2, arg3, arg4, std::move(arg5), arg6, std::move(arg7))}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6 const& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), std::move(arg1), std::move(arg2), std::move(arg3), std::move(arg4), arg5, arg6, std::move(arg7))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6 const& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, std::move(arg1), std::move(arg2), std::move(arg3), std::move(arg4), arg5, arg6, std::move(arg7))}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6 const& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), arg1, std::move(arg2), std::move(arg3), std::move(arg4), arg5, arg6, std::move(arg7))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6 const& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, arg1, std::move(arg2), std::move(arg3), std::move(arg4), arg5, arg6, std::move(arg7))}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6 const& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), std::move(arg1), arg2, std::move(arg3), std::move(arg4), arg5, arg6, std::move(arg7))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6 const& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, std::move(arg1), arg2, std::move(arg3), std::move(arg4), arg5, arg6, std::move(arg7))}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6 const& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), arg1, arg2, std::move(arg3), std::move(arg4), arg5, arg6, std::move(arg7))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6 const& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, arg1, arg2, std::move(arg3), std::move(arg4), arg5, arg6, std::move(arg7))}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6 const& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), std::move(arg1), std::move(arg2), arg3, std::move(arg4), arg5, arg6, std::move(arg7))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6 const& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, std::move(arg1), std::move(arg2), arg3, std::move(arg4), arg5, arg6, std::move(arg7))}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6 const& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), arg1, std::move(arg2), arg3, std::move(arg4), arg5, arg6, std::move(arg7))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6 const& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, arg1, std::move(arg2), arg3, std::move(arg4), arg5, arg6, std::move(arg7))}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6 const& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), std::move(arg1), arg2, arg3, std::move(arg4), arg5, arg6, std::move(arg7))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6 const& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, std::move(arg1), arg2, arg3, std::move(arg4), arg5, arg6, std::move(arg7))}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6 const& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), arg1, arg2, arg3, std::move(arg4), arg5, arg6, std::move(arg7))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6 const& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, arg1, arg2, arg3, std::move(arg4), arg5, arg6, std::move(arg7))}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6 const& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), std::move(arg1), std::move(arg2), std::move(arg3), arg4, arg5, arg6, std::move(arg7))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6 const& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, std::move(arg1), std::move(arg2), std::move(arg3), arg4, arg5, arg6, std::move(arg7))}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6 const& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), arg1, std::move(arg2), std::move(arg3), arg4, arg5, arg6, std::move(arg7))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6 const& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, arg1, std::move(arg2), std::move(arg3), arg4, arg5, arg6, std::move(arg7))}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6 const& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), std::move(arg1), arg2, std::move(arg3), arg4, arg5, arg6, std::move(arg7))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6 const& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, std::move(arg1), arg2, std::move(arg3), arg4, arg5, arg6, std::move(arg7))}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6 const& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), arg1, arg2, std::move(arg3), arg4, arg5, arg6, std::move(arg7))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6 const& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, arg1, arg2, std::move(arg3), arg4, arg5, arg6, std::move(arg7))}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6 const& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), std::move(arg1), std::move(arg2), arg3, arg4, arg5, arg6, std::move(arg7))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6 const& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, std::move(arg1), std::move(arg2), arg3, arg4, arg5, arg6, std::move(arg7))}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6 const& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), arg1, std::move(arg2), arg3, arg4, arg5, arg6, std::move(arg7))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6 const& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, arg1, std::move(arg2), arg3, arg4, arg5, arg6, std::move(arg7))}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6 const& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), std::move(arg1), arg2, arg3, arg4, arg5, arg6, std::move(arg7))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6 const& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, std::move(arg1), arg2, arg3, arg4, arg5, arg6, std::move(arg7))}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6 const& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), arg1, arg2, arg3, arg4, arg5, arg6, std::move(arg7))}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6 const& arg6, Arg7&& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, arg1, arg2, arg3, arg4, arg5, arg6, std::move(arg7))}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5, Arg6&& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), std::move(arg1), std::move(arg2), std::move(arg3), std::move(arg4), std::move(arg5), std::move(arg6), arg7)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5, Arg6&& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, std::move(arg1), std::move(arg2), std::move(arg3), std::move(arg4), std::move(arg5), std::move(arg6), arg7)}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5, Arg6&& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), arg1, std::move(arg2), std::move(arg3), std::move(arg4), std::move(arg5), std::move(arg6), arg7)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5, Arg6&& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, arg1, std::move(arg2), std::move(arg3), std::move(arg4), std::move(arg5), std::move(arg6), arg7)}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5, Arg6&& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), std::move(arg1), arg2, std::move(arg3), std::move(arg4), std::move(arg5), std::move(arg6), arg7)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5, Arg6&& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, std::move(arg1), arg2, std::move(arg3), std::move(arg4), std::move(arg5), std::move(arg6), arg7)}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5, Arg6&& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), arg1, arg2, std::move(arg3), std::move(arg4), std::move(arg5), std::move(arg6), arg7)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5, Arg6&& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, arg1, arg2, std::move(arg3), std::move(arg4), std::move(arg5), std::move(arg6), arg7)}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5&& arg5, Arg6&& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), std::move(arg1), std::move(arg2), arg3, std::move(arg4), std::move(arg5), std::move(arg6), arg7)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5&& arg5, Arg6&& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, std::move(arg1), std::move(arg2), arg3, std::move(arg4), std::move(arg5), std::move(arg6), arg7)}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5&& arg5, Arg6&& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), arg1, std::move(arg2), arg3, std::move(arg4), std::move(arg5), std::move(arg6), arg7)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5&& arg5, Arg6&& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, arg1, std::move(arg2), arg3, std::move(arg4), std::move(arg5), std::move(arg6), arg7)}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5&& arg5, Arg6&& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), std::move(arg1), arg2, arg3, std::move(arg4), std::move(arg5), std::move(arg6), arg7)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5&& arg5, Arg6&& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, std::move(arg1), arg2, arg3, std::move(arg4), std::move(arg5), std::move(arg6), arg7)}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5&& arg5, Arg6&& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), arg1, arg2, arg3, std::move(arg4), std::move(arg5), std::move(arg6), arg7)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5&& arg5, Arg6&& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, arg1, arg2, arg3, std::move(arg4), std::move(arg5), std::move(arg6), arg7)}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6&& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), std::move(arg1), std::move(arg2), std::move(arg3), arg4, std::move(arg5), std::move(arg6), arg7)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6&& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, std::move(arg1), std::move(arg2), std::move(arg3), arg4, std::move(arg5), std::move(arg6), arg7)}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6&& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), arg1, std::move(arg2), std::move(arg3), arg4, std::move(arg5), std::move(arg6), arg7)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6&& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, arg1, std::move(arg2), std::move(arg3), arg4, std::move(arg5), std::move(arg6), arg7)}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6&& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), std::move(arg1), arg2, std::move(arg3), arg4, std::move(arg5), std::move(arg6), arg7)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6&& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, std::move(arg1), arg2, std::move(arg3), arg4, std::move(arg5), std::move(arg6), arg7)}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6&& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), arg1, arg2, std::move(arg3), arg4, std::move(arg5), std::move(arg6), arg7)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6&& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, arg1, arg2, std::move(arg3), arg4, std::move(arg5), std::move(arg6), arg7)}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6&& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), std::move(arg1), std::move(arg2), arg3, arg4, std::move(arg5), std::move(arg6), arg7)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6&& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, std::move(arg1), std::move(arg2), arg3, arg4, std::move(arg5), std::move(arg6), arg7)}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6&& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), arg1, std::move(arg2), arg3, arg4, std::move(arg5), std::move(arg6), arg7)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6&& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, arg1, std::move(arg2), arg3, arg4, std::move(arg5), std::move(arg6), arg7)}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6&& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), std::move(arg1), arg2, arg3, arg4, std::move(arg5), std::move(arg6), arg7)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6&& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, std::move(arg1), arg2, arg3, arg4, std::move(arg5), std::move(arg6), arg7)}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6&& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), arg1, arg2, arg3, arg4, std::move(arg5), std::move(arg6), arg7)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6&& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, arg1, arg2, arg3, arg4, std::move(arg5), std::move(arg6), arg7)}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6&& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), std::move(arg1), std::move(arg2), std::move(arg3), std::move(arg4), arg5, std::move(arg6), arg7)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6&& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, std::move(arg1), std::move(arg2), std::move(arg3), std::move(arg4), arg5, std::move(arg6), arg7)}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6&& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), arg1, std::move(arg2), std::move(arg3), std::move(arg4), arg5, std::move(arg6), arg7)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6&& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, arg1, std::move(arg2), std::move(arg3), std::move(arg4), arg5, std::move(arg6), arg7)}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6&& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), std::move(arg1), arg2, std::move(arg3), std::move(arg4), arg5, std::move(arg6), arg7)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6&& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, std::move(arg1), arg2, std::move(arg3), std::move(arg4), arg5, std::move(arg6), arg7)}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6&& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), arg1, arg2, std::move(arg3), std::move(arg4), arg5, std::move(arg6), arg7)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6&& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, arg1, arg2, std::move(arg3), std::move(arg4), arg5, std::move(arg6), arg7)}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6&& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), std::move(arg1), std::move(arg2), arg3, std::move(arg4), arg5, std::move(arg6), arg7)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6&& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, std::move(arg1), std::move(arg2), arg3, std::move(arg4), arg5, std::move(arg6), arg7)}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6&& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), arg1, std::move(arg2), arg3, std::move(arg4), arg5, std::move(arg6), arg7)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6&& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, arg1, std::move(arg2), arg3, std::move(arg4), arg5, std::move(arg6), arg7)}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6&& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), std::move(arg1), arg2, arg3, std::move(arg4), arg5, std::move(arg6), arg7)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6&& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, std::move(arg1), arg2, arg3, std::move(arg4), arg5, std::move(arg6), arg7)}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6&& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), arg1, arg2, arg3, std::move(arg4), arg5, std::move(arg6), arg7)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6&& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, arg1, arg2, arg3, std::move(arg4), arg5, std::move(arg6), arg7)}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6&& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), std::move(arg1), std::move(arg2), std::move(arg3), arg4, arg5, std::move(arg6), arg7)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6&& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, std::move(arg1), std::move(arg2), std::move(arg3), arg4, arg5, std::move(arg6), arg7)}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6&& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), arg1, std::move(arg2), std::move(arg3), arg4, arg5, std::move(arg6), arg7)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6&& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, arg1, std::move(arg2), std::move(arg3), arg4, arg5, std::move(arg6), arg7)}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6&& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), std::move(arg1), arg2, std::move(arg3), arg4, arg5, std::move(arg6), arg7)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6&& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, std::move(arg1), arg2, std::move(arg3), arg4, arg5, std::move(arg6), arg7)}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6&& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), arg1, arg2, std::move(arg3), arg4, arg5, std::move(arg6), arg7)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6&& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, arg1, arg2, std::move(arg3), arg4, arg5, std::move(arg6), arg7)}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6&& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), std::move(arg1), std::move(arg2), arg3, arg4, arg5, std::move(arg6), arg7)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6&& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, std::move(arg1), std::move(arg2), arg3, arg4, arg5, std::move(arg6), arg7)}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6&& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), arg1, std::move(arg2), arg3, arg4, arg5, std::move(arg6), arg7)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6&& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, arg1, std::move(arg2), arg3, arg4, arg5, std::move(arg6), arg7)}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6&& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), std::move(arg1), arg2, arg3, arg4, arg5, std::move(arg6), arg7)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6&& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, std::move(arg1), arg2, arg3, arg4, arg5, std::move(arg6), arg7)}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6&& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), arg1, arg2, arg3, arg4, arg5, std::move(arg6), arg7)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6&& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, arg1, arg2, arg3, arg4, arg5, std::move(arg6), arg7)}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5, Arg6 const& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), std::move(arg1), std::move(arg2), std::move(arg3), std::move(arg4), std::move(arg5), arg6, arg7)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5, Arg6 const& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, std::move(arg1), std::move(arg2), std::move(arg3), std::move(arg4), std::move(arg5), arg6, arg7)}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5, Arg6 const& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), arg1, std::move(arg2), std::move(arg3), std::move(arg4), std::move(arg5), arg6, arg7)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5, Arg6 const& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, arg1, std::move(arg2), std::move(arg3), std::move(arg4), std::move(arg5), arg6, arg7)}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5, Arg6 const& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), std::move(arg1), arg2, std::move(arg3), std::move(arg4), std::move(arg5), arg6, arg7)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5, Arg6 const& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, std::move(arg1), arg2, std::move(arg3), std::move(arg4), std::move(arg5), arg6, arg7)}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5, Arg6 const& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), arg1, arg2, std::move(arg3), std::move(arg4), std::move(arg5), arg6, arg7)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5, Arg6 const& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, arg1, arg2, std::move(arg3), std::move(arg4), std::move(arg5), arg6, arg7)}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5&& arg5, Arg6 const& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), std::move(arg1), std::move(arg2), arg3, std::move(arg4), std::move(arg5), arg6, arg7)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5&& arg5, Arg6 const& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, std::move(arg1), std::move(arg2), arg3, std::move(arg4), std::move(arg5), arg6, arg7)}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5&& arg5, Arg6 const& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), arg1, std::move(arg2), arg3, std::move(arg4), std::move(arg5), arg6, arg7)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5&& arg5, Arg6 const& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, arg1, std::move(arg2), arg3, std::move(arg4), std::move(arg5), arg6, arg7)}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5&& arg5, Arg6 const& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), std::move(arg1), arg2, arg3, std::move(arg4), std::move(arg5), arg6, arg7)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5&& arg5, Arg6 const& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, std::move(arg1), arg2, arg3, std::move(arg4), std::move(arg5), arg6, arg7)}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5&& arg5, Arg6 const& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), arg1, arg2, arg3, std::move(arg4), std::move(arg5), arg6, arg7)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5&& arg5, Arg6 const& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, arg1, arg2, arg3, std::move(arg4), std::move(arg5), arg6, arg7)}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6 const& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), std::move(arg1), std::move(arg2), std::move(arg3), arg4, std::move(arg5), arg6, arg7)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6 const& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, std::move(arg1), std::move(arg2), std::move(arg3), arg4, std::move(arg5), arg6, arg7)}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6 const& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), arg1, std::move(arg2), std::move(arg3), arg4, std::move(arg5), arg6, arg7)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6 const& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, arg1, std::move(arg2), std::move(arg3), arg4, std::move(arg5), arg6, arg7)}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6 const& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), std::move(arg1), arg2, std::move(arg3), arg4, std::move(arg5), arg6, arg7)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6 const& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, std::move(arg1), arg2, std::move(arg3), arg4, std::move(arg5), arg6, arg7)}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6 const& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), arg1, arg2, std::move(arg3), arg4, std::move(arg5), arg6, arg7)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6 const& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, arg1, arg2, std::move(arg3), arg4, std::move(arg5), arg6, arg7)}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6 const& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), std::move(arg1), std::move(arg2), arg3, arg4, std::move(arg5), arg6, arg7)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6 const& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, std::move(arg1), std::move(arg2), arg3, arg4, std::move(arg5), arg6, arg7)}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6 const& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), arg1, std::move(arg2), arg3, arg4, std::move(arg5), arg6, arg7)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6 const& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, arg1, std::move(arg2), arg3, arg4, std::move(arg5), arg6, arg7)}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6 const& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), std::move(arg1), arg2, arg3, arg4, std::move(arg5), arg6, arg7)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6 const& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, std::move(arg1), arg2, arg3, arg4, std::move(arg5), arg6, arg7)}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6 const& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), arg1, arg2, arg3, arg4, std::move(arg5), arg6, arg7)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5&& arg5, Arg6 const& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, arg1, arg2, arg3, arg4, std::move(arg5), arg6, arg7)}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6 const& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), std::move(arg1), std::move(arg2), std::move(arg3), std::move(arg4), arg5, arg6, arg7)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6 const& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, std::move(arg1), std::move(arg2), std::move(arg3), std::move(arg4), arg5, arg6, arg7)}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6 const& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), arg1, std::move(arg2), std::move(arg3), std::move(arg4), arg5, arg6, arg7)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6 const& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, arg1, std::move(arg2), std::move(arg3), std::move(arg4), arg5, arg6, arg7)}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6 const& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), std::move(arg1), arg2, std::move(arg3), std::move(arg4), arg5, arg6, arg7)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6 const& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, std::move(arg1), arg2, std::move(arg3), std::move(arg4), arg5, arg6, arg7)}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6 const& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), arg1, arg2, std::move(arg3), std::move(arg4), arg5, arg6, arg7)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6 const& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, arg1, arg2, std::move(arg3), std::move(arg4), arg5, arg6, arg7)}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6 const& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), std::move(arg1), std::move(arg2), arg3, std::move(arg4), arg5, arg6, arg7)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6 const& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, std::move(arg1), std::move(arg2), arg3, std::move(arg4), arg5, arg6, arg7)}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6 const& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), arg1, std::move(arg2), arg3, std::move(arg4), arg5, arg6, arg7)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6 const& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, arg1, std::move(arg2), arg3, std::move(arg4), arg5, arg6, arg7)}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6 const& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), std::move(arg1), arg2, arg3, std::move(arg4), arg5, arg6, arg7)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6 const& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, std::move(arg1), arg2, arg3, std::move(arg4), arg5, arg6, arg7)}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6 const& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), arg1, arg2, arg3, std::move(arg4), arg5, arg6, arg7)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4&& arg4, Arg5 const& arg5, Arg6 const& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, arg1, arg2, arg3, std::move(arg4), arg5, arg6, arg7)}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6 const& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), std::move(arg1), std::move(arg2), std::move(arg3), arg4, arg5, arg6, arg7)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6 const& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, std::move(arg1), std::move(arg2), std::move(arg3), arg4, arg5, arg6, arg7)}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6 const& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), arg1, std::move(arg2), std::move(arg3), arg4, arg5, arg6, arg7)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6 const& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, arg1, std::move(arg2), std::move(arg3), arg4, arg5, arg6, arg7)}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6 const& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), std::move(arg1), arg2, std::move(arg3), arg4, arg5, arg6, arg7)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6 const& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, std::move(arg1), arg2, std::move(arg3), arg4, arg5, arg6, arg7)}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6 const& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), arg1, arg2, std::move(arg3), arg4, arg5, arg6, arg7)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3&& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6 const& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, arg1, arg2, std::move(arg3), arg4, arg5, arg6, arg7)}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6 const& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), std::move(arg1), std::move(arg2), arg3, arg4, arg5, arg6, arg7)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6 const& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, std::move(arg1), std::move(arg2), arg3, arg4, arg5, arg6, arg7)}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6 const& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), arg1, std::move(arg2), arg3, arg4, arg5, arg6, arg7)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2&& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6 const& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, arg1, std::move(arg2), arg3, arg4, arg5, arg6, arg7)}) };
				}
				auto operator()(Arg0&& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6 const& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), std::move(arg1), arg2, arg3, arg4, arg5, arg6, arg7)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1&& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6 const& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, std::move(arg1), arg2, arg3, arg4, arg5, arg6, arg7)}) };
				}
				auto operator()(Arg0&& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6 const& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(std::move(arg0), arg1, arg2, arg3, arg4, arg5, arg6, arg7)}) };
				}
				auto operator()(Arg0 const& arg0, Arg1 const& arg1, Arg2 const& arg2, Arg3 const& arg3, Arg4 const& arg4, Arg5 const& arg5, Arg6 const& arg6, Arg7 const& arg7)const
				{
					return data_type_base{ std::make_shared<inside_type>(id_type<ID,detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>> {detail::inside_tuple<Type0, Type1, Type2, Type3, Type4, Type5, Type6, Type7>(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7)}) };
				}
			};

			template<std::size_t ID>static instance_type<ID, typename tuple_element<ID, tuple<Ts...>>::type> get_instance()
			{
				return instance_type<ID, typename tuple_element<ID, tuple<Ts...>>::type>();
			}
		};
	}

	template<class DataType, class... Ts>struct data_type :detail::data_type_base<DataType, std::make_index_sequence<sizeof...(Ts)>, Ts...>
	{
		typedef detail::data_type_base<DataType, std::make_index_sequence<sizeof...(Ts)>, Ts...> base;
		template<class T>data_type(T&& val) :base(std::forward<T>(val))
		{

		}
	};
}

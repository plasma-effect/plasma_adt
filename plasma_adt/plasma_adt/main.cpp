#include<iostream>
#include<string>
#include<plasma_adt/light_adt.hpp>

using namespace plasma_adt;
struct data :data_type<data,
	int,
	tuple<int, data>>
{
	template<class T>data(T&& arg) :data_type(std::forward<T>(arg))
	{

	}
};
auto const atomic = data::get_instance<0>();
auto const list = data::get_instance<1>();
typedef decltype(std::declval<data>().get(atomic)->get()) atomic_t;
typedef decltype(std::declval<data>().get(list)->get()) list_t;

int sum(data const& v);

int sumi(atomic_t const& v)
{
	return v;
}

int sumi(list_t const& v)
{
	return v.get<0>() + sum(v.get<1>());
}

int sum(data const& v)
{
	return visit([](auto const& v) {return sumi(v); }, v);
}

int main()
{
	std::cout << sum(list(1, list(2, atomic(3)))) << std::endl;
}

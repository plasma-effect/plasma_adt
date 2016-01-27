#include<iostream>
#include<string>
#include<list>

#include<plasma_adt/generic_data_type.hpp>

using namespace generic_adt;
using namespace place_holder;

struct list : generic_data_type<list, void, tuple<generic_tag, list>>{};

const auto Nil = list::instance_function<0>();
const auto Tree = list::instance_function<1>();

const auto StdList = pattern_match::generic_recursion<std::list<generic_tag>,list>()
| Nil <= [](auto, auto t) {return std::list<typename decltype(t)::type>{};}
| Tree(0_, 1_) <= [](auto recur, auto, auto v, auto next) {auto lis = recur(next);lis.push_front(v);return lis;};

int main()
{

	const auto IntTree = Tree(type_tag<int>{});
	const auto IntNil = Nil(type_tag<int>{});

	auto v = IntTree(1, IntTree(2, IntNil()));
	
	auto x = StdList(v);
	
	for (auto i : x)
	{
		std::cout << i << std::endl;
	}
}
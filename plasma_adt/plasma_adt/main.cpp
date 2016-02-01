#include<iostream>
#include<string>
#include<plasma_adt/generic_data_type.hpp>

using namespace generic_adt;
using namespace place_holder;

struct list :generic_data_type<list, void, tuple<generic_tag, list>>{};

const auto Nil = list::instance_function<0>();
const auto Tree = list::instance_function<1>();

const auto Sum = pattern_match::generic_recursion<generic_tag, list, generic_tag>()
| Nil <= [](auto, auto t, auto v) {return v;}
| Tree(0_, 1_) <= [](auto recur, auto, auto v, auto next, auto u) {return v + recur(next, u);};

const auto nil = Nil(type_tag<void>{})();

int main()
{
	const auto int_tree = Tree(type_tag<int>{});
	const auto string_tree = Tree(type_tag<std::string>{});

	auto v = int_tree(1, int_tree(2, int_tree(3, nil)));
	auto u = string_tree("this ", string_tree("is ", string_tree("test", nil)));

	std::cout << Sum(v, 1) << std::endl;
	std::cout << Sum(u, "hage") << std::endl;
}

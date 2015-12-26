#include<plasma_adt/algebraic_data_type.hpp>
#include<iostream>
#include<boost/optional/optional_io.hpp>

using namespace plasma_adt;
using namespace plasma_adt::place_holder;
using namespace plasma_adt::pattern_match;

struct list :data_type_base<list, void, tuple<int, list>>
{
	list(data_type_base const& v) :data_type_base(v) {}
};
const auto Nil = list::instance_function<0>();
const auto Tree = list::instance_function<1>();


int main()
{
	auto x = Nil();
	auto y = Tree(1, Nil());
	auto z = Tree(2, Tree(1, Nil()));

	auto f =
		recursion_match<int, list>()
		| Tree(0_, 1_) <= [](auto rec, int x, list next) {std::cout << "Tree:" << x << std::endl; return x + rec(next);}
		| Nil <= [](auto) {std::cout << "Nil" << std::endl; return 0;};
	
	std::cout << f(x) << std::endl << std::endl;
	std::cout << f(y) << std::endl << std::endl;
	std::cout << f(z) << std::endl << std::endl;

	auto g=
		pattern_match::pattern_match<bool,list>()
		| Tree <= []() {return true;}
		| Nil <= []() {return false;};
	std::cout << g(x) << std::endl;
	std::cout << g(y) << std::endl;
	std::cout << g(z) << std::endl;
}
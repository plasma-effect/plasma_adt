#include<plasma_adt/algebraic_data_type.hpp>
#include<iostream>

using namespace plasma_adt;
using namespace plasma_adt::place_holder;
using namespace plasma_adt::pattern_match;

struct test :data_type_base<test, int, std::vector<test>>
{
	test(data_type_base const& v) :data_type_base(v) {}
};
auto const atomic = test::instance_function<0>();
auto const vector = test::instance_function<1>();

int main()
{
	auto x = atomic(1);
	auto y = atomic(2);
	auto z = atomic(3);
	auto s = vector(std::vector<test>{x, y, z});

	auto const func = 
		recursion_match<int,test>()
		| atomic(0_) <= [](auto, int x) {return x;}
		| vector(0_) <= [](auto f, std::vector<test>const& x) 
			{
				int ret{};
				for (auto v : x) 
				{
					ret += f(v);
				}
				return ret;
			};
	std::cout << func(s) << std::endl;
}
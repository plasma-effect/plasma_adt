#include<iostream>
#include<vector>

#include<plasma_adt/generic_data_type.hpp>

using namespace generic_adt;

int main()
{
	std::cout << 
		typeid(detail::trans_type<std::vector<recursion_tag>, recursion_tag, int>::type).name() 
		<< std::endl;
}
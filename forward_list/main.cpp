#include "forward_list.hpp"
#include <iostream>
#include<algorithm>
#include <type_traits>

int main(int argc, char* argv[]) {
	brian::forward_list<int> fl {1,3};
	fl.insert_after(++fl.begin(), 2);
	fl.insert_after(fl.before_begin(), 0);
	for (auto const& each : fl) {
		std::cout << each << '\n';
	}
}

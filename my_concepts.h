#include <type_traits>
#include <iterator>
namespace brian {
	template <typename U>
	concept __less_than_comparable = requires(U a, U b) {
		{ a < b } -> std::convertible_to<bool>;
	};
	template <typename It>
	concept at_least_random_access_iterator = std::is_same<typename It::iterator_category, std::random_access_iterator_tag>::value || 
		std::is_same<typename It::iterator_category,std::contiguous_iterator_tag>::value;
	template <typename It>
	concept at_most_bidirectional_iterator = !at_least_random_access_iterator<It>;
}


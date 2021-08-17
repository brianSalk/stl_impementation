#include <type_traits>
namespace brian {
	template <typename U>
	concept __less_than_comparable = requires(U a, U b) {
		{ a < b } -> std::convertible_to<bool>;
	};
}

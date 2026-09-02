#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <doctest/doctest.h>
#include <dice/hypertrie.hpp>

#define DICE_TEMPLATE_LIBRARY_WITH_BOOST 1
#include <dice/template-library/polymorphic_allocator.hpp>

using namespace dice::hypertrie;

using htt_t = default_bool_Hypertrie_trait;

TEST_SUITE("Polymorphic Allocator") {
	TEST_CASE("default alloc") {
		using allocator_type = dice::template_library::polymorphic_allocator<std::byte, std::allocator>;

		HypertrieContext<htt_t, allocator_type> context{allocator_type{}};
		Hypertrie<htt_t, allocator_type> hyp{3, &context};

		hyp.set({{1, 2, 3}}, true);
		CHECK(hyp[SliceKey<htt_t>{{1, 2, 3}}].to_scalar() == true);
		CHECK(hyp[Key<htt_t>{{1, 2, 3}}] == true);
		CHECK(hyp[Key<htt_t>{{0, 0, 0}}] == false);
	}

	TEST_CASE("default alloc with offset ptr") {
		using allocator_type = dice::template_library::polymorphic_allocator<std::byte, dice::template_library::offset_ptr_stl_allocator>;

		HypertrieContext<htt_t, allocator_type> context{allocator_type{}};
		Hypertrie<htt_t, allocator_type> hyp{3, &context};

		hyp.set({{1, 2, 3}}, true);
		CHECK(hyp[SliceKey<htt_t>{{1, 2, 3}}].to_scalar() == true);
		CHECK(hyp[Key<htt_t>{{1, 2, 3}}] == true);
		CHECK(hyp[Key<htt_t>{{0, 0, 0}}] == false);
	}
}

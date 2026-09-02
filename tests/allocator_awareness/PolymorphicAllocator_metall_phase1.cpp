#include "PolymorphicAllocator_metall_common.hpp"

int main(int argc, char **argv) {

	{ // try normal alloc first
		HypertrieContext<htt_t, allocator_type> context{allocator_type{dice::template_library::offset_ptr_stl_allocator<std::byte>{}}};
		Hypertrie<htt_t, allocator_type> hyp{3, &context};

		hyp.set({{1, 2, 3}}, true);

		auto x = hyp[SliceKey<htt_t>{{1, 2, 3}}];
		assert(x.to_scalar() == true);
		auto y = hyp[Key<htt_t>{{1, 2, 3}}];
		assert(y == true);
		auto z = hyp[Key<htt_t>{{0, 0, 0}}];
		assert(z == false);
	}

	assert(argc >= 2);

	auto const *path = argv[1];

	{ //create segment
		metall::manager manager(metall::create_only, path);
	}

	{ // read and write stuff into segement
		metall::manager manager(metall::open_only, path);

		auto ctx_ptr = manager.construct<HypertrieContext<htt_t, allocator_type>>(context_name)(allocator_type{manager.get_allocator()});
		auto ht_ptr = manager.construct<Hypertrie<htt_t, allocator_type>>(hypertrie_name)(3, ctx_ptr);

		auto &hyp = *ht_ptr;

		hyp.set({{1, 2, 3}}, true);

		auto x = hyp[SliceKey<htt_t>{{1, 2, 3}}];
		assert(x.to_scalar() == true);
		auto y = hyp[Key<htt_t>{{1, 2, 3}}];
		assert(y == true);
		auto z = hyp[Key<htt_t>{{0, 0, 0}}];
		assert(z == false);

		dice::hypertrie::tests::core::node::dump_context(ctx_ptr->raw_context(), "before");
	}
}

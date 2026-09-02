#include "PolymorphicAllocator_metall_common.hpp"

int main(int argc, char **argv) {
	assert(argc >= 2);

	auto const *path = argv[1];

	{ // reopen and read the segment
		metall::manager manager(metall::open_only, path);
		auto ctx_ptr = std::get<0>(manager.find<HypertrieContext<htt_t, allocator_type>>(context_name));
		auto ht_ptr = std::get<0>(manager.find<Hypertrie<htt_t, allocator_type>>(hypertrie_name));

		auto &hyp = *ht_ptr;

		auto x = hyp[SliceKey<htt_t>{{1, 2, 3}}];
		assert(x.to_scalar() == true);
		auto y = hyp[Key<htt_t>{{1, 2, 3}}];
		assert(y == true);
		auto z = hyp[Key<htt_t>{{0, 0, 0}}];
		assert(z == false);

		dice::hypertrie::tests::core::node::dump_context(ctx_ptr->raw_context(), "after");

		hyp.set({{2, 4, 5}}, true);

		auto a = hyp[Key<htt_t>{{2, 4, 5}}];
		assert(a == true);
		fmt::print("end\n");
	}

	metall::manager::remove(path);
}

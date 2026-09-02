#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <metall/metall.hpp>
#include <boost/interprocess/offset_ptr.hpp>

#include <dice/hypertrie/internal/raw/node/NodePtr.hpp>
#include <dice/hypertrie/internal/raw/node/FullNode.hpp>
#include <dice/hypertrie/internal/raw/node/SingleEntryNode.hpp>
#include <dice/hypertrie/internal/raw/node/CartesianNode.hpp>
#include <dice/hypertrie/Hypertrie_default_traits.hpp>
#include <dice/hypertrie/HypertrieContextConfig.hpp>
#include <dice/hypertrie/internal/util/Overloaded.hpp>
#include <dice/template-library/for.hpp>

#include <random>

TEST_SUITE("NodePtr") {
	using namespace dice::hypertrie;
	using namespace internal::raw;
	using namespace internal::util;

	template<typename T>
	using metall_allocator = metall::manager::allocator_type<std::byte>;

	template<typename T>
	using metall_pointer = metall_allocator<T>::pointer;

	TEST_CASE("alignment requirements") {
		SUBCASE("std::allocator") {
			using htt_t = tagged_bool_Hypertrie_trait;
			using allocator_type = std::allocator<std::byte>;

			static constexpr size_t required_bits = 2; // based on IdentifierTag
			static constexpr size_t align_mod = 1 << required_bits;

			// require:   node_align % align_mod == 0
			//
			// because:   node_align       % align_mod == 0
			//         => (node_align * n) % align_mod == 0
			//                ||
			//        <=> node_addr        % align_mod == 0
			//
			// and therefore:  pointer's required bits least-significant bits are unused for all pointers

			dice::template_library::for_range<1, hypertrie_max_depth + 1>([](auto depth_arg) {
				static_assert(alignof(FullNode<depth_arg, htt_t, allocator_type>) % align_mod == 0);
				static_assert(alignof(SingleEntryNode<depth_arg, htt_t>) % align_mod == 0);
				static_assert(alignof(CartesianNode<depth_arg, htt_t, allocator_type>) % align_mod == 0);
			});
		}

		SUBCASE("metall") {
			using allocator_type = metall_allocator<std::byte>;

			// offset pointer addressing: addr = offset_ptr_addr + offset

			// require:   node_align % align_mod == 0 && offset_ptr_align % align_mod == 0
			//
			// because:   node_align % align_mode == 0 && offset_ptr_align % align_mod == 0
			//         => ((node_align * n) % align_mod - (offset_ptr_align * m) % align_mod) % align_mod == 0
			//         => ((node_align * n)             - (offset_ptr_align * m))             % align_mod == 0  (using simplification rule for mod)
			//                 ||                               ||
			//        <=> (node_addr                    - offset_ptr_addr)                    % align_mod == 0
			//        <=> offset                                                              % align_mod == 0
			//
			// and therefore:  offset's required_bits least-significant bits are unused for all offsets

			SUBCASE("taggable key part") {
				using htt_t = tagged_bool_Hypertrie_trait;

				static constexpr size_t required_bits = 3; // based on IdentifierTag + required inline tagging bit
				static constexpr size_t align_mod = 1 << required_bits;

				dice::template_library::for_range<1, hypertrie_max_depth + 1>([](auto depth_arg) {
					static_assert(alignof(FullNode<depth_arg, htt_t, allocator_type>) % align_mod == 0);
					static_assert(alignof(metall_pointer<FullNode<depth_arg, htt_t, allocator_type>>) % align_mod == 0);

					static_assert(alignof(SingleEntryNode<depth_arg, htt_t>) % align_mod == 0);
					static_assert(alignof(metall_pointer<SingleEntryNode<depth_arg, htt_t>>) % align_mod == 0);

					static_assert(alignof(CartesianNode<depth_arg, htt_t, allocator_type>) % align_mod == 0);
					static_assert(alignof(metall_pointer<CartesianNode<depth_arg, htt_t, allocator_type>>) % align_mod == 0);
				});
			}

			SUBCASE("non-taggable key part") {
				using htt_t = default_bool_Hypertrie_trait;

				static constexpr size_t required_bits = 2; // based on IdentifierTag
				static constexpr size_t align_mod = 1 << required_bits;

				dice::template_library::for_range<1, hypertrie_max_depth + 1>([](auto depth_arg) {
					static_assert(alignof(FullNode<depth_arg, htt_t, allocator_type>) % align_mod == 0);
					static_assert(alignof(metall_pointer<FullNode<depth_arg, htt_t, allocator_type>>) % align_mod == 0);

					static_assert(alignof(SingleEntryNode<depth_arg, htt_t>) % align_mod == 0);
					static_assert(alignof(metall_pointer<SingleEntryNode<depth_arg, htt_t>>) % align_mod == 0);

					static_assert(alignof(CartesianNode<depth_arg, htt_t, allocator_type>) % align_mod == 0);
					static_assert(alignof(metall_pointer<CartesianNode<depth_arg, htt_t, allocator_type>>) % align_mod == 0);
				});
			}
		}
	}

	TEST_CASE("simple instantiation") {
		using htt_t = tagged_bool_Hypertrie_trait;
		using allocator_type = std::allocator<std::byte>;
		auto *fn = new FullNode<2, htt_t, allocator_type>{RawIdentifier<2, htt_t>{}.retag_as_fn(), 0, allocator_type{}};
		NodePtr<2, htt_t, allocator_type> fn_ptr{fn};
		CHECK(fn_ptr.tag() == IdentifierTag::FN);

		fn_ptr.visit_ptr(Overloaded{
				[](FNPtr<2, htt_t, allocator_type> fn_ptr) {
					CHECK(fn_ptr->size() == 0);
				},
				[](auto) {
					assert(false);
				}});

		delete fn;
	}

	TEST_CASE("offset ptr") {
		using allocator_type = metall::manager::allocator_type<std::byte>;

		SUBCASE("taggable key_part") {
			using htt_t = tagged_bool_Hypertrie_trait;

			std::string const path = "/tmp/" + std::to_string(std::random_device{}());

			{
				metall::manager mng{metall::create_only, path.c_str()};
			}

			{
				metall::manager mng{metall::open_only, path.c_str()};

				SUBCASE("encode pointer") {
					auto *fn_raw = mng.find_or_construct<FullNode<2, htt_t, allocator_type>>("fn_1")(RawIdentifier<2, htt_t>{}.retag_as_fn(), 42, mng.get_allocator());

					boost::interprocess::offset_ptr<FullNode<2, htt_t, allocator_type>> fn_ptr{fn_raw};
					CHECK(fn_raw == fn_ptr.get());

					NodePtr<2, htt_t, allocator_type> node_ptr{fn_ptr};
					CHECK(fn_ptr.get() == node_ptr.ptr().get());

					auto node_ptr2 = node_ptr;
					CHECK(node_ptr.ptr().get() == node_ptr2.ptr().get());

					node_ptr = node_ptr2;
					CHECK(node_ptr.ptr().get() == node_ptr2.ptr().get());

					CHECK(node_ptr.specific_ptr<FullNode>()->ref_count() == 42);
				}

				SUBCASE("encode key_part") {
					typename htt_t::key_part_type key_part = 99;
					auto xn_prefix_operand = NodePtr<1, htt_t, allocator_type>::encode_key_part(key_part);
					CHECK(xn_prefix_operand.tag() == IdentifierTag::SEN);
					CHECK(xn_prefix_operand.decode_key_part() == 99);

					auto cpy = xn_prefix_operand;
					CHECK(cpy.tag() == IdentifierTag::SEN);
					CHECK(cpy.decode_key_part() == 99);
				}
			}

			metall::manager::remove(path.c_str());
		}

		SUBCASE("non-taggable key part") {
			using htt_t = default_bool_Hypertrie_trait;

			std::string const path = "/tmp/" + std::to_string(std::random_device{}());

			{
				metall::manager mng{metall::create_only, path.c_str()};
			}

			{
				metall::manager mng{metall::open_only, path.c_str()};

				SUBCASE("encode pointer") {
					auto *fn_raw = mng.find_or_construct<FullNode<2, htt_t, allocator_type>>("fn_1")(RawIdentifier<2, htt_t>{}.retag_as_fn(), 42, mng.get_allocator());

					boost::interprocess::offset_ptr<FullNode<2, htt_t, allocator_type>> fn_ptr{fn_raw};
					CHECK(fn_raw == fn_ptr.get());

					NodePtr<2, htt_t, allocator_type> node_ptr{fn_ptr};
					CHECK(fn_ptr.get() == node_ptr.ptr().get());

					auto node_ptr2 = node_ptr;
					CHECK(node_ptr.ptr().get() == node_ptr2.ptr().get());

					node_ptr = node_ptr2;
					CHECK(node_ptr.ptr().get() == node_ptr2.ptr().get());

					CHECK(node_ptr.specific_ptr<FullNode>()->ref_count() == 42);
				}
			}

			metall::manager::remove(path.c_str());
		}
	}
}

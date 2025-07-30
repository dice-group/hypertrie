#ifndef HYPERTRIE_DUMP_RAW_CONTEXT_HPP
#define HYPERTRIE_DUMP_RAW_CONTEXT_HPP

#include <dice/hypertrie/internal/raw/node/FullNode.hpp>
#include <dice/hypertrie/internal/raw/node/NodeContainer.hpp>
#include <dice/hypertrie/internal/raw/node/SingleEntryNode.hpp>
#include <dice/hypertrie/internal/raw/node_context/RawHypertrieContext.hpp>

#include <fmt/format.h>

#include <iostream>

namespace dice::hypertrie::tests::core::node {

	using namespace ::dice::hypertrie::internal::raw;
	using namespace ::dice::hypertrie::internal;

	template<size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
	static void dump_full_node(RawIdentifier<depth, htt_t> const ident,
	typename FNContainer<depth, htt_t, allocator_type>::NodePtr const node) {
		std::cout << fmt::format("{} (rc = {}, size = {}): {{\n", ident, node->ref_count(), node->size());

		if constexpr (depth > 1) {
			for (size_t pos = 0; pos < depth; ++pos) {
				std::cout << fmt::format("    (.{}): {{\n", pos);

				for (auto const &[keypart, child] : node->edges(pos)) {
					if constexpr (depth - 1 == 1 && HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
						if (child.is_sen()) {
							std::cout << fmt::format("        .{} = inplace {}\n", keypart, child.get_entry().key()[0]);
							continue;
						}
					}

					std::cout << fmt::format("        .{} = {}\n", keypart, child);
				}

				std::cout << "    }\n";
			}
		} else {
			std::cout << fmt::format("    (.0): {{\n");

			for (auto const &child : node->edges()) {
				if constexpr (htt_t::is_bool_valued) {
					std::cout << fmt::format("        .{} = true\n", child);
				} else {
					std::cout << fmt::format("        .{} = {}\n", child.first, child.second);
				}
			}

			std::cout << "    }\n";
		}

		std::cout << "}\n";
	}

	/**
	 * Prints an easily human readable representation of a single depth of a RawHypertrieContext to stdout
	 * for debugging purposes.
	 */
	template<size_t depth, size_t max_depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
	static void dump_context_level(RawHypertrieContext<max_depth, htt_t, allocator_type> const &context) {
		auto const &fns = context.node_storage_.template nodes<depth, FullNode>().nodes();
		for (auto const &[ident, node] : fns) {
			dump_full_node<depth, htt_t, allocator_type>(ident, node);
		}

		if constexpr (depth > 1 || !HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
			auto const &sens = context.node_storage_.template nodes<depth, SingleEntryNode>().nodes();
			for (auto const &[ident, node] : sens) {

				if constexpr (htt_t::is_bool_valued) {
					std::cout << fmt::format("{} (rc = {}, size = {}): ({}) = true\n",
											 ident,
											 node->ref_count(),
											 node->size(),
											 fmt::join(node->key(), ", "));
				} else {
					std::cout << fmt::format("{} (rc = {}, size = {}): ({}) = {}\n",
											 ident,
											 node->ref_count(),
											 node->size(),
											 fmt::join(node->key(), ", "),
											 node->value());
				}
			}

			std::cout << '\n';
		}
	}

	/**
	 * Prints an easily human readable representation of a RawHypertrieContext to stdout
	 * for debugging purposes.
	 */
	template<size_t max_depth, HypertrieTrait htt_t, ByteAllocator allocator_type, size_t depth = max_depth>
	void dump_context(RawHypertrieContext<max_depth, htt_t, allocator_type> const &context) {
		dump_context_level<depth>(context);

		if constexpr (depth > 1) {
			dump_context<max_depth, htt_t, allocator_type, depth - 1>(context);
		}
	}

	/**
	 * Prints a translation table (hash integer value -> generated identifier)
	 * to help correspond hash values of identifiers that you will see when using a debugger
	 * to the human readable identifiers that are also used to in dump_context.
	 */
	template<size_t max_depth, HypertrieTrait htt_t, ByteAllocator allocator_type, size_t depth = max_depth>
	void dump_context_hash_translation_table(RawHypertrieContext<max_depth, htt_t, allocator_type> const &context) {
		auto const &fns = context.node_storage_.template nodes<depth, FullNode>().nodes();

		for (auto const &[id, _] : fns) {
			std::cout << fmt::format("{:<20} = {}\n", id.hash(), id);
		}


		if constexpr (depth > 1 || !HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
			auto const &sens = context.node_storage_.template nodes<depth, SingleEntryNode>().nodes();

			for (auto const &[id, _] : sens) {
				std::cout << fmt::format("{:<20} = {}\n", id.hash(), id);
			}
		}

		if constexpr (depth > 1) {
			dump_context_hash_translation_table<max_depth, htt_t, allocator_type, depth - 1>(context);
		}
	}

} // namespace dice::hypertrie::tests::core::node

#endif// HYPERTRIE_DUMP_RAW_CONTEXT_HPP

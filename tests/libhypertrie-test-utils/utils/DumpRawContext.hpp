#ifndef HYPERTRIE_DUMP_RAW_CONTEXT_HPP
#define HYPERTRIE_DUMP_RAW_CONTEXT_HPP

#include <dice/hypertrie/internal/raw/node/FullNode.hpp>
#include <dice/hypertrie/internal/raw/node/SingleEntryNode.hpp>
#include <dice/hypertrie/internal/raw/node_context/RawHypertrieContext.hpp>
#include <dice/hypertrie/internal/raw/node/fmt_Identifier.hpp>

#include <fmt/format.h>

#include <iostream>

namespace dice::hypertrie::tests::core::node {

	using namespace ::dice::hypertrie::internal::raw;
	using namespace ::dice::hypertrie::internal;

	template<size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
	static void dump_full_node(FullNode<depth, htt_t, allocator_type> const &node, std::ostream &os = std::cout) {
		os << fmt::format("{} (rc = {}, size = {}): {{\n", node.identifier(), node.ref_count(), node.size());

		if constexpr (depth > 1) {
			for (size_t pos = 0; pos < depth; ++pos) {
				os << fmt::format("    (.{}): {{\n", pos);

				for (auto const &[keypart, child] : node.edges(pos)) {
					if constexpr (depth - 1 == 1 && HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
						if (child.is_sen()) {
							os << fmt::format("        .{} = inplace {}\n", keypart, child.decode_key_part());
							continue;
						}
					}

					os << fmt::format("        .{} = {}\n", keypart, child.identifier());
				}

				os << "    }\n";
			}
		} else {
			os << fmt::format("    (.0): {{\n");

			for (auto const &child : node.edges()) {
				if constexpr (htt_t::is_bool_valued) {
					os << fmt::format("        .{} = true\n", child);
				} else {
					os << fmt::format("        .{} = {}\n", child.first, child.second);
				}
			}

			os << "    }\n";
		}

		os << "}\n";
	}

	template<size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
	static void dump_cartesian_node(CartesianNode<depth, htt_t, allocator_type> const &node, std::ostream &os = std::cout) {
		os << fmt::format("{} (rc = {}, size = {}): ", node.identifier(), node.ref_count(), node.size());

		node.for_each_operand([&os]<size_t ix, size_t operand_depth>(NodePtr<operand_depth, htt_t, allocator_type> const &operand) {
			if constexpr (ix == 0) {
				if constexpr (operand_depth == 0) {
					os << "empty-operand@D0";
				} else {
					if constexpr (operand_depth == 1 && HypertrieTrait_taggable_key_part<htt_t>) {
						if (operand.is_sen()) {
							os << fmt::format("inplace {}", operand.decode_key_part());
							return;
						}
					}

					os << fmt::format("{:#}@D{}", operand.identifier(), operand_depth);
				}
			} else {
				if constexpr (operand_depth == 0) {
					os << " ⨯ empty-operand@D0";
				} else {
					if constexpr (operand_depth == 1 && HypertrieTrait_taggable_key_part<htt_t>) {
						if (operand.is_sen()) {
							os << fmt::format(" ⨯ inplace {}", operand.decode_key_part());
							return;
						}
					}

					os << fmt::format(" ⨯ {:#}@D{}", operand.identifier(), operand_depth);
				}
			}
		});

		os << std::endl;
	}

	/**
	 * Prints an easily human readable representation of a single depth of a RawHypertrieContext to stdout
	 * for debugging purposes.
	 */
	template<size_t depth, size_t max_depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
	static void dump_context_level(RawHypertrieContext<max_depth, htt_t, allocator_type> const &context, std::ostream &os = std::cout) {

		if constexpr (depth > 1) {
			auto const &xns = context.node_storage_.template nodes<depth, CartesianNode>().nodes();
			for (auto const &xn : xns) {
				dump_cartesian_node(*xn, os);
			}
		}

		auto const &fns = context.node_storage_.template nodes<depth, FullNode>().nodes();
		for (auto const &fn : fns) {
			dump_full_node(*fn, os);
		}

		if constexpr (depth > 1 || !HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
			auto const &sens = context.node_storage_.template nodes<depth, SingleEntryNode>().nodes();
			for (auto const &sen : sens) {

				if constexpr (htt_t::is_bool_valued) {
					os << fmt::format("{} (rc = {}, size = {}): ({}) = true\n",
											 sen->identifier(),
											 sen->ref_count(),
											 sen->size(),
											 fmt::join(sen->key(), ", "));
				} else {
					os << fmt::format("{} (rc = {}, size = {}): ({}) = {}\n",
											 sen->identifier(),
											 sen->ref_count(),
											 sen->size(),
											 fmt::join(sen->key(), ", "),
											 sen->value());
				}
			}

			os << std::endl;
		}
	}

	/**
	 * Prints an easily human readable representation of a RawHypertrieContext to stdout
	 * for debugging purposes.
	 */
	template<size_t max_depth, HypertrieTrait htt_t, ByteAllocator allocator_type, size_t depth = max_depth>
	void dump_context(RawHypertrieContext<max_depth, htt_t, allocator_type> const &context, std::string_view name = "", std::ostream &os = std::cout) {
		if (!name.empty()) {
			os << name << ":\n";
		}

		dump_context_level<depth>(context, os);

		if constexpr (depth > 1) {
			dump_context<max_depth, htt_t, allocator_type, depth - 1>(context, "", os);
		}
	}

	/**
	 * Prints a translation table (hash integer value -> generated identifier)
	 * to help correspond hash values of identifiers that you will see when using a debugger
	 * to the human readable identifiers that are also used to in dump_context.
	 */
	template<size_t max_depth, HypertrieTrait htt_t, ByteAllocator allocator_type, size_t depth = max_depth>
	void dump_context_hash_translation_table(RawHypertrieContext<max_depth, htt_t, allocator_type> const &context, std::ostream &os = std::cout) {
		auto const &fns = context.node_storage_.template nodes<depth, FullNode>().nodes();
		for (auto const &fn : fns) {
			os << fmt::format("{:<20} = {}\n", fn->identifier().hash(), fn->identifier());
		}

		if constexpr (depth > 1) {
			auto const &xns = context.node_storage_.template nodes<depth, CartesianNode>().nodes();
			for (auto const &xn : xns) {
				os << fmt::format("{:<20} = {}\n", xn->identifier().hash(), xn->identifier());
			}
		}

		if constexpr (depth > 1 || !HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
			auto const &sens = context.node_storage_.template nodes<depth, SingleEntryNode>().nodes();

			for (auto const &sen : sens) {
				os << fmt::format("{:<20} = {}\n", sen->identifier().hash(), sen->identifier());
			}
		}

		if constexpr (depth > 1) {
			dump_context_hash_translation_table<max_depth, htt_t, allocator_type, depth - 1>(context, os);
		}
	}

} // namespace dice::hypertrie::tests::core::node

#endif// HYPERTRIE_DUMP_RAW_CONTEXT_HPP

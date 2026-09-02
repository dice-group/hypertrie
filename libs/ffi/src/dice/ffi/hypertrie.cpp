#include <dice/ffi/hypertrie.h>
#include <dice/ffi/hypertrie_internal_ffi.hpp>
#include <dice/sparse-map/sparse_map.hpp>

#include <cerrno>

using namespace dice::hypertrie;
using namespace dice::hypertrie::internal::ffi;
using metall_manager_t = dice::metall_ffi::internal::metall_manager;

static constexpr char const *context_suffix = "_context";
static constexpr char const *hypertrie_suffix = "_hypertrie";

hypertrie *hypertrie_new_in_memory(hypertrie_value_discriminant value_type, size_t depth) {
	switch (value_type) {
		case HYPERTRIE_BOOL: {
			return reinterpret_cast<hypertrie *>(new AnyHypertrie{std::in_place_type<BoolHypertrie>, depth});
		}
		case HYPERTRIE_DOUBLE: {
			return reinterpret_cast<hypertrie *>(new AnyHypertrie{std::in_place_type<DoubleHypertrie>, depth});
		}
		case HYPERTRIE_FLOAT: {
			return reinterpret_cast<hypertrie *>(new AnyHypertrie{std::in_place_type<FloatHypertrie>, depth});
		}
		case HYPERTRIE_INT64: {
			return reinterpret_cast<hypertrie *>(new AnyHypertrie{std::in_place_type<Int64Hypertrie>, depth});
		}
		default:  {
			errno = EINVAL;
			return nullptr;
		}
	}
}

template<HypertrieTrait htt_t>
static hypertrie *persistent_construct_impl(metall_manager_t &manager, char const *name, size_t depth) {
	auto const context_name = name + std::string{context_suffix};
	auto const hypertrie_name = name + std::string{hypertrie_suffix};

	using Context_t = HypertrieContext<htt_t, allocator_type>;

	auto *context = manager.construct<AnyHypertrieContext>(context_name.c_str())(std::in_place_type<Context_t>, manager.get_allocator());
    auto *hyp = manager.construct<AnyHypertrie>(hypertrie_name.c_str())(std::in_place_type<Hypertrie<htt_t, allocator_type>>,
                                                                        depth,
                                                                        std::get_if<Context_t>(context));

	return reinterpret_cast<hypertrie *>(hyp);
}

hypertrie *hypertrie_create_persistent(hypertrie_value_discriminant value_type, size_t depth, metall_manager *manager_, char const *name) {
	auto *manager = reinterpret_cast<metall_manager_t *>(manager_);
	try {
		switch (value_type) {
			case HYPERTRIE_BOOL: {
				return persistent_construct_impl<bool_htt_t>(*manager, name, depth);
			}
			case HYPERTRIE_DOUBLE: {
				return persistent_construct_impl<double_htt_t>(*manager, name, depth);
			}
			case HYPERTRIE_FLOAT: {
				return persistent_construct_impl<float_htt_t>(*manager, name, depth);
			}
			case HYPERTRIE_INT64: {
				return persistent_construct_impl<int64_htt_t>(*manager, name, depth);
			}
			default: {
				errno = EINVAL;
				return nullptr;
			}
		}
	} catch (...) {
		errno = ENOMEM;
		return nullptr;
	}
}

hypertrie *hypertrie_open_persistent(metall_manager *manager_, char const *name) {
	auto *manager = reinterpret_cast<metall_manager_t *>(manager_);

	std::string const hypertrie_name = name + std::string{hypertrie_suffix};

	try {
		auto const ptr = reinterpret_cast<hypertrie *>(manager->find<AnyHypertrie>(hypertrie_name.c_str()).first);
		if (ptr == nullptr) {
			errno = ENOMEM;
		}

		return ptr;
	} catch (...) {
		errno = ENOMEM;
		return nullptr;
	}
}

hypertrie *hypertrie_clone(hypertrie const *hyp_) {
	auto const *hyp = reinterpret_cast<AnyHypertrie const *>(hyp_);

	return std::visit([]<typename H>(H const &hyp) -> hypertrie * {
		return reinterpret_cast<hypertrie *>(new AnyHypertrie{std::in_place_type<Hypertrie<typename H::htt_t, allocator_type>>, hyp});
	}, *hyp);
}

void hypertrie_copy_into(hypertrie const *src_, hypertrie *dst_) {
	auto const *src = reinterpret_cast<AnyHypertrie const *>(src_);
	auto *dst = reinterpret_cast<AnyHypertrie *>(dst_);

	std::visit([]<typename S, typename D>(S const &src, D &dst) {
		if constexpr (requires { dst.set(Key<typename D::htt_t>{}, typename D::htt_t::value_type{}); }) {
			dst.extend(src);
		} else {
			HYPERTRIE_UNREACHABLE;
		}
	}, *src, *dst);
}

void hypertrie_destroy(hypertrie const *hyp_) {
	auto const *hyp = reinterpret_cast<AnyHypertrie const *>(hyp_);

	auto const in_memory = std::visit([](auto &hyp) noexcept {
		return hyp.context()->get_allocator().template holds_allocator<std_allocator_type_t>();
	}, *hyp);

	if (!in_memory) {
		return;
	}

	delete hyp;
}

bool hypertrie_destroy_persistent(metall_manager *manager_, char const *name) {
	auto *manager = reinterpret_cast<metall_manager_t *>(manager_);
	auto const hypertrie_name = name + std::string{hypertrie_suffix};
	auto const context_name = name + std::string{context_suffix};

	return manager->destroy<AnyHypertrie>(hypertrie_name.c_str())
	        && manager->destroy<AnyHypertrieContext>(context_name.c_str());
}

template<typename V>
static constexpr V from_type_erased_hypertrie_value(hypertrie_value value) noexcept {
	if constexpr (std::is_same_v<V, bool>) {
		assert(value.discriminant == HYPERTRIE_BOOL);
		return value.bool_;
	} else if constexpr (std::is_same_v<V, double>) {
		assert(value.discriminant == HYPERTRIE_DOUBLE);
		return value.double_;
	} else if constexpr (std::is_same_v<V, float>) {
		assert(value.discriminant == HYPERTRIE_FLOAT);
		return value.float_;
	} else if constexpr (std::is_same_v<V, int64_t>) {
		assert(value.discriminant == HYPERTRIE_INT64);
		return value.int64_;
	} else {
		HYPERTRIE_UNREACHABLE;
	}
}

template<typename V>
static constexpr hypertrie_value into_type_erased_hypertrie_value(V value) noexcept {
	if constexpr (std::is_same_v<V, bool>) {
		return hypertrie_value{.discriminant = HYPERTRIE_BOOL, .bool_ = value};
	} else if constexpr (std::is_same_v<V, double>) {
		return hypertrie_value{.discriminant = HYPERTRIE_DOUBLE, .double_ = value};
	} else if constexpr (std::is_same_v<V, float>) {
		return hypertrie_value{.discriminant = HYPERTRIE_FLOAT, .float_ = value};
	} else if constexpr (std::is_same_v<V, int64_t>) {
		return hypertrie_value{.discriminant = HYPERTRIE_INT64, .int64_ = value};
	} else {
		HYPERTRIE_UNREACHABLE;
	}
}

hypertrie_value hypertrie_get(hypertrie const *hyp_, hypertrie_key const *key) {
	auto const *hyp = reinterpret_cast<AnyHypertrie const *>(hyp_);

	return std::visit([key]<typename H>(H const &hyp) noexcept {
		assert(key->size == hyp.depth());

		return dice::template_library::switch_cases<0, hypertrie_max_depth + 1>(hyp.depth(), [&hyp, key](auto depth_arg) noexcept {
			internal::raw::RawKey<depth_arg, typename H::htt_t> raw_key; {
				for (size_t ix = 0; ix < hyp.depth(); ++ix) {
					raw_key[ix] = key->key[ix];
				}
			}

			return into_type_erased_hypertrie_value(hyp[raw_key]);
		});
	}, *hyp);
}

hypertrie_value hypertrie_set(hypertrie *hyp_, hypertrie_key const *key, hypertrie_value value) {
	auto *hyp = reinterpret_cast<AnyHypertrie *>(hyp_);

	return std::visit([key, &value]<typename H>(H &hyp) noexcept -> hypertrie_value {
		if constexpr (requires { hyp.set(Key<typename H::htt_t>{}, typename H::htt_t::value_type{}); }) {
			assert(key->size == hyp.depth());

			auto const concrete_value = from_type_erased_hypertrie_value<typename H::value_type>(value);

			return dice::template_library::switch_cases<0, hypertrie_max_depth + 1>(hyp.depth(), [&hyp, key, concrete_value](auto depth_arg) noexcept {
				internal::raw::RawKey<depth_arg, typename H::htt_t> raw_key;
				{
					for (size_t ix = 0; ix < hyp.depth(); ++ix) {
						raw_key[ix] = key->key[ix];
					}
				}

				return into_type_erased_hypertrie_value(hyp.set(raw_key, concrete_value));
			});
		} else {
			HYPERTRIE_UNREACHABLE;
		}
	}, *hyp);
}

hypertrie const *hypertrie_slice(hypertrie const *hyp_, hypertrie_slice_key const *slice_key) {
	auto const *hyp = reinterpret_cast<AnyHypertrie const *>(hyp_);

	return std::visit([slice_key]<typename H>(H const &hyp) noexcept {
		assert(slice_key->size == hyp.depth());

		SliceKey<typename H::htt_t> hslice_key;
		hslice_key.as_inner().resize(hyp.depth());

		for (size_t ix = 0; ix < hyp.depth(); ++ix) {
			hslice_key.as_inner()[ix] = (slice_key->mask >> ix & 1) ? std::optional<hypertrie_key_part>{slice_key->key[ix]}
																	: std::nullopt;
		}

		return reinterpret_cast<hypertrie const *>(new AnyHypertrie{hyp[hslice_key]});
	}, *hyp);
}

size_t hypertrie_depth(hypertrie const *hyp_) {
	auto const *hyp = reinterpret_cast<AnyHypertrie const *>(hyp_);
	return std::visit([](auto const &hyp) noexcept {
		return hyp.depth();
	}, *hyp);
}

size_t hypertrie_size(hypertrie const *hyp_) {
	auto const *hyp = reinterpret_cast<AnyHypertrie const *>(hyp_);
	return std::visit([](auto const &hyp) noexcept {
		return hyp.size();
	}, *hyp);
}

hypertrie_cards hypertrie_get_cards(hypertrie const *hyp_) {
	auto const *hyp = reinterpret_cast<AnyHypertrie const *>(hyp_);

	return std::visit([](auto const &hyp) noexcept {
		std::vector<internal::pos_type> positions; {
			positions.resize(hyp.depth());
			std::iota(positions.begin(), positions.end(), 0);
		}

		auto const cards = hyp.get_cards(positions);

		hypertrie_cards ret_cards;
		ret_cards.size = cards.size();

		for (size_t ix = 0; ix < cards.size(); ++ix) {
			ret_cards.values[ix] = cards[ix];
		}

		return ret_cards;
	}, *hyp);
}

template<typename out_type, HypertrieTrait htt_t>
hypertrie_result hypertrie_einsum_impl(hypertrie const **operands_,
									   size_t n_operands,
									   char const *subscript_,
									   unsigned long timeout_ms,
									   hypertrie_einsum_solution_generator *out_generator__) {
	std::vector<const_Hypertrie<htt_t, allocator_type>> operands;
	operands.reserve(n_operands);

	for (size_t ix = 0; ix < n_operands; ++ix) {
		auto const *opp = reinterpret_cast<AnyHypertrie const *>(operands_[ix]);

		auto op = std::visit([&]<typename H>(H const &hyp) noexcept -> std::optional<const_Hypertrie<htt_t, allocator_type>> {
			if constexpr (std::is_same_v<typename H::value_type, typename htt_t::value_type>) {
				return hyp;
			} else {
				return std::nullopt; // type mismatch
			}
		}, *opp);

		if (!op.has_value()) {
			errno = EINVAL;
			return HYPERTRIE_FAILURE;
		}

		operands.push_back(std::move(*op));
	}

	std::shared_ptr<dice::einsum::Subscript> subscript;
	try {
		subscript = std::make_shared<dice::einsum::Subscript>(subscript_);
	} catch (...) {
		errno = EINVAL;
		return HYPERTRIE_FAILURE;
	}

	auto const end_time = timeout_ms == HYPERTRIE_NO_TIMEOUT ? std::chrono::steady_clock::time_point::max()
															 : std::chrono::steady_clock::now() + std::chrono::milliseconds{timeout_ms};

	auto *out_generator_ = reinterpret_cast<AnyEinsumGenerator *>(out_generator__);
	new (out_generator_) AnyEinsumGenerator{EinsumGenerator<out_type, htt_t>{.operands = std::move(operands),
																			 .subscript = std::move(subscript),
																			 .generator = {},
																			 .iter = {},
																			 .timeout = false}};

	auto &out_generator = std::get<EinsumGenerator<out_type, htt_t>>(*out_generator_);

	try {
		out_generator.generator = dice::einsum::einsum<out_type>(out_generator.subscript, out_generator.operands, end_time);
		out_generator.iter = out_generator.generator.begin();
		return HYPERTRIE_SUCCESS;
	} catch (std::invalid_argument const &) {
		out_generator_->~AnyEinsumGenerator();
		errno = EINVAL;
		return HYPERTRIE_FAILURE;
	} catch (...) {
		out_generator_->~AnyEinsumGenerator();
		errno = ETIMEDOUT;
		return HYPERTRIE_FAILURE;
	}
}

hypertrie_value_discriminant hypertrie_value_type(hypertrie const *hyp_) {
	auto const *hyp = reinterpret_cast<AnyHypertrie const *>(hyp_);
	return std::visit([]<typename H>([[maybe_unused]] H const &hyp) {
		return into_type_erased_hypertrie_value(typename H::value_type{}).discriminant;
	}, *hyp);
}

hypertrie_result hypertrie_einsum(hypertrie const **operands,
								  size_t n_operands,
								  char const *subscript,
								  unsigned long timeout_ms,
								  hypertrie_value_discriminant out_type,
								  hypertrie_einsum_solution_generator *out_generator) {
	auto const detected_operand_type = n_operands > 0 ? hypertrie_value_type(operands[0]) : HYPERTRIE_BOOL; // just choose any default, doesn't matter

	switch (detected_operand_type) {
		case HYPERTRIE_BOOL: {
			switch (out_type) {
				case HYPERTRIE_BOOL: {
					return hypertrie_einsum_impl<bool, bool_htt_t>(operands, n_operands, subscript, timeout_ms, out_generator);
				}
				case HYPERTRIE_INT64: {
					return hypertrie_einsum_impl<int64_t, bool_htt_t>(operands, n_operands, subscript, timeout_ms, out_generator);
				}
				case HYPERTRIE_FLOAT: {
					return hypertrie_einsum_impl<float, bool_htt_t>(operands, n_operands, subscript, timeout_ms, out_generator);
				}
				case HYPERTRIE_DOUBLE: {
					return hypertrie_einsum_impl<double, bool_htt_t>(operands, n_operands, subscript, timeout_ms, out_generator);
				}
				default: {
					errno = EINVAL;
					return HYPERTRIE_FAILURE;
				}
			}
		}
		case HYPERTRIE_INT64: {
			switch (out_type) {
				case HYPERTRIE_BOOL: {
					return hypertrie_einsum_impl<bool, int64_htt_t>(operands, n_operands, subscript, timeout_ms, out_generator);
				}
				case HYPERTRIE_INT64: {
					return hypertrie_einsum_impl<int64_t, int64_htt_t>(operands, n_operands, subscript, timeout_ms, out_generator);
				}
				case HYPERTRIE_FLOAT: {
					return hypertrie_einsum_impl<float, int64_htt_t>(operands, n_operands, subscript, timeout_ms, out_generator);
				}
				case HYPERTRIE_DOUBLE: {
					return hypertrie_einsum_impl<double, int64_htt_t>(operands, n_operands, subscript, timeout_ms, out_generator);
				}
				default: {
					errno = EINVAL;
					return HYPERTRIE_FAILURE;
				}
			}
		}
		case HYPERTRIE_FLOAT: {
			switch (out_type) {
				case HYPERTRIE_BOOL: {
					return hypertrie_einsum_impl<bool, float_htt_t>(operands, n_operands, subscript, timeout_ms, out_generator);
				}
				case HYPERTRIE_INT64: {
					return hypertrie_einsum_impl<int64_t, float_htt_t>(operands, n_operands, subscript, timeout_ms, out_generator);
				}
				case HYPERTRIE_FLOAT: {
					return hypertrie_einsum_impl<float, float_htt_t>(operands, n_operands, subscript, timeout_ms, out_generator);
				}
				case HYPERTRIE_DOUBLE: {
					return hypertrie_einsum_impl<double, float_htt_t>(operands, n_operands, subscript, timeout_ms, out_generator);
				}
				default: {
					errno = EINVAL;
					return HYPERTRIE_FAILURE;
				}
			}
		}
		case HYPERTRIE_DOUBLE: {
			switch (out_type) {
				case HYPERTRIE_BOOL: {
					return hypertrie_einsum_impl<bool, double_htt_t>(operands, n_operands, subscript, timeout_ms, out_generator);
				}
				case HYPERTRIE_INT64: {
					return hypertrie_einsum_impl<int64_t, double_htt_t>(operands, n_operands, subscript, timeout_ms, out_generator);
				}
				case HYPERTRIE_FLOAT: {
					return hypertrie_einsum_impl<float, double_htt_t>(operands, n_operands, subscript, timeout_ms, out_generator);
				}
				case HYPERTRIE_DOUBLE: {
					return hypertrie_einsum_impl<double, double_htt_t>(operands, n_operands, subscript, timeout_ms, out_generator);
				}
				default: {
					errno = EINVAL;
					return HYPERTRIE_FAILURE;
				}
			}
		}
		default: {
			errno = EINVAL;
			return HYPERTRIE_FAILURE;
		}
	}
}

size_t hypertrie_einsum_solution_generator_result_depth(hypertrie_einsum_solution_generator const *gen_) {
	auto const *gen = reinterpret_cast<AnyEinsumGenerator const *>(gen_);
	return std::visit([](auto const &gen) noexcept {
		return gen.subscript->resultLabelCount();
	}, *gen);
}

hypertrie_value_discriminant hypertrie_einsum_solution_generator_result_value_type(hypertrie_einsum_solution_generator const *gen_) {
	auto const *gen = reinterpret_cast<AnyEinsumGenerator const *>(gen_);
	return std::visit([]<typename G>([[maybe_unused]] G const &gen) noexcept {
		return into_type_erased_hypertrie_value(typename G::value_type{}).discriminant;
	}, *gen);
}

hypertrie_iter_result hypertrie_einsum_solution_generator_next(hypertrie_einsum_solution_generator *gen_, hypertrie_einsum_solution *out_solution) {
	auto &gen = *reinterpret_cast<AnyEinsumGenerator *>(gen_);

	return std::visit([out_solution](auto &gen) noexcept {
		if (gen.timeout) [[unlikely]] {
			errno = ETIMEDOUT;
			return HYPERTRIE_I_FAILURE;
		}

		if (gen.iter == std::default_sentinel) {
			return HYPERTRIE_I_ENDED;
		}

		auto const &solution = *gen.iter;

		assert(out_solution->key.size == solution.size());
		memcpy(out_solution->key.key, solution.key().as_inner().data(), solution.size() * sizeof(hypertrie_key_part));

		out_solution->value = into_type_erased_hypertrie_value(solution.value());

		try {
			++gen.iter;
		} catch (...) {
			gen.timeout = true;
		}

		return HYPERTRIE_I_YIELDED;
	}, gen);
}

bool hypertrie_consume_einsum_solution_generator(hypertrie_einsum_solution_generator *gen_, hypertrie *hyp_) {
	auto *gen = reinterpret_cast<AnyEinsumGenerator *>(gen_);
	auto *hyp = reinterpret_cast<AnyHypertrie *>(hyp_);

	auto const ret = std::visit([]<typename G, typename H>(G &gen, H &hyp) noexcept -> bool {
		if constexpr (requires { hyp.set(Key<typename H::htt_t>{}, typename H::htt_t::value_type{}); }) {
			if (gen.subscript->resultLabelCount() > hypertrie_max_depth || gen.subscript->resultLabelCount() != hyp.depth()) {
				errno = EINVAL;
				return false;
			}

			hyp.extend_from_iter(std::move(gen.iter), std::default_sentinel);
			return true;
		} else {
			HYPERTRIE_UNREACHABLE;
		}
	}, *gen, *hyp);

	if (ret) {
		gen->~AnyEinsumGenerator();
	}

	return ret;
}

void hypertrie_einsum_solution_generator_destroy(hypertrie_einsum_solution_generator *gen_) {
	auto *gen = reinterpret_cast<AnyEinsumGenerator *>(gen_);
	gen->~AnyEinsumGenerator();
}

void hypertrie_iterate(hypertrie const *hyp_, hypertrie_iterator *out_iter) {
	auto const *hyp = reinterpret_cast<AnyHypertrie const *>(hyp_);

	std::visit([out_iter]<typename H>(H const &hyp) noexcept {
		new (out_iter) AnyIterator{std::in_place_type<dice::hypertrie::Iterator<typename H::htt_t, allocator_type>>, hyp};
	}, *hyp);
}

hypertrie_iter_result hypertrie_iterator_next(hypertrie_iterator *iter_, hypertrie_key_value *out_elem) {
	auto *iter = reinterpret_cast<AnyIterator *>(iter_);

	return std::visit([out_elem](auto &iter) noexcept {
		if (iter == std::default_sentinel) {
			return HYPERTRIE_I_ENDED;
		}

		auto const &entry = *iter;

		out_elem->key.size = entry.key().size();
		memcpy(out_elem->key.key, entry.key().as_inner().data(), entry.key().size() * sizeof(hypertrie_key_part));
		out_elem->value = into_type_erased_hypertrie_value(entry.value());

		++iter;

		return HYPERTRIE_I_YIELDED;
	}, *iter);
}

void hypertrie_iterator_destroy(hypertrie_iterator *iter_) {
	auto *iter = reinterpret_cast<AnyIterator *>(iter_);
	iter->~AnyIterator();
}

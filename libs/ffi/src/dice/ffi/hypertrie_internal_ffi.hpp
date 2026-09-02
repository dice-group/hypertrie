#ifndef HYPERTRIE_FFI_HYPERTRIE_CONFIG_HPP
#define HYPERTRIE_FFI_HYPERTRIE_CONFIG_HPP

#include <dice/einsum.hpp>
#include <dice/hypertrie.hpp>
#include <dice/ffi/metall_internal.hpp>

#define DICE_TEMPLATE_LIBRARY_WITH_BOOST 1
#include <dice/template-library/polymorphic_allocator.hpp>

namespace dice::hypertrie::internal::ffi {
	template<typename T>
	using std_allocator_type_t = template_library::offset_ptr_stl_allocator<T>;
	using std_allocator_type = std_allocator_type_t<std::byte>;

	template<typename T>
	using metall_allocator_type_t = dice::metall_ffi::internal::metall_manager::allocator_type<T>;
	using metall_allocator_type = metall_allocator_type_t<std::byte>;

	template<typename T>
	using allocator_type_t = template_library::polymorphic_allocator<T, std_allocator_type_t, metall_allocator_type_t>;
	using allocator_type = allocator_type_t<std::byte>;

	using bool_htt_t = dice::hypertrie::tagged_bool_Hypertrie_trait;
	using double_htt_t = dice::hypertrie::tagged_double_Hypertrie_trait;
	using float_htt_t = dice::hypertrie::Hypertrie_trait<uint64_t,
														 float,
														 dice::hypertrie::internal::container::dice_sparse_map,
														 dice::hypertrie::internal::container::dice_sparse_set,
														 true>;
	using int64_htt_t = dice::hypertrie::Hypertrie_trait<uint64_t,
														 int64_t,
														 dice::hypertrie::internal::container::dice_sparse_map,
														 dice::hypertrie::internal::container::dice_sparse_set,
														 true>;

	using BoolHypertrie = dice::hypertrie::Hypertrie<bool_htt_t, allocator_type>;
	using DoubleHypertrie = dice::hypertrie::Hypertrie<double_htt_t, allocator_type>;
	using FloatHypertrie = dice::hypertrie::Hypertrie<float_htt_t, allocator_type>;
	using Int64Hypertrie = dice::hypertrie::Hypertrie<int64_htt_t, allocator_type>;

	using const_BoolHypertrie = dice::hypertrie::const_Hypertrie<bool_htt_t, allocator_type>;
	using const_DoubleHypertrie = dice::hypertrie::const_Hypertrie<double_htt_t, allocator_type>;
	using const_FloatHypertrie = dice::hypertrie::const_Hypertrie<float_htt_t, allocator_type>;
	using const_Int64Hypertrie = dice::hypertrie::const_Hypertrie<int64_htt_t, allocator_type>;

	using BoolHypertrieContext = dice::hypertrie::HypertrieContext<bool_htt_t, allocator_type>;
	using DoubleHypertrieContext = dice::hypertrie::HypertrieContext<double_htt_t, allocator_type>;
	using FloatHypertrieContext = dice::hypertrie::HypertrieContext<float_htt_t, allocator_type>;
	using Int64HypertrieContext = dice::hypertrie::HypertrieContext<int64_htt_t, allocator_type>;

	using BoolIterator = dice::hypertrie::Iterator<bool_htt_t, allocator_type>;
	using DoubleIterator = dice::hypertrie::Iterator<double_htt_t, allocator_type>;
	using FloatIterator = dice::hypertrie::Iterator<float_htt_t, allocator_type>;
	using Int64Iterator = dice::hypertrie::Iterator<int64_htt_t, allocator_type>;

	using AnyHypertrie = std::variant<BoolHypertrie,
									  const_BoolHypertrie,
									  DoubleHypertrie,
									  const_DoubleHypertrie,
									  FloatHypertrie,
									  const_FloatHypertrie,
									  Int64Hypertrie,
									  const_Int64Hypertrie>;

	using AnyHypertrieContext = std::variant<BoolHypertrieContext,
											 DoubleHypertrieContext,
											 FloatHypertrieContext,
											 Int64HypertrieContext>;

	using AnyIterator = std::variant<BoolIterator,
									 DoubleIterator,
									 FloatIterator,
									 Int64Iterator>;

	template<typename value_type_, dice::hypertrie::HypertrieTrait htt_t_>
	struct EinsumGenerator {
		using value_type = value_type_;
		using htt_t = htt_t_;
		using generator_t = std::generator<dice::einsum::Entry<value_type, htt_t> const &>;

		std::vector<dice::hypertrie::const_Hypertrie<htt_t, allocator_type>> operands;
		std::shared_ptr<dice::einsum::Subscript> subscript;
		generator_t generator;
		typename generator_t::iterator iter;
		bool timeout;
	};

	using AnyEinsumGenerator = std::variant<EinsumGenerator<bool, bool_htt_t>,
											EinsumGenerator<bool, int64_htt_t>,
											EinsumGenerator<bool, float_htt_t>,
											EinsumGenerator<bool, double_htt_t>,
											EinsumGenerator<int64_t, bool_htt_t>,
											EinsumGenerator<int64_t, int64_htt_t>,
											EinsumGenerator<int64_t, float_htt_t>,
											EinsumGenerator<int64_t, double_htt_t>,
											EinsumGenerator<float, bool_htt_t>,
											EinsumGenerator<float, int64_htt_t>,
											EinsumGenerator<float, float_htt_t>,
											EinsumGenerator<float, double_htt_t>,
											EinsumGenerator<double, bool_htt_t>,
											EinsumGenerator<double, int64_htt_t>,
											EinsumGenerator<double, float_htt_t>,
											EinsumGenerator<double, double_htt_t>>;

} // namespace dice::hypertrie::internal::ffi

#endif //HYPERTRIE_FFI_HYPERTRIE_CONFIG_HPP

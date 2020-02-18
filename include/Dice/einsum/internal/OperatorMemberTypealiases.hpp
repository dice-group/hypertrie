using Operator_t = Operator<value_type, key_part_type, map_type, set_type>;
constexpr static const bool bool_value_type = Operator_t::bool_value_type;
using const_BoolHypertrie_t = typename Operator_t::const_BoolHypertrie_t;
using CardinalityEstimation_t = typename Operator_t::CardinalityEstimation_t;
constexpr static const key_part_type default_key_part = Operator_t::default_key_part;
using Entry_t = Entry<key_part_type, value_type>;
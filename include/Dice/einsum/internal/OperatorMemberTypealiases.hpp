using tr = tr_t;

using Operator_t = Operator<value_type, tr>;
constexpr static const bool bool_value_type = Operator_t::bool_value_type;
using CardinalityEstimation_t = typename Operator_t::CardinalityEstimation_t;
using key_part_type = typename tr::key_part_type;
constexpr static const key_part_type default_key_part = Operator_t::default_key_part;
using Entry_t = Entry<value_type, tr>;

#include <dice/hypertrie.hpp>
#include <metall/metall.hpp>
#include <fmt/format.h>
#include <utils/DumpRawContext.hpp>

#define DICE_TEMPLATE_LIBRARY_WITH_BOOST 1
#include <dice/template-library/polymorphic_allocator.hpp>

using namespace dice::hypertrie;

using htt_t = default_bool_Hypertrie_trait;
using allocator_type = dice::template_library::polymorphic_allocator<std::byte, dice::template_library::offset_ptr_stl_allocator, metall::manager::allocator_type>;

constexpr auto context_name = "context";
constexpr auto hypertrie_name = "hypertrie001";

#include <dice/hypertrie.hpp>
#include <dice/query.hpp>

#include <iostream>

int main() {
    dice::hypertrie::Hypertrie<dice::hypertrie::tagged_bool_Hypertrie_trait, std::allocator<std::byte>> x(3);
}
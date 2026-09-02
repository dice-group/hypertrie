# Node concepts
## Single Entry Nodes
TODO

## Full Nodes
TODO

## Cartesian Nodes
### Supported Cartesian Types
- "general" cartesians. I.e. the thing from math where you take multiple sets of numbers and take the cartesian product.
    Here they are represented using the cartesian product of some number of 1d nodes (SENs or FNs)
- "xfix" cartesians aka. "pre-/postfix" cartesians: They consist of some 1d-1element prefix and/or postfix together 
    with a possibly high dimensional full node. 
    Example: {1} x {2} x {(3, 4), (5, 6)} x {7} x {8} represents the entries: {(1, 2, 3, 4, 7, 8), (1, 2, 5, 6, 7, 8)}. 
    So they basiscally add some pre and/or postfix to an existing full node. This doesn't really make much sense if you write 
    it out in math notation but in hypertries it reduces duplication.

### General Cartesian Invariants
- Cartesians only have children with depths less than themselves (e.g. a depth 3 cartesian cannot consist
  of a single depth 3 child)
- Cartesians cannot exist at depth 1 as a consequence of the point before
- Cartesians cannot have other cartesians as children, as cartesians like that can always be simplified
- Cartesians cannot have high-dimensional (depth > 1) SENs as children, as they could also be flattened
  into the cartesian.
- Whenever possible general cartesians are preferred over xfix cartesians, meaning the checks
  for them are performed in order "is this a general cartesian?" else "is this an xfix cartesian?"
- General cartesians only exist in bool valued configs as the obvious way to interpret the values
  would be the product of all values of the operands, but that would make the hypertrie's representation
  ambiguous if general cartesians where to be allowed. The interpretation stays the same here it 
  just so happens that in bool-valued configs the value is always 1 and (1 * 1 * ... * 1 = 1). 
  In xfix cartesians the pre and postfixes values are just always set to 1 which also sidesteps
  this problem by only storing the values in the high order operand.



# General Ownership Concepts
## The `Ownership` enum
- `Owned` means the object is itself responsible for lifetime management of the node
- `EphemeralBorrowed` means the object is only referencing some storage that contains the node, but it is not
    itself responsible for the lifetime management. Additionally, it is expected that the buffer the node resides
    in will soon be overwritten or otherwise invalidated.
- `ContextBorrowed` similar to `EphemeralBorrowed` the object is only referencing some storage that contains the node and
    is not responsible for the lifetime management.
    But, unlike `EphemeralBorrowed` the node resides in the hypertrie context and as such is expected to live
    at least as long as the object holding the reference

# Hypertrie Frontend Ownership Concepts

- `const_Hypertrie<>` is basically a `HypertrieView<>`
- `const_Hypertrie<>` requires that the parent node of the node it is referring to is alive as long as it is alive
  iself. "Parent node" here has an extended definitions as all slices of a node are considered it's children, even
  if they do not exist in the context as such.

# Hypertrie Backend Ownership Concepts

## `SliceResult<>`
- SliceResults are not copyable using the copy-ctor as that can be relatively expensive if done by accident.
    Instead `SliceResult::clone()` is provided which, if necessary, copies the node the SliceResult is pointing to.

## `RawIterator<>`
- **Every** iterator advance of a `RawIterator` invalidates the reference to the value
  given out. If you want the value to live longer: just copy it.

## `RawHashDiagonal<>`
- **Every** iterator advance of a `RawHashDiagonal` invalidates all slice results given out (**not only** the reference)
    it does **not** matter if you moved it out. If you want the slice result to live longer: call `SliceResult::clone()`


# General Code Concepts
## Common `if constexpr` expressions you will find in the codebase
- `if constexpr (depth == 1 && HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>)` when working with full nodes: depth-1 single entry nodes are
  encoded directly into the edges in taggable boolean-valued hypertrie configurations, these branches check for that
  and directly encode/decode the single entry from the edge instead of looking at a node (which doesn't exist)
- `if constexpr (operand_depth > 0)` when working with cartesian's `for_each_operand`: for each operand executes the given
  function for every single operand. This means it also executes it for the empty operands that may be present at the end of
  xfix cartesians. These are by convention ignored for most calculations, and are marked as to be ignored by being encoded as depth 0.
- `if constexpr (operand_depth == 1 && Hypertrie_taggable_key_part<htt_t>)` when working with cartesians: Cartesians do something simmilar
  to the above point. They directly encode depth-1 1-entry operands in any taggable configuration into the edges.
  This is possible because in bool-valued configurations there is no value to encode and in non-bool-valued configurations
  only xfix cartesians exist. Those store the values only in their high order operand. All other (SEN) operands will by convention
  have their value set to `value_type{1}`.

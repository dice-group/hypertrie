#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file

#include <catch2/catch.hpp>

//#include "TestRawBoolhypertrie.cpp"
//#include "TestRawDiagonal.cpp"
//#include "TestBoolHypertrie.cpp"
//#include "TestHashDiagonal.cpp"
#include "TestLeftJoin.cpp"
#include "TestJoinCombination.cpp"
//#include "TestDirectedGraph.cpp"
//#include "TestHashJoin.cpp"

#ifdef HYPERTRIE_ENABLE_LIBTORCH
#include "TestEinsum.cpp"
#endif
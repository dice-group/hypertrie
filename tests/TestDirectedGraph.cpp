#include <Dice/einsum/internal/util/DependencyGraph.hpp>
#include <Dice/hypertrie/hypertrie.hpp>
#include <catch2/catch.hpp>
#include <fmt/core.h>
#include <fmt/format.h>

namespace hypertrie::tests::directedgraph {

	using namespace einsum::internal::util;

    TEST_CASE("weakly connected components", "[directed graph]") {
        DirectedGraph g{};
        g.addVertex();
        g.addVertex();
        g.addVertex();
        g.addVertex();
        g.addVertex();
        g.addVertex();
        g.addVertex();
        g.addEdge('x', 0, 1);
        g.addEdge('x', 1, 0);
        g.addEdge('a', 0, 1);
        g.addEdge('v', 1, 5);
        g.addEdge('v', 5, 1);
        g.addEdge('y', 1, 2);
        g.addEdge('z', 2, 3);
        g.addEdge('z', 3, 4);
        g.addEdge('z', 4, 3);
        g.addEdge('t', 4, 6);
        g.addEdge('t', 6, 4);
		auto wccs = g.getWeaklyConnectedComponents();
		std::cout << wccs.size() << std::endl;
        for(auto& wcc : wccs)
			fmt::print("{} \n", wcc);
	}

    TEST_CASE("strongly connected components", "[directed graph]") {
        DirectedGraph g{};
        g.addVertex();
        g.addVertex();
        g.addVertex();
        g.addVertex();
        g.addVertex();
        g.addVertex();
        g.addVertex();
        g.addEdge('x', 0, 1);
        g.addEdge('x', 1, 0);
        g.addEdge('v', 1, 5);
        g.addEdge('v', 5, 1);
        g.addEdge('x', 1, 2);
        g.addEdge('y', 2, 3);
        g.addEdge('z', 3, 4);
        g.addEdge('z', 4, 3);
        g.addEdge('t', 4, 6);
        g.addEdge('t', 6, 4);
        auto sccs = g.getStronglyConnectedComponents();
        std::cout << sccs.size() << std::endl;
        for(auto& scc : sccs)
            fmt::print("{}\n", scc);
    }

}
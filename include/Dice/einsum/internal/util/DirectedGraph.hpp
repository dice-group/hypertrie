#ifndef SPARSETENSOR_EINSUM_UTIL_DIRECTEDGRAPH_HPP
#define SPARSETENSOR_EINSUM_UTIL_DIRECTEDGRAPH_HPP

#include <set>
#include <stack>
#include <string>
#include <variant>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/connected_components.hpp>
#include <boost/graph/strong_components.hpp>


namespace einsum::internal::util {

    template<typename T = std::size_t, typename R = char>
    class DirectedGraph {

    private:

		struct EdgeLabel {
			R label;
		};

		using BoostGraph = boost::adjacency_list<boost::vecS,
												 boost::vecS,
												 boost::directedS,
												 boost::no_property,
												 EdgeLabel>;
        using Vertex =  boost::graph_traits<BoostGraph>::vertex_descriptor;

		std::map<T, Vertex> vertex_name_map{};
		BoostGraph graph{};

    public:

		struct StrongComponent {
			std::set<R> incoming_labels{};
			std::set<R> component_labels{};
			std::set<R> outgoing_labels{};
		};

		using StrongComponent_t = StrongComponent;

        DirectedGraph() = default;

		void addVertex(T new_vertex) {
			auto new_v = boost::add_vertex(graph);
			vertex_name_map[new_vertex] = new_v;
		}

		void addEdge(R label, T source, T target) {
			boost::add_edge(vertex_name_map[source], vertex_name_map[target], EdgeLabel{label}, graph);
		}

		// treats the directed graph as an undirected graph
		// finds the connected components of the undirected graph
		// returns the labels of each component
        [[nodiscard]] std::vector<std::set<R>> getWeaklyConnectedComponents() {

            std::vector<int> component(boost::num_vertices(graph));
            auto num_components = boost::connected_components(graph, &component[0]);

            std::vector<std::set<R>> weakly_connected_components(num_components);

			for(std::vector<int>::size_type i = 0; i < component.size(); i++) {
                auto out_edge_iterators = boost::out_edges(i, graph);
                for(auto out_edge_it = out_edge_iterators.first; out_edge_it != out_edge_iterators.second; out_edge_it++) {
					weakly_connected_components[component[i]].insert(graph[*out_edge_it].label);
				}
			}

            return weakly_connected_components;
        }

        // finds the strongly connected components of the directed graph
        // returns the labels of each strong component
		// https://www.boost.org/doc/libs/1_74_0/libs/graph/example/strong_components.cpp
        [[nodiscard]] std::vector<std::set<R>> getStronglyConnectedComponents() {

            std::vector< int > component(num_vertices(graph)),
                    discover_time(num_vertices(graph));
            std::vector<boost::default_color_type > color(num_vertices(graph));
            std::vector<Vertex> root(num_vertices(graph));
            int num_components = strong_components(graph,
												   make_iterator_property_map(component.begin(), get(boost::vertex_index, graph)),
												   root_map(make_iterator_property_map(root.begin(), get(boost::vertex_index, graph)))
														   .color_map(
																   make_iterator_property_map(color.begin(), get(boost::vertex_index, graph)))
														   .discover_time_map(make_iterator_property_map(
																   discover_time.begin(), get(boost::vertex_index, graph))));

            std::vector<std::set<R>> strongly_connected_components(num_components);

            for(std::vector<int>::size_type i = 0; i < component.size(); i++) {
                auto out_edge_iterators = boost::out_edges(i, graph);
                for(auto out_edge_it = out_edge_iterators.first; out_edge_it != out_edge_iterators.second; out_edge_it++) {
                    strongly_connected_components[component[i]].insert(graph[*out_edge_it].label);
                }
            }

			return strongly_connected_components;
        }

        [[nodiscard]] StrongComponent_t getIndependentStrongComponent() {

			std::vector<int> component(num_vertices(graph)),
					discover_time(num_vertices(graph));
			std::vector<boost::default_color_type> color(num_vertices(graph));
			std::vector<Vertex> root(num_vertices(graph));
			int num_components = strong_components(graph,
												   make_iterator_property_map(component.begin(), get(boost::vertex_index, graph)),
												   root_map(make_iterator_property_map(root.begin(), get(boost::vertex_index, graph)))
														   .color_map(
																   make_iterator_property_map(color.begin(), get(boost::vertex_index, graph)))
														   .discover_time_map(make_iterator_property_map(
																   discover_time.begin(), get(boost::vertex_index, graph))));

			std::vector<StrongComponent_t> strongly_connected_components(num_components);

			for (std::vector<int>::size_type i = 0; i < component.size(); i++) {
				auto cur_component_idx = component[i];
				auto out_edges_iterator = boost::out_edges(i, graph);
				for (auto out_edge = out_edges_iterator.first; out_edge != out_edges_iterator.second; out_edge++) {
					auto out_vertex = boost::target(*out_edge, graph);
					if(i == out_vertex)
						continue;
					auto out_edge_label = graph[*out_edge].label;
					auto out_vertex_component_idx = component[out_vertex];
					if (cur_component_idx == out_vertex_component_idx) {
						strongly_connected_components[cur_component_idx].component_labels.insert(out_edge_label);
					} else {
						strongly_connected_components[cur_component_idx].outgoing_labels.insert(out_edge_label);
						strongly_connected_components[out_vertex_component_idx].incoming_labels.insert(out_edge_label);
					}
				}
			}
			StrongComponent_t independent_sc;
			for (auto scc : strongly_connected_components) {
				if (!scc.incoming_labels.size()) {
					independent_sc = scc;
					break;
				}
		    }
			return independent_sc;
		}

    };

}

#endif //SPARSETENSOR_EINSUM_UTIL_DIRECTEDGRAPH_HPP

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

    template<typename R = char>
    class DirectedGraph {

    private:

		struct LabelledEdge {
			R label;
			bool wwd;
		};

		using DirectedLabelledGraph = boost::adjacency_list<boost::vecS,
															boost::vecS,
															boost::directedS,
															boost::no_property,
															LabelledEdge>;

        using DirectedUnLabelledGraph = boost::adjacency_list<boost::vecS,
															  boost::vecS,
															  boost::directedS,
															  boost::no_property,
															  boost::no_property>;

		using Vertex = typename boost::graph_traits<DirectedLabelledGraph>::vertex_descriptor;
		using StrongComponentID = uint16_t ;
		using WeakComponentID = uint16_t ;
		static_assert(std::is_same_v<Vertex, std::size_t>);

		// stores for each vertex to which strong component it belongs
        std::vector<StrongComponentID> strong_components;

        // stores for each vertex to which strong component it belongs
        std::vector<WeakComponentID> weak_components;

		// it used to capture dependencies between operands in joins and left-joins
		DirectedLabelledGraph graph{};

		// it used to capture dependencies between sub_operators in cartesian joins
        DirectedUnLabelledGraph unlabelled_graph{};

    public:

		struct StrongComponentLabels {
			std::map<Vertex, std::set<R>> vertices_out_edges_labels{};
			std::set<R> incoming_labels{};
			std::set<R> component_labels{};
			std::set<R> outgoing_labels{};
			std::set<R> wwd_labels{};
		};

		using StrongComponentLabels_t = StrongComponentLabels;

		using WeakComponentLabels_t = std::set<R>;

        DirectedGraph() = default;

		[[maybe_unused]] Vertex addVertex() {
			boost::add_vertex(unlabelled_graph);
			return boost::add_vertex(graph);
		}

		void addEdge(R label, Vertex source, Vertex target, bool wwd = false) {
			boost::add_edge(source, target, LabelledEdge{label, wwd}, graph);
		}

		void addEdge(Vertex source, Vertex target) {
			boost::add_edge(source, target, unlabelled_graph);
		}

		std::vector<Vertex> getNeighborsLabelled(Vertex v) {
			std::vector<Vertex> target_vertices{};
            auto out_edges_iterators = boost::out_edges(v, graph);
			for(auto out_edge_iter = out_edges_iterators.first; out_edge_iter != out_edges_iterators.second; out_edge_iter++) {
				auto target = boost::target(*out_edge_iter, graph);
				if(target == v)
					continue;
				target_vertices.push_back(boost::target(*out_edge_iter, graph));
			}
			return target_vertices;
		}

        std::set<Vertex> transitivelyGetNeighborsLabelled(Vertex vertex) {
            std::set<Vertex> target_vertices{};
			std::deque<Vertex> to_check{vertex};
			std::set<Vertex> visited{};
			bool flag = true;
			while(!to_check.empty()) {
				auto v = to_check.front();
				to_check.pop_front();
				if(visited.find(v) != visited.end())
					continue;
			    visited.insert(v);
                auto out_edges_iterators = boost::out_edges(v, graph);
                for(auto out_edge_iter = out_edges_iterators.first; out_edge_iter != out_edges_iterators.second; out_edge_iter++) {
					// skip the edge if it is used to capture wwd dependencies and is an immediate neighbor
					if(graph[*out_edge_iter].wwd and flag)
						continue;
                    auto target = boost::target(*out_edge_iter, graph);
                    if(target == v)
                        continue;
                    target_vertices.insert(boost::target(*out_edge_iter, graph));
                    to_check.push_back(boost::target(*out_edge_iter, graph));
                }
				flag = false;
            }
            return target_vertices;
        }

		std::vector<Vertex> getNeighborsUnlabelled(Vertex v) {
            std::vector<Vertex> target_vertices{};
            auto out_edges_iterators = boost::out_edges(v, unlabelled_graph);
            for(auto out_edge_iter = out_edges_iterators.first; out_edge_iter != out_edges_iterators.second; out_edge_iter++) {
                auto target = boost::target(*out_edge_iter, unlabelled_graph);
                if(target == v)
                    continue;
                target_vertices.push_back(boost::target(*out_edge_iter, unlabelled_graph));
            }
            return target_vertices;
		}

        std::set<Vertex> transitivelyGetNeighborsUnlabelled(Vertex vertex) {
            std::set<Vertex> target_vertices{};
            std::deque<Vertex> to_check{vertex};
            std::set<Vertex> visited{};
            while(!to_check.empty()) {
                auto v = to_check.front();
                to_check.pop_front();
                if(visited.find(v) != visited.end())
                    continue;
                visited.insert(v);
                auto out_edges_iterators = boost::out_edges(v, unlabelled_graph);
                for(auto out_edge_iter = out_edges_iterators.first; out_edge_iter != out_edges_iterators.second; out_edge_iter++) {
                    auto target = boost::target(*out_edge_iter, unlabelled_graph);
                    if(target == v)
                        continue;
                    target_vertices.insert(boost::target(*out_edge_iter, unlabelled_graph));
                    to_check.push_back(boost::target(*out_edge_iter, unlabelled_graph));
                }
            }
            return target_vertices;
        }

		std::vector<Vertex> getStrongComponentNeighbors(Vertex v) {
			std::vector<Vertex> neighbors{};
			auto component = strong_components[v];
			for(auto n : iter::range(strong_components.size())) {
				if(n == v)
					continue;
				if(strong_components[n] == component)
					neighbors.emplace_back(n);
			}
			return neighbors;
		}

		std::vector<WeakComponentID>& getWeakComponentsOfVertices() {
			return weak_components;
		}

		// treats the directed graph as an undirected graph
		// finds the connected components of the undirected graph
		// returns the labels of each component
        [[nodiscard]] std::vector<std::set<R>> getWeaklyConnectedComponents() {

			// stores by position (Vertex) which component it belongs to (entry value)
            std::vector<WeakComponentID> component(boost::num_vertices(graph));
            auto num_components = boost::connected_components(graph, &component[0]);

			weak_components = component;
            std::vector<WeakComponentLabels_t> weakly_connected_components(num_components);

            for(std::size_t i : iter::range(component.size())) {
                auto out_edges_iterator = boost::out_edges(i, graph);
                for(auto out_edge_it = out_edges_iterator.first; out_edge_it != out_edges_iterator.second; out_edge_it++) {
					weakly_connected_components[component[i]].insert(graph[*out_edge_it].label);
				}
			}

            return weakly_connected_components;
        }

        // https://www.boost.org/doc/libs/1_74_0/libs/graph/example/strong_components.cpp
        [[nodiscard]] StrongComponentLabels_t getIndependentStrongComponent() {

			std::vector<StrongComponentID> component(num_vertices(graph)),
					discover_time(num_vertices(graph));
			std::vector<boost::default_color_type> color(num_vertices(graph));
			std::vector<Vertex> root(num_vertices(graph));
			auto num_components = boost::strong_components(graph,
												   make_iterator_property_map(component.begin(), get(boost::vertex_index, graph)),
												   root_map(make_iterator_property_map(root.begin(), get(boost::vertex_index, graph)))
														   .color_map(
																   make_iterator_property_map(color.begin(), get(boost::vertex_index, graph)))
														   .discover_time_map(make_iterator_property_map(
																   discover_time.begin(), get(boost::vertex_index, graph))));

			std::vector<StrongComponentLabels_t> strongly_connected_components(num_components);
			strong_components = component;

            for(std::size_t i : iter::range(component.size())) {
				auto cur_component_idx = component[i];
				auto out_edges_iterator = boost::out_edges(i, graph);
				strongly_connected_components[cur_component_idx].vertices_out_edges_labels[i];
				for (auto out_edge = out_edges_iterator.first; out_edge != out_edges_iterator.second; out_edge++) {
					auto out_vertex = boost::target(*out_edge, graph);
					if(i == out_vertex)
						continue;
					auto out_edge_label = graph[*out_edge].label;
                    strongly_connected_components[cur_component_idx].vertices_out_edges_labels[i].insert(out_edge_label);
					auto out_vertex_component_idx = component[out_vertex];
					if (cur_component_idx == out_vertex_component_idx) {
						strongly_connected_components[cur_component_idx].component_labels.insert(out_edge_label);
					} else {
						strongly_connected_components[cur_component_idx].outgoing_labels.insert(out_edge_label);
						strongly_connected_components[out_vertex_component_idx].incoming_labels.insert(out_edge_label);
						if(graph[*out_edge].wwd)
                            strongly_connected_components[cur_component_idx].wwd_labels.insert(out_edge_label);
					}
				}
			}
			StrongComponentLabels_t independent_sc;
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

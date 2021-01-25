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

#include <tsl/hopscotch_set.h>
#include <tsl/hopscotch_map.h>
#include <itertools.hpp>

namespace einsum::internal::util {

	/**
	 * Captures the dependencies between the operands of an Einstein Summation
	 * @tparam VertexType the type of the vertices of the graph
	 * @tparam EdgeLabel the type of the labels of the edges of the graph
	 */
    template<typename VertexType = uint32_t, typename EdgeLabel = char>
    class DependencyGraph {

    private:

		struct LabelledEdge {
			EdgeLabel label;
			bool wwd;
		};

		using DirectedLabelledGraph = boost::adjacency_list<boost::vecS,
															boost::vecS,
															boost::directedS,
															boost::no_property,
															LabelledEdge>;

        using DirectedGraph = boost::adjacency_list<boost::vecS,
													boost::vecS,
													boost::directedS,
													boost::no_property,
													boost::no_property>;

		using VertexDesc = typename boost::graph_traits<DirectedLabelledGraph>::vertex_descriptor;
		using StrongComponentID = VertexType ;
		using WeakComponentID = VertexType ;
		static_assert(std::is_same_v<VertexDesc, std::size_t>);
        using WeakComponentLabels_t = tsl::hopscotch_set<EdgeLabel>;

		// it used to capture dependencies between operands in joins and left-joins
		DirectedLabelledGraph graph{};

		// it used to capture dependencies between sub_operators in cartesian joins
		DirectedGraph unlabelled_graph{};

    public:

		bool wwd = false;

		struct StrongComponent {
			std::map<VertexDesc, tsl::hopscotch_set<EdgeLabel>> vertices_out_edges_labels{};
			tsl::hopscotch_set<EdgeLabel> incoming_labels{};
			tsl::hopscotch_set<EdgeLabel> component_labels{};
			tsl::hopscotch_set<EdgeLabel> outgoing_labels{};
			tsl::hopscotch_set<EdgeLabel> wwd_labels{};
		};

	private:

		using IndependentStrongComponents = std::vector<StrongComponent>;
        IndependentStrongComponents ind_strong_comp{};

	public:

		DependencyGraph() = default;

		void addVertex() {
			boost::add_vertex(unlabelled_graph);
			boost::add_vertex(graph);
		}

		void addEdge(EdgeLabel label, VertexDesc source, VertexDesc target, bool wwd = false) {
			if(!this->wwd and wwd)
			    this->wwd = wwd;
			boost::add_edge(source, target, LabelledEdge{label, wwd}, graph);
		}

		void addEdge(VertexDesc source, VertexDesc target) {
			boost::add_edge(source, target, unlabelled_graph);
		}

        tsl::hopscotch_set<VertexType> getTransitiveNeighbors(VertexDesc vertex) {
            tsl::hopscotch_set<VertexType> target_vertices{};
			std::deque<VertexDesc> to_check{vertex};
			tsl::hopscotch_set<VertexDesc> visited{};
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

        tsl::hopscotch_set<VertexType> getTransitiveNeighborsUnlabelled(VertexDesc vertex) {
            tsl::hopscotch_set<VertexType> target_vertices{};
            std::deque<VertexDesc> to_check{vertex};
            tsl::hopscotch_set<VertexDesc> visited{};
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

		// treats the directed graph as an undirected graph
		// finds the connected components of the undirected graph
		// returns the labels of each component
        [[nodiscard]] std::vector<WeakComponentLabels_t> getWeaklyConnectedComponentsLabels() {
			// stores by position (Vertex) which component it belongs to (entry value)
            std::vector<WeakComponentID> component(boost::num_vertices(graph));
            auto num_components = boost::connected_components(graph, &component[0]);

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
        [[nodiscard]] IndependentStrongComponents getIndependentStrongComponent() {
			if(not ind_strong_comp.empty())
				return ind_strong_comp;

			std::vector<StrongComponentID> component(num_vertices(graph)),
					discover_time(num_vertices(graph));
			std::vector<boost::default_color_type> color(num_vertices(graph));
			std::vector<VertexDesc> root(num_vertices(graph));
			auto num_components = boost::strong_components(graph,
												   make_iterator_property_map(component.begin(), get(boost::vertex_index, graph)),
												   root_map(make_iterator_property_map(root.begin(), get(boost::vertex_index, graph)))
														   .color_map(
																   make_iterator_property_map(color.begin(), get(boost::vertex_index, graph)))
														   .discover_time_map(make_iterator_property_map(
																   discover_time.begin(), get(boost::vertex_index, graph))));

			std::vector<StrongComponent> strongly_connected_components(num_components);

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
			// find all independent components - components that do not have any incoming edges
			for (auto scc : strongly_connected_components) {
				if (scc.incoming_labels.empty()) {
					ind_strong_comp.push_back(scc);
				}
		    }
			return ind_strong_comp;
		}

    };

}

#endif //SPARSETENSOR_EINSUM_UTIL_DIRECTEDGRAPH_HPP

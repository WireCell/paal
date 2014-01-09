/**
 * @file read_bounded_deg_mst.hpp
 * @brief
 * @author Piotr Godlewski
 * @version 1.0
 * @date 2013-06-10
 */
#ifndef READ_BOUNDED_DEG_MST_HPP
#define READ_BOUNDED_DEG_MST_HPP

#include <string>

namespace paal {

template <typename Graph, typename Cost>
void addEdge(Graph & g, Cost & cost, int u, int v, double c) {
    bool b;
    typename boost::graph_traits < Graph >::edge_descriptor e;
    std::tie(e, b) = add_edge(u, v, g);
    assert(b);
    cost[e] = c;
}

template <typename Graph, typename Cost, typename Bounds>
inline void readBDMST(std::istream & is, int verticesNum, int edgesNum,
                Graph & g, Cost & costs, Bounds & degBounds, double & bestCost) {
    std::string s;
    int u, v, b;
    double c;

    is >> s; is >> s; is >> s;

    for (int i = 0; i < verticesNum; i++) {
        is >> u >> b;
        degBounds[u] = b;
    }

    is >> s; is >> s; is >> s;

    for (int i = 0; i < edgesNum; i++) {
        is >> u >> v >> b >> c;
        addEdge(g, costs, u, v, c);
    }

    is >> s; is >> s;
    is >> bestCost;
}

}
#endif /* READ_BOUNDED_DEG_MST_HPP */

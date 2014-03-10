/**
 * @file bounded_degree_mst_long_test.cpp
 * @brief
 * @author Piotr Godlewski
 * @version 1.0
 * @date 2013-06-10
 */

#include "utils/logger.hpp"
#include "utils/read_bounded_deg_mst.hpp"
#include "utils/parse_file.hpp"

#include "paal/data_structures/components/components_replace.hpp"
#include "paal/iterative_rounding/iterative_rounding.hpp"
#include "paal/iterative_rounding/bounded_degree_min_spanning_tree/bounded_degree_mst.hpp"
#include "paal/utils/functors.hpp"

#include <boost/test/unit_test.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>

#include <iterator>
#include <iostream>
#include <fstream>
#include <iomanip>

typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS,
                        boost::property<boost::vertex_index_t, int>,
                        boost::property<boost::edge_weight_t, double>> Graph;
typedef boost::adjacency_list_traits<boost::vecS, boost::vecS, boost::undirectedS> Traits;
typedef boost::graph_traits<Graph>::edge_descriptor Edge;
typedef boost::graph_traits<Graph>::vertex_descriptor Vertex;

typedef boost::property_map<Graph, boost::edge_weight_t>::type Cost;
typedef std::set<Edge> ResultTree;

template <typename Bound>
void check_result(const Graph & g, const ResultTree & tree,
                 const Cost & costs, const Bound & degBounds,
                 int verticesNum, double bestCost, double treeCost) {
    int treeEdges(tree.size());
    double resultCost = std::accumulate(tree.begin(), tree.end(), 0.,
                        [&](double cost, Edge e){return cost + costs[e];});
    BOOST_CHECK_EQUAL(resultCost, treeCost);

    LOGLN("tree edges: " << treeEdges);
    BOOST_CHECK(treeEdges == verticesNum - 1);
    BOOST_CHECK(treeCost <= bestCost);

    auto verts = vertices(g);
    int numOfViolations(0);

    for (const Vertex & v : boost::make_iterator_range(verts.first, verts.second)) {
        int treeDeg(0);
        auto adjVertices = adjacent_vertices(v, g);

        for(const Vertex & u : boost::make_iterator_range(adjVertices.first, adjVertices.second)) {
            bool b; Edge e;
            std::tie(e, b) = boost::edge(v, u, g);
            assert(b);

            if (tree.count(e)) {
                ++treeDeg;
            }
        }

        BOOST_CHECK(treeDeg <= degBounds(v) + 1);
        if (treeDeg > degBounds(v)) {
            ++numOfViolations;
        }
    }

    LOGLN("Found cost = " << treeCost << ", cost upper bound = " << bestCost);
    LOGLN("Number of violated constraints = " << numOfViolations);

    Graph treeG(verticesNum);

    for (auto const & e : tree) {
        add_edge(source(e, g), target(e, g), treeG);
    }

    std::vector<int> component(verticesNum);
    BOOST_CHECK(connected_components(treeG, &component[0]) == 1);
}

template <template <typename> class Oracle, typename Bound>
void run_test(const Graph & g, const Cost & costs, const Bound & degBounds,
             const int verticesNum, const double bestCost) {
    namespace ir = paal::ir;
    {
        LOGLN("Unlimited relaxations");
        ResultTree tree;
        auto result = ir::bounded_degree_mst_iterative_rounding<
                        ir::bdmst_oracle<Oracle>>(
                            g, degBounds, std::inserter(tree, tree.end()));
        BOOST_CHECK(result.first == paal::lp::OPTIMAL);
        check_result(g, tree, costs, degBounds, verticesNum, bestCost, *(result.second));
    }
    {
        LOGLN("Relaxations limit = 1/iter");
        ResultTree tree;
        ir::bdmst_ir_components<> comps;
        auto components = paal::data_structures::replace<ir::RelaxationsLimit>(
                            ir::relaxations_limit_condition(), comps);
        auto result = ir::bounded_degree_mst_iterative_rounding<
                        ir::bdmst_oracle<Oracle>>(
                            g, degBounds, std::inserter(tree, tree.end()), components);
        BOOST_CHECK(result.first == paal::lp::OPTIMAL);
        check_result(g, tree, costs, degBounds, verticesNum, bestCost, *(result.second));
    }
}

BOOST_AUTO_TEST_CASE(bounded_degree_mst_long) {
    std::string testDir = "test/data/BOUNDED_DEGREE_MST/";
    paal::parse(testDir + "bdmst.txt", [&](const std::string & fname, std::istream & is_test_cases) {
        int verticesNum, edgesNum;
        double bestCost;
        is_test_cases >> verticesNum >> edgesNum;

        LOGLN(fname);
        std::ifstream ifs(testDir + "/cases/" + fname + ".lgf");

        Graph g(verticesNum);
        Cost costs      = get(boost::edge_weight, g);
        std::vector<int> degBounds(verticesNum);

        paal::read_bdmst(ifs, verticesNum, edgesNum, g, costs,
                        degBounds, bestCost);
        auto bounds = paal::utils::make_array_to_functor(degBounds);

        // default heuristics
        for (int i : boost::irange(0, 5)) {
            LOGLN("random violated, seed " << i);
            srand(i);
            run_test<paal::lp::random_violated_separation_oracle>(
                            g, costs, bounds, verticesNum, bestCost);
        }

        // non-default heuristics
        if (verticesNum <= 80) {
            LOGLN("most violated");
            run_test<paal::lp::most_violated_separation_oracle>(
                            g, costs, bounds, verticesNum, bestCost);
        }

        // non-default heuristics
        if (verticesNum <= 60) {
            LOGLN("first violated");
            run_test<paal::lp::first_violated_separation_oracle>(
                            g, costs, bounds, verticesNum, bestCost);
        }
    });
}

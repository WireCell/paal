/**
 * @file steiner_network_oracle.hpp
 * @brief 
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-06-24
 */
#ifndef STEINER_NETWORK_ORACLE_HPP
#define STEINER_NETWORK_ORACLE_HPP

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/boykov_kolmogorov_max_flow.hpp>
#include <boost/bimap.hpp>
#include <unordered_set>

#include "paal/iterative_rounding/steiner_network/prune_restrictions_to_tree.hpp"
#include "paal/utils/floating.hpp"
#include "paal/utils/functors.hpp"


namespace paal {
namespace ir {

struct FindMostViolated {
    template <typename Problem, typename Oracle>
    bool operator()(Problem & problem, Oracle & oracle, int restrictionsNum) {
        return oracle.findMostViolatedConstraint(problem);
    };
};

struct FindAnyViolated {
    template <typename Problem, typename Oracle>
    bool operator()(Problem & problem, Oracle & oracle, int restrictionsNum) {
        return oracle.findAnyViolatedConstraint(problem);
    };
};

struct FindRandViolated {
    template <typename Problem, typename Oracle>
    bool operator()(Problem & problem, Oracle & oracle, int restrictionsNum) {
        return oracle.findAnyViolatedConstraint(problem, rand() % restrictionsNum);
    };
};

template <typename FindViolated = FindRandViolated>
class SteinerNetworkOracleComponents {
public:
    template <typename Problem, typename Oracle>
    bool findViolated(Problem & problem, Oracle & oracle, int restrictionsNum) {
        return m_findViolated(problem, oracle, restrictionsNum);
    };

private:
    FindViolated m_findViolated;
};

template <typename Graph, typename Restrictions, typename ResultNetworkSet,
            typename OracleComponents = SteinerNetworkOracleComponents<>>
class SteinerNetworkOracle {
    typedef utils::Compare<double> Compare;
public:
    typedef typename boost::graph_traits<Graph>::edge_descriptor Edge;
    typedef typename boost::graph_traits<Graph>::vertex_descriptor Vertex;
    
    
    SteinerNetworkOracle(
            const Graph & g, 
            const Restrictions & restrictions, 
            const ResultNetworkSet & res)
             : m_g(g), 
               m_restrictions(restrictions), 
               m_restrictionsVec(pruneRestrictionsToTree(m_restrictions, num_vertices(m_g))),
               m_auxGraph(num_vertices(g)),
               m_resultNetwork(res)
               {}

    template <typename Problem>
    bool checkIfSolutionExists(Problem & problem) {
        remove_edge_if(utils::ReturnTrueFunctor(), m_auxGraph);
        m_cap = get(boost::edge_capacity, m_auxGraph);
        m_rev = get(boost::edge_reverse, m_auxGraph);

        for (auto const & e : boost::make_iterator_range(edges(m_g))) {
            Vertex u = source(e, m_g);
            Vertex v = target(e, m_g);
            addEdge(u, v, 1);
        }
        return !findAnyViolatedConstraint(problem);
    }
                           
    template <typename Problem, typename LP>
    bool feasibleSolution(Problem & problem, const LP & lp) {
        fillAuxiliaryDigraph(problem, lp);
        return !m_oracleComponents.findViolated(problem, *this, m_restrictionsVec.size());
    }
    
    template <typename Problem, typename LP>
    void addViolatedConstraint(Problem & problem, LP & lp) {
        lp.addRow(lp::LO, m_violatedRestriction);
        
        for (auto const & e : problem.getEdgeMap()) {
            const Vertex & u = source(e.second, m_g);
            const Vertex & v = target(e.second, m_g);

            if ((m_violatingSet.find(u) != m_violatingSet.end()) !=
                (m_violatingSet.find(v) != m_violatingSet.end())) {
                    lp.addNewRowCoef(e.first);
            }
        }
        
        lp.loadNewRow();
    }

    template <typename Problem>
    bool findAnyViolatedConstraint(Problem & problem, int startIndex = 0) {
        auto startIter = m_restrictionsVec.begin();
        std::advance(startIter, startIndex);

        for (auto const & src_trg : boost::make_iterator_range(startIter, m_restrictionsVec.end())) {
            assert(src_trg.first != src_trg.second);
            if (problem.getCompare().g(checkViolationBiggerThan(problem, src_trg.first, src_trg.second), 0)) {
                return true;
            }
        }

        for (auto const & src_trg : boost::make_iterator_range(m_restrictionsVec.begin(), startIter)) {
            assert(src_trg.first != src_trg.second);
            if (problem.getCompare().g(checkViolationBiggerThan(problem, src_trg.first, src_trg.second), 0)) {
                return true;
            }
        }

        return false;
    }

    template <typename Problem>
    bool findMostViolatedConstraint(Problem & problem) {
        double max = 0;

        for (auto const & src_trg : m_restrictionsVec) {
            assert(src_trg.first != src_trg.second);
            max = std::max(checkViolationBiggerThan(problem, src_trg.first, src_trg.second, max), max);
        }

        return problem.getCompare().g(max, 0);
    }

private:
    //TODO make it signed type
    typedef decltype(std::declval<Restrictions>()(0,0)) Dist;

    typedef boost::adjacency_list_traits < boost::vecS, boost::vecS, boost::directedS > Traits;
    typedef Traits::edge_descriptor AuxEdge;
    typedef Traits::vertex_descriptor AuxVertex;
    typedef boost::adjacency_list < boost::vecS, boost::vecS, boost::directedS,
                                    boost::property < boost::vertex_color_t, boost::default_color_type,
                                        boost::property < boost::vertex_distance_t, long,
                                            boost::property < boost::vertex_predecessor_t, AuxEdge >
                                                        >
                                                    >,
                                    boost::property < boost::edge_capacity_t, double,
                                        boost::property < boost::edge_residual_capacity_t, double,
                                            boost::property < boost::edge_reverse_t, AuxEdge >
                                                        >
                                                    >
                                  > AuxGraph;
    typedef std::unordered_set < AuxVertex > ViolatingSet;
                                  
    template <typename Problem, typename LP>
    void fillAuxiliaryDigraph(Problem & problem, const LP & lp) {
        remove_edge_if(utils::ReturnTrueFunctor(), m_auxGraph);
        m_cap = get(boost::edge_capacity, m_auxGraph);
        m_rev = get(boost::edge_reverse, m_auxGraph);

        for (auto const & e : problem.getEdgeMap()) {
            lp::ColId colIdx = e.first;
            double colVal = lp.getColPrim(colIdx);

            if (problem.getCompare().g(colVal, 0)) {
                Vertex u = source(e.second, m_g);
                Vertex v = target(e.second, m_g);
                addEdge(u, v, colVal);
            }
        }

        for (auto const & e : m_resultNetwork) {
            Vertex u = source(e, m_g);
            Vertex v = target(e, m_g);
            addEdge(u, v, 1);
        }
    }

    template <typename SrcVertex, typename TrgVertex>
    AuxEdge addEdge(const SrcVertex & vSrc, const TrgVertex & vTrg, double cap) {
        bool b, bRev;
        AuxEdge e, eRev;

        std::tie(e, b) = add_edge(vSrc, vTrg, m_auxGraph);
        std::tie(eRev, bRev) = add_edge(vTrg, vSrc, m_auxGraph);

        assert(b && bRev);

        m_cap[e] = cap;
        m_cap[eRev] = cap;

        m_rev[e] = eRev;
        m_rev[eRev] = e;

        return e;
    }

    template <typename Problem>
    double checkViolationBiggerThan(Problem & problem, Vertex src, Vertex trg,
                double minViolation = 0.) {
        double minCut = boost::boykov_kolmogorov_max_flow(m_auxGraph, src, trg);
        double restriction = std::max(m_restrictions(src, trg), m_restrictions(trg, src));
        double violation = restriction - minCut;

        if (problem.getCompare().g(violation, minViolation)) {
            m_violatedRestriction = restriction;
            m_violatingSet.clear();
            
            auto colors = get(boost::vertex_color, m_auxGraph);
            auto srcColor = get(colors, src);
            assert(srcColor != get(colors, trg));
            for (const Vertex & v : boost::make_iterator_range(vertices(m_auxGraph))) {
                if (v != trg && get(colors, v) == srcColor) {
                    m_violatingSet.insert(v);
                }
            }
        }

        return violation;
    }

    OracleComponents m_oracleComponents;

    const Graph & m_g;
    
    const Restrictions & m_restrictions;
    RestrictionsVector m_restrictionsVec;
    
    AuxGraph    m_auxGraph;

    ViolatingSet        m_violatingSet;
    Dist                m_violatedRestriction;
    
    const ResultNetworkSet &  m_resultNetwork;
    
    boost::property_map < AuxGraph, boost::edge_capacity_t >::type m_cap;
    boost::property_map < AuxGraph, boost::edge_reverse_t >::type  m_rev;
};

template <typename OracleComponents = SteinerNetworkOracleComponents<>,
            typename Graph, typename Restrictions, typename ResultNetworkSet>
SteinerNetworkOracle<Graph, Restrictions, ResultNetworkSet, OracleComponents>
make_SteinerNetworkSeparationOracle(const Graph & g, const Restrictions & restrictions, const ResultNetworkSet & res) {
    return SteinerNetworkOracle<Graph, Restrictions, ResultNetworkSet, OracleComponents>(g, restrictions, res);
}


} //ir
} //paal
#endif /* STEINER_NETWORK_ORACLE_HPP */

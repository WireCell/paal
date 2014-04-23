/**
 * @file steiner_tree.hpp
 * @brief
 * @author Maciej Andrejczuk
 * @version 1.0
 * @date 2013-08-01
 */
#ifndef STEINER_TREE_HPP
#define STEINER_TREE_HPP

#define BOOST_RESULT_OF_USE_DECLTYPE

#include <boost/range/join.hpp>

#include "paal/iterative_rounding/iterative_rounding.hpp"
#include "paal/iterative_rounding/ir_components.hpp"
#include "paal/lp/lp_row_generation.hpp"
#include "paal/lp/separation_oracles.hpp"
#include "paal/utils/floating.hpp"
#include "paal/data_structures/metric/basic_metrics.hpp"
#include "paal/iterative_rounding/steiner_tree/steiner_tree_oracle.hpp"
#include "paal/iterative_rounding/steiner_tree/steiner_components.hpp"
#include "paal/iterative_rounding/steiner_tree/steiner_strategy.hpp"
#include "paal/iterative_rounding/steiner_tree/steiner_utils.hpp"

namespace paal {
namespace ir {

namespace {
struct steiner_tree_compare_traits {
    static const double EPSILON;
};

const double steiner_tree_compare_traits::EPSILON = 1e-10;
}


template <template <typename> class OracleStrategy = lp::random_violated_separation_oracle>
using steiner_treeOracle = OracleStrategy<steiner_tree_violation_checker>;


/**
 * @class steiner_tree
 */
template<typename OrigMetric, typename Terminals, typename Result,
    typename Strategy=all_generator,
    typename Oracle = steiner_treeOracle<>>
class steiner_tree {
public:
    typedef data_structures::metric_traits<OrigMetric> MT;
    typedef typename MT::VertexType Vertex;
    typedef typename MT::DistanceType Dist;
    typedef typename std::pair<Vertex, Vertex> Edge;
    typedef utils::Compare<double> Compare;
    typedef data_structures::array_metric<Dist> Metric;

    /**
     * Constructor.
     */
    steiner_tree(const OrigMetric& metric, const Terminals& terminals,
            const Terminals& steinerVertices, Result result,
            const Strategy& strategy = Strategy(), Oracle oracle = Oracle()) :
        m_cost_map(metric, boost::begin(boost::range::join(terminals, steinerVertices)),
                boost::end(boost::range::join(terminals, steinerVertices))),
        m_terminals(terminals), m_steiner_vertices(steinerVertices),
        m_strategy(strategy), m_result_iterator(result),
        m_compare(steiner_tree_compare_traits::EPSILON), m_oracle(oracle) {
    }

    /**
     * Move constructor
     */
    steiner_tree(steiner_tree&& other) = default;

    /**
     * Returns the separation oracle.
     */
    Oracle & get_oracle() {
        return m_oracle;
    }

    /**
     * Generates all the components using specified strategy.
     */
    void gen_components() {
        m_strategy.gen_components(m_cost_map, m_terminals, m_steiner_vertices, m_components);
        //std::cout << "Generated: " << m_components.size() << " components\n";
    }

    /**
     * Gets reference to all the components.
     */
    const steiner_components<Vertex, Dist>& get_components() const {
        return m_components;
    }

    /**
     * Gets reference to all the terminals.
     */
    const Terminals& get_terminals() const {
        return m_terminals;
    }

    /**
     * Adds map entry from component id to LP lp::col_id.
     */
    void add_column_lp(int id, lp::col_id col) {
        bool b = m_elements_map.insert(std::make_pair(id, col)).second;
        assert(b);
    }

    /**
     * Finds LP lp::col_id based on component id.
     */
    lp::col_id find_column_lp(int id) const {
        return m_elements_map.at(id);
    }

    void add_to_solution(const std::vector<Vertex>& steinerElements) {
        std::copy(steinerElements.begin(), steinerElements.end(), m_result_iterator);
    }

    /**
     * Recalculates distances after two vertices were merged.
     */
    void merge_vertices(Vertex u, Vertex w) {
        auto allElements = boost::range::join(m_terminals, m_steiner_vertices);
        for (Vertex i: allElements) {
            for (Vertex j: allElements) {
                Dist x = m_cost_map(i, u) + m_cost_map(w, j);
                m_cost_map(i, j) = std::min(m_cost_map(i, j), x);
            }
        }
    }

    /**
     * Merges a component into its sink.
     */
    void update_graph(const steiner_component<Vertex, Dist>& selected) {
        const std::vector<Vertex>& v = selected.get_elements();
        auto allElementsExceptFirst = boost::make_iterator_range(++v.begin(), v.end());
        for (auto e : allElementsExceptFirst) {
            merge_vertices(v[0], e);
            auto ii = std::find(m_terminals.begin(), m_terminals.end(), e);
            assert(ii != m_terminals.end());
            m_terminals.erase(ii);
        }
        // Clean components, they will be generated once again
        m_components.clear();
        m_elements_map.clear();
    }

    /**
     * Gets comparison method.
     */
    utils::Compare<double> get_compare() const {
        return m_compare;
    }

private:
    Metric m_cost_map; // metric in current state
    Terminals m_terminals; // terminals in current state
    Terminals m_steiner_vertices; // vertices that are not terminals
    steiner_components<Vertex, Dist> m_components; // components in current state
    Strategy m_strategy; // strategy to generate the components
    Result m_result_iterator; // list of selected Steiner Vertices
    Compare m_compare; // comparison method

    std::unordered_map<int, lp::col_id> m_elements_map; // maps componentId -> col_id in LP
    Oracle m_oracle;
};


class steiner_tree_init {
public:
    /**
     * Initializes LP.
     */
    template <typename Problem, typename LP>
    void operator()(Problem& problem, LP & lp) {
        lp.clear();
        lp.set_lp_name("steiner tree");
        problem.gen_components();
        lp.set_min_obj_fun();
        add_variables(problem, lp);
        lp.load_matrix();
    }
private:
    /**
     * Adds all the components as columns of LP.
     */
    template <typename Problem, typename LP>
    void add_variables(Problem& problem, LP & lp) {
        for (int i = 0; i < problem.get_components().size(); ++i) {
            lp::col_id col = lp.add_column(problem.get_components().find(i).get_cost(), lp::DB, 0, 1);
            problem.add_column_lp(i, col);
        }
    }
};

/**
 * Round Condition: step of iterative-randomized rounding algorithm.
 */
class steiner_tree_round_condition {
public:
    steiner_tree_round_condition() {}

    /**
     * Selects one component according to probability, adds it to solution and merges selected vertices.
     */
    template<typename Problem, typename LP>
    void operator()(Problem& problem, LP& lp) {
        std::vector<double> weights;
        weights.reserve(problem.get_components().size());
        for (int i = 0; i < problem.get_components().size(); ++i) {
            lp::col_id cId = problem.find_column_lp(i);
            weights.push_back(lp.get_col_prim(cId));
        }
        int selected = paal::utils::random_select(weights.begin(), weights.end()) - weights.begin();
        const auto & comp = problem.get_components().find(selected);
        problem.add_to_solution(comp.get_steiner_elements());
        problem.update_graph(comp);
        steiner_tree_init()(problem, lp);
    }
};

class steiner_tree_stop_condition {
public:
    template<typename Problem, typename LP>
    bool operator()(Problem& problem, LP &) {
        return problem.get_terminals().size() < 2;
    }
};

/**
 * Makes steiner_tree object. Just to avoid providing type names in template.
 */
template<typename Oracle = steiner_treeOracle<>,
        typename OrigMetric, typename Terminals, typename Result, typename Strategy>
steiner_tree<OrigMetric, Terminals, Result, Strategy, Oracle> make_steiner_tree(
        const OrigMetric& metric, const Terminals& terminals,
        const Terminals& steinerVertices, Result result, const Strategy& strategy,
        Oracle oracle = Oracle()) {
    return steiner_tree<OrigMetric, Terminals, Result, Strategy, Oracle>(metric,
            terminals, steinerVertices, result, strategy, oracle);
}

template <
         typename SolveLPToExtremePoint = lp::row_generation_solve_lp,
         typename Resolve_lp_to_extreme_point = lp::row_generation_resolve_lp,
         typename RoundCondition = steiner_tree_round_condition,
         typename RelaxCondition = utils::always_false,
         typename StopCondition = steiner_tree_stop_condition,
         typename Init = steiner_tree_init,
         typename SetSolution = utils::skip_functor>
             using  steiner_treeIRcomponents = IRcomponents<SolveLPToExtremePoint,
                        Resolve_lp_to_extreme_point, RoundCondition,
                        RelaxCondition, Init, SetSolution, StopCondition>;


template <typename Oracle = steiner_treeOracle<>, typename Strategy = all_generator,
    typename OrigMetric, typename Terminals, typename Result,
    typename IRcomponents = steiner_treeIRcomponents<>, typename Visitor = trivial_visitor>
void steiner_tree_iterative_rounding(const OrigMetric& metric, const Terminals& terminals, const Terminals& steinerVertices,
        Result result, Strategy strategy, IRcomponents comps = IRcomponents(),
        Oracle oracle = Oracle(), Visitor vis = Visitor()) {

    auto steiner = paal::ir::make_steiner_tree(metric, terminals, steinerVertices, result, strategy, oracle);
    paal::ir::solve_dependent_iterative_rounding(steiner, std::move(comps), std::move(vis));
}

} //ir
} //paal
#endif /* STEINER_TREE_HPP */

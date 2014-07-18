/**
 * @file metric_to_bgl.hpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-02-01
 */
#define BOOST_RESULT_OF_USE_DECLTYPE

#include "paal/data_structures/bimap.hpp"
#include "paal/data_structures/metric/metric_traits.hpp"
#include "paal/data_structures/metric/metric_on_idx.hpp"
#include "paal/utils/functors.hpp"

#include <boost/graph/adjacency_matrix.hpp>
#include <boost/range/adaptor/transformed.hpp>

namespace paal {
namespace data_structures {
// TODO it would be nice to adapt Matrix + something to bgl

/**
 * @brief type of adjacency_matrix, for given metric
 *
 * @tparam Metric
 */
template <typename Metric> struct adjacency_matrix {
    typedef data_structures::metric_traits<Metric> MT;
    typedef boost::adjacency_matrix<
        boost::undirectedS, boost::no_property,
        boost::property<boost::edge_weight_t, typename MT::DistanceType>> type;
};

/**
 * @brief we assume that vertices is sequence
 * of values  (0, vertices.size()).
 *
 * @param m
 * @param vertices
 */
template <typename Metric, typename Vertices>
typename adjacency_matrix<Metric>::type metric_to_bgl(const Metric &m,
                                                      Vertices && vertices) {
    typedef typename adjacency_matrix<Metric>::type Graph;
    const unsigned N = boost::distance(vertices);
    typedef metric_traits<Metric> MT;
    typedef typename MT::DistanceType Dist;
    Graph g(N);
    for (auto && v : vertices) {
        for (auto && w : vertices) {
            if (v < w) {
                bool succ = add_edge(
                    v, w, boost::property<boost::edge_weight_t, Dist>(m(v, w)),
                    g).second;
                assert(succ);
            }
        }
    }
    return g;
}

/**
 * @brief  produces graph from metric with index
 *
 * @tparam Metric
 * @tparam Vertices
 * @param m
 * @param vertices
 * @param idx
 *
 * @return
 */
template <typename Metric, typename Vertices>
typename adjacency_matrix<Metric>::type metric_to_bgl_with_index(
    const Metric &m, Vertices && vertices,
    bimap<typename boost::range_value<Vertices>::type> &idx) {
    typedef data_structures::metric_traits<Metric> MT;
    typedef typename MT::VertexType VertexType;
    idx = data_structures::bimap<VertexType>(vertices);
    auto idxMetric = data_structures::make_metric_on_idx(m, idx);
    auto transLambda = [&](VertexType v) { return idx.get_idx(v); };
    auto trans = utils::make_assignable_functor(transLambda);

    return metric_to_bgl(idxMetric, vertices | boost::adaptors::transformed(trans));
}

} //!data_structures
} //!paal

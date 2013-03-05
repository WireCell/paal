/**
 * @file metric_traits.hpp
 * @brief 
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-03-04
 */
#ifndef METRIC_TRAITS_HPP
#define METRIC_TRAITS_HPP 
#include <type_traits>

namespace paal {
namespace data_structures {

template <typename Metric>
struct MetricTraits {
    typedef int VertexType;
    typedef decltype(std::declval<Metric>()
             (std::declval<VertexType>(), std::declval<VertexType>())) DistanceType;
};

}
}
#endif /* METRIC_TRAITS_HPP */
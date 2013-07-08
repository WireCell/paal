/**
 * @file facility_location.hpp
 * @brief 
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-02-01
 */

#include "paal/data_structures/facility_location/facility_location_solution.hpp"
#include "paal/local_search/multi_solution_step/local_search_multi_solution.hpp"

#include "facility_location_update_element.hpp"
#include "facility_location_solution_adapter.hpp"
#include "facility_location_neighbor_getter.hpp"
#include "facility_location_checker.hpp"
#include "facility_location_updater.hpp"

namespace paal {
namespace local_search {
namespace facility_location {

/**
 * @class DefaultFLComponents 
 * @brief Model of MultiSearchComponents with default multi search components for facility location.
 *
 * @tparam VertexType
 */
template <typename VertexType> 
struct DefaultRemoveFLComponents {
    typedef MultiSearchComponents<
                FacilityLocationGetNeighborhoodRemove<VertexType>,
                FacilityLocationCheckerRemove        <VertexType>,
                FacilityLocationUpdaterRemove        <VertexType>> type;
};

template <typename VertexType> 
struct DefaultAddFLComponents {
    typedef MultiSearchComponents<
                FacilityLocationGetNeighborhoodAdd<VertexType>,
                FacilityLocationCheckerAdd        <VertexType>,
                FacilityLocationUpdaterAdd        <VertexType>> type;
};

template <typename VertexType> 
struct DefaultSwapFLComponents {
    typedef MultiSearchComponents<
                FacilityLocationGetNeighborhoodSwap<VertexType>,
                FacilityLocationCheckerSwap        <VertexType>,
                FacilityLocationUpdaterSwap        <VertexType>> type;
};

/**
 * @class FacilityLocationLocalSearchStep
 * @brief this is model of LocalSearchStepMultiSolution concept. See \ref local_search.<br>
 * The Update is facility_location::Update. <br>
 * The Solution is adapted data_structures::FacilityLocationSolution. <br>
 * The SolutionElement is facility_location::Facility  <br>
 * Use DefaultFLComponents for default search components.
 *
 * The FacilityLocationLocalSearchStep takes as constructor parameter  data_structures::FacilityLocationSolution.
 * <b> WARNING </b>
 * getSolution of the FacilityLocationLocalSearchStep returns type ObjectWithCopy<FacilityLocationSolution>.
 * If you want to perform search, then change the solution object and continue local search you should perform all the operations on ObjectWithCopy. <br>
 * example: 
    \snippet facility_location_example.cpp FL Search Example
 *
 * full example is facility_location_example.cpp
 *
 * @tparam Voronoi
 * @tparam FacilityCost
 * @tparam MultiSearchComponents
 */
template <typename Voronoi,
          typename FacilityCost,
          typename... MultiSearchComponents>

class FacilityLocationLocalSearchStep : 
    public LocalSearchStepMultiSolution<
               FacilityLocationSolutionAdapter<
                    data_structures::FacilityLocationSolution<FacilityCost, Voronoi>>,
               search_strategies::ChooseFirstBetter,
               MultiSearchComponents...>  {

public:
    typedef data_structures::FacilityLocationSolution<FacilityCost, Voronoi> FLSolution;
    typedef FacilityLocationSolutionAdapter<FLSolution> FLSolutionAdapter;
    
    typedef LocalSearchStepMultiSolution<
                FLSolutionAdapter,
                search_strategies::ChooseFirstBetter,
                MultiSearchComponents...>  base;

    FacilityLocationLocalSearchStep(
            FLSolution fls,
            MultiSearchComponents... sc) :
                base(FLSolutionAdapter(std::move(fls)), 
                                       std::move(sc)...) {}
};

}
}
}

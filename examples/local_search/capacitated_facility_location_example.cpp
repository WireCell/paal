/**
 * @file capacitated_facility_location_example.cpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-02-01
 */
#include "paal/local_search/facility_location/facility_location.hpp"
#include "paal/data_structures/voronoi/capacitated_voronoi.hpp"
#include "paal/utils/functors.hpp"
#include "test/utils/sample_graph.hpp"

using namespace paal::local_search::facility_location;

int main() {
//! [CFL Search Example]
    // sample data
    typedef sample_graphs_metrics SGM;
    auto gm = SGM::get_graph_metric_small();

    std::vector<int> fcostsv{7,8};
    auto facilityCost = paal::utils::make_array_to_functor(fcostsv);

    std::vector<int> fcapv{2, 2};
    auto facilityCapacity = paal::utils::make_array_to_functor(fcapv);

    std::vector<int> cdemv{2, 2, 1, 3, 3};
    auto clientDemand = paal::utils::make_array_to_functor(cdemv);

    //define voronoi and solution
    typedef paal::data_structures::capacitated_voronoi<
        decltype(gm), decltype(facilityCapacity), decltype(clientDemand)> VorType;
    typedef paal::data_structures::facility_location_solution
        <decltype(facilityCost), VorType> Sol;
    typedef paal::data_structures::voronoi_traits<VorType> VT;
    typedef typename VT::GeneratorsSet GSet;
    typedef typename VT::VerticesSet VSet;
    typedef typename Sol::UnchosenFacilitiesSet USet;

    //create voronoi and solution
    VorType voronoi(GSet{SGM::A}, VSet{SGM::A,SGM::B,SGM::C,SGM::D,SGM::E}, gm, facilityCapacity, clientDemand);
    Sol sol(std::move(voronoi), USet{SGM::B}, facilityCost);

    //search
    facility_location_local_search_simple(sol,
            default_remove_fl_components::type(),
            default_add_fl_components::type(),
            default_swap_fl_components::type());

    //print result
    auto const & ch = sol.get_chosen_facilities();
    std::copy(ch.begin(), ch.end(), std::ostream_iterator<int>(std::cout,","));
    std::cout << std::endl;
//! [CFL Search Example]

    return 0;

}

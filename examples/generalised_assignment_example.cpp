/**
 * @file iterative_rounding_test.cpp
 * @brief 
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-02-04
 */

#define BOOST_TEST_MODULE iterative_rounding_test

#include <boost/test/unit_test.hpp>

#include <vector>

#include "paal/iterative_rounding/iterative_rounding.hpp"
#include "paal/iterative_rounding/generalised_assignment/generalised_assignment.hpp"

using namespace  paal;
using namespace  paal::ir;



BOOST_AUTO_TEST_CASE(two_local_search_choose_first_better_test) {
    //sample problem
    auto machines = boost::irange(0,2);
    auto jobs = boost::irange(0,2);

    std::vector<std::vector<int>> cost(2, std::vector<int>(2));
    cost[0][0] = 2;
    cost[0][1] = 3;
    cost[1][0] = 1;
    cost[1][1] = 3;
    auto  costf = [&](int i, int j){return cost[i][j];}; 


    std::vector<std::vector<int>> time(2, std::vector<int>(2));
    time[0][0] = 2;
    time[0][1] = 2;
    time[1][0] = 1;
    time[1][1] = 1;
    auto  timef = [&](int i, int j){return time[i][j];}; 

    std::vector<int> T = {2, 2};
    auto  Tf = [&](int i){return T[i];}; 


    //solve it
    auto ga = make_GeneralAssignment(machines.begin(), machines.end(),
                    jobs.begin(), jobs.end(), 
                    costf, timef, Tf);

    IterativeRounding<decltype(ga)> ir(std::move(ga));

    solve(ir);

    // printing result
    auto const & j2m = ir.getSolution();
    for(const std::pair<int, int> & jm : j2m) {
        std::cout << "Job " << jm.first << " assigned to Machine " << jm.second << std::endl;
    }
}

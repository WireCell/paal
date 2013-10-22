/**
 * @file knapsack_0_1_two_app_example.cpp
 * @brief 
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-09-20
 */

#include <vector>
#include <boost/range/irange.hpp>
#include "paal/greedy/knapsack_0_1_two_app.hpp"

int main() {
//! [Knapsack Example]

    std::vector<int> sizes{1,2,3,1,2,4,1,10};
    int capacity = 6;
    std::vector<int> values{3,2,65,1,2,3,1,23};
    auto objects = boost::irange(size_t(0), sizes.size());

    //Knapsack 0/1
    std::vector<int> result;
    std::cout << "Knapsack 0 / 1" << std::endl;
    auto maxValue = paal::knapsack_0_1_two_app(std::begin(objects), std::end(objects), 
            capacity,
            std::back_inserter(result), 
            paal::utils::make_ArrayToFunctor(values),
            paal::utils::make_ArrayToFunctor(sizes));

    std::cout << "Max value " << maxValue.first << ", Total size "  << maxValue.second << std::endl;
    std::copy(result.begin(), result.end(), std::ostream_iterator<int>(std::cout, " "));
    std::cout << std::endl;
//! [Knapsack Example]

    return 0;
}

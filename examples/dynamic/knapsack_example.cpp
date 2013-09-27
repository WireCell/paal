/**
 * @file knapsack_example.cpp
 * @brief 
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-09-20
 */
#ifndef KNAPSACK_CPP
#define KNAPSACK_CPP 

#include <vector>
#include <boost/range/irange.hpp>
#include "paal/dynamic/knapsack.hpp"

int main() {
//! [Knapsack Example]

    std::vector<int> sizes{1,2,3,1,2,4,1,10};
    int capacity = 6;
    std::vector<int> values{3,2,65,1,2,3,1,23};
    auto objects = boost::irange(size_t(0), sizes.size());

    //Knapsack
    {
        std::vector<int> result;
        std::cout << "Knapsack" << std::endl;
        auto maxValue = paal::knapsack(std::begin(objects), std::end(objects), 
                   capacity,
                   std::back_inserter(result), 
                   paal::utils::make_Array2Functor(sizes), 
                   paal::utils::make_Array2Functor(values));

        std::cout << "Max value " << maxValue.first << ", Total size "  << maxValue.second << std::endl;
        for(auto r : result) {
            std::cout <<  r << " " ;
        }
        std::cout << std::endl;
    }

    //Knapsack 0/1
    {
        std::vector<int> result;
        std::cout << "Knapsack 0 / 1" << std::endl;
        auto maxValue = paal::knapsack_0_1(std::begin(objects), std::end(objects), 
                   capacity,
                   std::back_inserter(result), 
                   paal::utils::make_Array2Functor(sizes), 
                   paal::utils::make_Array2Functor(values));

        std::cout << "Max value " << maxValue.first << ", Total size "  << maxValue.second << std::endl;
        for(auto r : result) {
            std::cout <<  r << " ";
        }
        std::cout << std::endl;
    }
//! [Knapsack Example]

    return 0;
}


#endif /* KNAPSACK_CPP */

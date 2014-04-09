/**
 * @file fusion_algorithms_example.cpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-02-04
 */

#include <boost/fusion/include/vector.hpp>

#include "paal/utils/fusion_algorithms.hpp"



struct minus_infinity {
    template <typename T>
    bool operator<( const T &) const {
        return true;
    }
};

struct F {
template <class Num, class AccumulatorFunctor, class AccumulatorData, class Continuation>
void operator()(Num num, AccumulatorFunctor accFunctor, AccumulatorData accData,  Continuation continuation) const
//TODO if we  use auto -> decltype(accFunctor(accData))  here instead of void, the code does not compile on g++-4.8
{
        if(accData < num) {
            auto print = [](Num n){ std::cout << n << std::endl;};
            return continuation(print, num);
        } else {
            return continuation(accFunctor, accData);
        }
    }
};

int main() {
    boost::fusion::vector<int, float, long long> v(12, 5.5f, 2ll);

    paal::data_structures::polymorfic_fold fold{};
    fold(F{}, [](minus_infinity){std::cout << "Empty Collection" << std::endl;}, minus_infinity{}, v);

}


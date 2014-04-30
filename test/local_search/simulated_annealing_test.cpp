/**
 * @file simulated_annealing_test.cpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2014-02-04
 */

#include "utils/simple_single_local_search_components.hpp"
#include "utils/logger.hpp"

#include "paal/local_search/simulated_annealing.hpp"
#include "paal/local_search/custom_components.hpp"
#include "paal/data_structures/components/components_replace.hpp"

#include <boost/test/unit_test.hpp>


BOOST_AUTO_TEST_SUITE( simulated_annealing )

namespace ls = paal::local_search;
using namespace paal;

    BOOST_AUTO_TEST_CASE(simulated_annealing_gain_adaptor_test) {
        int currentSolution(0);
        int best(0);
        auto sa_gain = ls::make_simulated_annealing_gain_adaptor(gain{},
                            ls::make_exponential_cooling_schema_dependant_on_time(std::chrono::seconds(1), 10, 0.1));

        auto record_solution_commit =
                ls::make_record_solution_commit_adapter(
                        best,
                        commit(),
                        paal::utils::make_functor_to_comparator(f));

        ls::first_improving(currentSolution,
                ls::make_search_components(get_moves{}, sa_gain, record_solution_commit));
        BOOST_CHECK_EQUAL(best, 6);
        LOGLN("solution " << best);
    }

    BOOST_AUTO_TEST_CASE(start_temperature_test) {
        double t;

        int solution{0};
        auto cooling = [&](){ return t;};
        auto set_temperature = [&](double _t){ t = _t;};
        auto sa_gain = ls::make_simulated_annealing_gain_adaptor(gain{}, cooling);

        auto get_success_rate = [&](double temp, int repeats_number = 1000)  {
            set_temperature(temp);
            get_moves gm{};

            int number_of_success = 0;
            int total_moves = 0;
            for(int i = 0; i < repeats_number; ++i) {
                for(auto move : gm(solution)) {
                    ++total_moves;
                    if(sa_gain(solution, move) > 0) {
                        ++number_of_success;
                    }
                }
            }
            return double(number_of_success) / total_moves;
        };

        //0.5 success_rate is guarantied for this model
        auto temp = ls::start_temperature(solution, sa_gain, get_moves{}, set_temperature);
        BOOST_CHECK_EQUAL(temp, 0.);

        for(auto ratio : {0.6, 0.7, 0.8, 0.9, 0.95, 0.99}) {
            int repeat = 1000.;
            temp = ls::start_temperature(solution, sa_gain, get_moves{}, set_temperature, ratio, repeat);
            auto succ_rate = get_success_rate(temp, repeat);
            BOOST_CHECK(std::abs(succ_rate - ratio)  < 0.1 );
            LOGLN("temp = " << temp << "; success rate = "
                    << succ_rate <<"; expected = " << ratio << "; error = " << std::abs(succ_rate - ratio));
        }
    }



BOOST_AUTO_TEST_SUITE_END()

#include "linear_programming/lp/lp_compilation_error_setup.hpp"

BOOST_FIXTURE_TEST_SUITE(lp_compilation_error, compilation_error_setup)

BOOST_AUTO_TEST_CASE(lp_compilation_error_3_ge_expr_ge_2_mul_Y) {
#ifdef _COMPILATION_ERROR_TEST_
    lp.add_row(3 >= expr >= 2 * Y);
#else
    lp.add_row(3 >= expr);
    lp.add_row(expr - 2 * Y >= 0);
#endif
}

BOOST_AUTO_TEST_SUITE_END()
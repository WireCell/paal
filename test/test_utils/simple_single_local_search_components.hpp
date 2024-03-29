//=======================================================================
// Copyright (c)
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file simple_single_local_search_components.hpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2014-01-07
 */
#ifndef PAAL_SIMPLE_SINGLE_LOCAL_SEARCH_COMPONENTS_HPP
#define PAAL_SIMPLE_SINGLE_LOCAL_SEARCH_COMPONENTS_HPP

#include "paal/local_search/local_search.hpp"
#include <vector>

namespace {
inline int f(int x) { return -x * x + 12 * x - 27; }

struct get_moves {
    typedef const std::vector<int> Neighb;
    Neighb neighb;

  public:

    get_moves() : neighb{ 10, -10, 1, -1 } {}

    Neighb &operator()(int x) { return neighb; }
};

struct gain {
    int operator()(int s, int u) const { return f(s + u) - f(s); }
};

struct commit {
    bool operator()(int &s, int u) {
        s = s + u;
        return true;
    }
};

typedef paal::local_search::search_components<get_moves, gain, commit>
    search_comps;
} // anonymous namespace

#endif // PAAL_SIMPLE_SINGLE_LOCAL_SEARCH_COMPONENTS_HPP

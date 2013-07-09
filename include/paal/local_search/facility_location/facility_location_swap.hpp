/**
 * @file facility_location_swap.hpp
* @brief 
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-07-08
 */
#ifndef FACILITY_LOCATION_SWAP_HPP
#define FACILITY_LOCATION_SWAP_HPP 
#include <cassert>
#include <vector>
#include <numeric>
#include <cstdlib>

#include <boost/iterator/iterator_adaptor.hpp>
#include <boost/iterator/transform_iterator.hpp>

#include "paal/data_structures/facility_location/facility_location_solution_traits.hpp"
#include "paal/utils/type_functions.hpp"
#include "paal/utils/iterator_utils.hpp"
#include "paal/local_search/facility_location/facility_location_solution_element.hpp"

namespace paal {
namespace local_search {
namespace facility_location {

template <typename T> class Swap {
public:
    Swap(T from, T to) : m_from(from), m_to(to) {}
    Swap() {}

    T getFrom() const {
        return m_from;
    }

    T getTo() const {
        return m_to;
    }
    
    void setFrom(T from) {
        m_from = from;
    }

    void setTo(T to) {
        m_to = to;
    }

private:
    T m_from;
    T m_to;
};

template <typename VertexType> class VertexToSwapUpdate {
public:
    VertexToSwapUpdate(VertexType v) : m_from(v) {}
    
    VertexToSwapUpdate() = default;
    VertexToSwapUpdate(const VertexToSwapUpdate & u) = default;
    
    VertexToSwapUpdate & operator=(const VertexToSwapUpdate & u)  { 
        m_from = u.m_from;
        return *this;
    }

    const Swap<VertexType> & operator()(VertexType v) const {
        m_sw.setFrom(m_from); 
        m_sw.setTo(v);
        return m_sw;
    }

private:
    mutable Swap<VertexType> m_sw;
    VertexType m_from;
};

template <typename VertexType> 
class FacilityLocationCheckerSwap {
public:
        template <class Solution> 
    auto operator()(const Solution & sol, 
            const  typename utils::SolToElem<Solution>::type & se,  //SolutionElement 
            const Swap<VertexType> & s) ->
                typename data_structures::FacilityLocationSolutionTraits<puretype(sol.get())>::Dist {
        auto const & FLS = sol.get();
        typedef typename std::decay<decltype(FLS)>::type::ObjectType FLS_T;
        typename data_structures::FacilityLocationSolutionTraits<puretype(sol.get())>::Dist ret, back;
        
        ret  = FLS.invokeOnCopy(&FLS_T::addFacility, s.getTo());
        ret += FLS.invokeOnCopy(&FLS_T::remFacility, s.getFrom());
        back = FLS.invokeOnCopy(&FLS_T::addFacility, s.getFrom());
        back += FLS.invokeOnCopy(&FLS_T::remFacility, s.getTo());
        assert(ret == -back);
        return -ret;
    }
};


template <typename VertexType> 
class FacilityLocationUpdaterSwap {
public:
    template <typename Solution> 
    void operator()(Solution & sol, 
            const  typename utils::SolToElem<Solution>::type & se,  //SolutionElement 
            const Swap<VertexType> & s) {
        auto & FLS = sol.get();
        typedef typename std::decay<decltype(FLS)>::type::ObjectType FLS_T;
        FLS.invoke(&FLS_T::addFacility, s.getTo());
        FLS.invoke(&FLS_T::remFacility, s.getFrom());
    }
};

template <typename VertexType> 
class FacilityLocationGetNeighborhoodSwap {
    template <typename Solution>
    struct IterType {
        typedef puretype(std::declval<Solution>().get()) InnerSol; 
        typedef puretype(std::declval<InnerSol>()->getUnchosenFacilities()) Unchosen; 
        typedef typename utils::SolToIter<Unchosen>::type UchIter; 
        typedef boost::transform_iterator<VertexToSwapUpdate<VertexType>, 
                 UchIter, const Swap<VertexType> &> TransIter;
        typedef std::pair<TransIter, TransIter> TransRange;
    };

public: 
    typedef Facility<VertexType> Fac;

    //Due to the memory optimization at one moment only one Update is valid
    template <typename Solution> typename IterType<Solution>::TransRange
    operator()(const Solution &s, const Fac & el) {
        auto const & FCS = s.get(); 
        auto e = el.getElem();

        auto const & uch = FCS->getUnchosenFacilities();

        typedef boost::transform_iterator<VertexToSwapUpdate<VertexType>, 
             decltype(uch.begin()), const Swap<VertexType> &> TransIter;

        if(el.getIsChosen() == CHOSEN) {
            //the update of CHOSEN could be swap with some unchosen
            VertexToSwapUpdate<VertexType> uchToUE(e);
            return std::make_pair(TransIter(uch.begin(), uchToUE), 
                                        TransIter(uch.end()  , uchToUE)); 
        }
        return std::make_pair(TransIter(), TransIter()); 
    }
};

} // facility_location
} // local_search
} // paal

#endif /* FACILITY_LOCATION_SWAP_HPP */
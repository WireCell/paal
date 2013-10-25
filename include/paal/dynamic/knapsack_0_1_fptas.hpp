/**
 * @file knapsack_0_1_fptas.hpp
 * @brief 
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-10-04
 */
#ifndef KNAPSACK_0_1_FPTAS_HPP
#define KNAPSACK_0_1_FPTAS_HPP 

#include "paal/dynamic/knapsack_0_1.hpp"
#include "paal/dynamic/knapsack/get_lower_bound.hpp"

namespace paal {

template <typename OutputIterator, 
          typename ObjectsIter, 
          typename ObjectSizeFunctor, 
          typename ObjectValueFunctor>
typename detail::KnapsackBase<ObjectsIter, ObjectSizeFunctor, ObjectValueFunctor>::ReturnType
knapsack_0_1_on_value_fptas(double epsilon, ObjectsIter oBegin, 
        ObjectsIter oEnd, 
        detail::FunctorOnIteratorPValue<ObjectSizeFunctor, ObjectsIter> capacity, //capacity is of size type
        OutputIterator out, 
        ObjectSizeFunctor size, 
        ObjectValueFunctor value) {
    typedef detail::KnapsackBase<ObjectsIter, ObjectSizeFunctor, ObjectValueFunctor> base;
    typedef typename base::ObjectRef ObjectRef;
    typedef typename base::ValueType ValueType;
    typedef typename base::ReturnType ReturnType;
    if(oBegin == oEnd) {
        return ReturnType();
    }
    
    double maxValue = detail::getValueLowerBound(oBegin, oEnd, capacity, value, size, detail::ZeroOneTag()); 
    auto multiplier = getMultiplier(oBegin, oEnd, epsilon, maxValue);

    if(!multiplier) {
        return knapsack_0_1(oBegin, oEnd, capacity, out, size, value);
    }
    
    ValueType realValue = ValueType();
    auto addValue = [&](ObjectRef obj){realValue += value(obj); return *out = obj;};

    auto newOut =  boost::make_function_output_iterator(addValue);
    
    auto newValue = [=](ObjectRef obj){return ValueType(double(value(obj)) * *multiplier); };

    auto reducedReturn = knapsack_0_1(oBegin, oEnd, capacity, newOut, size, newValue);
    return std::make_pair(realValue, reducedReturn.second);
}

template <typename OutputIterator, 
          typename ObjectsIter, 
          typename ObjectSizeFunctor, 
          typename ObjectValueFunctor>
typename detail::KnapsackBase<ObjectsIter, ObjectSizeFunctor, ObjectValueFunctor>::ReturnType
knapsack_0_1_on_size_fptas(double epsilon, ObjectsIter oBegin, 
        ObjectsIter oEnd, 
        detail::FunctorOnIteratorPValue<ObjectSizeFunctor, ObjectsIter> capacity, //capacity is of size type
        OutputIterator out, 
        ObjectSizeFunctor size, 
        ObjectValueFunctor value) {
    typedef detail::KnapsackBase<ObjectsIter, ObjectSizeFunctor, ObjectValueFunctor> base;
    typedef typename base::ObjectRef ObjectRef;
    typedef typename base::SizeType SizeType;
    typedef typename base::ReturnType ReturnType;
    if(oBegin == oEnd) {
        return ReturnType();
    }
    auto multiplier = getMultiplier(oBegin, oEnd, epsilon, capacity);

    if(!multiplier) {
        return knapsack_0_1(oBegin, oEnd, capacity, out, size, value);
    }
    
    SizeType realSize = SizeType();
    auto addSize = [&](ObjectRef obj){realSize += size(obj); return *out = obj;};
    
    auto newOut =  boost::make_function_output_iterator(addSize);
    
    auto newSize = [=](ObjectRef obj){return SizeType(double(size(obj)) * *multiplier); };
    auto reducedReturn = knapsack_0_1(oBegin, oEnd, SizeType(capacity / *multiplier) , newOut, newSize, value);
    return std::make_pair(reducedReturn.first, realSize);
    
}

template <typename ObjectsIter, 
          typename ObjectSizeFunctor, 
          typename ObjectValueFunctor>
typename detail::KnapsackBase<ObjectsIter, ObjectSizeFunctor, ObjectValueFunctor>::ReturnType
knapsack_0_1_no_output_on_value_fptas(double epsilon, ObjectsIter oBegin, 
        ObjectsIter oEnd, 
        detail::FunctorOnIteratorPValue<ObjectSizeFunctor, ObjectsIter> capacity, //capacity is of size type
        ObjectSizeFunctor size, 
        ObjectValueFunctor value) {
    typedef detail::KnapsackBase<ObjectsIter, ObjectSizeFunctor, ObjectValueFunctor> base;
    typedef typename base::ObjectRef ObjectRef;
    typedef typename base::ValueType ValueType;
    typedef typename base::ReturnType ReturnType;
    if(oBegin == oEnd) {
        return ReturnType();
    }
    
    double maxValue = detail::getValueLowerBound(oBegin, oEnd, capacity, value, size, detail::ZeroOneTag()); 
    auto multiplier = getMultiplier(oBegin, oEnd, epsilon, maxValue);

    if(!multiplier) {
        return knapsack_0_1_no_output(oBegin, oEnd, capacity, size, value);
    }
    
    auto newValue = [=](ObjectRef obj){return ValueType(double(value(obj)) * *multiplier); };
    auto ret = knapsack_0_1_no_output(oBegin, oEnd, capacity, size, newValue);
    return std::make_pair(ValueType(double(ret.first) / *multiplier), ret.second);
}

template <typename ObjectsIter, 
          typename ObjectSizeFunctor, 
          typename ObjectValueFunctor>
typename detail::KnapsackBase<ObjectsIter, ObjectSizeFunctor, ObjectValueFunctor>::ReturnType
knapsack_0_1_no_output_on_size_fptas(double epsilon, ObjectsIter oBegin, 
        ObjectsIter oEnd, 
        detail::FunctorOnIteratorPValue<ObjectSizeFunctor, ObjectsIter> capacity, //capacity is of size type
        ObjectSizeFunctor size, 
        ObjectValueFunctor value) {
    typedef detail::KnapsackBase<ObjectsIter, ObjectSizeFunctor, ObjectValueFunctor> base;
    typedef typename base::ObjectRef ObjectRef;
    typedef typename base::SizeType SizeType;
    typedef typename base::ReturnType ReturnType;
    if(oBegin == oEnd) {
        return ReturnType();
    }
    
    auto multiplier = getMultiplier(oBegin, oEnd, epsilon, capacity);

    if(!multiplier) {
        return knapsack_0_1_no_output(oBegin, oEnd, capacity, size, value);
    }
    
    auto newSize = [=](ObjectRef obj){return SizeType(double(size(obj)) * *multiplier); };
    auto ret = knapsack_0_1_no_output(oBegin, oEnd, SizeType(capacity / *multiplier), newSize, value);
    return ReturnType(ret.first, double(ret.second) / *multiplier);
}

} //paal


#endif /* KNAPSACK_0_1_FPTAS_HPP */

/**
 * @file local_search.hpp
 * @brief Costam
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-02-01
 */

/*! \page local_search Local Search
Index:
<ul>
    <li> \ref Preliminaries - containing preliminaries to Local Search and our interpretation of it.
    <li> LOCAL SEARCH INTERFACE - containing c++ interfaces of Local Search classes.
    <li> SPECIFIC IMPLEMENTATIONS - containing the general idea of the library.
    <li> 2-opt for TSP  - containing the implementation of the 2_local_search for TSP and basic usage of this class
</ul>


\section Preliminaries 
Let us consider maximization problem	: <br><br>

max f(x) over x in X. <br><br>

A well known heuristic for solving problem formulated in such a way is the local search(LS). <br>
Assume that we have feasible solution x' in X. <br>
The LS algorithm searches the neighborhood N(x') of x' and tries to localize the better solution x'' with f(x'') > f(x'). <br>
If the better solution is found we set x' equal x'' and rerun previous step. <br>
If better solution couldn't be found we finish the search with resulting local optimum x*. <br><br>

This algorithm can be repeated and the best local optimum is presented. <br><br>

Let us write the pseudo code for this operation:  <br><br>

<pre>
 local_search() 
 {
     x -> random_solution(X)  
     for_each(Update u in N(x)) 
     { 
         if(gain(apply u on x) > 0) 
         {
             x -> apply u on x 
         }	
     }
     return x
 }
</pre>

Note that we are working on updates (not on the full solution). This idea is going to be used in the c++ code. <br> 
The reason of this is the fact that usually the updates are much lighter than the full solutions. <br>
<br>
Note that in many cases, eg. k-local search or facility location, our solution is  the collection of the elements.<br>
In many cases for such solutions we search the neighborhood of each solution element and try to improve it by changing some part of the solution near the chosen solution element. <br>
<br>
In this cases it is more convenient to proceed in the following way:<br>

<pre>
 local_search() 
 {
     x -> random_solution(X) 
     for_each(Element e of x)
     {
      for_each(Update u in N(e)) 
      { 
         if(gain(apply u on x) > 0) 
         {
             x -> apply u on x 
         }	
      }
     }
     return x
 }
</pre>
<br>
We find this algorithm schema  extremely useful in our implementation! <br>
We will refer to this schema as LocalSearchMultiSolution. <br>
Also if necessary we will refer to the "normal" LS as LocalSearchSingleSolution.

\section local_search_interface  LOCAL SEARCH INTERFACE
Our local search is based on the LocalSearchStep concept. LocalSearchStep is a class which is responsible for one step of the local search. By the one step of the local search we anderstend one lookup of the neighborhood. The lookup is finished by one update if the better solution is found.
So the LocalSearchStep archepyte is of the form:
<pre>
   class LocalSearchStepArchetype {
       //perform one step of local search
       bool search();

       //get solution
       Solution & getSolution();
   }
</pre>

The LocalSearchStep is actually the core of the design. In the simplest variant, if we've  got th LocalSearchStep, all we have to do is to run search as long as it's  returning true.
In the more general case one can check some additional stop condition and perform some operations between the local search steps. In order to make it possible we introduce two additional concepts:
<pre>
    class PostSearchActionArchetype {
        void operator()(Solution &);
    }
</pre>
The PostSearchAction functor is invoked after each succesfull search step.
<pre>
    class GlobalStopConditionArchetype {
        bool operator()(Solution &);
    }
</pre>
The GlobalStopCondition is checked after each succesfull search step.

Now we introduce the search function interface:
<pre>
template <typename LocalSearchStep, 
          typename PostSearchAction = utils::DoNothingFunctor,
          typename GlobalStopCondition = utils::ReturnFalseFunctor>
bool search(LocalSearchStep & lss, 
            PostSearchAction psa = utils::DoNothingFunctor(),
            GlobalStopCondition gsc = utils::ReturnFalseFunctor());
</pre>

\subsection local_search_single LOCAL SEARCH SINGLE SOLUTION STEP

In order to present the local search step interface we need to introduce several concepts.<br>
Note that <i>Solution</i> is the solution type and <i>Update</i> is the type of single update. <br>
<i>UpdateIteratorsRange</i> is assumed to be std::pair of iterators, pointing to the begin and end of the updates collection. <br><br>

Concepts:
<ol>
    <li> <i>GetNeighborhood</i>  is a concept class responsible for getting the neighborhood of the current solution  
    <pre>
    GetNeighborhoodArchetype {
        UpdateIteratorsRange operator()(const Solution & s)
    }
    </li>
    </pre>
    <li> <i>Gain</i> is a concept class responsible for checking if the specific update element improve the solution.
    <pre>
    GainArchetype {
        int operator()(const Solution & s, const Update & update);
    }
    </pre>
    </li>
    <li> <i>UpdateSolution</i> is a concept class responsible for updating the solution with the Update.
    <pre>
    UpdateSolutionArchetype {
        int operator()(Solution & s, const Update & update);
    }
    </pre>
    
    </li>
    <li> <i>StopCondition</i> is a concept class responsible for stop condition.
    <pre>
    StopConditionArchetype {
        bool operator()(const Solution & s, const Update & update);
    }
    </pre>
    </li>
    <li> <i>SearchComponents</i>All of the previous concepts are grouped togheter into one class.
    <pre>
    SearchComponentsArchetype {
        GetNeighborhood & getNeighborhood();
        Gain & gain();
        UpdateSolution & updateSolution();
        StopCondition & stopCondition();
    }
    </pre>
    </li>
</ol>

Now we can introduce the paal::local_search::LocalSearchStep interface.

\subsubsection Example
full example: local_search_example.cpp

In this example we are going to maximize function -x^2 + 12x -27 for integral x. 
In this problem solution is just a integral and update is also a number which denotes the shift on the solution.
So new potential solution is just old solution plus the update.
We start with defining search components, that is:
<ol>
<li> GetNeighborhood functor </li>
<li> Gain functor </li>
<li> UpdateSolution functor </li>
</ol>
Note that we don't define StopCondition i.e. we're using default TrivialStopCondition.

\snippet local_search_example.cpp Local Search Components Example

After we've defined components we run LS.

\snippet local_search_example.cpp Local Search Example

\subsection local_search_multi LOCAL SEARCH MULTI SOLUTION 

The interface and conceptes of the  LocalSearchMultiSolution are very simmilar to the LocalSearchSingleSolution ones.<br>

<br>
Note that <i>SolutionElement</i> is the type of the specific element of the solution. <br>
<i>SolutionElementIterator</i> is assumed to be the type of iterators over solution. <br><br>

Concepts:
<ol>
    <li> <i>MultiSolution</i>  is a concept class representing the  solution  
    <pre>
    MultiSolutionArchetype  begin();
        SolutionElementIterator end();
        InnerSolution get(); // OPTIONAL, very often solution concept is just adapter containing real solution, 
                             // The inner solution type is InnerSolution
                             // If this member fuction is provided, the LocalSearchStep getSolution() returns InnerSolution.
    }
    </pre>
    <li> <i>MultiGetNeighborhood</i>  is a concept class responisble for getting the neighborhood of the current solution  
    <pre>
    MultiGetNeighborhoodArchetype {
        UpdateIteratorsRange operator()(const Solution & s, const SolutionElement &)
    }
    </pre>
    <li> <i>MultiGain</i> is a concept class responsible for checking if the specific update element improve the solution.
    <pre>
    MultiGainArchetype {
        int operator()(const Solution & s, const SolutionElement &, const Update & update);
    }
    </pre>
    <li> <i>MultiUpdateSolution</i> is a concept class responsible for updating the solution with the Update.
    <pre>
    MultiSolutionUdaterArchetype {
        int operator()(Solution & s, const SolutionElement &, const Update & update);
    }
    </pre>
    
    <li> <i>MultiStopCondition</i> is a concept class responsible for stop condition.
    <pre>
    StopCondition {
        bool operator()(const Solution & s, const SolutionElement & se, const Update & update);
    }
    </pre>
    
    <li> <i>MultiSearchComponents</i>All of the previous concepts are grouped togheter into one class.
    <pre>
    MultiSearchComponentsArchetype {
        MultiGetNeighborhood & getNeighborhood();
        MultiGain & gain();
        MultiUpdateSolution & updateSolution();
        MultiStopCondition & stopCondition();
    }
    </pre>

</ol>

Now we can introduce the paal::local_search::LocalSearchStepMultiSolution interface.

\subsubsection Example
full example: local_search_multi_solution_example.cpp

In this example we are going to maximize function<br> x1*x2 + x2*x3 + x3*x1 -3*x1*x2*x3<br> for  x1, x2, x3 in <0,1> interval.<br> 
In this problem solution is just a float vector, solution element is float and update is also a float which denotes the new value for solution element.
We start with defining search components, that is:
<ol>
<li> GetNeighborhood functor
<li> Gain functor
<li> UpdateSolution functor
</ol>
Note that we don't define StopCondition i.e. we're using default TrivialStopCondition.

\snippet local_search_multi_solution_example.cpp Local Search Components Example

After we've defined components we run LS.

\snippet local_search_multi_solution_example.cpp Local Search Example
 
*/


#include "single_solution_step/local_search_single_solution.hpp"
#include "single_solution_step/local_search_single_solution_obj_function.hpp"
#include "multi_solution_step/local_search_multi_solution.hpp"

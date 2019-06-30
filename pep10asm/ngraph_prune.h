#ifndef NGRAPH_PRUNE_H
#define NGRAPH_PRUNE_H

#include "ngraph.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <string>
#include <cmath>
#include <QDebug>
#include "setops.hpp"

// removes terminal branches from graph
//
// only works for directed graphs at the moment

template <class T>
const typename NGraph::tGraph<T>::vertex & sole_in_neighbor(const NGraph::tGraph<T> &A, const typename NGraph::tGraph<T>::vertex& v)
{
    return   *(A.in_neighbors(v).begin());
}

template <class T>
const typename NGraph::tGraph<T>::vertex & sole_in_neighbor(typename NGraph::tGraph<T>::const_iterator p)
{
    return    *(NGraph::tGraph<T>::in_neighbors(p).begin());
}

template <class T>
bool terminal_node(typename NGraph::tGraph<T>::const_iterator p)
{
    return (NGraph::tGraph<T>::out_neighbors(p).size() == 0 &&
            NGraph::tGraph<T>::in_neighbors(p).size()  != 0    );
}
template <class T>
bool leaf_node(typename NGraph::tGraph<T>::const_iterator p)
{
    return NGraph::tGraph<T>::out_neighbors(p).size() == 0  ;
}

template <class T>
bool reflexive_node(typename NGraph::tGraph<T>::const_iterator p)
{
    return (NGraph::Graph::in_neighbors(p) == NGraph::Graph::out_neighbors(p));
}

template <class T>
bool u_turn_node(typename NGraph::tGraph<T>::const_iterator p){
    const typename NGraph::tGraph<T>::vertex_set &in = NGraph::tGraph<T>::in_neighbors(p);
    const typename NGraph::tGraph<T>::vertex_set &out = NGraph::tGraph<T>::out_neighbors(p);

    return  (in.size() == 1) &&
            (out.size() ==1) &&
            (*in.begin() == *out.begin()) ;
}

template <class T>
bool single_incoming_edge(typename NGraph::tGraph<T>::const_iterator p)
{
    return (typename NGraph::tGraph<T>::in_neighbors(p).size() == 1);
}

// Returns the the graph formed by recursively removing all verticies with out degree 0.
// Only works for directed graphs.
template <class T>
NGraph::tGraph<T> reduce(const NGraph::tGraph<T>& input)
{
    typedef typename set<typename NGraph::tGraph<T>::vertex>::const_iterator vertex_set_iterator;
    // Make a copy of the input graph so that we may remove verticies from it
    NGraph::tGraph<T> workingCopy = input;
    std::set<typename NGraph::tGraph<T>::vertex> D;     // nodes to delete

    size_t previousSize = 0;
    size_t currentSize = workingCopy.num_vertices();
    // Remove any vertex from the graph with out degree 0.
    // If after two consectuive loops the graph does not shrink,
    // then some of the nodes are participating in a cycle OR
    // the graph is empty.
    while(previousSize != currentSize) {
        // Clear deletion set each time to reduce redundant deletes.
        D.clear();
        // The current number of nodes is now the number
        // the number to "beat".
        previousSize = currentSize;
        // Check each node in the graph to see if it may be deleted.
        for (typename NGraph::tGraph<T>::const_iterator p = workingCopy.begin();
            p != workingCopy.end(); p++) {
            if (leaf_node<T>(p)) {
                //std::cerr << "found terminal node: " << Graph::node(p) << "\n";
                D.insert(NGraph::tGraph<T>::node(p));
            }
        }
        // Remove all of the nodes marked for deletion.
        for (vertex_set_iterator v = D.begin();
            v != D.end(); v++) {
            //std::cerr << "About to remove vertex: " << *v << "\n";
            workingCopy.remove_vertex(*v);
        }
        currentSize = workingCopy.num_vertices();
    }
    return workingCopy;
}

#endif // NGRAPH_PRUNE_H

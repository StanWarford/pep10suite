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

template <class T>
NGraph::tGraph<T> prune(const NGraph::tGraph<T>& input)
{
    typedef typename set<typename NGraph::tGraph<T>::vertex>::const_iterator vertex_set_iterator;
    NGraph::tGraph<T> workingCopy = input;
    std::set<typename NGraph::tGraph<T>::vertex> D;     // nodes to delete

    size_t previousSize = 0;
    size_t currentSize = workingCopy.num_vertices();
    while(previousSize != currentSize) {
        D.clear();
        previousSize = currentSize;
        std::stringstream str;
        str << workingCopy;
        qDebug() << QString::fromStdString(str.str()).split("\n",QString::SplitBehavior::SkipEmptyParts);
        for (typename NGraph::tGraph<T>::const_iterator p = workingCopy.begin();
            p != workingCopy.end(); p++) {
            if (leaf_node<T>(p)) {
                //std::cerr << "found terminal node: " << Graph::node(p) << "\n";
                D.insert(NGraph::tGraph<T>::node(p));
            }
        }
        for (vertex_set_iterator v = D.begin();
            v != D.end(); v++) {
            qDebug() << "About to remove vertex: " << *v << "\n";
            workingCopy.remove_vertex(*v);
        }
        currentSize = workingCopy.num_vertices();
    }
    return workingCopy;
}

#endif // NGRAPH_PRUNE_H

#ifndef NGRAPH_PATH_H
#define NGRAPH_PATH_H
#include "ngraph.h"
#include <QtCore>
#include <list>

template <class T>
std::list<T> path(typename NGraph::tGraph<T> graph,
                  typename NGraph::tGraph<T>::vertex from,
                  typename NGraph::tGraph<T>::vertex to)
{
    // Check the case where we are trying to path to ourselves.
    if(to == from) return {to};
    // Track if we have already explored a node.
    std::vector<bool> reachable(graph.num_nodes(), false);
    // Track how we reached the node (if we have reached it)
    std::vector<T> reachedFrom(graph.num_nodes(), from);
    // The value of the node is NOT its index in the graph.
    // For example, a graph of size 3 may have the verticies labeld
    // {0, 3, 6}. This map translates from the label value to index.
    std::map<T, T> valueToIndex;

    // vertex_list returns the labels in their graph-indexed order.
    auto valueToIndexArr = graph.vertex_list();
    for(int it = 0; it < graph.num_vertices(); it++) {
        valueToIndex[valueToIndexArr[it]] = it;
    }

    // Start by checking the source vertex.
    std::list<T> toCheck {from};
    reachable[valueToIndex[from]] = true;

    // While we are still able to explore.
    while(!toCheck.empty()) {
        // Take the first item from the list
        auto at = toCheck.front();
        toCheck.pop_front();

        auto outNeighbors = graph.out_neighbors(at);
        // Iterate through all of at's neighbors
        for(auto neighbor : outNeighbors) {
            // If it hasn't already been reach, mark it as reached.
            if(reachable[valueToIndex[neighbor]] == false) {
                reachable[valueToIndex[neighbor]] = true;
                reachedFrom[valueToIndex[neighbor]] = at;
                toCheck.emplace_back(neighbor);
            }
            // If we made it to the destination vertex,
            // stop searching.
            if(neighbor == to) {
                toCheck.clear();
            }
        }

    }
    // If it was unreachable, then there is no path.
    if(reachable[valueToIndex[to]] == false ) return {};
    else {
        // Otherwise, construct a path by tracing
        // backwards through reachedFrom.
        std::list<T> retVal;
        auto at = to;
        while(at != from) {
            retVal.emplace_front(at);
            at = reachedFrom[valueToIndex[at]];
        }
        // Don't insert the source vertex, as it is implicit.
        return retVal;
    }
}


#endif // NGRAPH_PATH_H

#ifndef OSRM_CONTRACTOR_SEARCH_HPP
#define OSRM_CONTRACTOR_SEARCH_HPP

#include "contractor/contractor_heap.hpp"
#include "util/typedefs.hpp"

#include <cstddef>

namespace osrm
{
namespace contractor
{

namespace detail
{
template <typename GraphT>
void relaxNode(ContractorHeap &heap,
               const GraphT &graph,
               const NodeID node,
               const EdgeWeight node_weight,
               const NodeID forbidden_node)
{
    const short current_hop = heap.GetData(node).hop + 1;
    for (auto edge : graph.GetAdjacentEdgeRange(node))
    {
        const auto &data = graph.GetEdgeData(edge);
        if (!data.forward)
        {
            continue;
        }
        const NodeID to = graph.GetTarget(edge);
        if (forbidden_node == to)
        {
            continue;
        }
        const EdgeWeight to_weight = node_weight + data.weight;

        // New Node discovered -> Add to Heap + Node Info Storage
        if (!heap.WasInserted(to))
        {
            heap.Insert(to, to_weight, ContractorHeapData{current_hop, false});
        }
        // Found a shorter Path -> Update weight
        else if (to_weight < heap.GetKey(to))
        {
            heap.DecreaseKey(to, to_weight);
            heap.GetData(to).hop = current_hop;
        }
    }
}
}

template <typename GraphT>
void search(ContractorHeap &heap,
            const GraphT &graph,
            const unsigned number_of_targets,
            const int node_limit,
            const EdgeWeight weight_limit,
            const NodeID forbidden_node)
{
    int nodes = 0;
    unsigned number_of_targets_found = 0;
    while (!heap.Empty())
    {
        const NodeID node = heap.DeleteMin();
        const auto node_weight = heap.GetKey(node);
        if (++nodes > node_limit)
        {
            return;
        }
        if (node_weight > weight_limit)
        {
            return;
        }

        // Destination settled?
        if (heap.GetData(node).target)
        {
            ++number_of_targets_found;
            if (number_of_targets_found >= number_of_targets)
            {
                return;
            }
        }

        detail::relaxNode(heap, graph, node, node_weight, forbidden_node);
    }
}

} // namespace contractor
} // namespace osrm

#endif // OSRM_CONTRACTOR_DIJKSTRA_HPP
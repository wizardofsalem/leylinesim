# Flow solver reading list

The intended solver distributes flow through directed, capacity-limited edges.
Positive node supply produces flow, negative supply defines sink demand, and
zero-supply nodes are junctions. Sinks have strict priority by original demand
magnitude: satisfy larger demands as much as possible before smaller ones.

## Suggested reading order

1. **Maximum flow and conservation**
   - [Google OR-Tools: Maximum Flow](https://developers.google.com/optimization/flow/maxflow)
   - Learn edge capacities, source/sink limits, and conservation at junctions.
   - Includes C++ examples.

2. **Residual networks, augmenting paths, and Edmonds–Karp**
   - Study these concepts after the maximum-flow introduction above.
   - BFS finds an augmenting path; its smallest residual capacity limits how
     much additional flow can be sent.
   - Reverse residual edges undo earlier routing decisions. They are solver
     bookkeeping, not permission for actual flow against an original edge.
   - Start with Edmonds–Karp for understanding; study Dinic afterward if
     profiling shows that a faster solver is needed.

3. **Multiple sources and sinks**
   - Learn the super-source/super-sink transformation: extra edges encode
     source supply and sink demand as capacities.
   - Ordinary maximum flow maximizes total delivery; it does not by itself
     guarantee the strict sink priorities required here.

4. **Lexicographic flow**
   - [Dexter Kozen: Lexicographic Flow (Cornell PDF)](https://www.cs.cornell.edu/kozen/Papers/LexFlow.pdf)
   - Maximize the highest-priority allocation, then the next while preserving
     earlier allocations.
   - Preserve how much each higher-priority sink receives, but allow its route
     to change. Permanently locking routes can block otherwise feasible flow.
   - Strict priority is different from proportional sharing or max-min fairness.

## TDD progression

The named stubs and detailed scenarios are in `tests/FlowTest.cpp`.

1. One source and one sink: supply and demand limits.
2. An edge whose capacity is below demand.
3. Flow through a zero-supply junction.
4. Unreachable sinks and edges pointing the wrong way.
5. Multiple paths feeding one sink.
6. Multiple sources feeding one sink.
7. Competing sinks with insufficient supply: largest demand wins.
8. A capacity-limited priority sink leaves supply for smaller sinks.
9. Rerouting preserves higher-priority allocations while serving another sink.
10. Repeated solves do not accumulate old flow.

Also check conservation, edge capacity limits, source supply limits, and sink
demand limits. Where multiple routings are valid, assert delivery totals and
invariants rather than requiring one exact route.

## Allocation bookkeeping

Keep configured `supply` unchanged during a solve. A temporary
`std::vector<float>` indexed by node `LeyID` can track nonnegative allocations:
source production used, sink demand met, and zero for junctions. Initialize it
to zero for each solve. This assumes node IDs match indices in `graph_`.

Account for flow being undone during residual rerouting. If a source or sink
also passes flow through, its production/consumption is net outgoing/incoming
flow, not the raw total on just one side.

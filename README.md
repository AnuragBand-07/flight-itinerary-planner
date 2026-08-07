# Multicriteria Flight Itinerary Planner

A command-line flight itinerary planner in C++ that models a real-world airline dataset
as a weighted directed graph and answers two kinds of route queries:

- **Fewest layovers** — shortest path by number of hops (BFS).
- **Cheapest route** — minimum-fare path (Dijkstra with a binary-heap priority queue).

The graph is built from a real dataset of **10,683 flights** across major Indian cities.

## How it works

```
processed_data.csv
        │
        ▼
   parseCSV()      → vector<Flight>            (quoted-field-aware CSV parsing)
        │
        ▼
  buildGraph()
    ├─ CityEncoder: "Delhi" ↔ 0, "Mumbai" ↔ 1, …   (string ⇄ integer node IDs)
    └─ adjacency list: adj[i] = list of (neighborId, price)
        │                with parallel-edge dedup (keep the minimum fare)
        ▼
   query: source, destination, mode
        │
        ▼
   BFS (fewest hops)  /  Dijkstra (cheapest)  → path of node IDs
        │
        ▼
   decode IDs back to city names → print route
```

### Design highlights

- **String-to-int city encoder** — cities are mapped to integer IDs so the graph can use
  a `vector<vector<...>>` adjacency list (fast index access) instead of hashing strings on
  every edge traversal.
- **Parallel-edge deduplication** — when the dataset has multiple flights for the same
  `(source, destination)` pair, only the minimum-fare edge is kept.
- **Shared graph for both algorithms** — BFS and Dijkstra run over the exact same
  adjacency list; BFS simply ignores edge weights.

## Complexity

| Query               | Algorithm                     | Time            |
| ------------------- | ----------------------------- | --------------- |
| Fewest layovers     | BFS                           | O(V + E)        |
| Cheapest route      | Dijkstra (binary heap)        | O((V + E) log V)|
| Graph construction  | single pass over all flights  | O(N)            |

*V = cities, E = distinct routes, N = flights in the dataset.*

## Build & run

```bash
g++ -O2 -std=c++17 -o flight_planner flight_planner.cpp
./flight_planner        # reads processed_data.csv from the current directory
```

Then follow the prompts: enter a source city, a destination city, and choose
`1` (fewest layovers) or `2` (cheapest route).

### Example

```
Enter source city (or 'quit'): Banglore
Enter destination city: Delhi
Query type:
  1 - Minimum layovers (BFS)
  2 - Cheapest route   (Dijkstra)
Choice: 2

[Cheapest Route — Dijkstra]
Route:      Banglore -> Delhi
Total Cost: Rs. 3897
Layovers:   0
```

## Dataset

`processed_data.csv` — 10,683 rows of Indian domestic flights with columns for airline,
source, destination, date/time, and price. Prices are in INR.

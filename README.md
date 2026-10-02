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
./flight_planner --demo   # scripted queries, no keyboard input
./flight_planner          # interactive; reads processed_data.csv from the current directory
```

`make demo` does the same build and runs the scripted demo.

City names are case-sensitive and must match the dataset. The file spells Bengaluru as `Banglore`.

### Demo output

`./flight_planner --demo` loads the dataset, lists the cities, then prints the queries below. A screen recording of this run is at
[`demo/demo.mp4`](demo/demo.mp4)
([direct link](https://github.com/AnuragBand-07/flight-itinerary-planner/raw/main/demo/demo.mp4)).

```
--- Demo query 1: cheapest direct route (Dijkstra) ---
Banglore -> Delhi

[Cheapest Route — Dijkstra]
Route:      Banglore -> Delhi
Total Cost: Rs. 3257
Layovers:   0

--- Demo query 2: fewest layovers (BFS) ---
Banglore -> Cochin

[Minimum Layovers — BFS]
Route:    Banglore -> Delhi -> Cochin
Layovers: 1

--- Demo query 3: same trip, cheapest fare (Dijkstra) ---
Banglore -> Cochin

[Cheapest Route — Dijkstra]
Route:      Banglore -> Delhi -> Cochin
Total Cost: Rs. 7133
Layovers:   1

--- Demo query 4: unreachable pair ---
Mumbai -> Delhi

No route found.

--- Demo query 5: longer itinerary (Dijkstra) ---
Chennai -> Cochin

[Cheapest Route — Dijkstra]
Route:      Chennai -> Kolkata -> Banglore -> Delhi -> Cochin
Total Cost: Rs. 13758
Layovers:   3
```

The fare on a city pair is the minimum price in the dataset for that pair, so
Banglore → Delhi is Rs. 3257, not the fare of the first matching row.

## Dataset

`processed_data.csv` — 10,683 rows of Indian domestic flights with columns for airline,
source, destination, date/time, and price. Prices are in INR. The graph has 7 cities
and 5 directed routes; several city pairs are only reachable through layovers, and
some (for example Mumbai → Delhi) have no route.

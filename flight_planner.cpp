#include <bits/stdc++.h>
using namespace std;

// ============================================================
// ALGORITHM CLASS (BFS + Dijkstra on integer adjacency list)
// ============================================================
class algo {
public:
    // Dijkstra: Cheapest Flight (Minimum Cost)
    // Returns path as vector of node IDs, empty if unreachable.
    vector<int> dijkstra(int n, vector<vector<pair<int,int>>>& adj, int start, int dest) {
        vector<int> dist(n, INT_MAX);
        vector<int> parent(n, -1);

        // Min-heap: {cost, node} — custom comparator via greater<>
        priority_queue<pair<int,int>, vector<pair<int,int>>, greater<pair<int,int>>> pq;

        dist[start] = 0;
        pq.push({0, start});

        while (!pq.empty()) {
            auto [currCost, node] = pq.top(); pq.pop();

            // Stale entry: skip if we already found a cheaper path
            if (currCost > dist[node]) continue;
            if (node == dest) break;

            for (auto [neighbor, cost] : adj[node]) {
                if (dist[node] + cost < dist[neighbor]) {
                    dist[neighbor]   = dist[node] + cost;
                    parent[neighbor] = node;
                    pq.push({dist[neighbor], neighbor});
                }
            }
        }

        if (dist[dest] == INT_MAX) return {};

        vector<int> path;
        for (int cur = dest; cur != -1; cur = parent[cur])
            path.push_back(cur);
        reverse(path.begin(), path.end());
        return path;
    }

    // BFS: Fewest Layovers (Minimum Hops, ignores weights)
    // Returns path as vector of node IDs, empty if unreachable.
    vector<int> bfs(int n, vector<vector<pair<int,int>>>& adj, int start, int dest) {
        vector<int>  parent(n, -1);
        vector<bool> visited(n, false);
        queue<int>   q;

        visited[start] = true;
        q.push(start);

        bool found = false;
        while (!q.empty()) {
            int node = q.front(); q.pop();
            if (node == dest) { found = true; break; }

            for (auto [neighbor, cost] : adj[node]) {
                if (!visited[neighbor]) {
                    visited[neighbor] = true;
                    parent[neighbor]  = node;
                    q.push(neighbor);
                }
            }
        }

        if (!found) return {};

        vector<int> path;
        for (int cur = dest; cur != -1; cur = parent[cur])
            path.push_back(cur);
        reverse(path.begin(), path.end());
        return path;
    }
};

// ============================================================
// CITY ENCODER: city name <-> integer ID
// ============================================================
class CityEncoder {
private:
    unordered_map<string,int> cityToId;
    vector<string>            idToCity;
public:
    // Returns existing ID or assigns a new one
    int encode(const string& city) {
        auto it = cityToId.find(city);
        if (it != cityToId.end()) return it->second;
        int id = (int)idToCity.size();
        cityToId[city] = id;
        idToCity.push_back(city);
        return id;
    }

    string decode(int id) const { return idToCity.at(id); }
    bool   has(const string& city) const { return cityToId.count(city); }
    int    getId(const string& city) const { return cityToId.at(city); }
    int    size() const { return (int)idToCity.size(); }

};

// ============================================================
// CSV PARSER
// CSV columns (0-based):
//   0:Airline 1:Airline_encoded 2:Source 3:Source_encoded
//   4:Destination 5:Destination_encoded 6:Date 7:Month 8:Year
//   9:Hour 10:Minute 11:Price
// ============================================================
struct Flight {
    string airline, source, destination;
    int price;
};

static vector<string> splitCSV(const string& line) {
    vector<string> tokens;
    string token;
    bool inQuotes = false;
    for (char c : line) {
        if (c == '"') { inQuotes = !inQuotes; }
        else if (c == ',' && !inQuotes) { tokens.push_back(token); token.clear(); }
        else { token += c; }
    }
    tokens.push_back(token);
    return tokens;
}

static vector<Flight> parseCSV(const string& filepath) {
    ifstream file(filepath);
    if (!file.is_open()) throw runtime_error("Cannot open file: " + filepath);

    vector<Flight> flights;
    string line;
    getline(file, line); // skip header

    while (getline(file, line)) {
        if (line.empty()) continue;
        auto t = splitCSV(line);
        if ((int)t.size() < 12) continue;
        try {
            flights.push_back({t[0], t[2], t[4], stoi(t[11])});
        } catch (...) { continue; }
    }
    return flights;
}

// ============================================================
// GRAPH BUILDER
// For duplicate (src, dst) pairs keeps the minimum price edge.
// ============================================================
static void buildGraph(const vector<Flight>& flights,
                       CityEncoder& encoder,
                       vector<vector<pair<int,int>>>& adj)
{
    // First pass: register all cities
    for (const auto& f : flights) {
        encoder.encode(f.source);
        encoder.encode(f.destination);
    }

    int n = encoder.size();
    adj.assign(n, {});

    // key = srcId * 1000 + dstId  ->  index in adj[srcId]
    unordered_map<int,int> edgeIndex;

    for (const auto& f : flights) {
        int src = encoder.getId(f.source);
        int dst = encoder.getId(f.destination);
        int key = src * 1000 + dst;

        auto it = edgeIndex.find(key);
        if (it == edgeIndex.end()) {
            edgeIndex[key] = (int)adj[src].size();
            adj[src].push_back({dst, f.price});
        } else {
            // Keep minimum price for this city pair
            adj[src][it->second].second = min(adj[src][it->second].second, f.price);
        }
    }
}

// ============================================================
// MAIN — CLI input loop
// Query format:
//   Line 1: source destination
//   Line 2: 1 (min layovers/BFS)  or  2 (cheapest/Dijkstra)
// ============================================================
int main() {
    string csvPath = "processed_data.csv";
    if (!ifstream(csvPath).is_open()) {
        cout << "Enter path to processed_data.csv: ";
        getline(cin, csvPath);
    }

    cout << "Loading flight data...\n";
    vector<Flight> flights;
    try {
        flights = parseCSV(csvPath);
    } catch (const exception& ex) {
        cerr << "Error: " << ex.what() << "\n";
        return 1;
    }
    cout << "Loaded " << flights.size() << " flights.\n";

    CityEncoder encoder;
    vector<vector<pair<int,int>>> adj;
    buildGraph(flights, encoder, adj);

    int n = encoder.size();
    cout << "Graph built with " << n << " cities:\n";
    for (int i = 0; i < n; ++i)
        cout << "  [" << i << "] " << encoder.decode(i) << "\n";
    cout << "\n";

    algo routingEngine;

    while (true) {
        cout << "========================================\n";
        cout << "  Multicriteria Flight Itinerary Planner\n";
        cout << "========================================\n";
        cout << "Enter source city (or 'quit'): ";
        string src;
        getline(cin, src);
        if (src == "quit" || src == "q") break;

        cout << "Enter destination city: ";
        string dst;
        getline(cin, dst);

        if (!encoder.has(src)) { cout << "Unknown city: " << src << "\n\n"; continue; }
        if (!encoder.has(dst)) { cout << "Unknown city: " << dst << "\n\n"; continue; }
        if (src == dst)        { cout << "Source and destination are the same.\n\n"; continue; }

        cout << "Query type:\n"
             << "  1 - Minimum layovers (BFS)\n"
             << "  2 - Cheapest route   (Dijkstra)\n"
             << "Choice: ";
        string choice;
        getline(cin, choice);

        int srcId = encoder.getId(src);
        int dstId = encoder.getId(dst);

        if (choice == "1") {
            vector<int> path = routingEngine.bfs(n, adj, srcId, dstId);
            if (path.empty()) {
                cout << "No route found.\n";
            } else {
                cout << "\n[Minimum Layovers — BFS]\n";
                cout << "Route:    ";
                for (int i = 0; i < (int)path.size(); ++i) {
                    cout << encoder.decode(path[i]);
                    if (i + 1 < (int)path.size()) cout << " -> ";
                }
                int layovers = (int)path.size() - 2;
                cout << "\nLayovers: " << max(0, layovers) << "\n";
            }
        } else if (choice == "2") {
            vector<int> path = routingEngine.dijkstra(n, adj, srcId, dstId);
            if (path.empty()) {
                cout << "No route found.\n";
            } else {
                // Recompute total cost from path
                int totalCost = 0;
                for (int i = 0; i + 1 < (int)path.size(); ++i) {
                    int u = path[i], v = path[i+1];
                    for (auto [nb, cost] : adj[u])
                        if (nb == v) { totalCost += cost; break; }
                }
                cout << "\n[Cheapest Route — Dijkstra]\n";
                cout << "Route:      ";
                for (int i = 0; i < (int)path.size(); ++i) {
                    cout << encoder.decode(path[i]);
                    if (i + 1 < (int)path.size()) cout << " -> ";
                }
                int layovers = (int)path.size() - 2;
                cout << "\nTotal Cost: Rs. " << totalCost << "\n";
                cout << "Layovers:   " << max(0, layovers) << "\n";
            }
        } else {
            cout << "Invalid choice.\n";
        }
        cout << "\n";
    }

    cout << "Goodbye!\n";
    return 0;
}

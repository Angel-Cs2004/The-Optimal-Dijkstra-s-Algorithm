#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <utility>
#include <queue>
#include <limits>
#include <algorithm>

using namespace std;

const bool DIRIGIDO = false;  // cambiar según el enunciado
const long long INF = numeric_limits<long long>::max();

using Grafo = vector<vector<pair<int, long long>>>;

void dijkstra(const Grafo& adj, int s, int t, vector<long long>& dist, vector<int>& padre) {
    int n = adj.size();
    dist.assign(n, INF);
    padre.assign(n, -1);
    vector<bool> visitado(n, false);

    // min-heap de (distancia, vértice)
    priority_queue<pair<long long, int>, vector<pair<long long, int>>, greater<>> pq;

    dist[s] = 0;
    pq.push({0, s});

    while (!pq.empty()) {
        auto [d, u] = pq.top();
        pq.pop();

        if (visitado[u]) continue;  // entrada obsoleta
        visitado[u] = true;

        if (u == t) break;  // distancia al destino ya es definitiva

        for (auto [v, w] : adj[u]) {
            if (dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                padre[v] = u;
                pq.push({dist[v], v});
            }
        }
    }
}

vector<int> reconstruirCamino(const vector<int>& padre, int t) {
    vector<int> camino;
    for (int x = t; x != -1; x = padre[x]) camino.push_back(x);
    reverse(camino.begin(), camino.end());
    return camino;
}

int main(int argc, char* argv[]) {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

// lectura del TXT
    ifstream archivo;
    if (argc > 1) {
        archivo.open(argv[1]);
        if (!archivo) { cerr << "No se pudo abrir " << argv[1] << '\n'; return 1; }
        cin.rdbuf(archivo.rdbuf());
    }

// <<===COMIENZO===>>
    int n, m;
    cin >> n >> m;

    int sId, tId; // source & target
    cin >> sId >> tId;

    unordered_map<int, int> index_table; // ID original -> índice 0..n-1
    Grafo adj(n);
    vector<int> idOriginal(n);           // índice 0..n-1 -> ID original

  //====================================================

    for (int i = 0; i < n; ++i) {
        cin >> idOriginal[i];
        index_table[idOriginal[i]] = i;
    }

    for (int i = 0; i < m; ++i) {
        int uId, vId;
        long long w;
        cin >> uId >> vId >> w;

        if (!index_table.count(uId) || !index_table.count(vId)) {
            cerr << "Arista con vertice desconocido: " << uId << ' ' << vId << '\n';
            return 1;
        }
        if (w < 0) {
            cerr << "Peso negativo en arista " << uId << ' ' << vId << '\n';
            return 1;
        }

        int u = index_table[uId], v = index_table[vId];
        adj[u].push_back({v, w});
        if (!DIRIGIDO) adj[v].push_back({u, w});
    }

    if (!index_table.count(sId) || !index_table.count(tId)) {
        cerr << "Origen o destino no existe\n";
        return 1;
    }
    int s = index_table[sId];
    int t = index_table[tId];

  //====================================================

    vector<long long> dist;
    vector<int> padre;
    dijkstra(adj, s, t, dist, padre);

    if (dist[t] == INF) {
        cout << "No existe camino de " << sId << " a " << tId << '\n';
        return 0;
    }

    cout << "Costo minimo: " << dist[t] << '\n';
    cout << "Camino: ";
    vector<int> camino = reconstruirCamino(padre, t);
    for (size_t i = 0; i < camino.size(); ++i) {
        if (i) cout << " -> ";
        cout << idOriginal[camino[i]];
    }
    cout << '\n';

    return 0;
}

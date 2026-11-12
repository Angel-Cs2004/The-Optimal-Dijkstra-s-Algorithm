#include <cstdio>
#include <vector>
#include <queue>
#include <limits>
#include <algorithm>
#include <utility>
#include <chrono>

using namespace std;
using Reloj = chrono::steady_clock;

const size_t MAX_CAMINO_IMPRESO = 30;  // caminos más largos solo muestran su cantidad de vértices

double microsegundos(Reloj::time_point a, Reloj::time_point b) {
    return chrono::duration<double, micro>(b - a).count();
}

const bool DIRIGIDO = false; 
const long long INF = numeric_limits<long long>::max();

// ===================== LECTURA DEL ARCHIVO=====================

static vector<char> buffer;
static size_t posBuf = 0;

void cargarEntrada(FILE* f) {
    const size_t BLOQUE = 1 << 16;
    size_t total = 0;
    while (true) {
        buffer.resize(total + BLOQUE);
        size_t leidos = fread(buffer.data() + total, 1, BLOQUE, f);
        total += leidos;
        if (leidos < BLOQUE) break;
    }
    buffer.resize(total);
}

long long leerEntero() {
    size_t n = buffer.size();
    while (posBuf < n && buffer[posBuf] != '-' && (buffer[posBuf] < '0' || buffer[posBuf] > '9')) ++posBuf;

    bool negativo = false;
    if (posBuf < n && buffer[posBuf] == '-') { negativo = true; ++posBuf; }

    long long x = 0;
    while (posBuf < n && buffer[posBuf] >= '0' && buffer[posBuf] <= '9')
        x = x * 10 + (buffer[posBuf++] - '0');
    return negativo ? -x : x;
}

// ===================== ID ORIGINAL -> ÍNDICE =====================

struct Indice {
    bool directo = false;
    long long minId = 0;
    vector<int> tabla;                 // modo directo: tabla[id - minId] = índice (o -1)
    vector<pair<int, int>> ordenados;  // modo binario: (id, índice) ordenados

    void construir(const vector<int>& idOriginal) {
        int n = idOriginal.size();
        if (n == 0) return;
        long long mn = *min_element(idOriginal.begin(), idOriginal.end());
        long long mx = *max_element(idOriginal.begin(), idOriginal.end());
        long long rango = mx - mn + 1;

        if (rango <= 4LL * n + 1024) {
            directo = true;
            minId = mn;
            tabla.assign(rango, -1);
            for (int i = 0; i < n; ++i) tabla[idOriginal[i] - mn] = i;  // si se repite un ID, gana el último (igual que antes)
        } else {
            ordenados.resize(n);
            for (int i = 0; i < n; ++i) ordenados[i] = {idOriginal[i], i};
            sort(ordenados.begin(), ordenados.end());
        }
    }

    // Devuelve el índice interno, o -1 si el ID no existe
    int buscar(long long id) const {
        if (directo) {
            long long k = id - minId;
            if (k < 0 || k >= (long long)tabla.size()) return -1;
            return tabla[k];
        }
        // upper_bound - 1: si el ID está repetido, toma el último (igual que antes)
        auto it = upper_bound(ordenados.begin(), ordenados.end(),
                              make_pair((int)id, numeric_limits<int>::max()));
        if (it == ordenados.begin() || prev(it)->first != id) return -1;
        return prev(it)->second;
    }
};

// ===================== GRAFO EN FORMATO CSR =====================

struct GrafoCSR {
    int n = 0;
    vector<int> inicio;
    vector<int> destino;
    vector<long long> peso;
};

GrafoCSR construirCSR(int n, const vector<int>& eu, const vector<int>& ev, const vector<long long>& ew) {
    GrafoCSR g;
    g.n = n;
    int m = eu.size();

    g.inicio.assign(n + 1, 0);
    for (int i = 0; i < m; ++i) {
        g.inicio[eu[i] + 1]++;
        if (!DIRIGIDO) g.inicio[ev[i] + 1]++;
    }
    for (int i = 0; i < n; ++i) g.inicio[i + 1] += g.inicio[i];

    g.destino.resize(g.inicio[n]);
    g.peso.resize(g.inicio[n]);
    vector<int> pos(g.inicio.begin(), g.inicio.end() - 1);

    // Se llena en el mismo orden que la lectura -> mismo orden de vecinos que la versión original
    for (int i = 0; i < m; ++i) {
        int u = eu[i], v = ev[i];
        g.destino[pos[u]] = v; g.peso[pos[u]++] = ew[i];
        if (!DIRIGIDO) { g.destino[pos[v]] = u; g.peso[pos[v]++] = ew[i]; }
    }
    return g;
}

// ===================== DIJKSTRA =====================

void dijkstra(const GrafoCSR& g, int s, int t, vector<long long>& dist, vector<int>& padre) {
    dist.assign(g.n, INF);
    padre.assign(g.n, -1);

    // min-heap de (distancia, vértice)
    priority_queue<pair<long long, int>, vector<pair<long long, int>>, greater<>> pq;

    dist[s] = 0;
    pq.push({0, s});

    while (!pq.empty()) {
        auto [d, u] = pq.top();
        pq.pop();

        if (d > dist[u]) continue;  // entrada obsoleta (reemplaza al arreglo visitado)
        if (u == t) break;          // distancia al destino ya es definitiva

        for (int k = g.inicio[u]; k < g.inicio[u + 1]; ++k) {
            int v = g.destino[k];
            long long nd = d + g.peso[k];
            if (nd < dist[v]) {
                dist[v] = nd;
                padre[v] = u;
                pq.push({nd, v});
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

// ===================== MAIN =====================

int main(int argc, char* argv[]) {
    // lectura del TXT
    if (argc < 2) {
        fprintf(stderr, "Uso: %s archivo.txt\n", argv[0]);
        return 1;
    }

    auto tInicio = Reloj::now();  // inicio de lectura + construcción del grafo

    FILE* f = fopen(argv[1], "rb");
    if (!f) { fprintf(stderr, "No se pudo abrir %s\n", argv[1]); return 1; }
    cargarEntrada(f);
    fclose(f);

    int n = leerEntero();
    int m = leerEntero();

    int sId = leerEntero();  // source
    int tId = leerEntero();  // target

    vector<int> idOriginal(n);  // índice 0..n-1 -> ID original
    for (int i = 0; i < n; ++i) idOriginal[i] = leerEntero();

    Indice indice;              // ID original -> índice 0..n-1
    indice.construir(idOriginal);

    vector<int> eu(m), ev(m);
    vector<long long> ew(m);
    for (int i = 0; i < m; ++i) {
        int uId = leerEntero();
        int vId = leerEntero();
        long long w = leerEntero();

        int u = indice.buscar(uId), v = indice.buscar(vId);
        if (u == -1 || v == -1) {
            fprintf(stderr, "Arista con vertice desconocido: %d %d\n", uId, vId);
            return 1;
        }
        if (w < 0) {
            fprintf(stderr, "Peso negativo en arista %d %d\n", uId, vId);
            return 1;
        }
        eu[i] = u; ev[i] = v; ew[i] = w;
    }

    int s = indice.buscar(sId);
    int t = indice.buscar(tId);
    if (s == -1 || t == -1) {
        fprintf(stderr, "Origen o destino no existe\n");
        return 1;
    }

    GrafoCSR g = construirCSR(n, eu, ev, ew);

    auto tLectura = Reloj::now();  // fin de lectura, inicio de Dijkstra

    vector<long long> dist;
    vector<int> padre;
    dijkstra(g, s, t, dist, padre);

    auto tFin = Reloj::now();      // fin de Dijkstra

    // ----- resultado -----
    if (dist[t] == INF) {
        printf("No existe camino de %d a %d\n", sId, tId);
    } else {
        printf("Distancia minima de %d a %d: %lld\n", sId, tId, dist[t]);

        vector<int> camino = reconstruirCamino(padre, t);
        if (camino.size() <= MAX_CAMINO_IMPRESO) {
            printf("Camino: ");
            for (size_t i = 0; i < camino.size(); ++i) {
                if (i) printf(" -> ");
                printf("%d", idOriginal[camino[i]]);
            }
            printf("\n");
        } else {
            printf("Camino: %zu vertices (demasiado largo para mostrarlo)\n", camino.size());
        }
    }

    // ----- tiempos -----
    printf("---------------------------\n");
    printf("Tiempo de lectura : %.3f µs\n", microsegundos(tInicio, tLectura));
    printf("Tiempo de Dijkstra: %.3f µs\n", microsegundos(tLectura, tFin));
    printf("Tiempo total      : %.3f µs\n", microsegundos(tInicio, tFin));

    return 0;
}

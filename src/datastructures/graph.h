#ifndef SHAREDMAP_GRAPH_H
#define SHAREDMAP_GRAPH_H

#include <fstream>
#include <regex>
#include <vector>

#include "src/utility/definitions.h"
#include "src/utility/macros.h"
#include "src/utility/utils.h"

namespace SharedMap {
    struct Edge {
        u64 v;
        u64 w;

        Edge(u64 v, u64 w) {
            this->v = v;
            this->w = w;
        }

        bool operator<(const Edge& y) const {
            return v < y.v;
        }
    };

    /**
    * Standard undirected Graph that can hold vertex and edge weights.
    */
    class Graph {
    private:
        u64 m_n; // number of vertices
        u64 m_m; // number of edges

        // edges per vertex
        std::vector<u64> m_v_edges;

        // vertex weight
        std::vector<u64> m_v_weights;
        u64 m_g_weight;

        // adjacency and edge weights
        std::vector<std::vector<Edge>> m_adj;

    public:
        /**
         * Reads in a graph, that is in METIS format.
         *
         * @param file_path Path to the file.
         */
        explicit Graph(const std::string& file_path) {
            if (!file_exists(file_path)) {
                std::cerr << "File " << file_path << " does not exist!" << std::endl;
                exit(EXIT_FAILURE);
            }

            bool has_v_weights = false;
            bool has_e_weights = false;
            u64 expected_edges = 0;

            std::ifstream file(file_path);
            std::string line(64, ' ');
            if (file.is_open()) {
                while (std::getline(file, line)) {
                    if (line[0] == '%') { continue; }

                    // remove leading and trailing whitespaces, replace double whitespaces
                    line.erase(0, line.find_first_not_of(' ')).erase(line.find_last_not_of(' ') + 1);
                    line = std::regex_replace(line, std::regex("\\s{2,}"), " ");

                    // read in header
                    std::vector<std::string> header = split(line, ' ');
                    m_n                             = std::stoi(header[0]);
                    m_m                             = 0;
                    expected_edges                  = std::stoi(header[1]);

                    m_v_edges.resize(m_n, 0);
                    m_v_weights.resize(m_n, 1);
                    m_g_weight = m_n;
                    m_adj.resize(m_n);

                    std::string fmt = "000";
                    if (header.size() == 3 && header[2].size() == 3) {
                        fmt = header[2];
                    }

                    has_v_weights = fmt[1] == '1';
                    has_e_weights = fmt[2] == '1';

                    break;
                }

                // read in edges
                u64 u = 0;
                std::vector<u64> ints;

                if (has_v_weights) {
                    if (has_e_weights) {
                        // v_weights and e_weights
                        while (std::getline(file, line)) {
                            if (line[0] == '%') { continue; }
                            // remove leading and trailing whitespaces, replace double whitespaces

                            line_to_ints(line, ints);
                            set_vertex_weight(u, ints[0]);

                            for (u64 i = 1; i < ints.size(); i += 2) {
                                u64 v = ints[i] - 1;
                                u64 w = ints[i + 1];
                                add_edge_if_not_exist(u, v, w);
                            }

                            u += 1;
                        }
                    } else {
                        // v_weights and no e_weights
                        while (std::getline(file, line)) {
                            if (line[0] == '%') { continue; }
                            // remove leading and trailing whitespaces, replace double whitespaces

                            line_to_ints(line, ints);
                            set_vertex_weight(u, ints[0]);

                            for (u64 i = 1; i < ints.size(); ++i) {
                                u64 v = ints[i] - 1;
                                add_edge_if_not_exist(u, v);
                            }

                            u += 1;
                        }
                    }
                } else {
                    if (has_e_weights) {
                        // no v_weights and e_weights
                        while (std::getline(file, line)) {
                            if (line[0] == '%') { continue; }
                            // remove leading and trailing whitespaces, replace double whitespaces

                            line_to_ints(line, ints);
                            for (u64 i = 0; i < ints.size(); i += 2) {
                                u64 v = ints[i] - 1;
                                u64 w = ints[i + 1];
                                add_edge_if_not_exist(u, v, w);
                            }

                            u += 1;
                        }
                    } else {
                        // no v_weights and no e_weights
                        while (std::getline(file, line)) {
                            if (line[0] == '%') { continue; }
                            // remove leading and trailing whitespaces, replace double whitespaces

                            line_to_ints(line, ints);
                            for (u64 i = 0; i < ints.size(); ++i) {
                                u64 v = ints[i] - 1;
                                add_edge_if_not_exist(u, v);
                            }

                            u += 1;
                        }
                    }
                }

                if (expected_edges != m_m) {
                    std::cerr << "Number of expected edges " << expected_edges << " not equal to number edges " << m_m << " found!" << std::endl;
                    exit(EXIT_FAILURE);
                }
            } else {
                std::cerr << "Could not open file " << file_path << "!" << std::endl;
                exit(EXIT_FAILURE);
            }
        }

        /**
         * Constructor for the graph.
         *
         * @param n Number of vertices.
         * @param has_vertex_w Whether the graph has vertex weights.
         * @param has_edge_w Whether the graph has edge weights.
         */
        explicit Graph(u64 n) {
            m_n = n;
            m_m = 0;

            m_v_edges.resize(m_n, 0);
            m_v_weights.resize(m_n, 1);
            m_g_weight = m_n;
            m_adj.resize(m_n);
        }

        /**
         * Sets the vertex weight of vertex u.
         *
         * @param u The vertex.
         * @param weight The weight.
         */
        void set_vertex_weight(u64 u, u64 weight = 1) {
            ASSERT(u < m_n);
            m_g_weight     = m_g_weight - m_v_weights[u] + weight;
            m_v_weights[u] = weight;
        }

        /**
         * Adds an edge.
         *
         * @param u Vertex u.
         * @param v Vertex v.
         * @param weight The weight of the edge.
         */
        void add_edge(u64 u, u64 v, u64 weight = 1) {
            ASSERT(u < m_n);
            ASSERT(v < m_n);
            ASSERT(weight > 0);
            ASSERT(!edge_exists(u, v));

            u64 min = std::min(u, v);
            u64 max = std::max(u, v);

            m_adj[min].emplace_back(max, weight);
            m_v_edges[min] += 1;
            m_v_edges[max] += 1;
            m_m += 1;

            ASSERT(m_adj[min].size() < m_n);
        }

        /**
         * Adds an edge if it does not exist.
         *
         * @param u Vertex u.
         * @param v Vertex v.
         * @param weight The weight of the edge.
         */
        void add_edge_if_not_exist(u64 u, u64 v, u64 weight = 1) {
            ASSERT(u < m_n);
            ASSERT(v < m_n);
            ASSERT(weight > 0);

            u64 min = std::min(u, v);
            u64 max = std::max(u, v);
            Edge e(max, weight);

            // Find the position where the new item should be inserted
            auto it = std::lower_bound(m_adj[min].begin(), m_adj[min].end(), e);

            // Check if the item is already present
            if (it == m_adj[min].end() || it->v != e.v) {
                // Item is not present, insert it at the position found
                m_adj[min].insert(it, e);

                m_v_edges[min] += 1;
                m_v_edges[max] += 1;
                m_m += 1;
            }

            ASSERT(m_adj[min].size() < m_n);
        }

        /**
         * Returns the number of vertices.
         *
         * @return Number of vertices.
         */
        u64 get_n() const {
            return m_n;
        }

        /**
         * Returns the number of edges.
         *
         * @return Number of edges.
         */
        u64 get_m() const {
            return m_m;
        }

        /**
         * Determines if an edge exists between u and v in the graph.
         *
         * @param u Vertex u.
         * @param v Vertex v.
         * @return True if the edge exists, false else.
         */
        bool edge_exists(u64 u, u64 v) const {
            ASSERT(u < m_n);
            ASSERT(v < m_n);

            u64 min = std::min(u, v);
            u64 max = std::max(u, v);

            for (const Edge& e : m_adj[min]) {
                if (e.v == max) {
                    return true;
                }
            }
            return false;
        }

        /**
         * Returns the number of edges of vertex u.
         *
         * @param u The vertex.
         * @return Number of edges.
         */
        u64 get_vertex_n_edge(u64 u) const {
            ASSERT(m_n == m_v_edges.size());
            ASSERT(u < m_n);
            return m_v_edges[u];
        }

        /**
         * Returns the weight of vertex u.
         *
         * @param u The vertex.
         * @return The weight.
         */
        u64 get_vertex_weight(u64 u) const {
            ASSERT(m_n == m_v_weights.size());
            ASSERT(u < m_n);
            return m_v_weights[u];
        }

        /**
         * Returns the sum of vertex weights.
         *
         * @return The sum of all vertex weights.
         */
        u64 get_weight() const {
            ASSERT(sum<u64>(m_v_weights) == m_g_weight);
            return m_g_weight;
        }

        /**
         * Returns the weight of edge between vertices u and v. Undefined
         * behaviour if edge does not exist.
         *
         * @param u The vertex u.
         * @param v The vertex v.
         * @return The weight.
         */
        u64 get_edge_weight(u64 u, u64 v) const {
            ASSERT(u < m_n);
            ASSERT(v < m_n);
            ASSERT(edge_exists(u, v));

            u64 min = std::min(u, v);
            u64 max = std::max(u, v);

            for (auto& e : m_adj[min]) {
                if (e.v == max) {
                    return e.w;
                }
            }
            // unreachable
            abort();
        }

        /**
         * Returns the sum of vertex weights.
         *
         * @return The sum of all vertex weights.
         */
        u64 get_sum_edge_weights() const {
            u64 edge_weights = 0;
            for (u64 u = 0; u < m_n; ++u) {
                for (const Edge& e : m_adj[u]) {
                    edge_weights += e.w;
                }
            }
            return edge_weights;
        }

        /**
         * Get the adjacency of vertex u.
         *
         * @param u The vertex.
         * @return Reference to the adjacency.
         */
        std::vector<Edge>& operator[](u64 u) {
            ASSERT(u < m_n);

            return m_adj[u];
        }

        /**
         * Get the adjacency of vertex u.
         *
         * @param u The vertex.
         * @return Reference to the adjacency.
         */
        const std::vector<Edge>& operator[](u64 u) const {
            ASSERT(u < m_n);

            return m_adj[u];
        }

        /**
         * Used for asserting that the graph is correctly set.
         */
        void assert_graph() const {
#if ASSERT_ENABLED
            ASSERT(m_n == m_adj.size()); // correct number of adjacencies

            u64 n_edges = 0;
            for (u64 u = 0; u < m_n; ++u) {
                ASSERT(m_adj[u].size() < m_n);
                ASSERT(m_v_edges[u] < m_n);
                for (u64 i = 0; i < m_adj[u].size(); ++i) {
                    for (u64 j = i + 1; j < m_adj[u].size(); ++j) {
                        ASSERT(m_adj[u][i].v != m_adj[u][j].v); // no duplicates
                    }
                }

                n_edges += m_adj[u].size();
            }

            ASSERT(m_m == n_edges); // correct number of edges
#endif
        }

        void write_metis_graph(const std::string& file_path) const {
            std::stringstream ss;
            ss << m_n << " " << m_m << " 011" << std::endl;
            for (u64 u = 0; u < m_n; ++u) {
                ss << m_v_weights[u] << " ";
                for (auto& e : m_adj[u]) {
                    ss << e.v + 1 << " " << e.w << " ";
                }
                ss << std::endl;
            }

            std::ofstream file(file_path);
            file << ss.rdbuf();
            file.close();
        };
    };
}

#endif //SHAREDMAP_GRAPH_H

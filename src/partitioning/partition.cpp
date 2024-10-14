#include "partition.h"

namespace SharedMap {

    void greedy_partition(const Graph &g,
                          const u64 k,
                          const f64 imbalance,
                          std::vector<u64> &partition) {
        ASSERT(imbalance >= 0.0);
        ASSERT(k > 0);

        // enough space
        partition.resize(g.get_n());

        u64 total_weight = g.get_weight();
        u64 avg_weight = (u64) ((f64) total_weight / (f64) k);
        u64 max_allowed_weight = (u64) ((1.0 + imbalance) * (f64) avg_weight);

        // vectors to store each partition
        std::vector<std::vector<u64>> sets(k, std::vector<u64>{});
        std::vector<u64> set_weight(k, 0);

        // assign each vertex to the set with the smallest weight currently
        for (u64 u = 0; u < g.get_n(); ++u) {
            u64 best_set = 0;
            u64 min_weight = set_weight[0];

            for (u64 i = 0; i < k; ++i) {
                if (set_weight[i] < min_weight) {
                    best_set = i;
                    min_weight = set_weight[i];
                }
            }

            sets[best_set].push_back(u);
            set_weight[best_set] += g.get_vertex_weight(u);
        }

        // move vertices, such that a balanced partition arises
        u64 max_tries = 1000;
        u64 n_tries = 0;
        while (max(set_weight) > max_allowed_weight && n_tries < max_tries) {
            n_tries += 1;
            // get the set with most and minimal weight
            u64 max_set = 0;
            u64 max_weight = set_weight[0];
            u64 min_set = 0;
            u64 min_weight = set_weight[0];

            for (u64 i = 0; i < k; ++i) {
                if (set_weight[i] > max_weight) {
                    max_set = i;
                    max_weight = set_weight[i];
                }
                if (set_weight[i] < min_weight) {
                    min_set = i;
                    min_weight = set_weight[i];
                }
            }

            // move a vertex from max to min
            for (u64 i = 0; i < sets[max_set].size(); ++i) {
                u64 u = sets[max_set][i];
                u64 u_weight = g.get_vertex_weight(u);

                bool min_set_overloaded = (set_weight[min_set] + u_weight) > max_allowed_weight;

                if (!min_set_overloaded) {
                    // move the vertex
                    sets[min_weight].push_back(u);
                    std::swap(sets[max_weight][i], sets[max_weight].back());
                    sets[max_weight].pop_back();

                    sets[min_weight][i] = u;

                    set_weight[max_set] = set_weight[max_set] - u_weight;
                    set_weight[min_set] = set_weight[min_set] + u_weight;

                    // terminate the loop
                    n_tries = 0;
                    break;
                }
            }
        }

        // enter the partition
        for (u64 i = 0; i < sets.size(); ++i) {
            for (u64 j = 0; j < sets[i].size(); ++j) {
                partition[sets[i][j]] = i;
            }
        }
    }


    void kaffpa_partition(const Graph &g,
                          const u64 k,
                          const f64 imbalance,
                          std::vector<u64> &partition,
                          const u64 kaffpa_config) {
        ASSERT(imbalance >= 0.0);
        ASSERT(k > 0);

        // enough space
        partition.resize(g.get_n());

        // number of vertices and edges
        int n = (int) g.get_n();
        int m = (int) g.get_m();

        // vertex weights
        int *v_weights = (int *) malloc(n * sizeof(int));
        for (int i = 0; i < n; ++i) { v_weights[i] = (int) g.get_vertex_weight(i); }

        // pointer to adjacency lists
        int *adj_ptr = (int *) malloc((n + 1) * sizeof(int));
        int *adj = (int *) malloc(2 * m * sizeof(int));
        int *e_weights = (int *) malloc(2 * m * sizeof(int));

        int *insert_idx = (int *) malloc((n + 1) * sizeof(int));

        // set adj_ptr
        adj_ptr[0] = 0;
        insert_idx[0] = 0;
        for (int u = 0; u < n; ++u) {
            adj_ptr[u + 1] = adj_ptr[u] + (int) g.get_vertex_n_edge(u);
            insert_idx[u + 1] = insert_idx[u] + (int) g.get_vertex_n_edge(u);
        }

        // adjacency and edge weights
        for (int u = 0; u < n; ++u) { // process each vertex
            for (auto &e: g[u]) {
                adj[insert_idx[u]] = (int) e.v;
                e_weights[insert_idx[u]] = (int) e.w;
                insert_idx[u]++;

                adj[insert_idx[e.v]] = u;
                e_weights[insert_idx[e.v]] = (int) e.w;
                insert_idx[e.v]++;
            }
        }
        free(insert_idx);

        // number of partitions
        int n_partitions = (int) k;

        // imbalance
        double kaffpa_imbalance = imbalance;

        // edge cut
        int edge_cut;

        // partition result
        int *kaffpa_partition = (int *) malloc(n * sizeof(int));

        // mode
        int mode;
        switch (kaffpa_config) {
            case KAFFPA_STRONG:
                mode = STRONG;
                break;
            case KAFFPA_ECO:
                mode = ECO;
                break;
            case KAFFPA_FAST:
                mode = FAST;
                break;
            default:
                std::cerr << "Kaffpa Config " << kaffpa_config << " not known!" << std::endl;
                abort();
        }

        // execute kaffpa
        kaffpa(&n, v_weights, adj_ptr, e_weights, adj, &n_partitions, &kaffpa_imbalance, true, 0, mode, &edge_cut, kaffpa_partition);

        // get result
        for (int i = 0; i < n; ++i) {
            partition[i] = kaffpa_partition[i];
        }

        free(v_weights);
        free(adj_ptr);
        free(e_weights);
        free(adj);
        free(kaffpa_partition);
    }

    void mt_kahypar_partition_via_filesystem(const Graph &g,
                                             const u64 k,
                                             const f64 imbalance,
                                             std::vector<u64> &partition,
                                             const u64 mt_kahypar_config,
                                             const u64 n_threads) {
        ASSERT(imbalance >= 0.0);
        ASSERT(k > 0);
        g.assert_graph();

        const u64 id = ((u64) std::rand() << 32) | std::rand();


        // make temp directory
        std::string dir = "temp_" + std::to_string(id);
        if (!std::filesystem::is_directory(dir)) {
            std::filesystem::create_directory(dir);
        }

        // write the graph to a file
        std::string filename = "subgraph.graph";
        g.write_metis_graph(dir + "/" + filename);

        // let Mt-KaHyPar solve the graph
        std::string exe = "../extern/mt-kahypar/build/mt-kahypar/application/MtKaHyPar";
        std::string graph = " -h " + dir + "/" + filename + " --instance-type=graph --input-file-format=metis";
        std::string preset = " --preset-type=";
        if (mt_kahypar_config == MTKAHYPAR_DEFAULT) {
            preset += "default";
        } else if (mt_kahypar_config == MTKAHYPAR_QUALITY) {
            preset += "quality";
        } else if (mt_kahypar_config == MTKAHYPAR_HIGHEST_QUALITY) {
            preset += "highest_quality";
        } else {
            std::cerr << "Mt-KaHyPar Config " << mt_kahypar_config << " not known!" << std::endl;
            abort();
        }
        std::string threads = " -t " + std::to_string(n_threads);
        std::string k_str = " -k " + std::to_string(k);
        std::string e = " -e " + std::to_string(imbalance);
        std::string metric = " -o cut --seed=0";
        std::string partition_str = " --write-partition-file=true --partition-output-folder=" + dir;

        std::string cmd = exe + graph + preset + threads + k_str + e + metric + partition_str + " > /dev/null 2>&1";
        system(cmd.c_str());

        // enough space
        partition.resize(g.get_n());

        // read the partition
        std::string partition_file_name_start = dir + "/" + filename + ".part" + std::to_string(k) + ".epsilon";
        std::string partition_file_name_end = ".seed0.KaHyPar";
        std::string partition_file;

        // Iterate over the directory
        for (const auto &entry: std::filesystem::directory_iterator(dir)) {
            // Check if the entry is a file and matches the pattern
            if (entry.is_regular_file() && startsWith(entry.path(), partition_file_name_start) && endsWith(entry.path(), partition_file_name_end)) {
                partition_file = entry.path();
                break;
            }
        }

        std::ifstream file(partition_file);
        if (!file.is_open()) {
            std::cerr << "Unable to open file: " << partition_file << std::endl;
            exit(EXIT_FAILURE);
        }

        std::string line;
        u64 i = 0;
        while (std::getline(file, line)) {
            if (line.back() == '\n') {
                line.pop_back();
            }
            partition[i] = std::stoi(line);
            i += 1;
        }

        file.close();

        // remove folder
        std::filesystem::remove_all(dir);
    }

    void mt_kahypar_partition(const Graph &g,
                              const u64 k,
                              const f64 imbalance,
                              std::vector<u64> &partition,
                              const u64 mt_kahypar_config,
                              const u64 n_threads) {
        // mt_kahypar_partition_via_filesystem(g, k, imbalance, partition, mt_kahypar_config, n_threads);
        // return;

        ASSERT(imbalance >= 0.0);
        ASSERT(k > 0);
        g.assert_graph();

        // enough space
        partition.resize(g.get_n());

        // Initialize thread pool
        mt_kahypar_initialize_thread_pool(
                n_threads,
                true); // activate interleaved NUMA allocation policy

        // Setup partitioning context
        mt_kahypar_context_t *context = mt_kahypar_context_new();
        mt_kahypar_preset_type_t preset;
        switch (mt_kahypar_config) {
            case MTKAHYPAR_DEFAULT:
                preset = DEFAULT;
                break;
            case MTKAHYPAR_QUALITY:
                preset = QUALITY;
                break;
            case MTKAHYPAR_HIGHEST_QUALITY:
                preset = HIGHEST_QUALITY;
                break;
            default:
                std::cerr << "Mt-KaHyPar Config " << mt_kahypar_config << " not known!" << std::endl;
                abort();
        }
        mt_kahypar_load_preset(context, preset);
        // In the following, we partition a hypergraph into two blocks
        // with an allowed imbalance of 3% and optimize the connective metric (KM1)
        mt_kahypar_set_partitioning_parameters(context,
                                               (int) k /* number of blocks */,
                                               imbalance /* imbalance parameter */,
                                               CUT /* objective function */);

        // Enable logging
        mt_kahypar_set_context_parameter(context, VERBOSE, "0");

        // number of vertices and edges
        auto n = (mt_kahypar_hypernode_id_t) g.get_n();
        auto m = (mt_kahypar_hyperedge_id_t) g.get_m();

        // vertex weights
        auto *v_weights = (mt_kahypar_hypernode_weight_t *) malloc(n * sizeof(mt_kahypar_hypernode_weight_t));
        for (u64 i = 0; i < n; ++i) {
            v_weights[i] = (mt_kahypar_hypernode_weight_t) g.get_vertex_weight(i);
        }

        // edges
        u64 idx = 0;
        auto *edges = (mt_kahypar_hypernode_id_t *) malloc(2 * m * sizeof(mt_kahypar_hypernode_id_t));
        auto *e_weights = (mt_kahypar_hyperedge_weight_t *) malloc(m * sizeof(mt_kahypar_hyperedge_weight_t));
        for (u64 u = 0; u < n; ++u) {
            for (auto &e: g[u]) {
                edges[(2 * idx)] = (mt_kahypar_hypernode_id_t) u;
                edges[(2 * idx) + 1] = (mt_kahypar_hypernode_id_t) e.v;
                e_weights[idx] = (mt_kahypar_hyperedge_weight_t) e.w;
                idx += 1;
                ASSERT(idx <= m);
            }
        }

        // Create graph
        mt_kahypar_hypergraph_t graph = mt_kahypar_create_graph(preset, n, m, edges, e_weights, v_weights);

        // Partition graph
        mt_kahypar_partitioned_hypergraph_t partitioned_hg = mt_kahypar_partition(graph, context);

        // Extract Partition
        std::unique_ptr<mt_kahypar_partition_id_t[]> mt_kahypar_partition = std::make_unique<mt_kahypar_partition_id_t[]>(mt_kahypar_num_hypernodes(graph));
        mt_kahypar_get_partition(partitioned_hg, mt_kahypar_partition.get());

        // move partition
        for (u64 i = 0; i < n; ++i) {
            partition[i] = mt_kahypar_partition[i];
            ASSERT(partition[i] < k);
        }

        free(v_weights);
        free(edges);
        free(e_weights);
        mt_kahypar_free_context(context);
        mt_kahypar_free_hypergraph(graph);
        mt_kahypar_free_partitioned_hypergraph(partitioned_hg);
    }
}

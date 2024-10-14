#ifndef SHAREDMAP_SOLVER_H
#define SHAREDMAP_SOLVER_H

#include "src/utility/definitions.h"
#include "src/utility/macros.h"
#include "src/utility/utils.h"
#include "src/utility/JSON_utils.h"
#include "src/utility/algorithm_configuration.h"
#include "graph.h"
#include "item.h"
#include "translation_table.h"
#include "src/partitioning/serial.h"
#include "src/profiling/stat_collector.h"
#include "src/utility/context.h"

namespace SharedMap {
    class Solver {
    private:
        const AlgorithmConfiguration& m_ac;
        StatCollector stat_collector;

        f64 io_time = 0.0;
        f64 solve_time = 0.0;

    public:
        explicit Solver(AlgorithmConfiguration& ac) : m_ac(ac) {
            stat_collector.initialize(m_ac.hierarchy.size());
        }

        void solve() {
            // read graph
            auto sp = std::chrono::steady_clock::now();
            Graph g(m_ac.graph_in);
            auto ep = std::chrono::steady_clock::now();
            io_time += (f64) std::chrono::duration_cast<std::chrono::nanoseconds>(ep - sp).count() / 1e9;

            // solve problem
            sp = std::chrono::steady_clock::now();
            std::vector<u64> partition = internal_solve(g);
            ep = std::chrono::steady_clock::now();
            solve_time += (f64) std::chrono::duration_cast<std::chrono::nanoseconds>(ep - sp).count() / 1e9;

            // write output
            sp = std::chrono::steady_clock::now();
            // write_solution(partition);
            ep = std::chrono::steady_clock::now();
            io_time += (f64) std::chrono::duration_cast<std::chrono::nanoseconds>(ep - sp).count() / 1e9;

            write_statistics();
        }

    private:
        std::vector<u64> internal_solve(const Graph &g) {
            if (m_ac.n_threads == 1) {
                return solve_serial(g, m_ac, stat_collector);
            }

            switch (m_ac.parallel_strategy_id) {
            case SERIAL:
                return solve_serial(g, m_ac, stat_collector);
            case GRAPH_PARALLEL:
                return solve_naive(g);
            case LAYER:
                return solve_layer(g);
            case QUEUE:
                return solve_queue(g);
            case LAYER_NB:
                return solve_layer_nb(g);
            default:
                std::cerr << "Strategy ID " << m_ac.parallel_strategy_id << " not recognized!" << std::endl;
                abort();
            }
        }

        std::vector<u64> solve_naive(const Graph &g) {
            std::vector<u64> partition(g.get_n());

            return partition;
        }

        std::vector<u64> solve_layer(const Graph &g) {
            std::vector<u64> partition(g.get_n());

            return partition;
        }

        std::vector<u64> solve_queue(const Graph &g) {
            std::vector<u64> partition(g.get_n());

            return partition;
        }

        std::vector<u64> solve_layer_nb(const Graph &g) {
            std::vector<u64> partition(g.get_n());

            return partition;
        }

        void write_solution(const std::vector<u64>& partition) const {
            std::stringstream ss;
            for (u64 i : partition) {
                ss << i << "\n";
            }
            std::ofstream out(m_ac.mapping_out);
            out << ss.rdbuf();
            out.close();
        }

        void write_statistics() {
            std::string s = "{\n";

            s += to_JSON_MACRO(io_time);
            s += to_JSON_MACRO(solve_time);
            s += "\"ac\" : " + m_ac.to_JSON(2) + ",\n";

            s.pop_back();
            s.pop_back();
            s += "\n}";
            std::ofstream out(m_ac.statistics_out);
            out << s << std::endl;
            out.close();
        }
    };
}

#endif //SHAREDMAP_SOLVER_H

#ifndef SHAREDMAP_SOLVER_H
#define SHAREDMAP_SOLVER_H

#include "src/utility/definitions.h"
#include "src/utility/macros.h"
#include "src/utility/utils.h"
#include "src/utility/JSON_utils.h"
#include "src/utility/algorthm_configuration.h"
#include "graph.h"
#include "src/profiling/stat_collector.h"

namespace SharedMap {

    class Solver {

    private:
        const AlgorithmConfiguration &m_ac;
        StatCollector stat_collector;

        f64 io_time = 0.0;
        f64 solve_time = 0.0;

    public:
        explicit Solver(AlgorithmConfiguration &ac) : m_ac(ac) {}

        void solve() {
            // read graph
            auto sp = std::chrono::steady_clock::now();
            Graph g(m_ac.graph_in);
            auto ep = std::chrono::steady_clock::now();
            io_time += (f64) std::chrono::duration_cast<std::chrono::nanoseconds>(ep - sp).count() / 1e9;

            // solve problem
            sp = std::chrono::steady_clock::now();
            RecursiveMapping rec_map(g, m_ac, stat_collector);
            std::vector<u64> partition = rec_map.solve();
            ep = std::chrono::steady_clock::now();
            solve_time += (f64) std::chrono::duration_cast<std::chrono::nanoseconds>(ep - sp).count() / 1e9;

            // write output
            sp = std::chrono::steady_clock::now();
            write_solution(partition);
            ep = std::chrono::steady_clock::now();
            io_time += (f64) std::chrono::duration_cast<std::chrono::nanoseconds>(ep - sp).count() / 1e9;

            write_statistics();
        }

    private:
        void write_solution(const std::vector<u64> &partition) const {
            std::stringstream ss;
            for (u64 i: partition) {
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

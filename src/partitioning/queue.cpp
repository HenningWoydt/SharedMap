#include "queue.h"

namespace SharedMap {
    void queue_thread_work(Item item,
                           std::priority_queue<Item> &queue,
                           std::mutex &queue_lock,
                           std::atomic<u64> &queue_size,
                           std::atomic<u64> &n_available_threads,
                           u64 n_assigned_threads,
                           std::vector<u64> &solution,
                           const Graph &original_g,
                           const AlgorithmConfiguration &config,
                           StatCollector &stat_collector) {
        // references for better code readability
        const std::vector<u64> &hierarchy       = config.hierarchy;
        const size_t           l                = hierarchy.size();
        const std::vector<u64> &index_vec       = config.index_vec;
        const std::vector<u64> &k_rem_vec       = config.k_rem_vec;
        const f64              global_imbalance = config.imbalance;
        const u64              global_g_weight  = original_g.get_weight();
        const u64              global_k         = config.k;
        const u64              n_threads        = config.n_threads;

        // load item to process
        const Graph            &g          = (*item.g);
        const TranslationTable &tt         = (*item.tt);
        const std::vector<u64> &identifier = (*item.identifier);

        // get depth info
        const u64 depth           = l - 1 - identifier.size();
        const u64 local_k         = hierarchy[depth];
        const u64 local_k_rem     = k_rem_vec[depth];
        const f64 local_imbalance = determine_adaptive_imbalance(global_imbalance, global_g_weight, global_k, g.get_weight(), local_k_rem, depth + 1);

        // partition the subgraph
        std::vector<u64> partition(g.get_n());
        partition_graph(g, local_k, local_imbalance, partition, n_assigned_threads, depth, config.serial_alg_id, config.parallel_alg_id, config.seed, stat_collector);

        if (depth == 0) {
            // insert solution
            u64 offset = 0;

            for (u64 i = 0; i < identifier.size(); ++i) { offset += identifier[i] * index_vec[index_vec.size() - 1 - i]; }
            for (u64 u = 0; u < g.get_n(); ++u) { solution[tt.get_o(u)] = offset + partition[u]; }

            // free item
            item.free();

            // make threads available
            n_available_threads += n_assigned_threads;
        } else {
            // create the subgraphs and place them in the next stack
            std::vector<Item> temp_stack;
            create_sub_graphs(g, tt, local_k, partition, identifier, temp_stack, depth, n_threads, stat_collector);

            // free item
            item.free();

            // push graphs into queue
            queue_lock.lock();
            for (size_t i = 0; i < local_k; ++i) {
                queue.emplace(temp_stack[i]);
            }
            n_available_threads += n_assigned_threads;
            queue_size += temp_stack.size();
            queue_lock.unlock();
        }
    }

    std::vector<u64> solve_queue(const Graph &original_g,
                                 const AlgorithmConfiguration &config,
                                 StatCollector &stat_collector) {
        std::vector<u64> solution(original_g.get_n()); // end partition
        TranslationTable original_tt(original_g.get_n()); // default translation table

        // references for better code readability
        const u64 n_threads = config.n_threads;

        // initialize the queue
        std::priority_queue<Item> queue;
        queue.emplace(new std::vector<u64>(), const_cast<Graph *>(&original_g), &original_tt, false);
        std::mutex       queue_lock;
        std::atomic<u64> queue_size          = 1;
        std::atomic<u64> n_available_threads = n_threads;

        while (true) {
            while ((n_available_threads == 0 || queue_size == 0) && !(n_available_threads == n_threads && queue_size == 0)) {
                /* wait */
            }

            // get item
            queue_lock.lock();

            // all threads available and queue empty, so finished
            if (n_available_threads == n_threads && queue_size == 0) {
                queue_lock.unlock();
                break;
            }

            u64  n_assigned_threads = (n_available_threads + queue_size - 1) / queue_size;
            Item item               = queue.top();
            queue.pop();
            n_available_threads -= n_assigned_threads;
            queue_size -= 1;
            queue_lock.unlock();

            // start thread
            std::thread thread(queue_thread_work,
                               item,
                               std::ref(queue),
                               std::ref(queue_lock),
                               std::ref(queue_size),
                               std::ref(n_available_threads),
                               n_assigned_threads,
                               std::ref(solution),
                               std::ref(original_g),
                               std::ref(config),
                               std::ref(stat_collector));
            thread.detach();
        }

        return solution;
    }
}

/*******************************************************************************
 * MIT License
 *
 * This file is part of SharedMap.
 *
 * Copyright (C) 2025 Henning Woydt <henning.woydt@informatik.uni-heidelberg.de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 ******************************************************************************/

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>
#include "../include/libsharedmap.h"

namespace py = pybind11;

py::tuple hierarchical_multisection(
    py::array_t<int> v_weights,
    py::array_t<int> adj_ptrs,
    py::array_t<int> adj_weights,
    py::array_t<int> adj,
    py::array_t<int> hierarchy,
    py::array_t<int> distance,
    float imbalance,
    int n_threads,
    int seed,
    shared_map_strategy_type_t strategy,
    shared_map_algorithm_type_t parallel_alg,
    shared_map_algorithm_type_t serial_alg,
    bool verbose
) {
    int n = v_weights.size();
    int l = hierarchy.size();
    
    auto partition = py::array_t<int>(n);
    int comm_cost;
    
    shared_map_hierarchical_multisection(
        n,
        v_weights.mutable_data(),
        adj_ptrs.mutable_data(),
        adj_weights.mutable_data(),
        adj.mutable_data(),
        hierarchy.mutable_data(),
        distance.mutable_data(),
        l,
        imbalance,
        n_threads,
        seed,
        strategy,
        parallel_alg,
        serial_alg,
        comm_cost,
        partition.mutable_data(),
        verbose
    );
    
    return py::make_tuple(comm_cost, partition);
}

bool assert_input(
    py::array_t<int> v_weights,
    py::array_t<int> adj_ptrs,
    py::array_t<int> adj_weights,
    py::array_t<int> adj,
    py::array_t<int> hierarchy,
    py::array_t<int> distance,
    float imbalance,
    int n_threads,
    int seed,
    shared_map_strategy_type_t strategy,
    shared_map_algorithm_type_t parallel_alg,
    shared_map_algorithm_type_t serial_alg,
    bool verbose
) {
    int n = v_weights.size();
    int l = hierarchy.size();
    
    return shared_map_hierarchical_multisection_assert_input(
        n,
        v_weights.mutable_data(),
        adj_ptrs.mutable_data(),
        adj_weights.mutable_data(),
        adj.mutable_data(),
        hierarchy.mutable_data(),
        distance.mutable_data(),
        l,
        imbalance,
        n_threads,
        seed,
        strategy,
        parallel_alg,
        serial_alg,
        verbose
    );
}

PYBIND11_MODULE(_sharedmap, m) {
    m.doc() = "SharedMap: Parallel shared-memory hierarchical process mapping";
    
    py::enum_<shared_map_strategy_type_t>(m, "Strategy")
        .value("NAIVE", NAIVE)
        .value("LAYER", LAYER)
        .value("QUEUE", QUEUE)
        .value("NB_LAYER", NB_LAYER)
        .export_values();
    
    py::enum_<shared_map_algorithm_type_t>(m, "Algorithm")
        .value("KAFFPA_FAST", KAFFPA_FAST)
        .value("KAFFPA_ECO", KAFFPA_ECO)
        .value("KAFFPA_STRONG", KAFFPA_STRONG)
        .value("MTKAHYPAR_DEFAULT", MTKAHYPAR_DEFAULT)
        .value("MTKAHYPAR_QUALITY", MTKAHYPAR_QUALITY)
        .value("MTKAHYPAR_HIGHEST_QUALITY", MTKAHYPAR_HIGHEST_QUALITY)
        .export_values();
    
    m.def("hierarchical_multisection", &hierarchical_multisection,
          py::arg("v_weights"),
          py::arg("adj_ptrs"),
          py::arg("adj_weights"),
          py::arg("adj"),
          py::arg("hierarchy"),
          py::arg("distance"),
          py::arg("imbalance"),
          py::arg("n_threads"),
          py::arg("seed"),
          py::arg("strategy"),
          py::arg("parallel_alg"),
          py::arg("serial_alg"),
          py::arg("verbose") = false,
          "Execute hierarchical multisection on a graph");
    
    m.def("assert_input", &assert_input,
          py::arg("v_weights"),
          py::arg("adj_ptrs"),
          py::arg("adj_weights"),
          py::arg("adj"),
          py::arg("hierarchy"),
          py::arg("distance"),
          py::arg("imbalance"),
          py::arg("n_threads"),
          py::arg("seed"),
          py::arg("strategy"),
          py::arg("parallel_alg"),
          py::arg("serial_alg"),
          py::arg("verbose") = false,
          "Validate input parameters");
}

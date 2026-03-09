"""SharedMap: Parallel shared-memory hierarchical process mapping."""

import numpy as np
from typing import Tuple, Union, Optional
try:
    from ._sharedmap import hierarchical_multisection as _hierarchical_multisection
    from ._sharedmap import assert_input as _assert_input
    from ._sharedmap import Strategy, Algorithm
except ImportError:
    raise ImportError("SharedMap C++ extension not built. Please install with pip.")

__version__ = "1.0.0"

def partition_graph(
    v_weights,
    adj_ptrs,
    adj_weights,
    adj,
    hierarchy,
    distance,
    imbalance: float = 0.03,
    n_threads: int = 1,
    seed: int = 0,
    strategy = Strategy.NB_LAYER,
    parallel_alg = Algorithm.MTKAHYPAR_QUALITY,
    serial_alg = Algorithm.KAFFPA_STRONG,
    verbose: bool = False
) -> Tuple[int, np.ndarray]:
    """
    Partition a graph using hierarchical multisection.
    
    Parameters
    ----------
    v_weights : array_like
        Vertex weights (n integers, each >= 1)
    adj_ptrs : array_like
        CSR adjacency pointers (n+1 integers)
    adj_weights : array_like
        Edge weights (m integers, each >= 1)
    adj : array_like
        CSR adjacency list (m integers)
    hierarchy : array_like
        Hierarchy levels (e.g., [4, 8, 6])
    distance : array_like
        Communication distances (e.g., [1, 10, 100])
    imbalance : float, optional
        Maximum allowed imbalance (default: 0.03 = 3%)
    n_threads : int, optional
        Number of threads (default: 1)
    seed : int, optional
        Random seed (default: 0)
    strategy : Strategy, optional
        Thread distribution strategy (default: NB_LAYER)
    parallel_alg : Algorithm, optional
        Parallel algorithm (default: MTKAHYPAR_QUALITY)
    serial_alg : Algorithm, optional
        Serial algorithm (default: KAFFPA_STRONG)
    verbose : bool, optional
        Print statistics (default: False)
    
    Returns
    -------
    comm_cost : int
        Communication cost J(C, D, PI)
    partition : ndarray
        Partition assignment for each vertex
    
    Examples
    --------
    >>> import sharedmap as sm
    >>> v_weights = [1, 1, 1, 1, 1, 1, 1, 1]
    >>> adj_ptrs = [0, 3, 6, 8, 12, 16, 19, 21, 22]
    >>> adj_weights = [1] * 22
    >>> adj = [1, 2, 3, 0, 3, 4, 0, 3, 0, 1, 2, 5, 1, 5, 6, 7, 3, 4, 6, 4, 5, 4]
    >>> cost, partition = sm.partition_graph(
    ...     v_weights, adj_ptrs, adj_weights, adj,
    ...     hierarchy=[2, 2], distance=[1, 10],
    ...     imbalance=0.03, n_threads=1
    ... )
    """
    return _hierarchical_multisection(
        np.asarray(v_weights, dtype=np.int32),
        np.asarray(adj_ptrs, dtype=np.int32),
        np.asarray(adj_weights, dtype=np.int32),
        np.asarray(adj, dtype=np.int32),
        np.asarray(hierarchy, dtype=np.int32),
        np.asarray(distance, dtype=np.int32),
        float(imbalance),
        int(n_threads),
        int(seed),
        strategy,
        parallel_alg,
        serial_alg,
        verbose
    )

def validate_input(
    v_weights,
    adj_ptrs,
    adj_weights,
    adj,
    hierarchy,
    distance,
    imbalance: float = 0.03,
    n_threads: int = 1,
    seed: int = 0,
    strategy = Strategy.NB_LAYER,
    parallel_alg = Algorithm.MTKAHYPAR_QUALITY,
    serial_alg = Algorithm.KAFFPA_STRONG,
    verbose: bool = False
) -> bool:
    """
    Validate input parameters for hierarchical multisection.
    
    Returns True if input is valid, False otherwise.
    """
    return _assert_input(
        np.asarray(v_weights, dtype=np.int32),
        np.asarray(adj_ptrs, dtype=np.int32),
        np.asarray(adj_weights, dtype=np.int32),
        np.asarray(adj, dtype=np.int32),
        np.asarray(hierarchy, dtype=np.int32),
        np.asarray(distance, dtype=np.int32),
        float(imbalance),
        int(n_threads),
        int(seed),
        strategy,
        parallel_alg,
        serial_alg,
        verbose
    )

__all__ = ['partition_graph', 'validate_input', 'Strategy', 'Algorithm']

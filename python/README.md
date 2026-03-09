# Python Installation

## From PyPI (once published)

```bash
pip install sharedmap
```

## From source

```bash
# Clone the repository
git clone https://github.com/HenningWoydt/SharedMap.git
cd SharedMap

# Build dependencies first
./build.sh

# Install Python package
pip install .
```

## Usage

```python
import sharedmap as sm

# Define your graph in CSR format
v_weights = [1, 1, 1, 1, 1, 1, 1, 1]
adj_ptrs = [0, 3, 6, 8, 12, 16, 19, 21, 22]
adj_weights = [1] * 22
adj = [1, 2, 3, 0, 3, 4, 0, 3, 0, 1, 2, 5, 1, 5, 6, 7, 3, 4, 6, 4, 5, 4]

# Define hardware hierarchy and distances
hierarchy = [2, 2]  # 2 nodes, 2 cores per node
distance = [1, 10]  # distance within node: 1, between nodes: 10

# Partition the graph
comm_cost, partition = sm.partition_graph(
    v_weights, adj_ptrs, adj_weights, adj,
    hierarchy, distance,
    imbalance=0.03,
    n_threads=1,
    seed=0
)

print(f"Communication Cost: {comm_cost}")
print(f"Partition: {partition}")
```

## API Reference

### `partition_graph()`

Main function for graph partitioning.

**Parameters:**
- `v_weights`: Vertex weights (array of integers >= 1)
- `adj_ptrs`: CSR adjacency pointers (n+1 integers)
- `adj_weights`: Edge weights (array of integers >= 1)
- `adj`: CSR adjacency list
- `hierarchy`: Hierarchy levels (e.g., [4, 8, 6])
- `distance`: Communication distances (e.g., [1, 10, 100])
- `imbalance`: Maximum allowed imbalance (default: 0.03)
- `n_threads`: Number of threads (default: 1)
- `seed`: Random seed (default: 0)
- `strategy`: Thread distribution strategy (default: `Strategy.NB_LAYER`)
- `parallel_alg`: Parallel algorithm (default: `Algorithm.MTKAHYPAR_QUALITY`)
- `serial_alg`: Serial algorithm (default: `Algorithm.KAFFPA_STRONG`)
- `verbose`: Print statistics (default: False)

**Returns:**
- `comm_cost`: Communication cost J(C, D, PI)
- `partition`: Partition assignment for each vertex (numpy array)

### Enums

**Strategy:**
- `Strategy.NAIVE`
- `Strategy.LAYER`
- `Strategy.QUEUE`
- `Strategy.NB_LAYER`

**Algorithm:**
- `Algorithm.KAFFPA_FAST`
- `Algorithm.KAFFPA_ECO`
- `Algorithm.KAFFPA_STRONG`
- `Algorithm.MTKAHYPAR_DEFAULT`
- `Algorithm.MTKAHYPAR_QUALITY`
- `Algorithm.MTKAHYPAR_HIGHEST_QUALITY`

### `validate_input()`

Validates input parameters before partitioning.

**Returns:** `bool` - True if input is valid, False otherwise.


#  C++ General Directed Weighted Graph (GDWG) Library

![image](https://github.com/user-attachments/assets/6ff764d2-1da7-4e12-8f87-8a8d4bec1c1b)
Example of [Directed Weighted Graph](https://en.wikipedia.org/wiki/Directed_graph#:~:text=Weighted%20directed%20graphs%20(also%20known,undirected%20networks%20or%20weighted%20networks).)

## Overview

The General Directed Weighted Graph (GDWG) is a C++ library that provides a framework for creating and managing directed, weighted graphs with value semantics. The library allows users to efficiently perform operations on nodes and edges within a graph, such as adding nodes, inserting edges, replacing nodes, merging nodes, and checking for connectivity between nodes. It is designed to handle graphs with unique nodes and ordered edges based on source node, destination node, and edge weight.

## Features

## How to Use

```cpp
#include "gdwg_graph.h"
#include <optional>

int main() {
    using graph = gdwg::graph<int, int>;
    graph g;

    // Insert nodes
    g.insert_node(1);
    g.insert_node(7);
    g.insert_node(12);
    g.insert_node(14);
    g.insert_node(19);
    g.insert_node(21);
    g.insert_node(31);

    // Insert edges with and without weights
    g.insert_edge(1, 1, 4);                // (1 -> 1 | W | 4)
    g.insert_edge(1, 7, std::nullopt);     // (1 -> 7 | U)
    g.insert_edge(1, 7, 2);                // (1 -> 7 | W | 2)
    g.insert_edge(1, 12, 3);               // (1 -> 12 | W | 3)
    g.insert_edge(7, 21, std::nullopt);    // (7 -> 21 | U)
    g.insert_edge(14, 14, 7);              // (14 -> 14 | W | 7)
    g.insert_edge(19, 1, 3);               // (19 -> 1 | W | 3)
    g.insert_edge(19, 21, 2);              // (19 -> 21 | W | 2)
    g.insert_edge(21, 14, 23);             // (21 -> 14 | W | 23)
    g.insert_edge(21, 31, 14);             // (21 -> 31 | W | 14)

    // Output the graph structure
    std::cout << g << std::endl;

    return 0;
}
```

To use the GDWG library, include the header file in your C++ project:

```cpp
#include "gdwg_graph.h"
```

Traverse the graph using iterators:

```cpp
for (const auto& edge : g) {
    std::cout << "Edge from " << edge.from << " to " << edge.to;
    if (edge.weight) {
        std::cout << " with weight " << *edge.weight;
    }
    std::cout << std::endl;
}
```




# GDWG Function Headers

### Node Management

- **`bool insert_node(N const& value)`**
  - Adds a new node with value `value` to the graph if it doesn't already exist.
  - **Returns:** `true` if the node is added, `false` otherwise.

- **`bool replace_node(N const& old_data, N const& new_data)`**
  - Replaces the original data, `old_data`, stored at this particular node with `new_data`.
  - **Returns:** `true` if successful, `false` if `new_data` already exists.

- **`void merge_replace_node(N const& old_data, N const& new_data)`**
  - Replaces the `old_data` node with `new_data`, merging connections and removing duplicates.

- **`bool erase_node(N const& value)`**
  - Removes the node `value` from the graph.
  - **Returns:** `true` if the node is removed, `false` otherwise.

### Edge Management

- **`bool insert_edge(N const& src, N const& dst, std::optional<E> weight = std::nullopt)`**
  - Adds a new edge from `src` to `dst` with an optional `weight`.
  - **Returns:** `true` if the edge is added, `false` otherwise.

- **`bool erase_edge(N const& src, N const& dst, std::optional<E> weight = std::nullopt)`**
  - Removes the edge from `src` to `dst` with the specified `weight`.
  - **Returns:** `true` if the edge is removed, `false` otherwise.

- **`iterator erase_edge(iterator i)`**
  - Removes the edge pointed to by the iterator `i`.
  - **Returns:** An iterator pointing to the element immediately after `i`.

- **`iterator erase_edge(iterator i, iterator s)`**
  - Removes all edges between the iterators `[i, s)`.
  - **Returns:** An iterator equivalent to `s`.

### Graph Queries

- **`bool is_node(N const& value) const`**
  - Checks if the node `value` exists in the graph.
  - **Returns:** `true` if the node exists, `false` otherwise.

- **`bool empty() const noexcept`**
  - Checks if the graph is empty.
  - **Returns:** `true` if there are no nodes, `false` otherwise.

- **`bool is_connected(N const& src, N const& dst) const`**
  - Checks if there is a directed edge from `src` to `dst`.
  - **Returns:** `true` if the nodes are connected, `false` otherwise.

- **`std::vector<N> nodes() const`**
  - Retrieves all nodes in the graph, sorted in ascending order.
  - **Returns:** A vector of nodes.

- **`edge_list edges(N const& src, N const& dst) const`**
  - Retrieves all edges from `src` to `dst`.
  - **Returns:** A list of edges.

- **`iterator find(N const& src, N const& dst, std::optional<E> weight = std::nullopt) const`**
  - Finds an edge from `src` to `dst` with the specified `weight`.
  - **Returns:** An iterator pointing to the found edge, or `end()` if not found.

- **`std::vector<N> connections(N const& src) const`**
  - Retrieves all nodes connected to `src`.
  - **Returns:** A vector of connected nodes.

### Graph Traversal

- **`iterator begin() const noexcept`**
  - Returns an iterator to the beginning of the graph.

- **`iterator end() const noexcept`**
  - Returns an iterator to the end of the graph.

### Graph Output

- **`friend std::ostream& operator<<(std::ostream& os, graph<N, E> const& g)`**
  - Outputs the graph to an output stream `os`.
  - **Returns:** The output stream.


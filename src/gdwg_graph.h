#ifndef GDWG_GRAPH_H
#define GDWG_GRAPH_H
#include <algorithm>
#include <concepts>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

/**
 * @file gdwg_graph.h
 * @brief General Directed Weighted Graph (GDWG) with value-semantics in C++
 *
 * Example:
 *    using graph = gdwg::graph<std::N, E>;
 * This directed weighted graph G = (N, E) will consist of a set of nodes N and a set of unweighted/weighted edges E.
 *
 * All nodes are unique, meaning that no two nodes can have the same value and shall not compare equal (N1 == N2).
 * Edges are ordered first by source node, then by destination node, and finally
 * by edge weight (if exists) in ascending order.
 *
 * @code
 *   gdwg::graph<std::int, int> g { .. };
 *	 g.insert_node(..);
 *	 g.insert_edge(.., .., ..);
 *
 *   (src, dst, weight)
 *   (1, 1, 4)
 *   (1, 7, nullopt)
 *   (1, 7, 2)
 *   (1, 12, 3)
 *   (7, 21, nullopt)
 *   (14, 14, 7)
 *   (19, 1, 3)
 *   (19, 21, 2)
 *   (21, 14, 23)
 *   (21, 31, 14)
 * @endcode
 *
 * @tparam N The type of the nodes. Must be copyable, comparable, streamable, and hashable.
 * @tparam E The type of the edge weights. Must be copyable, comparable, streamable, and hashable.
 */
namespace gdwg {

	namespace { // utility
		template<typename T>
		[[nodiscard]] auto to_string(const T& value) -> std::string {
			std::ostringstream oss;
			oss << value;
			return oss.str();
		}

	} // namespace

	// forward declarations
	template<typename N, typename E>
	class graph;

	template<typename N, typename E>
	class edge;

	template<typename N, typename E>
	class weighted_edge;

	template<typename N, typename E>
	class unweighted_edge;

	/**
	 * @defgroup gdwg::edge
	 * @{
	 */

	template<typename N, typename E>
	class edge {
	 public:
		virtual ~edge() noexcept = default; // declared as virtual, but both weighted_edge and unweighted_edge aren't
		                                    // managing any resources that require custom cleanup; default implicit
		                                    // destructor is sufficient.

		/// @name Pure Virtual Functions
		/// @{
		/**
		 * @brief Returns a string representation of the edge.
		 * @return A string representation of the edge.
		 * @remarks The format of the string representation is src -> dst | W | weight if the edge is weighted, and src
		 * -> dst | U if the edge is unweighted, there should not be a newline character \n in the end.
		 */
		virtual auto print_edge() const -> std::string = 0;
		/**
		 * @brief Identifies whether the edge is weighted.
		 * @return true if the edge is weighted, and false otherwise.
		 */
		[[nodiscard]] virtual auto is_weighted() const noexcept -> bool = 0;
		/**
		 * @brief Returns the weight of the edge.
		 * @return The weight of the edge, or std::nullopt if the edge is unweighted.
		 * @note The weight can be a non-numeric value.
		 */
		[[nodiscard]] virtual auto get_weight() const noexcept -> std::optional<E> = 0;

		/// @}

		/// @name Other Member Functions
		/// @{
		/**
		 * 	@brief Returns the source and destination nodes.
		 * 	@return std::pair<N, N> containing the source and destination nodes of the edge.
		 */
		[[nodiscard]] auto get_nodes() const noexcept -> std::pair<N, N> {
			return {from_, to_};
		}

		/**
		 *	@param other The edge to compare with.
		 *  @details Compares if two edges are equal (i.e. same source node, same destination node, and same weight (if
		 *exists))
		 * 	@return true if the edges are equal, false otherwise.
		 */
		[[nodiscard]] auto operator==(const edge& other) const noexcept -> bool {
			return get_nodes() == other.get_nodes() and is_weighted() == other.is_weighted()
			       and get_weight() == other.get_weight();
		}

		/// @}

	 protected:
		edge(N from, N to)
		: from_{std::move(from)}
		, to_{std::move(to)} {}

		N from_;
		N to_;

	 private:
		friend class graph<N, E>;

	 public:
		class weighted_edge;
		class unweighted_edge;
	};

	template<typename N, typename E>
	class weighted_edge : public edge<N, E> {
	 public:
		weighted_edge(N const& src, N const& dst, E const& weight)
		: edge<N, E>{std::move(src), std::move(dst)}
		, weight_{std::move(weight)} {}

		auto print_edge() const -> std::string override {
			return to_string(this->from_) + " -> " + to_string(this->to_) + " | W | " + to_string(*this->get_weight());
		}
		[[nodiscard]] auto is_weighted() const noexcept -> bool override {
			return true;
		}
		[[nodiscard]] auto get_weight() const noexcept -> std::optional<E> override {
			return weight_;
		}

	 private:
		std::optional<E> weight_;
	};

	template<typename N, typename E>
	class unweighted_edge : public edge<N, E> {
	 public:
		unweighted_edge(N const& src, N const& dst)
		: edge<N, E>{std::move(src), std::move(dst)} {}

		auto print_edge() const -> std::string override {
			return to_string(this->from_) + " -> " + to_string(this->to_) + " | U";
		}
		[[nodiscard]] auto is_weighted() const noexcept -> bool override {
			return false;
		}
		[[nodiscard]] auto get_weight() const noexcept -> std::optional<E> override {
			return std::nullopt;
		}
	};

	/** @} */ // end of gdwg::edge

	/**
	 * @defgroup gdwg::graph
	 * @{
	 */

	template<typename N, typename E>
	class graph {
	 public:
		using edge_ptr = std::unique_ptr<edge<N, E>>;
		using edge_list = std::vector<edge_ptr>;
		using adjacency_list = std::map<N, edge_list>;
		class iterator;

		// ------------------- Constructors ---------------------

		// Default constructor
		graph() = default;

		// Initializer list constructor
		graph(std::initializer_list<N> il)
		: graph{il.begin(), il.end()} {}

		// Input Iterator constructor
		/**
		 * @brief Initializes the graph’s node collection with the range `[first, last)`.
		 * @param first The beginning of the range.
		 * @param last The end of the range.
		 * @note Type `InputIt` models Cpp17 Input Iterator and is indirectly readable as type `N`.
		 */
		template<typename InputIt>
		graph(InputIt first, InputIt last) {
			for (auto it = first; it != last; ++it) {
				insert_node(*it);
			}
		}

		// Move Constructor
		graph(graph&& other) noexcept
		: adjacency_list_(std::move(other.adjacency_list_)) {
			other.adjacency_list_.clear();
		}

		// Copy Constructor
		graph(graph const& other) {
			for (const auto& [node, edges_list] : other.adjacency_list_) {
				edge_list new_edges_list;
				for (const auto& e : edges_list) {
					if (e->is_weighted()) {
						new_edges_list.emplace_back(
						    std::make_unique<weighted_edge<N, E>>(e->from_, e->to_, *e->get_weight()));
					}
					else {
						new_edges_list.emplace_back(std::make_unique<unweighted_edge<N, E>>(e->from_, e->to_));
					}
				}
				adjacency_list_[node] = std::move(new_edges_list);
			}
		}

		// Destructor
		~graph() = default;

		// ---------------------- Operators -----------------------------

		// Move Assignment Operator
		[[nodiscard]] auto operator=(graph&& other) noexcept -> graph& {
			if (this != &other) {
				adjacency_list_ = std::move(other.adjacency_list_);
				other.adjacency_list_.clear();
			}
			return *this;
		}

		// Copy Assignment Operator
		[[nodiscard]] auto operator=(graph const& other) -> graph& {
			if (this != &other) {
				adjacency_list_.clear();
				for (const auto& [node, edges_list] : other.adjacency_list_) {
					edge_list new_edges_list;
					for (const auto& e : edges_list) {
						if (e->is_weighted()) {
							new_edges_list.emplace_back(
							    std::make_unique<weighted_edge<N, E>>(e->from_, e->to_, *e->get_weight()));
						}
						else {
							new_edges_list.emplace_back(std::make_unique<unweighted_edge<N, E>>(e->from_, e->to_));
						}
					}
					adjacency_list_[node] = std::move(new_edges_list);
				}
			}
			return *this;
		}

		// -------------------- Modifiers --------------------------

		/**
		 * @brief Adds a new node with value `value` to the graph if it doesn't already exist.
		 * @param value The value of the node to be inserted.
		 * @details Adds a new node with value `value` to the graph if, and only if, there is no node equivalent to
		 * `value` already stored.
		 * @note Can be used independently (and discarded afterwards), but use with caution as the success of operation
		 * isn't guaranteed.
		 * @note Postconditions: All iterators are invalidated.
		 * @returns `true` if the node is added to the graph and `false` otherwise.
		 */
		[[maybe_unused]] auto insert_node(N const& value) -> bool {
			if (is_node(value)) {
				return false;
			}

			adjacency_list_.emplace(value, edge_list());
			return true;
		}

		/**
		 * @brief Adds a new edge representing `src` → `dst` with an optional `weight`.
		 * @param src The source node.
		 * @param dst The destination node.
		 * @param weight The weight of the edge (optional).
		 * @details Adds a new edge representing `src` → `dst` with an optional `weight`.
		 *          If weight is `std::nullopt`, an `unweighted_edge` is created.
		 *          Otherwise, a `weighted_edge` with the specified weight is created.
		 *          The edge is only added if there is no existing edge between `src` and `dst` with the same weight.
		 * @note Nodes are allowed to be connected to themselves.
		 * @note Can be used independently (and discarded afterwards), but use with caution as the success of operation
		 * isn't guaranteed.
		 * @note Postconditions: All iterators are invalidated.
		 * @returns `true` if the edge is added to the graph and `false` otherwise.
		 * @throws std::runtime_error if either `src` or `dst` does not exist in the graph.
		 */
		[[maybe_unused]] auto insert_edge(N const& src, N const& dst, std::optional<E> weight = std::nullopt) -> bool {
			if (!is_node(src) or !is_node(dst)) {
				throw std::runtime_error("Cannot call gdwg::graph<N, E>::insert_edge when either src or dst node does "
				                         "not exist");
			}

			auto& edges_from_src = adjacency_list_[src];
			if (edge_exists(edges_from_src, dst, weight)) {
				return false;
			}

			auto new_edge = create_edge(src, dst, weight);
			edges_from_src.emplace_back(std::move(new_edge));

			sort_edges(edges_from_src);

			return true;
		}

		/**
		 * @brief Replaces the original data, `old_data`, stored at this particular node by the replacement data,
		 * `new_data`.
		 * @param old_data The old data to be replaced.
		 * @param new_data The new data to replace with.
		 * @note Postconditions: All iterators are invalidated.
		 * @note Can be used independently (and discarded afterwards), but use with caution as the success of operation
		 * isn't guaranteed.
		 * @returns `false` if a node that contains value `new_data` already exists and `true` otherwise.
		 * @throws std::runtime_error if `old_data` does not exist in the graph.
		 */
		[[maybe_unused]] auto replace_node(N const& old_data, N const& new_data) -> bool {
			// #1
			auto replace_outgoing_edges = [this](const N& old_data, const N& new_data) {
				if (adjacency_list_.count(old_data) > 0) {
					for (auto& edge : adjacency_list_[old_data]) {
						auto new_edge = create_edge(new_data, edge->to_, edge->get_weight());
						adjacency_list_[new_data].emplace_back(std::move(new_edge));
					}
				}
			};
			// #2
			auto replace_incoming_edges = [this](const N& old_data, const N& new_data) {
				for (auto& [src, edges_list] : adjacency_list_) {
					for (auto& edge : edges_list) {
						if (edge->from_ == old_data) {
							edge->from_ = new_data;
						}
						if (edge->to_ == old_data) {
							edge->to_ = new_data;
						}
					}
				}
			};

			if (!is_node(old_data)) {
				throw std::runtime_error("Cannot call gdwg::graph<N, E>::replace_node on a node that doesn't exist");
			}

			if (is_node(new_data)) {
				return false;
			}

			adjacency_list_[new_data] = edge_list();
			replace_outgoing_edges(old_data, new_data); // #1
			replace_incoming_edges(old_data, new_data); // #2

			adjacency_list_.erase(old_data);

			sort_edges(adjacency_list_[new_data]);

			return true;
		}

		/**
		 * @brief The node equivalent to `old_data` in the graph is replaced with instances of `new_data`.
		 * @param old_data The old node to be replaced.
		 * @param new_data The new node to replace with.
		 * @details After completing, every incoming and outgoing edge of `old_data` becomes an incoming/outgoing edge
		 *			of `new_data`, except that duplicate edges shall be removed.
		 *
		 *			Basic example: merge_replace_node(A, B)
		 *			before:
		 *			(A, B, 1), (A, C, 2), (A, D, 3)
		 *			after:
		 *			(B, B, 1), (B, C, 2), (B, D, 3)
		 *
		 *			Duplicate edge removed example:
		 *			merge_replace_node(A, B)
		 *			before: (A, B, 1), (A, C, 2), (A, D, 3), (B, B, 1)
		 *			after: (B, B, 1), (B, C, 2), (B, D, 3)
		 * @note All iterators are invalidated.
		 * @throws std::runtime_error if either `old_data` or `new_data` does not exist in the graph.
		 */
		auto merge_replace_node(N const& old_data, N const& new_data) -> void {
			if (!is_node(old_data) or !is_node(new_data)) {
				throw std::runtime_error("Cannot call gdwg::graph<N, E>::merge_replace_node on old or new data if they "
				                         "don't exist in the graph");
			}

			auto& old_edges = adjacency_list_[old_data];
			auto& new_edges = adjacency_list_[new_data];

			// add new edges (outgoing edges)
			for (const auto& edge : old_edges) {
				if (!edge_exists(new_edges, edge->get_nodes().second, edge->get_weight())) {
					new_edges.emplace_back(create_edge(new_data, edge->get_nodes().second, edge->get_weight()));
				}
			}

			// sync for edge in edges list (incoming edges)
			for (auto& [src, edges_list] : adjacency_list_) {
				for (auto& edge : edges_list) {
					if (edge->get_nodes().second == old_data) {
						edge->to_ = new_data;
					}
				}
			}

			sort_edges(new_edges);

			// remove duplicates
			new_edges.erase(std::unique(new_edges.begin(),
			                            new_edges.end(),
			                            [](const edge_ptr& a, const edge_ptr& b) {
				                            return a->get_nodes().second == b->get_nodes().second
				                                   and a->get_weight() == b->get_weight();
			                            }),
			                new_edges.end());

			adjacency_list_.erase(old_data);
		}

		/**
		 * @brief Erases all nodes equivalent to `value`, including all incoming and outgoing edges.
		 * @param value The value of the node to be erased.
		 * @returns `true` if `value` was removed; `false` otherwise.
		 * @note Postconditions: All iterators are invalidated.
		 * @note Can be used independently (and discarded afterwards), but use with caution as the success of operation
		 * isn't guaranteed.
		 */
		[[maybe_unused]] auto erase_node(N const& value) -> bool {
			if (!is_node(value)) {
				return false;
			}

			for (auto& [src, edges_list] : adjacency_list_) {
				edges_list.erase(std::remove_if(edges_list.begin(),
				                                edges_list.end(),
				                                [&value](const edge_ptr& e) { return e->get_nodes().second == value; }),
				                 edges_list.end());
			}

			adjacency_list_.erase(value);

			return true;
		}

		/**
		 * @brief Erases the edge representing `src` → `dst` with the specified weight.
		 * @param src The source node of the edge.
		 * @param dst The destination node of the edge.
		 * @param weight The weight of the edge (optional).
		 * @details If weight is `std::nullopt`, it erases the `unweighted_edge` between `src` and `dst`.
		 *          If weight has a value, it erases the weighted_edge between src and dst with the specified weight.
		 * @returns `true` if an edge was removed; `false` otherwise.
		 * @note Can be used independently (and discarded afterwards), but use with caution as the success of operation
		 * isn't guaranteed.
		 * @note Complexity: O(log(n) + e), where n is the total number of stored nodes and e is the total number of
		 * stored edges.
		 * @note: Postconditions All iterators are invalidated.
		 * @throws std::runtime_error if either `src` or `dst` does not exist in the graph.
		 */
		[[maybe_unused]] auto erase_edge(N const& src, N const& dst, std::optional<E> weight = std::nullopt) -> bool {
			if (!is_node(src) or !is_node(dst)) { // log(n)
				throw std::runtime_error("Cannot call gdwg::graph<N, E>::erase_edge on src or dst if they don't exist "
				                         "in the graph");
			}

			auto& edges_from_src = adjacency_list_.at(src); // log(n)
			auto edge_it =
			    std::remove_if(edges_from_src.begin(), // O(e)
			                   edges_from_src.end(),
			                   [&dst, &weight](const edge_ptr& e) {
				                   return e->get_nodes().second == dst
				                          and ((e->is_weighted() and e->get_weight() == weight) or (!e->is_weighted()));
			                   });

			if (edge_it == edges_from_src.end()) {
				return false;
			}

			edges_from_src.erase(edge_it, edges_from_src.end()); // O(e)

			return true; // Overall: O(log(n) + e)
		}

		/**
		 * @brief Erases the edge pointed to by an iterator `i`.
		 * @param i The iterator pointing to the edge to be erased.
		 * @returns An iterator pointing to the element immediately after `i` prior to the element being erased.
		 *          If no such element exists, returns `end()`.
		 * @note Complexity: O(log(n) + e), where n is the total number of stored nodes and e is the total number of
		 * stored edges.
		 * @note Postconditions: All iterators are invalidated.
		 */
		[[maybe_unused]] auto erase_edge(iterator i) -> iterator {
			if (i == end()) {
				return end();
			}

			auto& edges_from_src = adjacency_list_.at(i.node_it_->first); // O(log(n))
			auto next = edges_from_src.erase(i.edge_it_); // O(e);  erase() may throw exception
			std::size_t index = i.index();

			if (next == edges_from_src.end()) {
				return end();
			}

			// construct the iterator to return
			auto new_it = begin();
			while (index > 0 and new_it != end()) { // O(e)
				++new_it;
				--index;
			}

			return new_it; // Overall: O(log(n) + e)
		}

		/**
		 * @brief Erases all edges between the iterators `[i, s)`.
		 * @param i The beginning of the range.
		 * @param s The end of the range.
		 * @returns An iterator equivalent to `s` prior to the items iterated through being erased.
		 *          If no such element exists, returns `end()`.
		 * @note O(d(log(n) + e)), where d = std::distance(i, s) and n is the total number of stored nodes and e is the
		 * total number of stored edges. Postconditions: All iterators are invalidated.
		 * @throws std::runtime_error if iterator range is not given in ascending order (i must be less than s).
		 */
		[[maybe_unused]] auto erase_edge(iterator i, iterator s) -> iterator {
			if (i == s and (i == end())) {
				return s;
			}

			if (i.index() > s.index()) {
				throw std::runtime_error("Invalid iterator range: `i` is further down the graph than `s`");
			}

			while (i != end() and i != s) { // O(d)
				i = erase_edge(i); // O(log(n) + e)
				++i;
			}

			return s; // Overall: O(d(log(n) + e))
		}

		/**
		 * @brief Erases all nodes from the graph.
		 * @note Postconditions: `empty()` is `true`.
		 */
		auto clear() noexcept -> void {
			adjacency_list_.clear();
		}

		// ------------- ACCESSORS ------------

		/**
		 * @brief Checks if a node exists in the graph.
		 * @param value The node to check for existence in the graph.
		 * @returns `true` if a node equivalent to `value` exists in the graph, and `false` otherwise.
		 * @note Complexity: O(log n) time.
		 */
		[[nodiscard]] auto is_node(N const& value) const -> bool {
			return adjacency_list_.find(value) != adjacency_list_.end(); // map::find can throw exception
		}

		/**
		 * @brief Checks if the graph is empty.
		 * @returns `true` if there are no nodes in the graph, and `false` otherwise.
		 */
		[[nodiscard]] auto empty() const noexcept -> bool {
			return adjacency_list_.empty();
		}

		/**
		 * @brief Checks if an edge exists between two nodes.
		 * @param src The source node.
		 * @param dst The destination node.
		 * @returns `true` if an edge `src` → `dst` exists in the graph, and `false` otherwise.
		 * @throws std::runtime_error if either `src` or `dst` does not exist in the graph.
		 */
		[[nodiscard]] auto is_connected(N const& src, N const& dst) const -> bool {
			if (!is_node(src) or !is_node(dst)) {
				throw std::runtime_error("Cannot call gdwg::graph<N, E>::is_connected if src or dst node don't exist "
				                         "in the graph");
			}
			auto edges_between = edges(src, dst);
			return !edges_between.empty();
		}

		/**
		 * @brief Returns all stored nodes, sorted in ascending order.
		 * @returns A std::vector of all stored nodes, sorted in ascending order.
		 *          This returns **copies** of the specified data.
		 * @note Complexity: O(n), where n is the number of stored nodes.
		 */
		[[nodiscard]] auto nodes() const -> std::vector<N> {
			std::vector<N> nodes;
			for (const auto& [node, _] : adjacency_list_) { // O(n)
				nodes.emplace_back(node); // O(1) amortised   emplace_back may throw exception
			}
			return nodes; // Overall: O(n)
		}

		/**
		 * @brief Returns all edges from `src` to `dst`.
		 * @param src The source node.
		 * @param dst The destination node.
		 * @returns All edges from `src` to `dst`, starting with the unweighted edge (if exists), then the rest of the
		 * weighted edges sorted in ascending order by edge weights. This returns **copies** of the specified data.
		 * @note Complexity: O(log(n) + e), where n is the number of stored nodes and e is the number of stored edges.
		 * @throws std::runtime_error if either `src` or `dst` does not exist in the graph.
		 */
		[[nodiscard]] auto edges(N const& src, N const& dst) const -> edge_list {
			if (!is_node(src) or !is_node(dst)) { // O(log(n))
				throw std::runtime_error("Cannot call gdwg::graph<N, E>::edges if src or dst node don't exist in the "
				                         "graph");
			}

			edge_list connected_edges;
			const auto& edges_from_src = adjacency_list_.at(src); // O(log(n))
			for (const auto& e : edges_from_src) { // O(e_src)
				if (e->get_nodes().second == dst) { // O(1)
					if (e->is_weighted()) {
						connected_edges.emplace_back( // O(1) amortised
						    std::make_unique<weighted_edge<N, E>>(e->from_, e->to_, *e->get_weight())); // O(1)
					}
					else {
						connected_edges.emplace_back(std::make_unique<unweighted_edge<N, E>>(e->from_, e->to_)); // O(1)
						                                                                                         // amortised
					}
				}
			}

			return connected_edges; // Overall: O(log(n) + e)
		}

		/**
		 * @brief Finds an edge in the graph.
		 * @param src The source node.
		 * @param dst The destination node.
		 * @param weight The weight of the edge (optional).
		 * @returns An iterator pointing to an edge equivalent to the specified `src`, `dst`, and `weight`.
		 *          If weight is `std::nullopt`, it searches for an `unweighted_edge` between `src` and `dst`.
		 *          If weight has a value, it searches for a `weighted_edge` between `src` and `dst` with the specified
		 * weight. Returns `end()` if no such edge exists.
		 * @note Complexity: O(log(n) + e), where n is the number of stored nodes and e is the number of matching edges
		 * (either weighted or unweighted) between src and dst.
		 */
		[[nodiscard]] auto find(N const& src, N const& dst, std::optional<E> weight = std::nullopt) const -> iterator {
			if (!is_node(src) or !is_node(dst)) { // O(log(n))
				return end();
			}

			auto node_it = adjacency_list_.find(src); // O(log(n))
			if (node_it == adjacency_list_.end()) {
				return end();
			}

			auto& edges_list = node_it->second;
			auto edge_it = std::find_if(edges_list.begin(), edges_list.end(), [&dst, &weight](const edge_ptr& e) { // O(e)
				return e->get_nodes().second == dst and e->get_weight() == weight; // O(1)
			});

			if (edge_it == edges_list.end()) {
				return end();
			}

			iterator it(node_it, adjacency_list_.end());
			it.edge_it_ = edge_it;

			return it; // Overall: O(log(n) + e)
		}

		/**
		 * @brief Returns all nodes that are connected to `src`.
		 * @param src The source node.
		 * @returns All nodes (found from any immediate outgoing edge) connected to `src`, sorted in ascending order.
		 *          This returns **copies** of the specified data.
		 * @complexity O(log(n) + e), where e is the number of outgoing edges associated with `src`.
		 * @throws std::runtime_error if `src` does not exist in the graph.
		 */
		[[nodiscard]] auto connections(N const& src) const -> std::vector<N> {
			if (!is_node(src)) { // O(log(n))
				throw std::runtime_error("Cannot call gdwg::graph<N, E>::connections if src doesn't exist in the "
				                         "graph");
			}

			if (adjacency_list_.find(src) == adjacency_list_.end()) { // O(log(n))
				return {};
			}

			std::vector<N> connections;
			auto& edges_from_src = adjacency_list_.at(src); // O(log(n))

			connections.reserve(edges_from_src.size()); // O(1)

			for (const auto& e : edges_from_src) { // O(e)
				connections.emplace_back(e->get_nodes().second); // O(1)
			}

			return connections; // Overall: O(log(n) + e)
		}

		/**
		 * @brief Returns an iterator to the beginning of the graph.
		 * @returns An iterator pointing to the first element in the container.
		 */
		// Iterator access
		[[nodiscard]] auto begin() const noexcept -> iterator {
			return iterator(adjacency_list_.begin(), adjacency_list_.end());
		}

		/**
		 * @brief Returns an iterator to the end of the graph.
		 * @returns An iterator denoting the end of the iterable list that `begin()` points to.
		 * @remarks `[begin(), end())` shall denote a valid iterable list.
		 */
		[[nodiscard]] auto end() const noexcept -> iterator {
			return iterator(adjacency_list_.end(), adjacency_list_.end());
		}

		// Comparisons

		/**
		 * @brief Compares two graphs for equality.
		 * @param other The other graph to compare with.
		 * @returns `true` if `*this` and `other` contain exactly the same nodes and edges, and `false` otherwise.
		 * @note Complexity: O(n + e), where n is the sum of stored nodes in `*this` and `other`, and e is the sum of
		 * stored edges in `*this` and `other`.
		 */
		[[nodiscard]] auto operator==(graph const& other) const noexcept -> bool {
			if (adjacency_list_.size() != other.adjacency_list_.size()) { // O(1)
				return false;
			}

			auto it1 = this->begin();
			auto it2 = other.begin();

			while (it1 != this->end() and it2 != other.end()) { // O(n + e)  (the loop will end immediately for nodes
				                                                // with no edges)
				const auto& edge1 = *it1;
				const auto& edge2 = *it2;

				if (edge1.from != edge2.from or edge1.to != edge2.to or edge1.weight != edge2.weight) {
					return false;
				}

				++it1;
				++it2;
			}

			return it1 == this->end() and it2 == other.end(); // Overall: O(n + e)
		}

		/**
		 * @brief Outputs the graph to an output stream.
		 * @param os The output stream.
		 * @param g The graph to output.
		 * @returns The output stream.
		 * @details Behaves as a formatted output function of `os`.
		 * @remarks The format is specified thusly:
		 * @code
		 * [source_node_1] [edges_1]
		 * [source_node_2] [edges_2]
		 * ...
		 * [source_node_n] [edges_n]
		 * @endcode
		 * @code
		 * (
		 *   [node_1 -> node_1] | U
		 *   [node_n -> node_1] | W | [weight]
		 *   [node_n -> node_2] | W | [weight]
		 *   ...
		 *   [node_n -> node_n] | W | [weight]
		 * )
		 * @verbatim where [noden -> node1] | U, …, [noden -> noden] | W | [weight] are placeholders for each node’s
		 * connections edge type and the corresponding weight(if exists) the edge has, always start with unweighted
		 * edges, then weighted edges, sorted in ascending order separately.
		 * @endcode
		 * @note  If a node doesn’t have any connections, the corresponding field will be represented as a
		 * line-separated pair of parentheses.
		 * @note `print_edge` function used to format the output.
		 */
		template<typename N2, typename E2>
		friend auto operator<<(std::ostream& os, graph<N2, E2> const& g) -> std::ostream&;

	 private:
		adjacency_list adjacency_list_;
		auto edge_exists(const edge_list& edges, const N& dst, const std::optional<E>& weight) const -> bool {
			return find_edge(edges, dst, weight) != edges.end();
		}
		auto find_edge(const edge_list& edges, const N& dst, const std::optional<E>& weight) const
		    -> edge_list::const_iterator {
			return std::find_if(edges.begin(), edges.end(), [&dst, &weight](const edge_ptr& e) {
				return e->get_nodes().second == dst
				       and ((e->is_weighted() and e->get_weight() == weight) || (!e->is_weighted() and !weight));
			});
		}

		auto create_edge(const N& src, const N& dst, const std::optional<E>& weight) const -> edge_ptr {
			if (weight) {
				return std::make_unique<weighted_edge<N, E>>(src, dst, *weight);
			}
			return std::make_unique<unweighted_edge<N, E>>(src, dst);
		}

		auto sort_edges(edge_list& edges) const -> void {
			std::sort(edges.begin(), edges.end(), [](const edge_ptr& a, const edge_ptr& b) {
				if (a->get_nodes().second != b->get_nodes().second) {
					return a->get_nodes().second < b->get_nodes().second;
				}
				if (a->is_weighted() != b->is_weighted()) {
					return a->is_weighted() < b->is_weighted();
				}
				if (a->is_weighted()) {
					return *a->get_weight() < *b->get_weight();
				}
				return false;
			});
		}

	}; /** @} */ // end of gdwg::graph

	/**
	 * @defgroup graph::iterator
	 * @ingroup gdwg::graph
	 * @{
	 */
	template<typename N, typename E>
	class graph<N, E>::iterator {
	 public:
		using value_type = struct {
			N from;
			N to;
			std::optional<E> weight;
		};
		using reference = value_type;
		using pointer = void;
		using difference_type = std::ptrdiff_t;
		using iterator_category = std::bidirectional_iterator_tag;

		/**
		 * @brief Default constructor.
		 * @details Value-initializes all members.
		 * @remarks Pursuant to the requirements of `std::forward_iterator`, two value-initialized iterators shall
		 * compare equal.
		 */
		iterator() = default;

		// Iterator source
		[[nodiscard]] auto operator*() const noexcept -> reference {
			return {node_it_->first, (*edge_it_)->to_, (*edge_it_)->get_weight()};
		}

		// Iterator traversal
		auto operator++() noexcept -> iterator& {
			++edge_it_;
			// if hit the end of the edge list of the current node, and aren't at the end of the map
			while (node_it_ != node_end_ and edge_it_ == node_it_->second.end()) {
				// move to the next node
				++node_it_;
				// update edge if not at end
				if (node_it_ != node_end_) {
					edge_it_ = node_it_->second.begin();
				}
			}
			return *this;
		}

		auto operator++(int) noexcept -> iterator {
			iterator prev = *this;
			++(*this);
			return prev;
		}

		auto operator--() noexcept -> iterator& {
			// gotta do it the other way around, now
			// if we're at the end, step back until we find a non-empty node-edges pair
			// (don't need to handle if they don't exist,
			//  since if so `--r.end()` doesn't make sense anyway)
			while (node_it_ == node_end_ or edge_it_ == node_it_->second.begin()) {
				--node_it_;
				edge_it_ = node_it_->second.end();
			}
			--edge_it_;
			return *this;
		}

		auto operator--(int) noexcept -> iterator {
			iterator prev = *this;
			--(*this);
			return prev;
		}

		// Iterator comparison
		[[nodiscard]] auto operator==(iterator const& other) const noexcept -> bool {
			return node_it_ == other.node_it_ and (node_it_ == node_end_ or edge_it_ == other.edge_it_);
		}

	 private:
		adjacency_list::const_iterator node_it_;
		adjacency_list::const_iterator node_end_;
		edge_list::const_iterator edge_it_;

		friend class graph<N, E>;

		// explicit constructors
		/**
		 * @brief Constructs an iterator to a specific element in the graph.
		 * @param node_it Iterator pointing to the starting node.
		 * @param node_end Iterator pointing to the end of the adjacency list.
		 */
		[[nodiscard]] explicit iterator(adjacency_list::const_iterator node_it, adjacency_list::const_iterator node_end)
		: node_it_(node_it)
		, node_end_(node_end) {
			if (node_it_ != node_end_) {
				edge_it_ = node_it_->second.begin();
				// if hit the end of the edge list of the current node, and aren't at the end of the map
				while (node_it_ != node_end_ and edge_it_ == node_it_->second.end()) {
					// move to the next node
					++node_it_;
					// update edge if not at end
					if (node_it_ != node_end_) {
						edge_it_ = node_it_->second.begin();
					}
				}
			}
		}

		/**
		 * @brief Returns the index of the current edge in the adjacency list.
		 * @returns The index of the current edge.
		 */
		[[nodiscard]] auto index() const noexcept -> std::size_t {
			return static_cast<size_t>(std::distance(node_it_->second.begin(), edge_it_));
		}
	}; /** @} */ // end of graph::iterator

	/**
	 * @addtogroup gdwg::graph
	 * @{
	 */
	template<typename N, typename E>
	auto operator<<(std::ostream& os, graph<N, E> const& g) -> std::ostream& {
		for (const auto& [node, edges_list] : g.adjacency_list_) {
			os << node << " (" << std::endl;
			for (const auto& e : edges_list) {
				os << "  " << e->print_edge() << std::endl;
			}
			os << ")" << std::endl;
		}
		return os;
	}
	/** @} */

} // namespace gdwg

#endif // GDWG_GRAPH_H

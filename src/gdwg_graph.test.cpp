#include "gdwg_graph.h"

#include <catch2/catch.hpp>

// -------------------- CONSTRUCTORS ------------------

TEST_CASE("Default constructor created with an empty graph", "[graph]") {
	gdwg::graph<std::string, int> g;
	REQUIRE(g.empty());
	REQUIRE(g.nodes().empty());
}

TEST_CASE("Initializer list constructor successfully created graph with custom nodes", "[graph]") {
	gdwg::graph<std::string, int> g{"A", "B", "C"};
	REQUIRE(g.nodes() == std::vector<std::string>{"A", "B", "C"});
	REQUIRE(g.is_node("A"));
	REQUIRE(g.is_node("B"));
	REQUIRE(g.is_node("C"));
	REQUIRE_FALSE(g.is_node("D"));
}

TEST_CASE("Initializer list constructor successfully created graph with duplicate nodes", "[graph]") {
	gdwg::graph<std::string, int> g{"A", "B", "A", "C", "B"};
	REQUIRE(g.nodes() == std::vector<std::string>{"A", "B", "C"});
}

TEST_CASE("Range constructor successfully created graph with custom nodes", "[graph]") {
	std::vector<std::string> nodes{"X", "Y", "Z"};
	gdwg::graph<std::string, int> g(nodes.begin(), nodes.end());
	REQUIRE(g.nodes() == nodes);
	REQUIRE(g.is_node("X"));
	REQUIRE(g.is_node("Y"));
	REQUIRE(g.is_node("Z"));
	REQUIRE_FALSE(g.is_node("W"));
}

TEST_CASE("Move constructor successfully moved graph and emptied original graph") {
	gdwg::graph<std::string, int> g1{"A", "B", "C"};
	g1.insert_edge("A", "B", 1);

	gdwg::graph<std::string, int> g2(std::move(g1));

	REQUIRE(g2.nodes() == std::vector<std::string>{"A", "B", "C"});
	REQUIRE(g2.is_node("A"));
	REQUIRE(g2.is_node("B"));
	REQUIRE(g2.is_node("C"));
	REQUIRE(g2.is_connected("A", "B"));

	REQUIRE(g1.empty());
}
TEST_CASE("Copy constructor successfully created a deep copy of original graph") {
	gdwg::graph<std::string, int> g1{"A", "B", "C"};
	g1.insert_edge("A", "B", 1);

	gdwg::graph<std::string, int> g2(g1);

	REQUIRE(g2 == g1);
	REQUIRE(g2.is_connected("A", "B"));

	g2.insert_node("D"); // changing the copy won't affect the original
	REQUIRE(g2.is_node("D"));
	REQUIRE_FALSE(g1.is_node("D"));
}

// ----------------- EDGE CLASS HIERARCHY ------------------------

TEST_CASE("Edge class polymorphism works as expected") {
	gdwg::weighted_edge<std::string, int> we("A", "B", 5);
	gdwg::unweighted_edge<std::string, int> ue("C", "D");

	gdwg::edge<std::string, int>* e1 = &we;
	gdwg::edge<std::string, int>* e2 = &ue;

	REQUIRE(e1->is_weighted());
	REQUIRE_FALSE(e2->is_weighted());
	REQUIRE(e1->get_weight() == 5);
	REQUIRE(e2->get_weight() == std::nullopt);
}

TEST_CASE("print_edge for weighted_edge returns correct string", "[edge]") {
	gdwg::weighted_edge<std::string, int> we("a", "b", 121340);
	REQUIRE(we.print_edge() == "a -> b | W | 121340");
}

TEST_CASE("print_edge for unweighted_edge returns correct string", "[edge]") {
	gdwg::unweighted_edge<std::string, int> ue("1", "2");
	REQUIRE(ue.print_edge() == "1 -> 2 | U");
}

TEST_CASE("is_weighted for weighted_edge returns true", "[edge]") {
	gdwg::weighted_edge<std::string, int> we("a", "b", 121340);
	REQUIRE(we.is_weighted());
}

TEST_CASE("is_weighted for unweighted_edge returns false", "[edge]") {
	gdwg::unweighted_edge<std::string, int> ue("1", "2");
	REQUIRE_FALSE(ue.is_weighted());
}

TEST_CASE("get_weight for weighted_edge returns correct weight", "[edge]") {
	gdwg::weighted_edge<std::string, int> we("a", "b", 121340);
	REQUIRE(we.get_weight() == 121340);
}

TEST_CASE("get_weight for unweighted_edge returns std::nullopt", "[edge]") {
	gdwg::unweighted_edge<std::string, int> ue("1", "2");
	REQUIRE(ue.get_weight() == std::nullopt);
}

TEST_CASE("get_nodes returns correct nodes", "[edge]") {
	gdwg::weighted_edge<std::string, int> we("a", "b", 121340);
	gdwg::unweighted_edge<std::string, int> ue("1", "2");
	const auto& nodes = we.get_nodes();
	REQUIRE(nodes.first == "a");
	REQUIRE(nodes.second == "b");
	const auto& nodes2 = ue.get_nodes();
	REQUIRE(nodes2.first == "1");
	REQUIRE(nodes2.second == "2");
}

TEST_CASE("operator== between edges works correctly", "[edge]") {
	gdwg::weighted_edge<std::string, int> we1("a", "b", 121340);
	gdwg::weighted_edge<std::string, int> we2("a", "b", 76776);
	gdwg::weighted_edge<std::string, int> we3("a", "b", 76776);
	REQUIRE_FALSE(we1 == we2);
	REQUIRE(we2 == we3);
	REQUIRE(we1 != we2);

	gdwg::unweighted_edge<std::string, int> ue1("a", "b");
	gdwg::unweighted_edge<std::string, int> ue2("c", "d");
	gdwg::unweighted_edge<std::string, int> ue3("c", "d");
	REQUIRE_FALSE(ue1 == ue2);
	REQUIRE(ue2 == ue3);
	REQUIRE(ue1 != ue2);
}
// -------------------- MODIFIERS ------------------------

// tests for insert_node
TEST_CASE("insert_node behaves as expected", "[graph]") {
	auto graph = gdwg::graph<std::string, int>{};

	REQUIRE(graph.insert_node("a"));
	REQUIRE(graph.insert_node("b"));
	REQUIRE(graph.insert_node("c"));
	REQUIRE(graph.insert_node("d"));
	REQUIRE(graph.insert_node("e"));
	REQUIRE(graph.is_node("a"));
	REQUIRE(graph.is_node("b"));
	REQUIRE(graph.is_node("c"));
	REQUIRE(graph.is_node("d"));
	REQUIRE(graph.is_node("e"));
	REQUIRE_FALSE(graph.insert_node("a")); // duplicates aren't allowed
	REQUIRE_FALSE(graph.insert_node("b"));
	REQUIRE_FALSE(graph.insert_node("c"));
	REQUIRE_FALSE(graph.insert_node("d"));
	REQUIRE_FALSE(graph.insert_node("e"));
}

TEST_CASE("insert_node successfully inserts node with an empty string") {
	auto graph = gdwg::graph<std::string, int>{};

	REQUIRE(graph.insert_node(""));
	REQUIRE(graph.is_node(""));

	REQUIRE_FALSE(graph.insert_node(""));
	REQUIRE(graph.is_node(""));

	auto nodes = graph.nodes();
	REQUIRE(nodes.size() == 1);
	REQUIRE(nodes[0] == "");
}

TEST_CASE("insert_node successfully inserts node with a very long string", "[graph]") {
	auto graph = gdwg::graph<std::string, int>{};

	std::string long_string(10000, 'a');
	REQUIRE(graph.insert_node(long_string));
	REQUIRE(graph.is_node(long_string));

	auto nodes = graph.nodes();
	REQUIRE(nodes.size() == 1);
	REQUIRE(nodes[0] == long_string);
}

// tests for insert_edge
TEST_CASE("insert_edge successfully inserts node with string type node", "[graph]") {
	gdwg::graph<std::string, int> g;

	g.insert_node("a");
	g.insert_node("b");
	g.insert_node("c");
	g.insert_node("d");

	REQUIRE(g.insert_edge("a", "b", 42));
	REQUIRE_FALSE(g.insert_edge("a", "b", 42)); // duplicates not allowed
	REQUIRE(g.insert_edge("c", "a"));
	REQUIRE_FALSE(g.insert_edge("c", "a")); // duplicates not allowed

	REQUIRE(g.insert_edge("a", "b", 24));
	REQUIRE(g.insert_edge("b", "d", 7));
	REQUIRE(g.insert_edge("d", "d", 0)); // self-loop is allowed
}

TEST_CASE("insert_edge successfully inserts node with int type node", "[graph]") {
	gdwg::graph<int, int> g;

	g.insert_node(1);
	g.insert_node(2);
	g.insert_node(3);
	g.insert_node(4);

	REQUIRE(g.insert_edge(1, 2, 42));
	REQUIRE_FALSE(g.insert_edge(1, 2, 42)); // duplicates not allowed
	REQUIRE(g.insert_edge(3, 1));
	REQUIRE_FALSE(g.insert_edge(3, 1)); // duplicates not allowed

	REQUIRE(g.insert_edge(1, 2, 24));
	REQUIRE(g.insert_edge(2, 4, 7));
	REQUIRE(g.insert_edge(4, 4, 0)); // self-loop is allowed
}
TEST_CASE("insert_edge for non-existent nodes throws runtime error", "[graph]") {
	gdwg::graph<std::string, double> g;

	g.insert_node("a");
	g.insert_node("b");

	REQUIRE_THROWS_AS(g.insert_edge("a", "c", 3.5), std::runtime_error);
	REQUIRE_THROWS_AS(g.insert_edge("d", "b", 4.2), std::runtime_error);
	REQUIRE_THROWS_AS(g.insert_edge("e", "f"), std::runtime_error);
}

// tests for replace_node
TEST_CASE("replace_node successfully replaces an existing node", "[graph]") {
	gdwg::graph<std::string, int> g;
	g.insert_node("a");
	g.insert_node("b");
	g.insert_node("c");
	g.insert_edge("a", "b", 1);
	g.insert_edge("a", "c", 2);
	g.insert_edge("b", "c", 3);

	REQUIRE(g.replace_node("a", "d"));
	REQUIRE(g.is_node("d"));
	REQUIRE_FALSE(g.is_node("a"));
	REQUIRE(g.is_connected("d", "b"));
	REQUIRE(g.is_connected("d", "c"));
	REQUIRE(g.is_connected("b", "c"));
	REQUIRE(g.edges("d", "b").size() == 1);
	REQUIRE(g.edges("d", "c").size() == 1);
	REQUIRE(g.edges("b", "c").size() == 1);
	REQUIRE(g.replace_node("d", "random_node"));
	REQUIRE_FALSE(g.is_node("d"));
	REQUIRE(g.is_node("random_node"));
	REQUIRE(g.is_connected("random_node", "b"));
	REQUIRE(g.is_connected("random_node", "c"));
}

TEST_CASE("replace_node on non-existent data throws error", "[graph]") {
	gdwg::graph<std::string, int> g;
	g.insert_node("A");
	g.insert_node("B");

	REQUIRE_THROWS_AS(g.replace_node("non", "existent"), std::runtime_error);
	REQUIRE_THROWS_AS(g.replace_node("n/a", "A"), std::runtime_error);
}

TEST_CASE("replace_node called on self-loops", "[graph]") {
	gdwg::graph<std::string, int> g;
	g.insert_node("c");
	g.insert_edge("c", "c", 1);

	REQUIRE(g.replace_node("c", "d"));
	REQUIRE(g.is_node("d"));
	REQUIRE_FALSE(g.is_node("c"));
	REQUIRE(g.is_connected("d", "d"));
}

TEST_CASE("replace_node with multiple edges", "[graph]") {
	gdwg::graph<std::string, int> g;
	g.insert_node("z");
	g.insert_node("a");
	g.insert_node("b");
	g.insert_edge("z", "a", 1);
	g.insert_edge("a", "b", 2);
	g.insert_edge("b", "z", 3);
	g.insert_edge("z", "b", 4);

	REQUIRE(g.replace_node("z", "x"));
	REQUIRE(g.is_node("x"));
	REQUIRE_FALSE(g.is_node("z"));
	REQUIRE(g.is_connected("x", "a"));
	REQUIRE(g.is_connected("a", "b"));
	REQUIRE(g.is_connected("b", "x"));
	REQUIRE(g.is_connected("x", "b"));
}

TEST_CASE("replace_node does nothing a new_data already exists", "[graph]") {
	gdwg::graph<std::string, int> g;
	g.insert_node("myjob");
	g.insert_node("AI");
	g.insert_edge("myjob", "AI", 1);

	REQUIRE_FALSE(g.replace_node("myjob", "AI"));
	REQUIRE(g.is_node("myjob"));
	REQUIRE(g.is_node("AI"));
	REQUIRE(g.is_connected("myjob", "AI"));
}

// tests for merge_replace_node
TEST_CASE("merge_replace_node merges and replaces node with a new node", "[graph]") {
	gdwg::graph<std::string, int> g;
	g.insert_node("A");
	g.insert_node("B");
	g.insert_node("C");
	g.insert_edge("A", "B", 1);
	g.insert_edge("A", "C", 2);
	g.insert_edge("B", "C", 3);

	g.merge_replace_node("A", "B");
	REQUIRE(g.is_node("B"));
	REQUIRE_FALSE(g.is_node("A"));
	REQUIRE(g.is_connected("B", "B"));
	REQUIRE(g.is_connected("B", "C"));
	REQUIRE(g.is_connected("B", "C"));
	REQUIRE(g.edges("B", "B").size() == 1);
	REQUIRE(*(g.edges("B", "B")[0])->get_weight() == 1);
	REQUIRE(g.edges("B", "C").size() == 2);
	REQUIRE(*(g.edges("B", "C")[0])->get_weight() == 2);
	REQUIRE(*(g.edges("B", "C")[1])->get_weight() == 3);

	gdwg::graph<char, int> g2;
	g2.insert_node('A');
	g2.insert_node('B');
	g2.insert_node('C');
	g2.insert_edge('A', 'B', 1);
	g2.insert_edge('B', 'A', 2);
	g2.insert_edge('B', 'C', 3);
	g2.insert_edge('C', 'B', 4);
	g2.insert_edge('A', 'A', 5);

	g2.merge_replace_node('A', 'B');

	REQUIRE_FALSE(g2.is_node('A'));
	REQUIRE(g2.is_node('B'));
	REQUIRE(g2.is_node('C'));
	REQUIRE(g2.is_connected('B', 'B'));
	REQUIRE(g2.is_connected('B', 'C'));
	REQUIRE(g2.is_connected('C', 'B'));

	const auto& edges = g2.edges('B', 'B');
	REQUIRE(edges.size() == 3);
	REQUIRE(edges[0]->get_weight() == 1);
	REQUIRE(edges[1]->get_weight() == 2);
	REQUIRE(edges[2]->get_weight() == 5);
}

TEST_CASE("merge_replace_node throws exception a either node does not exist", "[graph]") {
	gdwg::graph<std::string, int> g;
	g.insert_node("A");
	g.insert_node("B");

	REQUIRE_THROWS_AS(g.merge_replace_node("C", "B"), std::runtime_error);
	REQUIRE_THROWS_AS(g.merge_replace_node("A", "D"), std::runtime_error);
}

TEST_CASE("merge_replace_node removes duplicate edges", "[graph]") {
	gdwg::graph<std::string, int> g;
	g.insert_node("A");
	g.insert_node("B");
	g.insert_node("C");
	g.insert_edge("A", "B", 1);
	g.insert_edge("A", "C", 2);
	g.insert_edge("B", "B", 1);

	g.merge_replace_node("A", "B");
	REQUIRE(g.is_node("B"));
	REQUIRE_FALSE(g.is_node("A"));
	REQUIRE(g.is_connected("B", "B"));
	REQUIRE(g.is_connected("B", "C"));
	REQUIRE(g.edges("B", "B").size() == 1);
	REQUIRE(g.edges("B", "B")[0]->get_weight() == 1);
	REQUIRE(g.edges("B", "C").size() == 1);
	REQUIRE(g.edges("B", "C")[0]->get_weight() == 2);
}

TEST_CASE("merge_replace_node with node that has self-loops", "[graph]") {
	gdwg::graph<std::string, int> g;
	g.insert_node("A");
	g.insert_node("B");
	g.insert_edge("A", "A", 1);
	g.insert_edge("A", "B", 2);

	g.merge_replace_node("A", "B");
	REQUIRE(g.is_node("B"));
	REQUIRE_FALSE(g.is_node("A"));
	REQUIRE(g.is_connected("B", "B"));
	REQUIRE(g.edges("B", "B").size() == 2);
	REQUIRE(g.edges("B", "B")[0]->get_weight() == 1);
	REQUIRE(g.edges("B", "B")[1]->get_weight() == 2);
}

TEST_CASE("merge_replace_node with multiple incoming and outgoing edges", "[graph]") {
	gdwg::graph<std::string, int> g;
	g.insert_node("A");
	g.insert_node("B");
	g.insert_node("C");
	g.insert_node("D");
	g.insert_edge("A", "B", 1);
	g.insert_edge("A", "C", 2);
	g.insert_edge("B", "C", 3);
	g.insert_edge("C", "A", 4);
	g.insert_edge("D", "A", 5);

	g.merge_replace_node("A", "B");
	REQUIRE(g.is_node("B"));
	REQUIRE_FALSE(g.is_node("A"));
	REQUIRE(g.is_connected("B", "B"));
	REQUIRE(g.is_connected("B", "C"));
	REQUIRE(g.is_connected("C", "B"));
	REQUIRE(g.is_connected("D", "B"));
	REQUIRE(g.edges("B", "B").size() == 1);
	REQUIRE(g.edges("B", "B")[0]->get_weight() == 1);
	REQUIRE(g.edges("B", "C").size() == 2);
	REQUIRE(g.edges("B", "C")[0]->get_weight() == 2);
	REQUIRE(g.edges("B", "C")[1]->get_weight() == 3);
	REQUIRE(g.edges("C", "B").size() == 1);
	REQUIRE(g.edges("C", "B")[0]->get_weight() == 4);
	REQUIRE(g.edges("D", "B").size() == 1);
	REQUIRE(g.edges("D", "B")[0]->get_weight() == 5);
}

// tests for erase_node
TEST_CASE("erase_node on a non-existent node") {
	gdwg::graph<std::string, int> g;
	REQUIRE_FALSE(g.erase_node("a"));
}

TEST_CASE("erase_node on existing nodes") {
	gdwg::graph<std::string, int> g;
	g.insert_node("A");
	g.insert_node("B");
	g.insert_edge("A", "B");
	REQUIRE(g.erase_node("A"));
	REQUIRE_FALSE(g.is_node("A"));
	REQUIRE_THROWS_AS(g.is_connected("A", "B"), std::runtime_error);
}

TEST_CASE("erase_node erases all outgoing and incoming edges of the target node, including self-loops") {
	gdwg::graph<std::string, int> g;
	g.insert_node("A");
	g.insert_node("B");
	g.insert_node("C");
	g.insert_edge("A", "B", 1);
	g.insert_edge("A", "B", 13);
	g.insert_edge("B", "B", 7);
	g.insert_edge("B", "C", 2);
	g.insert_edge("A", "B", 25);

	REQUIRE(g.erase_node("B") == true);
	REQUIRE(g.is_node("A") == true);
	REQUIRE(g.is_node("B") == false);
	REQUIRE_THROWS_AS(g.edges("A", "B"), std::runtime_error);
	REQUIRE_THROWS_AS(g.edges("B", "B"), std::runtime_error);
	REQUIRE_THROWS_AS(g.edges("B", "C"), std::runtime_error);
}

// tests for erase_edge by nodes
TEST_CASE("erase_edge by nodes for non-existent data") {
	gdwg::graph<std::string, int> empty_graph;
	REQUIRE_THROWS_AS(empty_graph.erase_edge("non", "existent"), std::runtime_error);

	gdwg::graph<std::string, int> g{"A"};
	REQUIRE_THROWS_AS(g.erase_edge("A", "B"), std::runtime_error);
	REQUIRE_THROWS_AS(g.erase_edge("B", "A"), std::runtime_error);
	REQUIRE_FALSE(g.erase_edge("A", "A"));

	g.insert_node("B");
	REQUIRE_FALSE(g.erase_edge("A", "B"));
	REQUIRE_FALSE(g.erase_edge("B", "A"));
	REQUIRE_FALSE(g.erase_edge("A", "A"));
	REQUIRE_FALSE(g.erase_edge("B", "B"));

	g.insert_edge("A", "B", 1);
	REQUIRE_THROWS_AS(g.erase_edge("a", "b", 1), std::runtime_error);
	REQUIRE_FALSE(g.erase_edge("A", "A"));
	REQUIRE_FALSE(g.erase_edge("A", "B", 100000));
	REQUIRE_FALSE(g.erase_edge("B", "A", 1));
	REQUIRE_FALSE(g.erase_edge("A", "A", 1));
	REQUIRE_FALSE(g.erase_edge("B", "B", 1));
}

TEST_CASE("erase_edge by nodes on unweighted edge") {
	gdwg::graph<std::string, int> g;
	g.insert_node("a");
	g.insert_node("b");
	g.insert_edge("a", "b");

	REQUIRE(g.is_connected("a", "b"));

	bool erased = g.erase_edge("a", "b");
	REQUIRE(erased);
	REQUIRE_FALSE(g.is_connected("a", "b"));
}

TEST_CASE("erase_edge by nodes on weighted edge") {
	gdwg::graph<std::string, int> g;
	g.insert_node("a");
	g.insert_node("b");
	g.insert_edge("a", "b", 42);

	REQUIRE(g.is_connected("a", "b"));

	bool erased = g.erase_edge("a", "b", 42);
	REQUIRE(erased);
	REQUIRE_FALSE(g.is_connected("a", "b"));
}

// Tests for erase_edge by a single iterator
TEST_CASE("erase_edge(iterator) on empty graph returns end()", "[graph]") {
	gdwg::graph<int, int> g;
	auto it = g.begin();
	auto new_it = g.erase_edge(it);
	REQUIRE(new_it == g.end());
}

TEST_CASE("erase_edge(iterator) correctly erases a single edge", "[graph]") {
	gdwg::graph<int, int> g;
	g.insert_node(1);
	g.insert_node(5);
	g.insert_edge(1, 5, 124);

	auto it = g.begin();
	REQUIRE(it != g.end());
	REQUIRE((*it).from == 1);
	REQUIRE((*it).to == 5);
	REQUIRE(*(*it).weight == 124);

	auto new_it = g.erase_edge(it);
	REQUIRE(new_it == g.end());

	auto find_it = g.find(1, 5, 124);
	REQUIRE(find_it == g.end());
}

TEST_CASE("erase_edge(iterator) correctly erases a self-loop", "[graph]") {
	gdwg::graph<int, int> g;
	g.insert_node(1);
	g.insert_edge(1, 1, 5);
	g.insert_edge(1, 1, 10);

	auto it = g.begin();
	REQUIRE((*it).from == 1);
	REQUIRE((*it).to == 1);
	REQUIRE(*(*it).weight == 5);

	auto new_it = g.erase_edge(it);
	REQUIRE((*new_it).from == 1);
	REQUIRE((*new_it).to == 1);
	REQUIRE(*(*new_it).weight == 10);
	REQUIRE(g.is_connected(1, 1));
}

TEST_CASE("erase_edge(iterator) correctly erases an edge in the middle of multiple edges", "[graph]") {
	gdwg::graph<int, int> g;
	g.insert_node(1);
	g.insert_node(2);
	g.insert_node(3);
	g.insert_node(4);
	g.insert_edge(1, 2, 124);
	g.insert_edge(1, 3, 132);
	g.insert_edge(1, 4, 88);

	auto it = g.begin();
	++it;
	REQUIRE((*it).from == 1);
	REQUIRE((*it).to == 3);
	REQUIRE(*(*it).weight == 132);

	auto new_it = g.erase_edge(it);
	REQUIRE((*new_it).from == 1);
	REQUIRE((*new_it).to == 4);
	REQUIRE(*(*new_it).weight == 88);
	REQUIRE(g.is_node(1));
	REQUIRE(g.is_node(2));
	REQUIRE(g.is_node(3));
	REQUIRE(g.is_node(4));
	REQUIRE(g.is_connected(1, 2));
	REQUIRE(g.is_connected(1, 4));
	REQUIRE_FALSE(g.is_connected(1, 3));
}

// Tests for erase_edge in range of two iterators
TEST_CASE("erase_edge(iterator, iterator) on empty graph returns end()", "[graph]") {
	gdwg::graph<int, int> g;
	auto it = g.begin();
	auto new_it = g.erase_edge(it, it);
	REQUIRE(new_it == g.end());
}

TEST_CASE("erase_edge(iterator, iterator) correctly erases a single edge", "[graph]") {
	gdwg::graph<int, int> g;
	g.insert_node(1);
	g.insert_node(5);
	g.insert_edge(1, 5, 124);

	auto it = g.begin();
	auto new_it = g.erase_edge(it, ++g.begin());
	REQUIRE(new_it == g.end());
	REQUIRE(g.begin() == g.end());
	REQUIRE(g.is_node(1));
	REQUIRE(g.is_node(5));
	REQUIRE_FALSE(g.is_connected(1, 5));
}

// --------------------- ACCESSORS -------------------------

// tests for is_connected
TEST_CASE("is_connected between existing nodes with an edge") {
	gdwg::graph<std::string, int> g;
	g.insert_node("src");
	g.insert_node("dst");
	g.insert_edge("src", "dst", 10);

	REQUIRE(g.is_connected("src", "dst") == true);
}

TEST_CASE("is_connected between existing nodes without an edge") {
	gdwg::graph<std::string, int> g;
	g.insert_node("src");
	g.insert_node("dst");

	REQUIRE(g.is_connected("src", "dst") == false);
}

TEST_CASE("is_connected for non-existent src or dst node") {
	gdwg::graph<std::string, int> g;
	g.insert_node("src");
	g.insert_node("dst");

	REQUIRE_THROWS_AS(g.is_connected("nonExistentSrc", "dst"), std::runtime_error);
	REQUIRE_THROWS_AS(g.is_connected("src", "nonExistentDst"), std::runtime_error);
}

// tests for edges
TEST_CASE("graph::edges called on non-existent src or dst node", "[graph]") {
	gdwg::graph<std::string, int> g;
	g.insert_node("src");
	g.insert_node("dst");
	g.insert_edge("src", "dst");

	REQUIRE_THROWS_AS(g.edges("src", "nectarine"), std::runtime_error);
	REQUIRE_THROWS_AS(g.edges("demon", "dst"), std::runtime_error);
	REQUIRE_THROWS_AS(g.edges("demon", "nectarine"), std::runtime_error);
}

TEST_CASE("graph::edges called on non-existent edge between existing nodes", "[graph]") {
	gdwg::graph<std::string, int> g;
	g.insert_node("dog");
	g.insert_node("elephant");

	auto edges = g.edges("dog", "elephant");
	REQUIRE(edges.empty());
}
TEST_CASE("graph::edges correctly returns existing unweighted edges", "[graph]") {
	gdwg::graph<std::string, int> g;
	g.insert_node("frog");
	g.insert_node("giraffe");

	g.insert_edge("frog", "giraffe");

	auto edges = g.edges("frog", "giraffe");
	REQUIRE(edges.size() == 1);
	REQUIRE(edges[0]->is_weighted() == false);
}

TEST_CASE("graph::edges returns existing edges in correct order (unweighted > weighted, ordered by weight(ascending))",
          "[graph]") {
	gdwg::graph<std::string, int> g;
	g.insert_node("hippo");
	g.insert_node("iguana");

	g.insert_edge("hippo", "iguana", std::numeric_limits<int>::max());
	g.insert_edge("hippo", "iguana", -4);
	g.insert_edge("hippo", "iguana", std::numeric_limits<int>::min());
	g.insert_edge("hippo", "iguana", 15);
	g.insert_edge("hippo", "iguana", 5);
	g.insert_edge("hippo", "iguana", std::nullopt);
	g.insert_edge("hippo", "iguana"); // this will count as duplicate edge and won't be added
	g.insert_edge("hippo", "iguana", 10);

	auto edges = g.edges("hippo", "iguana");
	REQUIRE(edges.size() == 7);
	REQUIRE(edges[0]->is_weighted() == false);
	REQUIRE(edges[1]->get_weight() == std::numeric_limits<int>::min());
	REQUIRE(edges[2]->get_weight() == -4);
	REQUIRE(edges[3]->get_weight() == 5);
	REQUIRE(edges[4]->get_weight() == 10);
	REQUIRE(edges[5]->get_weight() == 15);
	REQUIRE(edges[6]->get_weight() == std::numeric_limits<int>::max());
}

TEST_CASE("graph::edges correctly returns self-looped edges") {
	gdwg::graph<std::string, int> g;
	g.insert_node("quokka");

	g.insert_edge("quokka", "quokka");
	g.insert_edge("quokka", "quokka", 25);

	auto self_edges = g.edges("quokka", "quokka");
	REQUIRE(self_edges.size() == 2);
	REQUIRE(self_edges[0]->is_weighted() == false);
	REQUIRE(self_edges[1]->is_weighted() == true);
	REQUIRE(self_edges[1]->get_weight() == 25);
}

// tests for find
TEST_CASE("find returns end iterator for empty graph", "[graph]") {
	gdwg::graph<std::string, int> g;
	auto it = g.find("x", "z", 5);
	REQUIRE(it == g.end());
}

TEST_CASE("find returns end iterator for non-existent nodes", "[graph]") {
	gdwg::graph<std::string, int> g;
	g.insert_node("A");
	g.insert_node("B");
	g.insert_edge("A", "B", 5);
	auto it1 = g.find("A", "B");
	auto it2 = g.find("A", "C", 5);
	auto it3 = g.find("C", "B", 5);
	REQUIRE(it1 == g.end());
	REQUIRE(it2 == g.end());
	REQUIRE(it3 == g.end());
}

TEST_CASE("find returns end iterator for non-existent edge", "[graph]") {
	gdwg::graph<std::string, int> g;
	g.insert_node("A");
	g.insert_node("B");
	g.insert_edge("A", "B", 5);
	auto it1 = g.find("A", "B");
	auto it2 = g.find("A", "A", 5);
	auto it3 = g.find("B", "B", 5);
	auto it4 = g.find("A", "B", 3643435);
	REQUIRE(it1 == g.end());
	REQUIRE(it2 == g.end());
	REQUIRE(it3 == g.end());
	REQUIRE(it4 == g.end());
}

TEST_CASE("find returns correct iterator for existing edge", "[graph]") {
	gdwg::graph<std::string, int> g;
	g.insert_node("A");
	g.insert_node("B");
	g.insert_edge("A", "B", 5);

	auto it = g.find("A", "B", 5);
	REQUIRE(it != g.end());
	REQUIRE((*it).from == "A");
	REQUIRE((*it).to == "B");
	REQUIRE(*(*it).weight == 5);

	gdwg::graph<int, std::string> g2;
	g2.insert_node(2);
	g2.insert_node(345);
	g2.insert_edge(2, 345, "hello");

	auto it2 = g2.find(2, 345, "hello");
	REQUIRE(it2 != g2.end());
	REQUIRE((*it2).from == 2);
	REQUIRE((*it2).to == 345);
	REQUIRE(*(*it2).weight == "hello");

	gdwg::graph<std::string, int> g3;
	g3.insert_node("A");
	g3.insert_node("B");
	g3.insert_edge("A", "B");

	auto it3 = g3.find("A", "B", std::nullopt);
	REQUIRE(it3 != g3.end());
	REQUIRE((*it3).from == "A");
	REQUIRE((*it3).to == "B");
	REQUIRE((*it3).weight == std::nullopt);
}

TEST_CASE("find works correctly with self-loops", "[graph]") {
	gdwg::graph<std::string, int> g;
	g.insert_node("A");
	g.insert_edge("A", "A", 5);

	auto it = g.find("A", "A", 5);
	REQUIRE(it != g.end());
	REQUIRE((*it).from == "A");
	REQUIRE((*it).to == "A");
	REQUIRE(*(*it).weight == 5);
}

TEST_CASE("find works correctly with multiple edges between same nodes", "[graph]") {
	gdwg::graph<std::string, int> g;
	g.insert_node("A");
	g.insert_node("B");
	g.insert_edge("A", "B", 5);
	g.insert_edge("A", "B", 10);

	auto it1 = g.find("A", "B", 5);
	REQUIRE(it1 != g.end());
	REQUIRE((*it1).from == "A");
	REQUIRE((*it1).to == "B");
	REQUIRE(*(*it1).weight == 5);

	auto it2 = g.find("A", "B", 10);
	REQUIRE(it2 != g.end());
	REQUIRE((*it2).from == "A");
	REQUIRE((*it2).to == "B");
	REQUIRE(*(*it2).weight == 10);
}

// tests for connections
TEST_CASE("connections returns copies of the specified data", "[graph]") {
	gdwg::graph<std::string, int> g;
	g.insert_node("raiden");
	g.insert_node("shogun");
	g.insert_edge("raiden", "shogun", 1);

	auto connections = g.connections("raiden");
	REQUIRE(connections[0] == "shogun");
	REQUIRE(connections.size() == 1);

	connections[0] = "era"; // modifying copy wont affect the original

	auto new_connections = g.connections("raiden");
	REQUIRE(new_connections.size() == 1);
	REQUIRE(new_connections[0] == "shogun");
}

TEST_CASE("connections throws runtime error if src doesn't exist in graph", "[graph]") {
	gdwg::graph<int, std::string> g;
	g.insert_node(6);

	REQUIRE_THROWS_AS(g.connections(4), std::runtime_error);
}

TEST_CASE("connections returns correct results of all nodes connected to src", "[graph]") {
	gdwg::graph<char, int> g;
	g.insert_node('A');
	g.insert_node('B');
	g.insert_node('C');
	g.insert_node('D');
	g.insert_node('E');

	g.insert_edge('A', 'A', 1);
	g.insert_edge('A', 'B', 1);
	g.insert_edge('A', 'C', 1);
	g.insert_edge('A', 'D', 1);
	g.insert_edge('B', 'E', 1);
	g.insert_edge('C', 'A', 1); // connections only looks at outgoing edges, this will be ignored
	g.insert_edge('D', 'A', 1); // same as above

	auto connections = g.connections('A');
	REQUIRE(connections.size() == 4);
	REQUIRE(std::find(connections.begin(), connections.end(), 'A') != connections.end());
	REQUIRE(std::find(connections.begin(), connections.end(), 'B') != connections.end());
	REQUIRE(std::find(connections.begin(), connections.end(), 'C') != connections.end());
	REQUIRE(std::find(connections.begin(), connections.end(), 'D') != connections.end());
	REQUIRE(std::find(connections.begin(), connections.end(), 'E') == connections.end());
}

// ------------------ COMPARISONS ----------------------

TEST_CASE("graph equality comparison operator== works as expected") {
	gdwg::graph<std::string, int> g1;
	gdwg::graph<std::string, int> g2;
	gdwg::graph<std::string, int> g3;
	gdwg::graph<std::string, int> g4;

	g1.insert_node("a");
	g1.insert_node("s");
	g1.insert_edge("a", "s");

	g2.insert_node("a");
	g2.insert_node("s");
	g2.insert_edge("a", "s");

	g3.insert_node("d");
	g3.insert_node("f");
	g3.insert_edge("d", "f", 1);

	g4.insert_node("d");
	g4.insert_node("f");
	g4.insert_edge("d", "f", 1);

	REQUIRE(g1 == g1);
	REQUIRE(g1 == g2);
	REQUIRE_FALSE(g1 == g3);
	REQUIRE_FALSE(g1 == g4);
	REQUIRE(g3 == g3);
	REQUIRE(g3 == g4);
	REQUIRE_FALSE(g2 == g3);
	REQUIRE_FALSE(g2 == g4);
}

// -------------------- EXTRACTOR -------------------------

// tests for operator<<
TEST_CASE("operator<< on empty graph is empty string", "[graph]") {
	gdwg::graph<int, int> g;
	std::ostringstream oss;
	oss << g;
	REQUIRE(oss.str() == "");
}

TEST_CASE("operator<< with print_edge behaves correctly during runtime graph operations") {
	gdwg::graph<std::string, int> g = {"A", "B"};

	g.insert_edge("A", "B", 6);
	g.insert_node("Z");
	g.insert_node("E");
	g.insert_edge("Z", "E");
	g.insert_edge("A", "Z", 1000);
	g.insert_edge("Z", "Z", -123);

	auto out = std::ostringstream{};
	out << g;

	auto const expected_output = std::string(R"(A (
  A -> B | W | 6
  A -> Z | W | 1000
)
B (
)
E (
)
Z (
  Z -> E | U
  Z -> Z | W | -123
)
)");

	REQUIRE(out.str() == expected_output);

	out.str("");
	// runtime operations
	REQUIRE(g.replace_node("A", "V"));
	g.erase_edge("Z", "E");
	g.erase_node("B");

	out << g;

	auto const expected_output_2 = std::string(R"(E (
)
V (
  V -> Z | W | 1000
)
Z (
  Z -> Z | W | -123
)
)");

	REQUIRE(out.str() == expected_output_2);
}

TEST_CASE("operator<< with print_edge works as expected") {
	using graph = gdwg::graph<int, int>;

	auto const v = std::vector<std::tuple<int, int, std::optional<int>>>{
	    {4, 1, -4},
	    {3, 2, 2},
	    {2, 4, std::nullopt},
	    {2, 1, 1},
	    {6, 2, 5},
	    {6, 3, 10},
	    {1, 5, -1},
	    {3, 6, -8},
	    {4, 5, 3},
	    {5, 2, std::nullopt},
	};

	auto g = graph{};
	for (const auto& [from, to, weight] : v) {
		g.insert_node(from);
		g.insert_node(to);
		if (weight.has_value()) {
			g.insert_edge(from, to, weight.value());
		}
		else {
			g.insert_edge(from, to);
		}
	}
	g.insert_node(64);

	auto out = std::ostringstream{};
	out << g;

	auto const expected_output = std::string(R"(1 (
  1 -> 5 | W | -1
)
2 (
  2 -> 1 | W | 1
  2 -> 4 | U
)
3 (
  3 -> 2 | W | 2
  3 -> 6 | W | -8
)
4 (
  4 -> 1 | W | -4
  4 -> 5 | W | 3
)
5 (
  5 -> 2 | U
)
6 (
  6 -> 2 | W | 5
  6 -> 3 | W | 10
)
64 (
)
)");

	REQUIRE(out.str() == expected_output);
}

// ------------------- ITERATORS -----------------------

TEST_CASE("default constructor for iterator works as expected", "[graph::iterator]") {
	gdwg::graph<int, int> g1;
	gdwg::graph<int, int> g2;

	REQUIRE(g1.begin() == g1.end());
	REQUIRE(g2.begin() == g2.end());
	REQUIRE_FALSE(g1.begin() == g2.begin()); // they are pointing at different iterable list
}

TEST_CASE("iterator prefix increment and decrement traversal works correctly", "[graph::iterator]") {
	gdwg::graph<int, int> g;
	g.insert_node(0);
	g.insert_node(173);
	g.insert_node(-2938475);
	g.insert_edge(0, 173, 562);
	g.insert_edge(0, -2938475, -1289);
	g.insert_edge(173, -2938475, 7);

	auto it = g.begin();
	REQUIRE((*it).from == 0);
	REQUIRE((*it).to == -2938475);
	REQUIRE(*(*it).weight == -1289);

	++it;
	REQUIRE((*it).from == 0);
	REQUIRE((*it).to == 173);
	REQUIRE(*(*it).weight == 562);

	++it;
	REQUIRE((*it).from == 173);
	REQUIRE((*it).to == -2938475);
	REQUIRE(*(*it).weight == 7);

	++it;
	REQUIRE(it == g.end());

	--it;
	REQUIRE((*it).from == 173);
	REQUIRE((*it).to == -2938475);
	REQUIRE(*(*it).weight == 7);

	--it;
	REQUIRE((*it).from == 0);
	REQUIRE((*it).to == 173);
	REQUIRE(*(*it).weight == 562);

	--it;
	REQUIRE((*it).from == 0);
	REQUIRE((*it).to == -2938475);
	REQUIRE(*(*it).weight == -1289);

	REQUIRE(it == g.begin());
}

TEST_CASE("iterator comparison operator works correctly", "[graph::iterator]") {
	gdwg::graph<int, int> g;
	g.insert_node(1);
	g.insert_node(2);
	g.insert_edge(1, 2, 10);

	auto it1 = g.begin();
	auto it2 = g.begin();
	auto end = g.end();

	REQUIRE(it1 == it2); // comparing within the same container list
	REQUIRE_FALSE(it1 == end);
	REQUIRE(++it1 == end);
}

TEST_CASE("iterator postfix increment and decrement work correctly", "[graph::iterator]") {
	gdwg::graph<int, int> g;
	g.insert_node(1);
	g.insert_node(2);
	g.insert_edge(1, 2, 95);
	g.insert_edge(1, 2, 8);

	auto it = g.begin();
	auto old_it = it++;
	REQUIRE(old_it == g.begin());
	REQUIRE((*old_it).from == 1);
	REQUIRE((*old_it).to == 2);
	REQUIRE(*(*old_it).weight == 8);
	REQUIRE((*it).from == 1);
	REQUIRE((*it).to == 2);
	REQUIRE(*(*it).weight == 95);

	old_it = it++;
	REQUIRE((*old_it).from == 1);
	REQUIRE((*old_it).to == 2);
	REQUIRE(*(*old_it).weight == 95);
	REQUIRE(it == g.end());
}

TEST_CASE("iterator dereference operator returns correct value", "[graph::iterator]") {
	gdwg::graph<int, int> g;
	g.insert_node(83);
	g.insert_node(-2938475);
	g.insert_edge(83, -2938475, 415);

	auto it = g.begin();
	auto value = *it;

	REQUIRE(value.from == 83);
	REQUIRE(value.to == -2938475);
	REQUIRE(*value.weight == 415);
}

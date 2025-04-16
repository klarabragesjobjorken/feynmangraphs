#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/biconnected_components.hpp>
#include <boost/graph/vf2_sub_graph_iso.hpp>

bool get_yes_no(const std::string &prompt) {
  // std::cout << prompt << " (yes/no): ";
  std::string input;
  std::cin >> input;
  std::transform(input.begin(), input.end(), input.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  return input == "yes";
}

enum class GraphType { VACUUM, FEYNMAN };

std::optional<GraphType> get_graph_type() {
  // std::cout << "You may generate Feynman graphs or vacuum graphs.\n";
  if (get_yes_no("Do you want to generate Feynman graphs?")) {
    return GraphType::FEYNMAN;
  } else if (get_yes_no("Do you want to generate vacuum graphs?")) {
    return GraphType::VACUUM;
  }
  return std::nullopt;
}

std::optional<int> get_vertex_count() {
  // std::cout << "How many vertices do you want? ";
  int vertex_count;
  std::cin >> vertex_count;
  std::optional<int> vertex_count_opt = std::nullopt;
  if (!std::cin.fail() && vertex_count > 0) {
    vertex_count_opt = vertex_count;
  }
  return vertex_count_opt;
}

template <typename Graph1, typename Graph2> class noop_callback {
public:
  noop_callback(const Graph1 &, const Graph2 &) {}

  template <typename CorrespondenceMap1To2, typename CorrespondenceMap2To1>
  bool operator()(CorrespondenceMap1To2, CorrespondenceMap2To1) const {
    return false;
  }
};

class Multigraph {
private:
  using Graph =
      boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS>;
  using vertex_descriptor = boost::graph_traits<Graph>::vertex_descriptor;
  using edge_descriptor = boost::graph_traits<Graph>::edge_descriptor;

  Graph graph;
  std::vector<std::vector<int>> adj_matrix;

public:
  Multigraph(int node_count)
      : graph(node_count),
        adj_matrix(node_count, std::vector<int>(node_count, 0)) {}

  int node_count() const { return boost::num_vertices(graph); }

  int edge_count() const { return boost::num_edges(graph); }

  void add_edges(int u, int v, int count) {
    for (int i = 0; i < count; i++) {
      boost::add_edge(u, v, graph);
      adj_matrix[u][v]++;
      adj_matrix[v][u]++;
    }
  }

  void add_edge(int u, int v) { add_edges(u, v, 1); }

  auto edges() const { return boost::make_iterator_range(boost::edges(graph)); }

  vertex_descriptor source(edge_descriptor e) const {
    return boost::source(e, graph);
  }
  vertex_descriptor target(edge_descriptor e) const {
    return boost::target(e, graph);
  }

  bool maybe_isomorphic_with(const Multigraph &other) const {
    std::array<int, 5> multiplicity_distr = {0, 0, 0, 0, 0};
    std::array<int, 5> other_multiplicity_distr = {0, 0, 0, 0, 0};
    for (int u = 0; u < node_count(); u++) {
      for (int v = u; v < node_count(); v++) {
        multiplicity_distr[adj_matrix[u][v]]++;
        other_multiplicity_distr[other.adj_matrix[u][v]]++;
      }
    }
    for (int i = 0; i < multiplicity_distr.size(); i++) {
      if (multiplicity_distr[i] != other_multiplicity_distr[i]) {
        return false;
      }
    }
    return true;
  }

  bool operator==(const Multigraph &other) const {
    if (!maybe_isomorphic_with(other)) {
      return false;
    }

    return boost::vf2_graph_iso(
        graph, other.graph, noop_callback<Graph, Graph>(graph, other.graph));
  }

  bool has_articulation_point() const {
    std::vector<vertex_descriptor> ap;
    boost::articulation_points(graph, std::back_inserter(ap));
    return !ap.empty();
  }

  bool has_bridge() const {
    std::vector<bool> visited(node_count(), false);
    std::vector<int> tin(node_count(), -1);
    std::vector<int> low(node_count(), -1);
    int timer = 0;

    std::function<bool(int, int)> dfs = [&](int v, int p) {
      struct E {
        int u, v;
        E(int a, int b) : u(a), v(b) {
          if (u > v) {
            std::swap(u, v);
          }
        }
        bool operator<(const E &other) const {
          return std::tie(u, v) < std::tie(other.u, other.v);
        }
      };
      std::multiset<E> es;
      for (const edge_descriptor e : edges()) {
        es.insert(E(source(e), target(e)));
      }

      std::vector<std::vector<int>> adj(node_count());
      for (auto it = es.begin(); it != es.end(); it++) {
        auto [u, v] = *it;
        adj[u].push_back(v);
        adj[v].push_back(u);
      }

      visited[v] = true;
      tin[v] = low[v] = timer++;
      bool parent_skipped = false;
      for (int to : adj[v]) {
        if (to == p && !parent_skipped) {
          parent_skipped = true;
          continue;
        }
        if (visited[to]) {
          low[v] = std::min(low[v], tin[to]);
        } else {
          if (dfs(to, v)) {
            return true;
          }
          low[v] = std::min(low[v], low[to]);
          if (low[to] > tin[v] && es.count(E(v, to)) == 1)
            return true;
        }
      }
      return false;
    };

    for (int i = 0; i < node_count(); ++i) {
      if (!visited[i] && dfs(i, -1))
        return true;
    }

    return false;
  }

  void print() const {
    std::cout << node_count() << "\n";
    std::cout << edge_count() << "\n";
    for (auto e : edges()) {
      std::cout << source(e) << " " << target(e) << "\n";
    }
  }
};

std::vector<Multigraph> generate_vacuum_graphs(int vertex_count) {
  std::vector<std::vector<Multigraph>> vacuums(vertex_count + 1);

  {
    Multigraph g(2);
    g.add_edges(0, 1, 4);
    vacuums[2].push_back(g);
  }

  for (int i = 3; i <= vertex_count; i++) {
    // Operation 1: Remove two edges and add four new
    for (const Multigraph &graph : vacuums[i - 1]) {
      for (const auto edge_1 : graph.edges()) {
        const int u1 = graph.source(edge_1);
        const int v1 = graph.target(edge_1);
        for (const auto edge_2 : graph.edges()) {
          const int u2 = graph.source(edge_2);
          const int v2 = graph.target(edge_2);
          if (edge_1 == edge_2) {
            continue;
          }
          Multigraph temporary(i);
          for (const auto e : graph.edges()) {
            if (e != edge_1 && e != edge_2) {
              temporary.add_edge(graph.source(e), graph.target(e));
            }
          }
          temporary.add_edge(u1, i - 1);
          temporary.add_edge(v1, i - 1);
          temporary.add_edge(u2, i - 1);
          temporary.add_edge(v2, i - 1);

          if (std::none_of(vacuums[i].begin(), vacuums[i].end(),
                           [&temporary](const Multigraph &vacuum) {
                             return temporary == vacuum;
                           })) {
            vacuums[i].push_back(temporary);
          }
        }
      }
    }

    // Operation 2: Remove one edge and add five new (two single edges and one
    // triple edge)
    for (const Multigraph &graph : vacuums[i - 2]) {
      for (const auto edge : graph.edges()) {
        Multigraph temporary(i);
        for (const auto e : graph.edges()) {
          if (e != edge) {
            temporary.add_edge(graph.source(e), graph.target(e));
          }
        }
        int u = graph.source(edge);
        int v = graph.target(edge);
        temporary.add_edge(u, i - 2);
        temporary.add_edge(v, i - 1);
        temporary.add_edges(i - 2, i - 1, 3);

        if (std::none_of(vacuums[i].begin(), vacuums[i].end(),
                         [&temporary](const Multigraph &vacuum) {
                           return temporary == vacuum;
                         })) {
          vacuums[i].push_back(temporary);
        }
      }
    }
  }

  return vacuums.back();
}

std::vector<Multigraph>
generate_feynman_graphs(int vertex_count,
                        const std::vector<Multigraph> &vacuums) {
  std::vector<Multigraph> feynmans;

  for (const Multigraph &vacuum : vacuums) {
    if (vacuum.has_articulation_point()) {
      continue;
    }

    std::vector<Multigraph> same_feynmans;

    for (int node = 0; node < vacuum.node_count(); node++) {
      std::vector<int> neighbors;
      for (const auto e : vacuum.edges()) {
        int u = vacuum.source(e), v = vacuum.target(e);
        if (u != node && v != node) {
          continue;
        }
        int neighbor = u == node ? (v - (v > node)) : (u - (u > node));
        neighbors.push_back(neighbor);
      }

      Multigraph copy(vacuum.node_count() - 1);
      for (const auto e : vacuum.edges()) {
        int u = vacuum.source(e), v = vacuum.target(e);
        if (u != node && v != node) {
          copy.add_edge(u - (u > node), v - (v > node));
        }
      }

      if (copy.has_bridge()) {
        continue;
      }

      Multigraph copy2(copy.node_count() + 4);
      for (const auto e : copy.edges()) {
        copy2.add_edge(copy.source(e), copy.target(e));
      }

      for (int i = 0; i < 4; i++) {
        copy2.add_edge(copy2.node_count() - 1 - i, neighbors[i]);
      }

      if (std::none_of(same_feynmans.begin(), same_feynmans.end(),
                       [&copy2](const Multigraph &feynman) {
                         return copy2 == feynman;
                       })) {
        same_feynmans.push_back(copy2);
      }
    }

    for (const Multigraph &feynman : same_feynmans) {
      feynmans.push_back(feynman);
    }
  }

  return feynmans;
}

int main() {
  std::optional<GraphType> graph_type_opt = get_graph_type();

  if (!graph_type_opt.has_value()) {
    std::cout << "No graph type selected. Exiting.\n";
    return 0;
  }

  GraphType graph_type = graph_type_opt.value();

  std::optional<int> vertex_count_opt = get_vertex_count();

  if (!vertex_count_opt.has_value()) {
    std::cout
        << "The number of vertices must be a positive integer. Exiting.\n";
    return 0;
  }

  int vertex_count = vertex_count_opt.value();

  if (graph_type == GraphType::FEYNMAN) {
    vertex_count++;
  }

  std::vector<Multigraph> vacuums = generate_vacuum_graphs(vertex_count);

  if (graph_type == GraphType::FEYNMAN) {
    std::vector<Multigraph> feynmans =
        generate_feynman_graphs(vertex_count, vacuums);
    std::cout << feynmans.size() << "\n";
    for (const Multigraph &graph : feynmans) {
      graph.print();
    }
  } else {
    std::cout << vacuums.size() << "\n";
    for (const Multigraph &graph : vacuums) {
      graph.print();
    }
  }
}

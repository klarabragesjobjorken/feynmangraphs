#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/biconnected_components.hpp>
#include <boost/graph/vf2_sub_graph_iso.hpp>
#include <boost/program_options.hpp>
#include <graphviz/gvc.h>

namespace po = boost::program_options;

enum class GraphType { Feynman, Vacuum };

template <typename Graph1, typename Graph2> class NoopCallback {
public:
  NoopCallback(const Graph1 &, const Graph2 &) {}

  template <typename CorrespondenceMap1To2, typename CorrespondenceMap2To1>
  bool operator()(CorrespondenceMap1To2, CorrespondenceMap2To1) const {
    return false;
  }
};

using Graph =
    boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS>;
using Vertex = boost::graph_traits<Graph>::vertex_descriptor;
using Edge = boost::graph_traits<Graph>::edge_descriptor;

class Multigraph {
public:
  Multigraph(int node_count)
      : graph_(node_count),
        adj_matrix_(node_count, std::vector<int>(node_count, 0)) {}

  int node_count() const { return boost::num_vertices(graph_); }

  int edge_count() const { return boost::num_edges(graph_); }

  void add_edges(int u, int v, int count) {
    for (int i = 0; i < count; i++) {
      boost::add_edge(u, v, graph_);
    }
    adj_matrix_[u][v] += count;
    adj_matrix_[v][u] += count;
  }

  void add_edge(int u, int v) { add_edges(u, v, 1); }

  auto edges() const {
    return boost::make_iterator_range(boost::edges(graph_));
  }

  Vertex source(Edge e) const { return boost::source(e, graph_); }
  Vertex target(Edge e) const { return boost::target(e, graph_); }

  bool operator==(const Multigraph &other) const;

  bool has_articulation_point() const;
  bool has_bridge() const;

  void print() const;

private:
  Graph graph_;
  std::vector<std::vector<int>> adj_matrix_;

  bool maybe_isomorphic_with(const Multigraph &other) const;
};

bool Multigraph::operator==(const Multigraph &other) const {
  if (!maybe_isomorphic_with(other)) {
    return false;
  }

  return boost::vf2_graph_iso(graph_, other.graph_,
                              NoopCallback<Graph, Graph>(graph_, other.graph_));
}

bool Multigraph::has_articulation_point() const {
  std::vector<Vertex> ap;
  boost::articulation_points(graph_, std::back_inserter(ap));
  return !ap.empty();
}

bool Multigraph::has_bridge() const {
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
    for (const Edge e : edges()) {
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

void Multigraph::print() const {
  std::cout << node_count() << "\n";
  std::cout << edge_count() << "\n";
  for (auto e : edges()) {
    std::cout << source(e) << " " << target(e) << "\n";
  }
}

bool Multigraph::maybe_isomorphic_with(const Multigraph &other) const {
  std::array<int, 5> multiplicity_distr = {0, 0, 0, 0, 0};
  std::array<int, 5> other_multiplicity_distr = {0, 0, 0, 0, 0};
  for (int u = 0; u < node_count(); u++) {
    for (int v = u; v < node_count(); v++) {
      multiplicity_distr[adj_matrix_[u][v]]++;
      other_multiplicity_distr[other.adj_matrix_[u][v]]++;
    }
  }
  for (int i = 0; i < multiplicity_distr.size(); i++) {
    if (multiplicity_distr[i] != other_multiplicity_distr[i]) {
      return false;
    }
  }
  return true;
}

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

class SvgGraphRenderer {
public:
  SvgGraphRenderer() : current_graph_idx_(0) {
    gvc_ = gvContext();
    if (!gvc_) {
      std::cerr << "Failed to initialize Graphviz context!\n";
      exit(1);
    }
    g_ = agopen(const_cast<char *>("G"), Agundirected, nullptr);
    agattr(g_, AGNODE, std::string("shape").data(),
           std::string("circle").data());
    agattr(g_, AGNODE, std::string("width").data(), std::string("0.2").data());
    agattr(g_, AGNODE, std::string("label").data(), std::string("").data());
  }

  ~SvgGraphRenderer() {
    agclose(g_);
    gvFreeContext(gvc_);
  }

  void render_to_svg(const std::string &output_path) {
    gvLayout(gvc_, g_, "neato");
    gvRenderFilename(gvc_, g_, "svg", output_path.c_str());
    gvFreeLayout(gvc_, g_);
  }

  void add_graph(const Multigraph &multigraph, GraphType graph_type) {
    std::string clusterName = "cluster" + std::to_string(current_graph_idx_);
    Agraph_t *subg = agsubg(g_, clusterName.data(), 1);

    agsafeset(subg, std::string("style").data(), std::string("invis").data(),
              std::string("").data());

    for (const Edge &edge : multigraph.edges()) {
      int u = multigraph.source(edge), v = multigraph.target(edge);
      std::string name1 = clusterName + "_" + std::to_string(u);
      Agnode_t *node1 = agnode(subg, name1.data(), 1);
      if (graph_type == GraphType::Feynman &&
          u >= multigraph.node_count() - 4) {
        agset(node1, std::string("shape").data(), std::string("none").data());
      }
      std::string name2 = clusterName + "_" + std::to_string(v);
      Agnode_t *node2 = agnode(subg, name2.data(), 1);
      if (graph_type == GraphType::Feynman &&
          v >= multigraph.node_count() - 4) {
        agset(node1, std::string("shape").data(), std::string("none").data());
      }
      Agedge_t *e = agedge(subg, node1, node2, nullptr, 1);
    }

    current_graph_idx_++;
  }

private:
  GVC_t *gvc_;
  Agraph_t *g_;
  int current_graph_idx_;
};

int main(int argc, char *argv[]) {
  int vertex_count;
  GraphType graph_type;
  std::string output_path;

  po::options_description desc("Allowed options");
  desc.add_options()("help,h", "Show help message")(
      "type,t",
      po::value<std::string>()->default_value("feynman")->notifier(
          [&](const std::string &s) {
            if (s == "feynman") {
              graph_type = GraphType::Feynman;
            } else if (s == "vacuum") {
              graph_type = GraphType::Vacuum;
            } else {
              throw po::validation_error(
                  po::validation_error::invalid_option_value, "type", s);
            }
          }),
      "Type of graph to generate: feynman, vacuum")(
      "output,o", po::value(&output_path),
      "Optional path to output SVG visualization");

  po::options_description hidden("Hidden options");
  hidden.add_options()("vertices", po::value(&vertex_count)->required(),
                       "Number of vertices");

  po::positional_options_description pos_desc;
  pos_desc.add("vertices", 1);

  po::options_description all_options;
  all_options.add(desc).add(hidden);

  po::variables_map vm;

  try {
    po::store(po::command_line_parser(argc, argv)
                  .options(all_options)
                  .positional(pos_desc)
                  .run(),
              vm);

    if (vm.count("help")) {
      std::cout << "Usage: computingFeynman <num_vertices> [--type TYPE] "
                   "[--output FILE]\n\n";
      std::cout << desc << "\n";
      return 0;
    }

    po::notify(vm);
  } catch (po::error &e) {
    std::cerr << "Error: " << e.what() << "\n";
    std::cerr << "Use --help for usage info\n";
    return 1;
  }

  if (graph_type == GraphType::Feynman) {
    vertex_count++;
  }

  std::vector<Multigraph> vacuums = generate_vacuum_graphs(vertex_count);

  if (graph_type == GraphType::Feynman) {
    std::vector<Multigraph> feynmans =
        generate_feynman_graphs(vertex_count, vacuums);
    if (vm.count("output")) {
      std::cout << "Drawing graphs as svg.\n";
      SvgGraphRenderer svg_renderer;
      for (const Multigraph &multigraph : feynmans) {
        svg_renderer.add_graph(multigraph, GraphType::Feynman);
      }
      svg_renderer.render_to_svg(output_path);
    }
    std::cout << "Generated " << feynmans.size() << " feynman graphs.\n";
  } else {
    if (vm.count("output")) {
      SvgGraphRenderer svg_renderer;
      for (const Multigraph &multigraph : vacuums) {
        svg_renderer.add_graph(multigraph, GraphType::Vacuum);
      }
      svg_renderer.render_to_svg(output_path);
    }
    std::cout << "Generated " << vacuums.size() << " vacuum graphs.\n";
  }
}

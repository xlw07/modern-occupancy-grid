#include "sumproduct.hpp"
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/property_map/property_map.hpp>

/// Adjacency list graph for FactorGraph
typedef boost::adjacency_list<
  boost::vecS, boost::vecS, boost::undirectedS> FactorGraph;


/// Vertex type
typedef typename boost::graph_traits< FactorGraph >::vertex_descriptor Vertex;

typedef std::size_t codomain_type;
typedef typename std::vector<codomain_type>::const_iterator codomain_iterator;
typedef std::pair<codomain_iterator, codomain_iterator> codomain_iter_pair;
/// Codomain functor
//
codomain_iter_pair codomain_static_function() {
  const static std::size_t codomain_vector[] = {0, 1};
  return std::make_pair(codomain_vector, codomain_vector + 2);
}
struct codomain_func : public std::unary_function<Vertex,  codomain_iter_pair> {
  codomain_iter_pair operator()(const Vertex& v) const {
    const static std::size_t codomain_vector[] = {0, 1};
    return std::make_pair(codomain_vector, codomain_vector + 2);
  }
};

/// Codomain Property Map for variable nodes
class BinaryCodomainMap {
  public:
  typedef Vertex key_type;
  typedef codomain_iter_pair value_type;
  typedef value_type& reference;
  typedef boost::lvalue_property_map_tag category;
};
BinaryCodomainMap::value_type get(BinaryCodomainMap c, Vertex n) {
  const static std::size_t codomain_array[] = {0, 1};
  const static std::vector<codomain_type> codomain_vector(
      codomain_array, codomain_array + 2);
  return std::make_pair(codomain_vector.begin(), codomain_vector.end());
}

//=========================================================================
// An adaptor to turn a Unique Pair Associative Container like std::map or
// std::hash_map into an Lvalue Property Map.

template <typename UniquePairAssociativeContainer>
class associative_property_map
  : public boost::put_get_helper<
     typename UniquePairAssociativeContainer::value_type::second_type&,
     associative_property_map<UniquePairAssociativeContainer> >
{
  typedef UniquePairAssociativeContainer C;
public:
  typedef typename C::key_type key_type;
  typedef typename C::value_type::second_type value_type;
  typedef value_type& reference;
  typedef boost::lvalue_property_map_tag category;
  associative_property_map() : m_c() { }
  reference operator[](const key_type& k) const {
    return (const_cast<C&>(m_c))[k];
  }
private:
  C m_c;
};
//=========================================================================

/// Value assignment to variable nodes
typedef associative_property_map< boost::unordered_map<Vertex, codomain_type > > AssignmentMap;


 // class Real {
 // private:
 //   std::string expression_;
 //   friend Real operator*=(Real&);
 //   friend Real operator+=(Real&);
 // };
 // 
 // Real operator*=(Real& r) {
 //   return (r == "1") ? expression_ : expression_ + r;
 // }
 // Real operator+=(Real& r) {
 //   return expression_ " + " r;
 // }
typedef double Real;

/// Factor map for factor nodes
typedef boost::function<Real (AssignmentMap)> FactorType;
typedef boost::associative_property_map< boost::unordered_map<Vertex, FactorType > > FactorMap;

template<typename VertexIterator>
struct vertex_factor {
private:
  typename VertexIterator::value_type v_;
  VertexIterator v_begin_;
  VertexIterator v_end_;
public:
  vertex_factor(typename VertexIterator::value_type v, VertexIterator v_begin, VertexIterator v_end) 
    : v_(v), v_begin_(v_begin), v_end_(v_end) { 
      // std::cout << "Neigbours of " << v_ << " are:";
      // for (VertexIterator nbr(v_begin_); nbr != v_end_; ++nbr) std::cout << *nbr << "," ;
      // std::cout << std::endl;
    }

  Real operator()(AssignmentMap& amap) const {
    std::cout << "Neigbors of" << v_ << " start" << std::endl;
    for (VertexIterator v(v_begin_); v != v_end_; ++v) {
      std::cout << *v << " -> " << amap[*v] << std::endl;
    }
    std::cout << "Neigbors end" << std::endl;
    return 0;
  }
};

typedef std::pair< std::pair<Vertex, Vertex>, codomain_type> MessageKeyType;
typedef boost::associative_property_map< boost::unordered_map<MessageKeyType, Real > > MessageValues;
typedef boost::associative_property_map< boost::unordered_map<Vertex, bool > > IsFactorMap;

int main(int argc, const char *argv[])
{
  enum variables { x1, x2, x3, x4, x5 };
  enum factors { fa = x5 + 1, fb, fc, fd, fe };
  typedef std::pair<int, int> Edge;
  Edge edge_array[] = { Edge(fa, x1), Edge(fc, x3),
    Edge(fb, x2), Edge(fc, x2), Edge(fc, x1), Edge(fd, x3),
    Edge(fd, x4), Edge(fe, x3), Edge(fe, x5) };

  FactorGraph g(edge_array,
      edge_array + sizeof(edge_array) / sizeof(Edge),
       10 );
  using namespace boost;
  //std::cout << "Vertices:" << num_vertices(g) << std::endl;

  typedef typename boost::graph_traits<FactorGraph>::vertex_iterator vertex_iterator;
  vertex_iterator v, v_end;
  boost::tie(v, v_end) = vertices(g);
  std::size_t count = 0;
  boost::unordered_map<Vertex, bool> is_factor_map;
  IsFactorMap is_factor(is_factor_map);
  for (; v != v_end; ++v)
    put(is_factor, *v, (count++ < fa) ? false : true);

  boost::unordered_map<Vertex, FactorType> factors_map;
  FactorMap fmap(factors_map);
  boost::tie(v, v_end) = vertices(g);
  typedef typename  boost::graph_traits<FactorGraph>::vertex_iterator vertex_iterator;
  typedef typename  boost::graph_traits<FactorGraph>::vertex_descriptor vertex_descriptor;
  typedef typename  boost::graph_traits<FactorGraph>::edge_descriptor edge_descriptor;
  for (; v!=v_end;++v) {
    vertex_descriptor vd(*v);
    // Construct a vector of neigbors from out_edges
    // std::vector<vertex_descriptor> nbrs;
    // typedef typename std::vector<vertex_descriptor>::const_iterator nbr_iterator;
    // occgrid::neighbors<FactorGraph>(vd, g, std::back_inserter(nbrs));
    typedef typename boost::graph_traits<FactorGraph>::adjacency_iterator adjacency_iterator;
    std::pair<adjacency_iterator, adjacency_iterator> nbrs( adjacent_vertices(vd, g));

    // std::cout << "Neigbours of " << vd << " are:";
    // for (adjacency_iterator nbr(nbrs.first); nbr != nbrs.second; ++nbr) std::cout << *nbr << "," ;
    // std::cout << std::endl;

    vertex_factor<adjacency_iterator> vf(vd, nbrs.first, nbrs.second);
    put(fmap, *v, vf);
  }

  boost::unordered_map<MessageKeyType, Real> msg_values;

  codomain_iter_pair cdip(codomain_static_function());
  BinaryCodomainMap cdmap;
  MessageValues msgs(msg_values);
  typedef typename boost::graph_traits<FactorGraph>::edge_iterator edge_iterator;
  edge_iterator e, e_end;

  for(boost::tie(e, e_end) = edges(g); e != e_end; ++e)
    for (codomain_iterator cd = cdip.first; cd != cdip.second; ++cd)
      put(msgs, std::make_pair(std::make_pair(source(*e, g), target(*e, g)), *cd), 1);

  occgrid::sumproduct_visitor<
    FactorGraph,
    MessageValues,
    BinaryCodomainMap,
    FactorMap,
    IsFactorMap>
      spvis (msgs, cdmap, fmap, is_factor);

  boost::depth_first_search(g, boost::visitor(boost::make_dfs_visitor(spvis)));

  return 0;
}
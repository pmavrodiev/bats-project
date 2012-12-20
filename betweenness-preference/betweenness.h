#ifndef __BETWEENNESS_H
#define __BETWEENNESS_H

#include <map>
#include <vector>
#include <algorithm>
#include <set>

/*custom edge class*/
class custom_edge {
public:
  unsigned from;
  unsigned to;
  unsigned type;
  custom_edge();
  custom_edge(unsigned f, unsigned t, unsigned _type);
  friend bool operator==(custom_edge &c1, custom_edge &c2);
  friend bool operator!=(custom_edge &c1, custom_edge &c2);
};

struct edgecomp {
  bool operator() (const custom_edge lhs, const custom_edge rhs) const {
     //sort by from, to and type
    if (lhs.from != rhs.from) return lhs.from<rhs.from;
    else if (lhs.to != rhs.to) return lhs.to<rhs.to;
    else return lhs.type<rhs.type;
  }
};


class time_aggr_betweenness_preference_matrix {
public:
  bool initialised;
  unsigned node_id;
  bool defined;
  double betweenness_pref;
  time_aggr_betweenness_preference_matrix();
  //key = pair of source, destination
  //value = what fraction of total paths over all time is the particular source,dest. pair
  std::map<custom_edge, double, edgecomp> tabpm;
  std::map<custom_edge, double, edgecomp > normalized_tabpm;
  void init_node_id(unsigned nid);
  void print_tabpm();
  void print_normalized_tabpm();
  void normalize_tabpm();
  double source_fixed_sum(unsigned source);
  double destination_fixed_sum(unsigned destination);
  void betweenness_preference();
};


class per_node_betweenness_preference_matrix {
public:
  bool initialised;
  bool defined;
  unsigned node_id;
  /*(p)er (n)ode (b)etweeness (p)reference (m)atrix*/
  /*pnbpm[key] = time
   *pnbpm[value] = custom edges, such that
    a path from source to dest, through node_idx, exists
    */
  std::map<unsigned, std::vector<custom_edge> > pnbpm;
  per_node_betweenness_preference_matrix();
  void init_node_id(unsigned nid);
  void print_pnbpm();
  std::set<custom_edge,edgecomp> get_unique_pairs();
  bool pair_exists(custom_edge,unsigned time);
};

class baseline_betweenness_preference_matrix {
public:
   bool initialised;
   bool defined;
   unsigned node_id;
   std::map<custom_edge, double,edgecomp > baseline_bp_matrix;
   baseline_betweenness_preference_matrix();
   void init_node_id (unsigned nid);
   void print_baseline_bp_matrix();
};

class time_unfolded_temporal_network {
public:
  bool valid;
  bool aggregate_network_initialized;
  unsigned nnodes; //number of nodes
  unsigned ntimesteps; //number of time steps
  /*map[time_step] = vector of edges*/  
  std::map<unsigned, std::vector<custom_edge> > temporal_network;
  //std::map<unsigned, std::vector<std::pair<unsigned,unsigned> > > temporal_network;
  //map[edge]=edge weight:= #times this edge exists in the aggr. network
  //std::map<std::pair<unsigned,unsigned>, unsigned > aggregate_network;
  std::map<custom_edge, unsigned,edgecomp > aggregate_network;
  std::vector<per_node_betweenness_preference_matrix> per_node_bp_matrices;
  std::vector<time_aggr_betweenness_preference_matrix> time_aggr_bp_matrices;
  std::vector<baseline_betweenness_preference_matrix> baseline_bp_matrices;  
  time_unfolded_temporal_network();
  time_unfolded_temporal_network(unsigned other_nnodes, unsigned other_ntimesteps);
  /*compute the (p)er (n)ode (b)etweeness (p)reference (m)atrix for node_id*/
  int compute_pnbpm();
  int compute_tabpm();
  int compute_baseline_bpm();
  std::vector<custom_edge> find_source(unsigned source_id,unsigned time);
  std::vector<custom_edge> find_dest(unsigned dest_id,unsigned time);
  std::vector<custom_edge> merge(std::vector<custom_edge> *one,
  						   std::vector<custom_edge> *two);
  unsigned source_fixed_sum(unsigned source);
  unsigned destination_fixed_sum(unsigned destination);
  double source_fixed_sum(unsigned source, baseline_betweenness_preference_matrix* ptr);
  double destination_fixed_sum(unsigned destination, baseline_betweenness_preference_matrix *ptr);
  double betweenness_preference(unsigned nid);  
  int build_aggregate_network();
  void print_temporal_network();
  void print_aggregate_network();
  void normalize_tabpms();
  void betweenness_preferences();
  void print_all_betweenness_preferences();
  void print_baseline_bp_matrices();
  //void init_row(std::string row_binary_string,int row_idx);
};

#endif

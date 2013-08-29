#ifndef __CLASSES_H
#define __CLASSES_H


#include "/usr/local/include/igraph/igraph.h"
#include<vector>
#include<map>
#include<set>
#include<string>

using namespace std;

class myigraph {
public:
  bool rewired;
  igraph_t graph;
  igraph_t rewired_graph;
  igraph_adjlist_t adjlist;
  igraph_matrix_t weighted_adj_matrix;
  myigraph(igraph_matrix_t *adjmatrix);
  ~myigraph();
  int eigenvector_centrality(igraph_vector_t *result,int which_graph);
  /*Simplest rewiring mechanism. Each edge is assigned to 2 randomly chosen vertices*/
  void rewire_edges(unsigned long seed);
  /*Weighted random rewiring. Followers are still chosen at random, but
  leaders are chosen proportionate to their activity*/
  void rewire_edges2(vector<double> probs, unsigned long seed);
  /*preserve out-degree distribution*/
  void rewire_edges3(unsigned long seed);
  /*preserve out-degree distribution with weighted leaders proportionate to activity*/
  void rewire_edges4(std::vector< double > probs, unsigned long seed);
  /*preserve in-degree and out-degree distribution*/
  void rewire_edges5();

  long sample_rnd(std::vector< double > probs,igraph_rng_t *rnd);
 
  void print_adjacency_matrix(int which_graph);
};

class bat {
public:
  int id;
  string hexid;
  vector<int> activity;
  //all years that this bat has been recorded at
  set<int> years;
  bat(string hexid,int id);
  bat();
  void add_year(int *year);
  void add_activity(int *a);
  double get_sum_activity();
};


#endif
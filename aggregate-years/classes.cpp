#include "classes.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <cctype>
#include <algorithm>
#include "/usr/local/include/igraph/igraph.h"
#include <stdio.h>
#include <ctime>
#include <numeric>
#include "standard.yy.h"

#include "classes.h"
 
 
myigraph::myigraph(igraph_matrix_t *adjmatrix) {
    long nrow = igraph_matrix_nrow(adjmatrix);
    long ncol = igraph_matrix_ncol(adjmatrix);
    igraph_matrix_init(&weighted_adj_matrix,nrow,ncol);
    igraph_matrix_null(&weighted_adj_matrix);
    igraph_matrix_copy(&weighted_adj_matrix,adjmatrix);
    /*create adjacency list from weighted_adj_matrix*/
    igraph_adjlist_init_empty(&adjlist,nrow);
    for (long vid=0; vid < nrow; vid++) {
      //the neighbours of vid from the adjacency list
      igraph_vector_t *vid_ptr = igraph_adjlist_get(&adjlist,vid);
      //igraph_vector_init(vid_ptr);
      //the neighbours of vid from weighted_adj_matrix
      igraph_vector_t vid_ptr_adj_matrix;
      igraph_vector_init(&vid_ptr_adj_matrix,nrow);
      igraph_vector_null(&vid_ptr_adj_matrix);      
      igraph_matrix_get_row(&weighted_adj_matrix,&vid_ptr_adj_matrix,vid);
      igraph_real_t vid_n_outgoing_edges = igraph_vector_sum(&vid_ptr_adj_matrix);
      //resize the vector for vid in the adjacency list
      igraph_vector_resize(vid_ptr,vid_n_outgoing_edges);
      //fill the vector for vid in the adjacency list
      unsigned counter = 0;
      for (long vid_neighbour=0; vid_neighbour<igraph_vector_size(&vid_ptr_adj_matrix); vid_neighbour++)
	  for (long j=0; j<VECTOR(vid_ptr_adj_matrix)[vid_neighbour]; j++) 
	      VECTOR(*vid_ptr)[counter++] = vid_neighbour;
	  
      igraph_vector_destroy(&vid_ptr_adj_matrix);
    }
    //finally create the graph
    igraph_adjlist(&graph,&adjlist,IGRAPH_OUT,false);
    rewired=false;
}

int myigraph::eigenvector_centrality(igraph_vector_t *result,int which_graph) {
    //which_graph: 0 for original graph, 1 for the rewired
    /*the idea is to create a new temporary graph from the adjacency matrix of this.graph,
     and calculate the eigenvector centralities from there*/
    //get the adjacency matrix
    igraph_matrix_t m;
    igraph_matrix_init(&m,igraph_matrix_nrow(&weighted_adj_matrix),igraph_matrix_ncol(&weighted_adj_matrix));
    igraph_matrix_null(&m);
    igraph_get_adjacency((which_graph==0 ? &graph : &rewired_graph),&m,IGRAPH_GET_ADJACENCY_BOTH,false);    
    igraph_t temp_g;
    igraph_weighted_adjacency(&temp_g,&m,IGRAPH_ADJ_DIRECTED,"importance",true);
    /*get the edge attributes*/
    igraph_real_t eigenvalue;
    igraph_vector_t edge_importance;
    igraph_es_t edge_selector;
    igraph_eit_t edge_iterator;    
    igraph_es_all(&edge_selector,IGRAPH_EDGEORDER_ID);
    igraph_eit_create(&temp_g,edge_selector,&edge_iterator);    
    igraph_vector_init(&edge_importance,IGRAPH_EIT_SIZE(edge_iterator));
    igraph_vector_null(&edge_importance);
    igraph_eit_destroy(&edge_iterator);
    igraph_cattribute_EANV(&temp_g,"importance",edge_selector,&edge_importance);
    igraph_es_destroy(&edge_selector);
    /**/
    igraph_arpack_options_t aroptions;
    igraph_arpack_options_init(&aroptions);	      
    /*set up a new error handler which doesnt automatically quit on errors*/
    //igraph_error_type_t new_handler;
    igraph_error_handler_t *old_handler = igraph_set_error_handler(&igraph_error_handler_ignore);    
    igraph_warning_handler_t *old_warning_handler = igraph_set_warning_handler(&igraph_warning_handler_ignore);
    
    int errcode = igraph_eigenvector_centrality(&temp_g,result,&eigenvalue,/*directed=*/true,/*scale=*/false,&edge_importance,&aroptions);    
    /**/
    igraph_vector_destroy(&edge_importance);
    igraph_matrix_destroy(&m);
    igraph_destroy(&temp_g);
    return errcode;
}
  /*Simplest rewiring mechanism. Each edge is assigned to 2 randomly chosen vertices*/
void myigraph::rewire_edges(unsigned long seed) {   
   //igraph_copy(&rewired_gaph,&graph);
   //igraph_rewire_edges(&rewired_graph,1.0,/*loops=*/false,/*multiple edges=*/true);  
  //init the random number generators
  igraph_rng_t rng;   
  igraph_rng_init(&rng,&igraph_rngtype_mt19937);
  igraph_rng_seed(&rng,seed); 
  igraph_matrix_t original_adj_matrix, rewired_adj_matrix;
  igraph_matrix_init(&original_adj_matrix,igraph_matrix_nrow(&weighted_adj_matrix),igraph_matrix_ncol(&weighted_adj_matrix));
  igraph_matrix_null(&original_adj_matrix);
  igraph_matrix_init(&rewired_adj_matrix,igraph_matrix_nrow(&weighted_adj_matrix),igraph_matrix_ncol(&weighted_adj_matrix));
  igraph_matrix_fill(&rewired_adj_matrix,0);
  
  igraph_get_adjacency(&graph,&original_adj_matrix,IGRAPH_GET_ADJACENCY_BOTH,false); 
  long n_vs = igraph_adjlist_size(&adjlist);
  
  for (long int r=0; r<igraph_matrix_nrow(&original_adj_matrix); r++) {
    for (long int c=0; c<igraph_matrix_ncol(&original_adj_matrix); c++) {
      long nedges = MATRIX(original_adj_matrix,r,c);
      while (nedges) {
	long int row_idx = igraph_rng_get_integer(&rng,0,n_vs-1);
	long int col_idx = igraph_rng_get_integer(&rng,0,n_vs-1);
	MATRIX(rewired_adj_matrix,row_idx,col_idx)++;
	nedges--;
      }
    }
  }
  if (rewired)
     igraph_destroy(&rewired_graph);
  
  igraph_weighted_adjacency(&rewired_graph,&rewired_adj_matrix,IGRAPH_ADJ_DIRECTED,"importance",true);
  rewired=true;
  igraph_rng_destroy(&rng);
}

/*Weighted random rewiring. Followers are still chosen at random, but
  leaders are chosen proportionate to their activity*/
void myigraph::rewire_edges2(vector<double> probs, unsigned long seed) {   
   //igraph_copy(&rewired_gaph,&graph);
   //igraph_rewire_edges(&rewired_graph,1.0,/*loops=*/false,/*multiple edges=*/true);  
  //init the random number generators
  igraph_rng_t rng;   
  igraph_rng_init(&rng,&igraph_rngtype_mt19937);
  igraph_rng_seed(&rng,seed); 
  igraph_matrix_t original_adj_matrix, rewired_adj_matrix;
  igraph_matrix_init(&original_adj_matrix,igraph_matrix_nrow(&weighted_adj_matrix),igraph_matrix_ncol(&weighted_adj_matrix));
  igraph_matrix_null(&original_adj_matrix);
  igraph_matrix_init(&rewired_adj_matrix,igraph_matrix_nrow(&weighted_adj_matrix),igraph_matrix_ncol(&weighted_adj_matrix));
  igraph_matrix_fill(&rewired_adj_matrix,0);
  
  igraph_get_adjacency(&graph,&original_adj_matrix,IGRAPH_GET_ADJACENCY_BOTH,false); 
  long n_vs = igraph_adjlist_size(&adjlist);
  
  for (long int r=0; r<igraph_matrix_nrow(&original_adj_matrix); r++) {
    for (long int c=0; c<igraph_matrix_ncol(&original_adj_matrix); c++) {
      long nedges = MATRIX(original_adj_matrix,r,c);
      while (nedges) {
	long int row_idx = igraph_rng_get_integer(&rng,0,n_vs-1); //follower
	long int col_idx = sample_rnd(probs,&rng);//igraph_rng_get_integer(&rng,0,n_vs-1); //leader
	MATRIX(rewired_adj_matrix,row_idx,col_idx)++;
	nedges--;
      }
    }
  }
  if (rewired)
     igraph_destroy(&rewired_graph);
  
  igraph_weighted_adjacency(&rewired_graph,&rewired_adj_matrix,IGRAPH_ADJ_DIRECTED,"importance",true);
  rewired=true;
  igraph_rng_destroy(&rng);
}

/*preserve out-degree distribution*/
void myigraph::rewire_edges3(unsigned long seed) {  
   //init the random number generators
   igraph_rng_t rng;
   //igraph_rngtype_mt19937 rng_type;
   igraph_rng_init(&rng,&igraph_rngtype_mt19937);
   igraph_rng_seed(&rng,seed); 
   /**/
   long n_vs = igraph_adjlist_size(&adjlist);
   for (long vid=0; vid<n_vs; vid++) {
    //the neighbours of vid from the adjacency list
      igraph_vector_t *vid_ptr = igraph_adjlist_get(&adjlist,vid);
      for (long neighbours=0; neighbours<igraph_vector_size(vid_ptr); neighbours++) {
	long random_number = vid;
	while (random_number == vid)
	  random_number=igraph_rng_get_integer(&rng,0,n_vs-1);	
	VECTOR(*vid_ptr)[neighbours] = random_number;
      }
   }
   /*now init the rewired graph*/   
   if (rewired)
     igraph_destroy(&rewired_graph); 
   
   igraph_adjlist(&rewired_graph,&adjlist,IGRAPH_OUT,false);
   rewired=true;
   igraph_rng_destroy(&rng);

}

/*preserve out-degree distribution with weighted leaders proportionate to activity*/
void myigraph::rewire_edges4(std::vector< double > probs, unsigned long seed) {
   //init the random number generators
   igraph_rng_t rng;   
   igraph_rng_init(&rng,&igraph_rngtype_mt19937);      
   igraph_rng_seed(&rng,seed); 
   
   long n_vs = igraph_adjlist_size(&adjlist);
   
   for (long vid=0; vid<n_vs; vid++) {   
      igraph_vector_t *vid_ptr = igraph_adjlist_get(&adjlist,vid);
       //the neighbours of vid from the adjacency list
      for (long neighbours=0; neighbours<igraph_vector_size(vid_ptr); neighbours++) {
	long random_number = vid;
	while (random_number == vid)
	  random_number = sample_rnd(probs,&rng);
	  //random_number=igraph_rng_get_integer(&rng,0,n_vs-1);	
	VECTOR(*vid_ptr)[neighbours] = random_number;
      }
   }
   /*now init the rewired graph*/   
   if (rewired)
     igraph_destroy(&rewired_graph);
   
   igraph_adjlist(&rewired_graph,&adjlist,IGRAPH_OUT,false);
   rewired=true;
   igraph_rng_destroy(&rng);  
}
/*preserve in-degree and out-degree distribution*/
void myigraph::rewire_edges5() {
    bool repeat = true;
    //init the random number generators
    igraph_rng_t rng;
    //igraph_rngtype_mt19937 rng_type;
    igraph_rng_init(&rng,&igraph_rngtype_mt19937);
    igraph_rng_seed(&rng,time(0)*1000); //current time in milliseconds
    /**/
    long n_vs = igraph_adjlist_size(&adjlist);
    while (repeat) {
      /*auxillary structures*/
      map<long,int> in_degree, out_degree;
      for (long vid=0; vid<n_vs; vid++) {
	//the neighbours of vid from the adjacency list
	igraph_vector_t *vid_ptr = igraph_adjlist_get(&adjlist,vid);
	if (igraph_vector_size(vid_ptr) == 0) continue;
	out_degree[vid] = igraph_vector_size(vid_ptr);      
	for (long neighbours=0; neighbours<igraph_vector_size(vid_ptr); neighbours++) {
	  in_degree[VECTOR(*vid_ptr)[neighbours]]++;
	}
      }
      /*start the real work*/
      igraph_matrix_fill(&weighted_adj_matrix,0); //init the adjacency matrix   
      /*select the sources and destinations of the edges*/
      long source = 0, destination = 0;
      while (out_degree.size() > 0 && in_degree.size() > 0) {
	source = igraph_rng_get_integer(&rng,0,out_degree.size()-1);      
	/*get an iterator pointing to the element with index "source"*/
	map<long,int>::iterator itr_src = out_degree.begin();
	long tmp = source;
	while (tmp-- != 0) 
	  itr_src++;                     
	long tmp2 = itr_src->first;      
	map<long,int>::iterator itr_dst;
	if (in_degree.size() == 1 && out_degree.size() == 1 &&
	  itr_src->first == in_degree.begin()->first) {
	  repeat = true;
	  break;
	}	
	if (in_degree.size() == 1 && itr_src->first == in_degree.begin()->first) 
	  continue;
	while (tmp2 == itr_src->first) {//exclude self-links
	  destination = igraph_rng_get_integer(&rng,0,in_degree.size()-1);	      
	  /*get an iterator pointing to the element with index "dest"*/
	  itr_dst = in_degree.begin();
	  tmp = destination;
	  while (tmp-- != 0) 
	    itr_dst++;
	  tmp2 = itr_dst->first;
	}      
	MATRIX(weighted_adj_matrix,itr_src->first,itr_dst->first)++;   
	/*finish off the logic*/
	itr_src->second--;
	itr_dst->second--;
	if (itr_src->second == 0)
	  out_degree.erase(itr_src);
	if (itr_dst->second == 0)
	  in_degree.erase(itr_dst);
	repeat = false;
      }
    /**/
    /*sanity check*/
    if (!repeat && out_degree.size() != in_degree.size())
      cerr<<"Error: Map sizes are different"<<endl;
    }
    
   myigraph dummy_graph(&weighted_adj_matrix);
   igraph_copy(&rewired_graph,&dummy_graph.graph);  
   rewired=true;
   igraph_rng_destroy(&rng);
  }
  

long myigraph::sample_rnd(std::vector< double > probs,igraph_rng_t *rnd) {
  /*create a vector with the cumulative frequencies*/
  std::vector<double> cdf(probs.size()+1,0);
  for (unsigned i=1; i<= probs.size(); i++) 
    cdf[i] = cdf[i-1] + probs[i-1];  
  
  cdf[cdf.size()-1] = 1.0; //deal with rounding issues
  igraph_real_t random_number = igraph_rng_get_unif01(rnd);
  bool found = false;
  long j=1;
  for (j=1; j< cdf.size(); j++) {    
    if (random_number > cdf[j-1] && random_number <= cdf[j])
      found=true;
    if (found) break;
  }  
  return j-1;
}
 
 
void myigraph::print_adjacency_matrix(int which_graph) {
    //0 for original graph, 1 for the rewired
    igraph_matrix_t m;
    igraph_matrix_init(&m,igraph_matrix_nrow(&weighted_adj_matrix),igraph_matrix_ncol(&weighted_adj_matrix));
    igraph_matrix_null(&m);
    igraph_get_adjacency((which_graph==0 ? &graph: &rewired_graph),&m,IGRAPH_GET_ADJACENCY_BOTH,false);    
    for (long r=0; r<igraph_matrix_nrow(&m); r++) {
      for (long c=0; c<igraph_matrix_ncol(&m); c++) {
	cout<<MATRIX(m,r,c)<<"\t";      
      }
      cout<<endl;   
    }
    igraph_matrix_destroy(&m);
  }
  
myigraph::~myigraph() {
    igraph_destroy(&graph);    
    if (rewired)
      igraph_destroy(&rewired_graph);
    igraph_adjlist_destroy(&adjlist);
    igraph_matrix_destroy(&weighted_adj_matrix);    
}

bat::bat(string hexid,int id) {
    this->hexid = hexid;
    years.clear();
    activity.clear();
    this->id = id;
}

bat::bat() {
    this->id=-1;
}

void bat::add_year(int *year) {
    pair<set<int>::iterator, bool> insert_pair;
    insert_pair = this->years.insert(*year);
    //maybe do something with insert_pair->second
}

void bat::add_activity(int *a) {
    this->activity.push_back(*a);
}

double bat::get_sum_activity() {
    //let us be fancy
    return (double) accumulate(activity.begin(),activity.end(),0);
}
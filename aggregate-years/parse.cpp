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

using namespace std;

bool paircomp(pair<unsigned,unsigned> lhs, pair<unsigned,unsigned> rhs) {
    if (lhs.first == rhs.first) 
      return (lhs.second < rhs.second);   
    else 
      return (lhs.first < rhs.first);
}
bool(*fn_pt3)(pair<unsigned,unsigned>,pair<unsigned,unsigned>) = paircomp;


map<string,bat> bats_to_ids;
vector<pair<string,string> > lfpairs;
double activities_sum = 0.0;

/* ==================================================================== */
/* ============================   MAIN   ============================== */
/* ===                                                              === */
/* ===
 *                                                                      */
int main(int argc, char**argv) {
     /*this call is in the beginning, as advised in the igraph manual*/
    igraph_i_set_attribute_table(&igraph_cattribute_table);
    /*first read the combined network*/   
    
    string file_name = "/home/pmavrodiev/Documents/bats/result_files/output_files_new_2/combined_networks.txt";
    yyin = fopen(file_name.c_str(),"r");
    if (yyin == NULL) {
      perror(file_name.c_str());
      exit(1);
    }
    yylex();	
    fclose(yyin);
    unsigned nbats=bats_to_ids.size();
    /*how many bats are present in all years*/
    unsigned nbats_all_years = 0;
    for (map<string,bat>::iterator i=bats_to_ids.begin(); i!=bats_to_ids.end(); i++) {
      if (i->second.years.size() >= 0) {
	nbats_all_years++;
	cout<<i->first<<":";
	for (set<int>::iterator j=i->second.years.begin(); j != i->second.years.end(); j++)
	  cout<<*j<<" ";
	cout<<endl;
      }
    }
    cout<<nbats_all_years<<endl;
    exit(1);
    igraph_matrix_t lf_adjmatrix;
    igraph_matrix_init(&lf_adjmatrix,nbats,nbats); //init the square matrix
    igraph_matrix_fill(&lf_adjmatrix,0);
    
    //fill in the edsges
    map<pair<unsigned,unsigned>,double,bool(*)(pair<unsigned,unsigned>,pair<unsigned,unsigned>) > edges(fn_pt3); 
    
    for (unsigned i=0; i<lfpairs.size(); i++) {
      string follower = lfpairs[i].first;
      string leader = lfpairs[i].second;
      if (bats_to_ids[follower].id == -1 || bats_to_ids[leader].id ==- 1) {
	cerr<<"Error: some bat ids in an lfpair are uninitialized"<<endl;
	exit(1);
      }
      MATRIX(lf_adjmatrix,bats_to_ids[follower].id,bats_to_ids[leader].id)++;
      pair<unsigned,unsigned> new_pair((unsigned) bats_to_ids[follower].id,(unsigned) bats_to_ids[leader].id);
      if (edges.find(new_pair) == edges.end())
	edges[new_pair] = 1.0;
      else 
	edges[new_pair]++;
    }   
 
    
    myigraph my_graph(&lf_adjmatrix);
    igraph_vector_t ev_centralities;
    igraph_vector_init(&ev_centralities,igraph_matrix_nrow(&lf_adjmatrix));        
    igraph_vector_null(&ev_centralities);
    int g = my_graph.eigenvector_centrality(&ev_centralities,/*rewired=*/0);    
    cout<<"Outputting the lf network ...";              
      string graphml="/home/pmavrodiev/Documents/bats/result_files/output_files_new_2/combined-network.graphml";
      ofstream orig_eigens("/home/pmavrodiev/Documents/bats/result_files/output_files_new_2/combined-original-evs.txt",ios::trunc);
      ofstream graphmlfile(graphml.c_str(),ios::trunc);
      //add the header
      graphmlfile<<"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"<<endl;
      graphmlfile<<"<graphml xmlns=\"http://graphml.graphdrawing.org/xmlns\""<<endl;
      graphmlfile<<"xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\""<<endl;
      graphmlfile<<"xsi:schemaLocation=\"http://graphml.graphdrawing.org/xmlns"<<endl;
      graphmlfile<<"http://graphml.graphdrawing.org/xmlns/1.0/graphml.xsd\">"<<endl;
      graphmlfile<<"<!-- Created by Pavlin Mavrodiev -->"<<endl;	
      graphmlfile<<"<key id=\"d0\" for=\"node\" attr.name=\"Label\" attr.type=\"string\"/>"<<endl;
      graphmlfile<<"<key id=\"d2\" for=\"node\" attr.name=\"ev_centrality\" attr.type=\"double\"/>"<<endl;
      graphmlfile<<"<key id=\"d1\" for=\"edge\" attr.name=\"importance\" attr.type=\"double\"/>"<<endl;
      graphmlfile<<"<graph id=\"G\" edgedefault=\"directed\">"<<endl;
      //add the nodes map<string,bat>
      for (map<string,bat>::iterator i=bats_to_ids.begin(); i!=bats_to_ids.end(); i++) {	     
	graphmlfile<<"<node id = \"n"<<i->second.id<<"\">"<<endl<<"<data key=\"d0\">"<<endl;
	graphmlfile<<"<Label>"<<i->first.substr(i->first.length()-4,4)<<"</Label>"<<endl;
	graphmlfile<<"</data>"<<endl;
	graphmlfile<<"<data key=\"d2\">"<<endl;	    
	graphmlfile<<"<ev_centrality>"<<VECTOR(ev_centralities)[i->second.id]<<"</ev_centrality>"<<endl;
	orig_eigens<<i->first<<"\t"<<VECTOR(ev_centralities)[i->second.id]<<endl;
	graphmlfile<<"</data>\n</node>"<<endl;	 
      }
      //add the edges
      for (map<pair<unsigned,unsigned>, double>::iterator itr2=edges.begin(); itr2 != edges.end(); itr2++) {
	 graphmlfile<<"<edge source=\"n"<<itr2->first.first<<"\" target=\"n"<<itr2->first.second<<"\">"<<endl;
	 graphmlfile<<"<data key=\"d1\">\n<importance>"<<itr2->second<<"</importance>\n</data>\n</edge>\n";
      }
      graphmlfile<<"</graph>\n</graphml>"<<endl;
      graphmlfile.close();
      orig_eigens.close();
      igraph_vector_destroy(&ev_centralities);	
      cout<<"DONE"<<endl;
      /*calculate activity*/
      vector<double> probs(nbats,0);
      for (map<string,bat>::iterator i=bats_to_ids.begin(); i!=bats_to_ids.end(); i++) 
	probs[i->second.id] = i->second.get_sum_activity() / activities_sum;
      /********/
     
      cout<<endl<<"Start rewiring ..."<<endl;
      ofstream evectorfile_shuffled("/home/pmavrodiev/Documents/bats/result_files/output_files_new_2/combined-model-1.dat",ios::trunc);	
      srand (time(NULL));
      unsigned nsimu = 10000;
      cout<<"\t Model 1 ..."<<endl;
      for (unsigned k=0; k<nsimu; k++) {
	igraph_vector_t rewired_centralities;
	igraph_vector_init(&rewired_centralities,igraph_matrix_nrow(&lf_adjmatrix));
	igraph_vector_null(&rewired_centralities);
	my_graph.rewire_edges(rand());
	while (my_graph.eigenvector_centrality(&rewired_centralities,1)) 
	  my_graph.rewire_edges(rand());
	//get the edge attributes
	for (map<string,bat>::iterator i=bats_to_ids.begin(); i!=bats_to_ids.end(); i++) 
	  evectorfile_shuffled<<i->first<<"\t"<<VECTOR(rewired_centralities)[i->second.id]<<endl;		
		    
	igraph_vector_destroy(&rewired_centralities);
      }
      evectorfile_shuffled.close();      
      cout<<"\t Done model 1"<<endl;
      evectorfile_shuffled.open("/home/pmavrodiev/Documents/bats/result_files/output_files_new_2/combined-model-2.dat",ios::trunc);
      cout<<"\t Model 2 ..."<<endl;
      for (unsigned k=0; k<nsimu; k++) {
	igraph_vector_t rewired_centralities;
	igraph_vector_init(&rewired_centralities,igraph_matrix_nrow(&lf_adjmatrix));
	igraph_vector_null(&rewired_centralities);
	my_graph.rewire_edges2(probs,rand());
	while (my_graph.eigenvector_centrality(&rewired_centralities,1)) 
	  my_graph.rewire_edges2(probs,rand());
	//get the edge attributes
	for (map<string,bat>::iterator i=bats_to_ids.begin(); i!=bats_to_ids.end(); i++) 
	  evectorfile_shuffled<<i->first<<"\t"<<VECTOR(rewired_centralities)[i->second.id]<<endl;		
		    
	igraph_vector_destroy(&rewired_centralities);
      }
      evectorfile_shuffled.close();
      cout<<"\t Done model 2"<<endl;
      evectorfile_shuffled.open("/home/pmavrodiev/Documents/bats/result_files/output_files_new_2/combined-model-3.dat",ios::trunc);
      cout<<"\t Model 3 ..."<<endl;
      for (unsigned k=0; k<nsimu; k++) {
	igraph_vector_t rewired_centralities;
	igraph_vector_init(&rewired_centralities,igraph_matrix_nrow(&lf_adjmatrix));
	igraph_vector_null(&rewired_centralities);
	my_graph.rewire_edges3(rand());
	while (my_graph.eigenvector_centrality(&rewired_centralities,1)) 
	  my_graph.rewire_edges3(rand());
	//get the edge attributes
	for (map<string,bat>::iterator i=bats_to_ids.begin(); i!=bats_to_ids.end(); i++) 
	  evectorfile_shuffled<<i->first<<"\t"<<VECTOR(rewired_centralities)[i->second.id]<<endl;		
		    
	igraph_vector_destroy(&rewired_centralities);
      }
      evectorfile_shuffled.close();
      cout<<"\t Done model 3"<<endl;
      evectorfile_shuffled.open("/home/pmavrodiev/Documents/bats/result_files/output_files_new_2/combined-model-4.dat",ios::trunc);
      cout<<"\t Model 4 ..."<<endl;
      for (unsigned k=0; k<nsimu; k++) {
	igraph_vector_t rewired_centralities;
	igraph_vector_init(&rewired_centralities,igraph_matrix_nrow(&lf_adjmatrix));
	igraph_vector_null(&rewired_centralities);
	my_graph.rewire_edges4(probs,rand());
	while (my_graph.eigenvector_centrality(&rewired_centralities,1)) 
	  my_graph.rewire_edges4(probs,rand());
	//get the edge attributes
	for (map<string,bat>::iterator i=bats_to_ids.begin(); i!=bats_to_ids.end(); i++) 
	  evectorfile_shuffled<<i->first<<"\t"<<VECTOR(rewired_centralities)[i->second.id]<<endl;		
		    
	igraph_vector_destroy(&rewired_centralities);
      }
      evectorfile_shuffled.close();
      cout<<"\t Done model 4"<<endl;
      evectorfile_shuffled.open("/home/pmavrodiev/Documents/bats/result_files/output_files_new_2/combined-model-5.dat",ios::trunc);
      cout<<"\t Model 5 ..."<<endl;
      for (unsigned k=0; k<nsimu; k++) {
	igraph_vector_t rewired_centralities;
	igraph_vector_init(&rewired_centralities,igraph_matrix_nrow(&lf_adjmatrix));
	igraph_vector_null(&rewired_centralities);
	my_graph.rewire_edges5();
	while (my_graph.eigenvector_centrality(&rewired_centralities,1)) 
	  my_graph.rewire_edges5();
	//get the edge attributes
	for (map<string,bat>::iterator i=bats_to_ids.begin(); i!=bats_to_ids.end(); i++) 
	  evectorfile_shuffled<<i->first<<"\t"<<VECTOR(rewired_centralities)[i->second.id]<<endl;		
		    
	igraph_vector_destroy(&rewired_centralities);
      }
      evectorfile_shuffled.close();
      cout<<"\t Done model 5"<<endl;
      
      cout<<"DONE"<<endl;
      return 0;
	
}

   

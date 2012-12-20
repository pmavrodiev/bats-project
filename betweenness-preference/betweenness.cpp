  #include <iostream>
  #include <fstream>
  #include <sstream>
  #include <vector>
  #include <list>
  #include "standard.yy.h"
  #include <map>
  #include <set>
  #include <dirent.h>
  #include <unistd.h>
  #include <sys/stat.h>
  #include <sys/types.h>
  #include <cctype>
  #include <algorithm>
  #include <stdio.h>
  #include <stdlib.h>
  #include <math.h>
  #include "betweenness.h"
  using namespace std;

  /*CLASS custom_edge*/
  custom_edge::custom_edge() {
    from=0; to=0; type=0;    
  }
  custom_edge::custom_edge(unsigned int f, unsigned int t, unsigned int _type) {
    from=f; to=t; type=_type;
  }
  bool operator==(custom_edge& c1, custom_edge& c2) {
    return (c1.from == c2.from) && (c1.to == c2.to) && (c1.type == c2.type);
  }
  bool operator!=(custom_edge& c1, custom_edge& c2) {
    return !(c1 == c2);
  }
 
 
  
  /*********************/
  /*CLASS time_unfolded_temporal_network*/
  time_unfolded_temporal_network::time_unfolded_temporal_network() {
    nnodes=0; ntimesteps=0;valid=true;aggregate_network_initialized=false;}
    time_unfolded_temporal_network::time_unfolded_temporal_network(unsigned other_nnodes, unsigned other_ntimesteps) {
    nnodes = other_nnodes;
    ntimesteps = other_ntimesteps;
    valid=true;
  }

  void time_unfolded_temporal_network::print_temporal_network() {
    for (unsigned j=0; j<temporal_network.size(); j++){
      vector<custom_edge> &ref = temporal_network[j];
      cout<<"Time step "<<j<<": ";
      for (unsigned k=0; k<ref.size(); k++) 
	  cout<<"("<<ref[k].type<<"):"<<ref[k].from<<"->"<<ref[k].to<<" ";      
      cout<<endl;
    }
  }

  vector<custom_edge> time_unfolded_temporal_network::find_source(unsigned source_id,unsigned time) {
    vector<custom_edge> &ref = temporal_network[time];
    vector<custom_edge> return_vector;
    for (unsigned i=0; i<ref.size(); i++) 
      if (ref[i].from == source_id)
	return_vector.push_back(ref[i]);      
    return return_vector;
  }

  vector<custom_edge> time_unfolded_temporal_network::find_dest(unsigned dest_id,unsigned time) {
    vector<custom_edge> &ref = temporal_network[time];
    vector<custom_edge > return_vector;
    for (unsigned i=0; i<ref.size(); i++) 
      if (ref[i].to == dest_id)
	return_vector.push_back(ref[i]);    
    return return_vector;
  }

  vector<custom_edge> time_unfolded_temporal_network::merge(vector<custom_edge> *one,
								         vector<custom_edge> *two) {
    vector<custom_edge> return_vector;
    for (unsigned i=0; i<one->size(); i++) 
      for (unsigned j=0; j<two->size(); j++) {
	//merge only if the two edges are from the same type
	if ((*one)[i].type == (*two)[i].type)
	  return_vector.push_back(custom_edge((*one)[i].from,(*two)[i].to,(*one)[i].type));	  
      }
    return return_vector;
  }

  int time_unfolded_temporal_network::compute_pnbpm() {
    for (unsigned i=0; i<per_node_bp_matrices.size(); i++) {
    per_node_betweenness_preference_matrix *pnbpm_ptr = &per_node_bp_matrices[i];
    if (!pnbpm_ptr->initialised) {
      cerr<<"Cannot compute per-node-betweenness-preference-matrix for uninitialised node id"<<endl;
      continue;
    }
    //loop over all time steps
    unsigned which_node = pnbpm_ptr->node_id;
    for (unsigned t=0; t<(ntimesteps-2); t++) {
      //does a pair with destination=node_id exist at time t?      
      vector<custom_edge> dest = find_dest(which_node,t);
      if (dest.size() > 0) {
	//does a pair with source=node_id exist at time t+1
	//vector<pair<unsigned,unsigned> > &ref_t_plus_1 = temporal_network[t+1];
	vector<custom_edge> source = find_source(which_node,t+1);
	if (source.size() > 0) {
	  vector<custom_edge> merged = merge(&dest,&source);
	  for (unsigned ii=0; ii<merged.size(); ii++) {
	    vector<custom_edge> &ref = pnbpm_ptr->pnbpm[t];
	    ref.push_back(merged[ii]);
	  }
	}
	else {
	  
	}
      }
      else {
	
      }
    }
    if (pnbpm_ptr->pnbpm.size() != 0) pnbpm_ptr->defined=true;
    }  
    return 0;
  }

  int time_unfolded_temporal_network::compute_tabpm() {
    for (unsigned i=0; i<time_aggr_bp_matrices.size(); i++) {
      time_aggr_betweenness_preference_matrix *tabpm_ptr = &time_aggr_bp_matrices[i];
      per_node_betweenness_preference_matrix *pnbpm_ptr = &per_node_bp_matrices[i];
      if (!tabpm_ptr->initialised) {
	cerr<<"Cannot compute time-aggregated-betweenness-preference matrix for uninitialised node id"<<endl;
	continue;
      }
      if (!pnbpm_ptr->defined) {
	tabpm_ptr->defined=false; 
	continue;	
      }      
      tabpm_ptr->defined=true;
      //get all unique pairs
      set<custom_edge,edgecomp> unique_pairs = pnbpm_ptr->get_unique_pairs();
      //loop over all unique pairs
      for (set<custom_edge,edgecomp>::iterator itr=unique_pairs.begin(); itr!=unique_pairs.end(); itr++) {
	double pair_sum = 0.0;
	//loop over all time steps
	for (unsigned t=0; t<(ntimesteps-2); t++) {
	  vector<custom_edge> &pnbpm_ref = pnbpm_ptr->pnbpm[t];
	  unsigned total_paths_at_this_time = pnbpm_ref.size();
	  if (total_paths_at_this_time > 0) {
	    if (pnbpm_ptr->pair_exists(*itr,t)) 
	      pair_sum += (1.0 / total_paths_at_this_time);	  
	  }
	}
	tabpm_ptr->tabpm[(*itr)] = pair_sum;
      }
    }
    return 0;  
  }

  int time_unfolded_temporal_network::build_aggregate_network() {
    if (!valid) {
      cerr<<"Cannot compute aggregate network with unitialized temporal one"<<endl;
      return 1;
    }
    //set<pair<unsigned,unsigned> > tmp;
    set<custom_edge,edgecomp> tmp;
    pair<set<custom_edge,edgecomp>::iterator,bool > tmp_insert_itr;
    for (map<unsigned, vector<custom_edge> >::iterator itr=temporal_network.begin();itr!=temporal_network.end(); itr++) {
      for (unsigned j=0; j<itr->second.size(); j++) {
	tmp_insert_itr = tmp.insert(itr->second[j]);
	if (tmp_insert_itr.second)  //new element was inserted
	  aggregate_network[itr->second[j]] = 1;
	
	else  //element exists 
	  aggregate_network[itr->second[j]]++;      
      }    
    }
    aggregate_network_initialized=true;
    return 0;
    }

  void time_unfolded_temporal_network::print_aggregate_network() {
    for (map<custom_edge, unsigned >::iterator itr=aggregate_network.begin(); itr!=aggregate_network.end(); itr++) 
      cout<<itr->first.from<<"->"<<itr->first.to<<": "<<itr->second<<endl;  
  }

  unsigned time_unfolded_temporal_network::source_fixed_sum(unsigned source ) {
    unsigned sum=0;
    for (map<custom_edge, unsigned >::iterator itr=aggregate_network.begin(); itr!=aggregate_network.end(); itr++)
      if (itr->first.from == source) sum += itr->second;     
    return sum;
  }
  
  unsigned time_unfolded_temporal_network::destination_fixed_sum(unsigned destination) {
    unsigned sum=0;
    for (map<custom_edge, unsigned >::iterator itr=aggregate_network.begin(); itr!=aggregate_network.end(); itr++)
      if (itr->first.to == destination) sum += itr->second;     
    return sum;
  }

  int time_unfolded_temporal_network::compute_baseline_bpm() {    
    for (unsigned i=0; i<baseline_bp_matrices.size(); i++) {
      baseline_betweenness_preference_matrix *baseline_bpm_ptr = &baseline_bp_matrices[i];
      if (!baseline_bpm_ptr) {
	cerr<<"Cannot compute baseline betweenness preference matrix - node ids do not match"<<endl;
	continue;//return 1; FLAG
      }
      if (!baseline_bpm_ptr->initialised) {
	cerr<<"Cannnot compute baseline betweenness preference matrix, because matrix is uninitialised"<<endl;
	continue;//return 1;
      }
      if (!aggregate_network_initialized) {
	cerr<<"Cannnot compute baseline betweenness preference matrix, because aggregate network is uninitialised"<<endl;
	continue;//return 1; 
      }   
      unsigned nid = baseline_bpm_ptr->node_id;
      unsigned total_inflow_into_nid = destination_fixed_sum(nid);
      unsigned total_outflow_from_nid = source_fixed_sum(nid);
      if (total_inflow_into_nid == 0 || total_outflow_from_nid == 0) {	  
	baseline_bpm_ptr->defined=false;
	continue;//return 0;
      }
      baseline_bpm_ptr->defined=true;
      //find all pairs with destination=nid
      vector<pair<unsigned,unsigned> > sources;
      for (map<custom_edge, unsigned >::iterator itr=aggregate_network.begin(); itr!=aggregate_network.end(); itr++) 
	if (itr->first.to == nid) sources.push_back(pair<unsigned,unsigned>(itr->first.from,itr->second));      
      //find all pairs with source=nid
      vector< pair<unsigned,unsigned> > destinations;
      for (map<custom_edge, unsigned >::iterator itr=aggregate_network.begin(); itr!=aggregate_network.end(); itr++)
	if (itr->first.from == nid) destinations.push_back(pair<unsigned,unsigned>(itr->first.to,itr->second));
      //process all possible combinations
      for (unsigned j=0; j<sources.size(); j++) {
	for (unsigned k=0; k<destinations.size(); k++) {
	  custom_edge sd(sources[j].first,destinations[k].first,0);
	  double product = sources[j].second*(1.0 / total_inflow_into_nid)*
			  destinations[k].second*(1.0 / total_outflow_from_nid);
	  baseline_bpm_ptr->baseline_bp_matrix[sd] = product;
	}
      }	  
    }
  return 0;  
  }


  double time_unfolded_temporal_network::source_fixed_sum(unsigned source,baseline_betweenness_preference_matrix *ptr) {
    double sum=0.0;
    for (map<custom_edge, double>::iterator itr=ptr->baseline_bp_matrix.begin(); itr!=ptr->baseline_bp_matrix.end(); itr++) 
      if (itr->first.from == source) sum += itr->second;        
    return sum;
  }

  double time_unfolded_temporal_network::destination_fixed_sum(unsigned destination,baseline_betweenness_preference_matrix *ptr) {
    double sum=0.0;
    for (map<custom_edge, double>::iterator itr=ptr->baseline_bp_matrix.begin(); itr!=ptr->baseline_bp_matrix.end(); itr++) 
      if (itr->first.to == destination) sum += itr->second;        
    return sum;
    }

  double time_unfolded_temporal_network::betweenness_preference(unsigned nid) {
    baseline_betweenness_preference_matrix *baseline_bpm_ptr=0;
    for (unsigned i=0; i<baseline_bp_matrices.size(); i++) {
      if (baseline_bp_matrices[i].node_id == nid) {
	baseline_bpm_ptr = &baseline_bp_matrices[i];
	break;
      }
    }
    if (!baseline_bpm_ptr) {
      cerr<<"Cannot compute betweenness preference: node ids do not match"<<endl;
      return -1.0;
    }
    double bp=0.0;
    for (map<custom_edge, double>::iterator itr=baseline_bpm_ptr->baseline_bp_matrix.begin(); itr!=baseline_bpm_ptr->baseline_bp_matrix.end(); itr++) 
      bp += itr->second*log2(itr->second / (source_fixed_sum(itr->first.from,baseline_bpm_ptr)*destination_fixed_sum(itr->first.to,baseline_bpm_ptr)));
    return bp; 
  }

void time_unfolded_temporal_network::normalize_tabpms() {
  for (unsigned i=0; i<time_aggr_bp_matrices.size(); i++)
      time_aggr_bp_matrices[i].normalize_tabpm();
}

void time_unfolded_temporal_network::betweenness_preferences() {
  for (unsigned i=0; i<time_aggr_bp_matrices.size(); i++)
      time_aggr_bp_matrices[i].betweenness_preference();

}

void time_unfolded_temporal_network::print_all_betweenness_preferences() {
  for (unsigned i=0; i<time_aggr_bp_matrices.size(); i++) {
    cout<<"Node id "<<time_aggr_bp_matrices[i].node_id<<": ";
    if (!time_aggr_bp_matrices[i].defined)
      cout<<"UNDEFINED"<<endl;
    else
      cout<<time_aggr_bp_matrices[i].betweenness_pref<<endl;    
  }
}

void time_unfolded_temporal_network::print_baseline_bp_matrices() {
 for (unsigned i=0; i<baseline_bp_matrices.size(); i++)
	baseline_bp_matrices[i].print_baseline_bp_matrix();
}

  /**********************************************/
  /*CLASS per_node_betweenness_preference_matrix*/
  per_node_betweenness_preference_matrix::per_node_betweenness_preference_matrix() {
    initialised = false;
  }
  void per_node_betweenness_preference_matrix::init_node_id(unsigned nid) {
    node_id = nid;
    initialised = true;
    defined = false;
  }

  void per_node_betweenness_preference_matrix::print_pnbpm() {
    if (!defined) 
    cout<<"UNDEFINED"<<endl;
    else {
      for (map<unsigned,vector<custom_edge> >::iterator itr=pnbpm.begin(); itr!=pnbpm.end();itr++) {
	cout<<itr->first<<": ";
	for (unsigned i=0; i<itr->second.size();i++) 
	  cout<<"("<<itr->second[i].type<<"):"<<itr->second[i].from<<" -> "<<itr->second[i].to<<" , ";	
	cout<<endl;
      }
    }
  }

  set<custom_edge,edgecomp> per_node_betweenness_preference_matrix::get_unique_pairs() {
    set<custom_edge,edgecomp> return_set;
    pair<set<custom_edge,edgecomp>::iterator, bool>  set_itr;
    for (map<unsigned, vector<custom_edge> >::iterator itr=pnbpm.begin(); itr!=pnbpm.end(); itr++) 
      for (unsigned j=0; j<itr->second.size(); j++) 
	set_itr = return_set.insert(itr->second[j]);         
    return return_set;
  }

  bool per_node_betweenness_preference_matrix::pair_exists(custom_edge c,unsigned time) {
    vector<custom_edge> &ref = pnbpm[time];
    for (unsigned i=0; i<ref.size(); i++) 
      if (c == ref[i]) 
	return true;     
    return false;
  }

  /**********************************************/

  /**********************************************/

  /*CLASS time_aggr_betweenness_preference_matrix*/
  time_aggr_betweenness_preference_matrix::time_aggr_betweenness_preference_matrix() {
    initialised = false;
  }
  void time_aggr_betweenness_preference_matrix::init_node_id(unsigned nid) {
    node_id = nid;
    initialised = true;
    defined=false;
  }

  void time_aggr_betweenness_preference_matrix::print_tabpm() {
    if (!defined) 
    cout<<"UNDEFINED"<<endl;
    else {
      for (map<custom_edge, double,edgecomp>::iterator itr=tabpm.begin(); itr!=tabpm.end(); itr++) 
	cout<<itr->first.from<<" -> "<<itr->first.to<<": "<<itr->second<<endl;
    }
  }

  void time_aggr_betweenness_preference_matrix::normalize_tabpm() {
    if (defined) {
      double total_sum = 0.0;
      for (map<custom_edge,double,edgecomp>::iterator itr=tabpm.begin(); itr!=tabpm.end();itr++) 
	total_sum += itr->second;  
      for (map<custom_edge,double,edgecomp>::iterator itr=tabpm.begin(); itr!=tabpm.end();itr++) 
	normalized_tabpm[itr->first] = itr->second / total_sum;      
    }
  }

  void time_aggr_betweenness_preference_matrix::print_normalized_tabpm() {
    for (map<custom_edge, double,edgecomp>::iterator itr=normalized_tabpm.begin(); itr!=normalized_tabpm.end(); itr++) 
      cout<<itr->first.from<<" -> "<<itr->first.to<<": "<<itr->second<<endl;  
  }

  double time_aggr_betweenness_preference_matrix::source_fixed_sum(unsigned source) {
    double sum=0.0;
    for (map<custom_edge, double,edgecomp>::iterator itr=normalized_tabpm.begin(); itr!=normalized_tabpm.end(); itr++) 
      if (itr->first.from == source) 
	sum += itr->second;      
  return sum;
  }

  double time_aggr_betweenness_preference_matrix::destination_fixed_sum(unsigned destination) {
    double sum=0.0;
    for (map<custom_edge, double,edgecomp>::iterator itr=normalized_tabpm.begin(); itr!=normalized_tabpm.end(); itr++) 
      if (itr->first.to == destination) 
	sum += itr->second;      
    return sum;
  }

  void time_aggr_betweenness_preference_matrix::betweenness_preference() {
    if (defined) {
      double bp = 0.0;
      for (map<custom_edge, double,edgecomp>::iterator itr=normalized_tabpm.begin(); itr!=normalized_tabpm.end(); itr++)
	bp += itr->second*log2(itr->second / (source_fixed_sum(itr->first.from)*destination_fixed_sum(itr->first.to)) );
      betweenness_pref=bp; 
    }
    else
      betweenness_pref=-1.0;
  }

  /**********************************************/

  /*CLASS baseline_betweenness_preference_matrix*/
  baseline_betweenness_preference_matrix::baseline_betweenness_preference_matrix() {
    initialised = false;
  }
  void baseline_betweenness_preference_matrix::init_node_id ( unsigned nid ) {
    node_id = nid;
    initialised = true;
    defined = false;
  }

  void baseline_betweenness_preference_matrix::print_baseline_bp_matrix() {
    cout<<node_id<<": ";
    if (!defined) 
         cout<<"UNDEFINED"<<endl;
    else {
      for (map<custom_edge, double >::iterator itr=baseline_bp_matrix.begin(); itr!=baseline_bp_matrix.end(); itr++)
	cout<<"("<<itr->first.type<<"):"<<itr->first.from<<" -> "<<itr->first.to<<": "<<itr->second<<endl;  
    }    
  }
  /**********************************************/

  time_unfolded_temporal_network tutn;
  custom_edge new_edge;
  
  int main(int argc, char**argv) {
  //argv++;
  argc--;
  if (argc < 1 || argc >= 2) {       
      cout<<"Usage: betweenness-preference <config_file>"<<endl<<endl;        
      cout<<"<config_file>\t full path to the configuration file"<<endl<<endl;
      return 0;
  }
  if (argc == 1) {	  
      char* file_name = argv[1];
      cout<<file_name<<endl;	
      yyin = fopen(file_name,"r");	
      if (yyin == NULL) {
	  perror(file_name);
	  exit(1);
      }        
      int parse_success=yylex();
      if (!parse_success) {
	int aggregate_network_flag = tutn.build_aggregate_network();
	if (!aggregate_network_flag) {	//success
	  //tutn.print_aggregate_network();	  	  
	  //tutn.print_temporal_network();	  
	  for (unsigned node_id=1; node_id<=tutn.nnodes; node_id++) {	  
	    /*compute the (p)er (n)ode (b)etweeness (p)reference (m)atrix for node_id*/
	    tutn.per_node_bp_matrices.push_back(per_node_betweenness_preference_matrix());
	    tutn.time_aggr_bp_matrices.push_back(time_aggr_betweenness_preference_matrix());
	    tutn.baseline_bp_matrices.push_back(baseline_betweenness_preference_matrix());
	    tutn.per_node_bp_matrices[node_id-1].init_node_id(node_id);
	    tutn.time_aggr_bp_matrices[node_id-1].init_node_id(node_id);
	    tutn.baseline_bp_matrices[node_id-1].init_node_id(node_id);	    
	  }	  
	  int baseline_bp_matrix_success = tutn.compute_baseline_bpm();
	  if (!baseline_bp_matrix_success) {	    
	    //tutn.print_baseline_bp_matrices();
	    //tutn.baseline_bp_matrices[counter].print_baseline_bp_matrix();
	    //cout<<tutn.betweenness_preference(node_id)<<endl;	    
	  }
	  int pnbpm_success = tutn.compute_pnbpm();
	  if (!pnbpm_success) { //success
	    //tutn.per_node_bp_matrices[3].print_pnbpm();	
	    int tabpm_sucess = tutn.compute_tabpm();
	    if (!tabpm_sucess) { //success
	      //tutn.time_aggr_bp_matrices[3].print_tabpm();
	      tutn.normalize_tabpms();
	      //tutn.time_aggr_bp_matrices[3].print_normalized_tabpm();
	      tutn.betweenness_preferences();
	      tutn.print_all_betweenness_preferences();
	    }  
	  }	
      }
  }
  }
  return 0;    
  }
  







/* ======================== CLASS DECLARATIONS ========================= */
//class Box; //represents a box
//class Bat; //represents a bat
//class BatEntry; //represents an entry of a bat into a box
//class Lf_pair; //represents a leading following pair
/* ===================================================================== */
#include "classes.h"
#include <iostream>
#include <math.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/greg_month.hpp>
#include <boost/date_time/gregorian/formatters.hpp>
#include "/usr/local/include/igraph/igraph.h"
using namespace boost;
using namespace boost::posix_time;
using namespace boost::gregorian;
extern map<string,vector<string> > box_programming;
extern tm start_exp;
extern tm end_exp;
/* ======================== CLASS DEFINITIONS ========================== */



BatEntry::BatEntry(string entry_time, string hex, string name) {
        TimeOfEntry = ptime(from_iso_string(entry_time));
        hexid = hex;
        occurence = 1;
        box_name = name;
  
}
void BatEntry::print(ostream *out) const {      
  if (!out->good()) {
    std::cerr<<"Cannot print to output file: bad file descriptor"<<std::endl;
  }     
    (*out)<<to_simple_string(TimeOfEntry)<<" "<<hexid<<" "<<occurence<<" "<<box_name<<std::endl;
}


LF_FLAG::LF_FLAG() {
  this_lf_flag = UNINIT;
}

LF_FLAG::LF_FLAG(lf_flag_types other) {
  this_lf_flag = other;
}


bool Box::lfcomp::operator()(Lf_pair l1, Lf_pair l2) const {
   if (l1.leader->hexid == l2.leader->hexid) 
      return l1.follower->hexid < l2.follower->hexid;    
   return (l1.leader->hexid < l2.leader->hexid);    
}
 
event::event(string event_name, Bat* whichbat , Box* whichbox, ptime whattime, Bat *leader) {
  eventname = event_name;
  bat = whichbat;
  aux_bat = leader;
  box = whichbox;
  eventtime = whattime;
  valid = true;
}

event::event() {
 eventname = "";
 box = NULL;
 bat = NULL;
 eventtime = not_a_date_time;
 valid = true;
}


void event::print(ofstream *out) {
  tm eventtimetm = to_tm(eventtime);
  tm occupationtm, installationtm;
  double occupation;
  double seconds_per_day = 24.0 * 3600.0;
  double minutes_per_day = 60.0;
  installationtm = to_tm(box->discoveredBy.second);//to_tm(box->installedWhen);
  if (box->occupiedWhen.is_pos_infinity())
    occupation = 0;
  else {
    occupationtm = to_tm(box->occupiedWhen);
    occupation = difftime(mktime(&occupationtm),mktime(&installationtm)) / seconds_per_day;
  }  
  double event_dur = difftime(mktime(&eventtimetm),mktime(&installationtm)) / seconds_per_day;
  //(*out)<<box->name<<"\t"<<eventname<<" \t"<<bat->hexid<<"\t"<<to_iso_extended_string(eventtime)<<"\t"<<to_iso_extended_string(box->occupiedWhen); 
  (*out)<<box->name<<"\t"<<eventname<<" \t"<<bat->hexid<<"\t"<<event_dur<<" \t"<<occupation;
  if (aux_bat != NULL)
    (*out)<<"\t"<<aux_bat->hexid;
  else
    (*out)<<"\t ignore";
  (*out)<<endl;
  
  
}


Box::Box(short Type, string Name, ptime occ, ptime installation) {
  type = Type;
  name = Name;
  this->status = UNDISCOVERED;
  occupiedWhen = occ;
  installedWhen = installation;
  total_lf_events = 0;
  social_ud_lf_events= 0; 
  social_d_lf_events= 0; 
  personal_ud_lf_events= 0; 
  personal_d_lf_events= 0;   
  discoveredBy = pair<string,ptime>("",pos_infin);
}
Box::Box() {
  status = UNDISCOVERED;
  occupiedWhen = pos_infin;
  total_lf_events = 0;
  social_ud_lf_events= 0; 
  social_d_lf_events= 0; 
  personal_ud_lf_events= 0; 
  personal_d_lf_events= 0;   
  discoveredBy = pair<string,ptime>("",pos_infin);
}
void Box::print() {
  set<BatEntry,batEntryCompare>::iterator i;
  std::cout<<"Box "<<name<<" "<<activity.size()<<" Occ. Date "<<to_simple_string(occupiedWhen)<<std::endl;
  for (i=activity.begin(); i != activity.end(); i++) {
    i->print(&cout);
  }
}

void Box::print_detailed_occupation() {
  cout<<name<<endl;
  for (unsigned i=0; i<occupationHistory.size(); i++) {
    cout<<to_simple_string(occupationHistory[i].first)<<":"<<endl;
    for (unsigned j=0; j<occupationHistory[i].second.size();j++) {
      cout<<occupationHistory[i].second[j]->hexid<<",";
    }
    cout<<endl;
  }      
}


void Box::discovered(string bat_id, ptime when) {
  if (!discoveredBy.second.is_pos_infinity()) {
    cerr<<"Error: Box already discovered"<<endl;exit(1);
  }
  discoveredBy.second=when;
  discoveredBy.first=bat_id;
}

bool eventcomp(event e1, event e2) {
   return e1.eventtime < e2.eventtime;
}

void Box::sort_information_spread() {
  std::sort(information_spread.begin(),information_spread.end(),eventcomp);
}

void Box::clean_information_spread() {
  set<string> tmp;
  for (unsigned i=0; i<information_spread.size(); i++) {
    if (information_spread[i].eventname == "undisturbed-following" ||
        information_spread[i].eventname == "disturbed-following") {
      pair<set<string>::iterator,bool> insert_itr;
      insert_itr = tmp.insert(information_spread[i].bat->hexid);
      if (!insert_itr.second) //could not insert it - bat exists!
	information_spread[i].valid=false;
    }
  }
}

unsigned Box::get_num_occ_bats() {
  return (occupyingBats.size());  
}

string Box::howmany_leaders_followers() {
  //#dist leaders,#undist. leaders,#dist followers, #undist. followers, #naiive, #experienced
  int stats[6] = {0,0,0,0,0,0};  
  for (unsigned i=0; i<get_num_occ_bats(); i++) {
    string bat_id = occupyingBats[i]->hexid;
    vector<string> programmed_bats = box_programming[this->name];
    bool found = false;
    for (unsigned j=0; j<programmed_bats.size();j++) 
      if (programmed_bats[j] == bat_id) {
	found=true; break;
      }
    /**/  
    if (leaders.find(bat_id) != leaders.end()) {            
      if (found)
	stats[0]++;
      else
	stats[1]++;
    }
    else if (followers.find(bat_id) != followers.end()) {      
      if (found)
	stats[2]++;
      else
	stats[3]++;      
    }
    else if (occupyingBats[i]->box_knowledge[name].box_knowledge == NAIIVE)
      stats[4]++;
    else if (occupyingBats[i]->box_knowledge[name].box_knowledge == EXPERIENCED)
      stats[5]++;
    else {
      stats[0]=-1;stats[1]=-1;stats[2]=-1;stats[3]=-1;stats[4]=-1;stats[5]=-1;
    }
  }
  stringstream ss;
  ss<<stats[0]<<"("<<leaders.size()<<")"<<"\t\t\t\t";
  ss<<stats[1]<<"("<<leaders.size()<<")"<<"\t\t\t\t\t";
  ss<<stats[2]<<"("<<followers.size()<<")"<<"\t\t\t\t\t";
  ss<<stats[3]<<"("<<followers.size()<<")"<<"\t\t\t   ";
  ss<<stats[4]<<"\t\t";
  ss<<stats[5];
  return ss.str();
}

time_duration Box::getOccupiedDiscoveredDelta() {
  if  (occupiedWhen.is_not_a_date_time() || discoveredBy.second.is_not_a_date_time()) {
    cerr<<"Error: Should never happen"<<endl; exit(1);
  }
  if (discoveredBy.second.is_pos_infinity())  
    return time_duration(0,0,0,0);
  
  time_duration td = occupiedWhen - discoveredBy.second;
  return (td);
}


Lf_pair::Lf_pair() {
  valid=true;
  tleader=not_a_date_time;
  tfollower=not_a_date_time;
  box_name="";
  leader_disturbed = false;
  is_passive_leading = false;
}
Lf_pair::Lf_pair(Bat *B1, Bat *B2) {
  leader = B1;
  follower = B2;
  leader_disturbed=false;
  valid=true;
  is_passive_leading = false;
}
Lf_pair::Lf_pair(Bat *B1, Bat *B2, ptime tB1, ptime tB2,string bname) {
  leader = B1;
  follower = B2;
  tleader = tB1;
  tfollower = tB2;
  box_name = bname;
  valid = true;
  leader_disturbed=false;
  is_passive_leading = false;
}
Lf_pair::Lf_pair(Bat *B1, Bat *B2, ptime tB1, ptime tB2,string bname, bool v) {
  leader = B1;
  follower = B2;
  tleader = tB1;
  tfollower = tB2;
  box_name = bname;
  valid = v;
  leader_disturbed=false;
  is_passive_leading = false;
}

bool Lf_pair::equals(Lf_pair &other) {
  if (((other.leader->hexid == leader->hexid && other.follower->hexid == follower->hexid) ||
      (other.leader->hexid == follower->hexid && other.follower->hexid == leader->hexid)) &&
      other.box_name == box_name)
     return true;
  return false;
}
void Lf_pair::init(Bat *B1, Bat *B2, ptime tB1, ptime tB2,string bname) {
  leader = B1;
  follower = B2;
  tleader = tB1;
  tfollower = tB2;
  box_name = bname;
  valid = true;
  leader_disturbed=false;
}

void Lf_pair::print(ofstream *out) {
  if (!out->good()) {
   std::cerr<<"Cannot print to output file: bad file descriptor"<<std::endl;
  }
  else {
    (*out)<<leader->hexid<<" "<<follower->hexid<<" "<<to_iso_string(tleader);
    (*out)<<" "<<to_iso_string(tfollower)<<" "<<box_name<<" "<<leader->box_knowledge[box_name].box_knowledge_how;
    (*out)<<" "<<leader->box_knowledge[box_name].box_knowledge<<std::endl;
  }
}

time_duration Lf_pair::get_lf_delta() {
  time_duration t = tfollower-tleader;
  if (t.is_negative()) t = t.invert_sign();
    return t;
}
void Lf_pair::validate(time_duration limit) {
   if (get_lf_delta() > limit) valid = false;
   else {
    leader->part_of_lf_event=true;
    follower->part_of_lf_event=true;
   }
}

string Lf_pair::getLeaderId() {
   return leader->hexid;
}
string Lf_pair::getFollowerId() {
   return follower->hexid;
}

bool Lf_pair_compare(Lf_pair lf1, Lf_pair lf2) {
      return (lf1.tleader < lf2.tleader);    
}


mybool::mybool() {custom_boolean = FALSE; /*UNINITIALIZED;*/}
mybool::mybool(MYBOOLEAN b) {custom_boolean=b;}


//the knowledge of bats per box
BatKnowledge::BatKnowledge() {box_knowledge=NAIIVE;box_knowledge_how=UNDEFINED;}
BatKnowledge::BatKnowledge(BatKnowledgeEnum b1, BatKnowledgeHow b2) {
    box_knowledge = b1;
    box_knowledge_how = b2;
}

//true if pair is inserted, false otherwise
bool Bat::insert_pair(Lf_pair lfp) {
  bool exists=false;
  for (unsigned k=0; k<my_lfpairs.size(); k++) {
    if (my_lfpairs[k].equals(lfp)) {
      if (lfp.get_lf_delta() < my_lfpairs[k].get_lf_delta())
	my_lfpairs[k]=lfp;
      exists=true;
      break;
    }
  }
  if (!exists) {
    my_lfpairs.push_back(lfp);
    return true;
  }
  return false;
}
    
void Bat::add_movement(ptime Time, Box * box_ptr) {
  pair<ptime,Box*> entry(Time,box_ptr);
  pair<set<pair<ptime, Box*>,movementCompare>::iterator,bool> insert_itr;
  insert_itr = movement_history.insert(entry);
  if (insert_itr.second == false)
     std::cerr<<"Warning: Duplicate movement entry for bat "<<hexid<<std::endl;
}
    //make the bat informed
void Bat::make_informed(string box_name,ptime informed_time) {
    ptime &ref = Bat::informed_since[box_name];
    if (ref.is_not_a_date_time()) {
      ref = informed_time;
    }
    else {
      std::cerr<<"Warning::Bat "<<hexid<<" informed flag set more than once"<<std::endl;
    }
}
//is the bat informed about a particular box at a given time
bool Bat::is_informed(string box_name, ptime when) {
   if (box_knowledge[box_name].box_knowledge == NAIIVE) return false;
   //sanity check
   if (informed_since[box_name] == not_a_date_time)
    std::cerr<<"Warning::Bat should have been informed."<<std::endl;
   if (informed_since[box_name] > when) return false;
   else return true;
}
pair<ptime, Box*> Bat::get_movement_history (int idx) {
  int counter = 0;
  set<pair<ptime, Box*>,movementCompare>::iterator i;
  i=movement_history.begin();
  while (counter < idx) {
    i++;
    counter++;
  }
  return *i;
}

Bat::Bat(string Id) {
  hexid = Id;
  n_daughters_following=0.0;
  total_following=0.0;
  cumulative_relatedness=0.0;
  part_of_lf_event=false;
  internal_check=0;  
  //last_revisit = neg_infin;
}
Bat::Bat() {
  hexid = "";
  n_daughters_following=0.0;
  total_following=0.0;
  cumulative_relatedness=0.0;
  part_of_lf_event=false;
  internal_check=0;  
  //last_revisit = neg_infin;
}
void Bat::print() {
  std::cout<<"BAT "<<hexid<<std::endl;
  for (unsigned i=0; i<movement_history.size(); i++) {
    pair<ptime, Box*> m = get_movement_history(i);
    std::cout<<to_simple_string(m.first)<<" "<<m.second->name<<std::endl;
  } 
}

void Bat::clean_movement_history() {
  set<pair<ptime, Box*>,movementCompare>::iterator itr;
  ptime previous_entry = neg_infin; 
  time_duration td = minutes(10);
  for (itr=movement_history.begin(); itr != movement_history.end(); itr++) {
    if (itr->first - previous_entry >= td) {
      cleaned_movement_history.push_back(*itr);
      previous_entry = itr->first;
    }
  }
}


assortativity_map::assortativity_map() {}

void assortativity_map::print_all(ofstream *out) {
    for (map<unsigned, vector<unsigned> >::iterator itr2=m.begin(); itr2 != m.end(); itr2++) {	  
      (*out)<<itr2->first<<" ";
      for (unsigned ttt=0; ttt<itr2->second.size(); ttt++)     
	(*out)<<itr2->second[ttt]<<" ";
      (*out)<<endl;
    }    
}  
  
void assortativity_map::print_average(ofstream *out) {
    for (map<unsigned, vector<unsigned> >::iterator itr2=m.begin(); itr2 != m.end(); itr2++) {	  
      double sum=0.0;      
      for (unsigned ttt=0; ttt<itr2->second.size(); ttt++)
	sum += itr2->second[ttt];      
      (*out)<<itr2->first<<" "<<sum / itr2->second.size() <<std::endl;	  
    }
}

int assortativity_map::avg_neighbour_connectivity(igraph_vector_t *indegrees,
			         igraph_t *graph,
			         unsigned nnodes) {
  //igraph_vector_add_constant(indegrees,1); //add the constant back 
  for (unsigned i=0; i<nnodes; i++) {
    igraph_integer_t v_id = i;
    unsigned v_indegree = (unsigned) VECTOR(*indegrees)[v_id];
    //select all neighbours of v_id
    igraph_vs_t vs;
    igraph_vs_adj(&vs,v_id,/*degree=*/IGRAPH_OUT);
    igraph_vit_t vit;
    igraph_vit_create(graph,vs,&vit);
    while (!IGRAPH_VIT_END(vit)) {
      igraph_integer_t neighbour_id = (igraph_integer_t) IGRAPH_VIT_GET(vit);
      m[v_indegree].push_back( VECTOR(*indegrees)[neighbour_id]);
      IGRAPH_VIT_NEXT(vit);
    }
    igraph_vit_destroy(&vit);
    igraph_vs_destroy(&vs);
  }  
  return 0;
}  


myigraph::myigraph(igraph_matrix_t* adjmatrix) {
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

int myigraph::eigenvector_centrality(igraph_vector_t* result, int which_graph) {
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
    igraph_error_handler_t *old_handler = igraph_set_error_handler(&igraph_error_handler_printignore);    
    igraph_warning_handler_t *old_warning_handler = igraph_set_warning_handler(&igraph_warning_handler_ignore);
    
    int errcode = igraph_eigenvector_centrality(&temp_g,result,&eigenvalue,/*directed=*/true,/*scale=*/false,&edge_importance,&aroptions);    
    /**/
    igraph_vector_destroy(&edge_importance);
    igraph_matrix_destroy(&m);
    igraph_destroy(&temp_g);
    return errcode;
}

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
}


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
}



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
    //cout<<cdf[j-1]<<"\t"<<cdf[j]<<"\t"<<random_number<<endl;
    if (random_number > cdf[j-1] && random_number <= cdf[j])
      found=true;
    if (found) break;
  }  
  return j-1;
}



void myigraph::print_adjacency_matrix(int which_graph, ostream *out) {
    //0 for original graph, 1 for the rewired
    igraph_matrix_t m;
    igraph_matrix_init(&m,igraph_matrix_nrow(&weighted_adj_matrix),igraph_matrix_ncol(&weighted_adj_matrix));
    igraph_matrix_null(&m);
    igraph_get_adjacency((which_graph==0 ? &graph: &rewired_graph),&m,IGRAPH_GET_ADJACENCY_BOTH,false);        
    *out<<";";
    for (unsigned i=0; i<igraph_matrix_nrow(&weighted_adj_matrix); i++) 
      *out<<i<<";";
    *out<<endl;
    for (long r=0; r<igraph_matrix_nrow(&m); r++) {
      *out<<r<<";";
      for (long c=0; c<igraph_matrix_ncol(&m); c++) {
	*out<<MATRIX(m,r,c)<<";";      
      }
      *out<<endl;   
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


/* ==================================================================== */

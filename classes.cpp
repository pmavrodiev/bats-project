
/* ======================== CLASS DECLARATIONS ========================= */
//class Box; //represents a box
//class Bat; //represents a bat
//class BatEntry; //represents an entry of a bat into a box
//class Lf_pair; //represents a leading following pair
/* ===================================================================== */
#include "classes.h"
#include <iostream>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/greg_month.hpp>
#include <boost/date_time/gregorian/formatters.hpp>
#include "/usr/local/include/igraph/igraph.h"
using namespace boost;
using namespace boost::posix_time;
using namespace boost::gregorian;
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



Box::Box(short Type, string Name, ptime occ) {
  type = Type;
  name = Name;
  this->status = UNDISCOVERED;
  occupiedWhen = occ;
  total_lf_events = 0;
  social_lf_events = 0;
  personal_lf_events = 0;
}
Box::Box() {
  status = UNDISCOVERED;
  occupiedWhen = pos_infin;
  total_lf_events = 0;
  social_lf_events = 0;
  personal_lf_events = 0;
}
void Box::print() {
  set<BatEntry,batEntryCompare>::iterator i;
  std::cout<<"Box "<<name<<" "<<activity.size()<<" Occ. Date "<<to_simple_string(occupiedWhen)<<std::endl;
  for (i=activity.begin(); i != activity.end(); i++) {
    i->print(&cout);
  }
}


Lf_pair::Lf_pair() {
  valid=true;
  tleader=not_a_date_time;
  tfollower=not_a_date_time;
  box_name="";
  leader_disturbed=false;  
}
Lf_pair::Lf_pair(Bat *B1, Bat *B2) {
  leader = B1;
  follower = B2;
  leader_disturbed=false;
}
Lf_pair::Lf_pair(Bat *B1, Bat *B2, ptime tB1, ptime tB2,string bname) {
  leader = B1;
  follower = B2;
  tleader = tB1;
  tfollower = tB2;
  box_name = bname;
  valid = true;
  leader_disturbed=false;
}
Lf_pair::Lf_pair(Bat *B1, Bat *B2, ptime tB1, ptime tB2,string bname, bool v) {
  leader = B1;
  follower = B2;
  tleader = tB1;
  tfollower = tB2;
  box_name = bname;
  valid = v;
  leader_disturbed=false;
}

bool Lf_pair::equals(Lf_pair &other) {
  if ((other.leader->hexid == leader->hexid && other.follower->hexid == follower->hexid) ||
      (other.leader->hexid == follower->hexid && other.follower->hexid == leader->hexid))
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

void Bat::insert_pair(Lf_pair lfp) {
  bool exists=false;
  for (unsigned k=0; k<my_lfpairs.size(); k++) {
    if (my_lfpairs[k].equals(lfp)) {
      if (lfp.get_lf_delta() < my_lfpairs[k].get_lf_delta())
	my_lfpairs[k]=lfp;
	exists=true;
	break;
    }
  }
  if (!exists)
    my_lfpairs.push_back(lfp);
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
}
Bat::Bat() {
  hexid = "";
  n_daughters_following=0.0;
  total_following=0.0;
  cumulative_relatedness=0.0;
  part_of_lf_event=false;
  internal_check=0;
}
void Bat::print() {
  std::cout<<"BAT "<<hexid<<std::endl;
  for (unsigned i=0; i<movement_history.size(); i++) {
    pair<ptime, Box*> m = get_movement_history(i);
    std::cout<<to_simple_string(m.first)<<" "<<m.second->name<<std::endl;
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
  igraph_vector_add_constant(indegrees,1); //add the constant back 
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












/* ==================================================================== */

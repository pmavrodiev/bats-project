
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
/*
#include <boost/numeric/ublas/fwd.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector_proxy.hpp>
#include <boost/numeric/ublas/triangular.hpp>
#include <boost/numeric/ublas/lu.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/ublas/exception.hpp>
*/
#include "/usr/local/include/igraph/igraph.h"
//#include "/usr/local/include/igraph/igraph_sparsemat.h"
using namespace boost;
using namespace boost::posix_time;
using namespace boost::gregorian;
extern map<string,vector<string> > box_programming;


/*helper functions*/
/*
template<class T>
void
 InverseMatrix2(const boost::numeric::ublas::matrix<T> &m, boost::numeric::ublas::matrix<T> &return_matrix ,
           bool &singular) {
     using namespace boost::numeric::ublas;
     const int size = m.size1();
     // Cannot invert if non-square matrix or 0x0 matrix.
     // Report it as singular in these cases, and return 
     // a 0x0 matrix.
     if (size != m.size2() || size == 0) {
         singular = true;
         return_matrix= matrix<T>(0,0);
	 return;
     }
     // Handle 1x1 matrix edge case as general purpose 
     // inverter below requires 2x2 to function properly.
     if (size == 1) {
         matrix<T> A(1, 1);
         if (m(0,0) == 0.0) {
             singular = true;
             return_matrix= matrix<T>(1,1);
	     return;
         }
         singular = false;
         A(0,0) = 1/m(0,0);
	 return_matrix = A;       
	 return;
     }
     // Create an augmented matrix A to invert. Assign the
     // matrix to be inverted to the left hand side and an
     // identity matrix to the right hand side.
     matrix<T> A(size, 2*size);
     matrix_range<matrix<T> > Aleft(A, 
                                    boost::numeric::ublas::range(0, size), 
                                    boost::numeric::ublas::range(0, size));
     Aleft = m;
     matrix_range<matrix<T> > Aright(A, 
                                     boost::numeric::ublas::range(0, size), 
                                     boost::numeric::ublas::range(size, 2*size));
     Aright = identity_matrix<T>(size);
     // Swap rows to eliminate zero diagonal elements.
     for (int k = 0; k < size; k++) {
         if ( A(k,k) == 0 ) // XXX: test for "small" instead
         {
             // Find a row(l) to swap with row(k)
             int l = -1;
             for (int i = k+1; i < size; i++)  {
                 if ( A(i,k) != 0 ) {
                     l = i; 
                     break;
                 }
             }
             // Swap the rows if found
             if ( l < 0 ) {
                 std::cerr << "Error:" <<  __FUNCTION__ << ":"
                           << "Input matrix is singular, because cannot find"
                           << " a row to swap while eliminating zero-diagonal.";
                 singular = true;
		 return;
             }
             else 
             {
                 matrix_row<matrix<T> > rowk(A, k);
                 matrix_row<matrix<T> > rowl(A, l);
                 rowk.swap(rowl); 
             }
         }
     }
     // Doing partial pivot
     for (int k = 0; k < size; k++)  {
         // normalize the current row
         for (int j = k+1; j < 2*size; j++)
             A(k,j) /= A(k,k);
         A(k,k) = 1;
         // normalize other rows
         for (int i = 0; i < size; i++) {
             if ( i != k )  // other rows  // FIX: PROBLEM HERE
             {
                 if ( A(i,k) != 0 )
                 {
                     for (int j = k+1; j < 2*size; j++)
                         A(i,j) -= A(k,j) * A(i,k);
                     A(i,k) = 0;
                 }
             }
         }
     }
     singular = false;
     return_matrix = Aright;
     return;     
 }

bool InvertMatrix(const boost::numeric::ublas::matrix<double>& input, boost::numeric::ublas::matrix<double>& inverse) {
 	using namespace boost::numeric::ublas;
 	typedef permutation_matrix<std::size_t> pmatrix;
 	// create a working copy of the input
 	matrix<double> A(input);
 	// create a permutation matrix for the LU-factorization
 	pmatrix pm(A.size1());
 	// perform LU-factorization
 	int res;
	try {
	  res = lu_factorize(A,pm);
	}
	catch (std::exception exc) {
	  cout<<"CAUGHT"<<endl;
	  return 1;
	}	
        if( res != 0 ) return false;
 	// create identity matrix of "inverse"
 	inverse.assign(boost::numeric::ublas::identity_matrix<double>(A.size1()));
 	// backsubstitute to get the inverse
 	lu_substitute(A, pm, inverse);
 	return true;
 }
*/
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
 
bool Box::sortby_tleader::operator()(Lf_pair l1, Lf_pair l2) const {
   if (l1.tleader == l2.tleader)
    return (l1.tfollower < l2.tfollower);
   return l1.tleader < l2.tleader;
};
  
 
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
  std::cout<<"Box "<<name<<" "<<activity.size()<<" Occupation Date "<<to_simple_string(occupiedWhen)<<std::endl;
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
      if (programmed_bats[j] == bat_id || programmed_bats[j] == "all") {
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


void Box::print_detailed_lfo(ostream* os) {  
  for (map<Lf_pair, pair<unsigned, LF_FLAG>,lfcomp >::iterator i=lf_events.begin(); i!=lf_events.end(); i++) {
    Lf_pair lf = i->first;
    Bat *l = lf.leader;
    Bat *f = lf.follower;
    (*os)<<name<<"\t"<<l->hexid<<"\t"<<f->hexid<<"\t";
    (*os)<<to_iso_string(lf.tleader)<<"\t";
    (*os)<<(std::find(l->occuppied_boxes.begin(),l->occuppied_boxes.end(),name) != l->occuppied_boxes.end() ? 1:0)<<"\t";
    (*os)<<(std::find(f->occuppied_boxes.begin(),f->occuppied_boxes.end(),name) != f->occuppied_boxes.end() ? 1:0)<<"\t";        
    (*os)<<(l->disturbed_in_box[name].custom_boolean == TRUE ? 1:0)<<"\t";
    (*os)<<(f->disturbed_in_box[name].custom_boolean == TRUE ? 1:0)<<"\t";
    (*os)<<to_iso_string(occupiedWhen)<<endl;    
  }  
}


sequences Box::print_longest_sequence() {
  //first sort the lfevents map according to time of the leader
  map<Lf_pair, pair<unsigned, LF_FLAG>, sortby_tleader > temp;  
  for (map<Lf_pair, pair<unsigned, LF_FLAG>,lfcomp >::iterator i=lf_events.begin(); i!=lf_events.end(); i++) {
    temp[i->first] = i->second;
  }
  //now it's sorted
  //the data structure we need
  //very unlikeley we need more than 40
  vector< vector<string> > data_structure(40,vector<string>());
  vector<int> frequencies(40,0);
  int start = 1; int largest=1; 
  for (map<Lf_pair, pair<unsigned, LF_FLAG>,sortby_tleader >::iterator i=temp.begin(); i!=temp.end(); i++) {
    Lf_pair lfp = i->first;
    //lfp.print(&cout);
    //every lf_event is a 1-length sequence
    frequencies[1]++;
    string leader = lfp.getLeaderId();
    string follower = lfp.getFollowerId();
    vector<string>::iterator itr;
    bool found=false;
    start = largest;
    while (start >= 1) {
      itr = find(data_structure[start].begin(),data_structure[start].end(),leader);
      if (itr != data_structure[start].end() ) {// found
	start++;
	frequencies[start]++;
	if (start > largest) 
	  largest=start;	
	data_structure[start].push_back(follower);
	found=true;
	break;
      }
      start--; //not found
    }   
    if (!found) {
      data_structure[start++].push_back(leader);    
      data_structure[start].push_back(follower);
    }
  } 
  sequences new_seq;
  new_seq.longest = largest;
  new_seq.freqs = frequencies;
  return (new_seq);
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

void Lf_pair::print(ostream *out) {
  if (!out->good()) {
   std::cerr<<"Cannot print to output file: bad file descriptor"<<std::endl;
  }
  else {
    time_duration t = get_lf_delta();
    (*out)<<leader->hexid<<" "<<follower->hexid<<" "<<to_iso_string(tleader);
    (*out)<<" "<<to_iso_string(tfollower)<<" "<<box_name<<" "<<leader->box_knowledge[box_name].box_knowledge_how;
    (*out)<<" "<<leader->box_knowledge[box_name].box_knowledge;
    (*out)<<" "<<t.hours()*60.0+t.minutes()+t.seconds()/60.0<<std::endl;
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
/*leader_or_follower:
   TRUE - as leader
   FALSE - as follower*/
bool Bat::insert_pair(Lf_pair lfp, bool leader_or_follower) {
  bool exists=false;
  vector<Lf_pair> *where = (leader_or_follower ? &my_lfpairs:&my_lfpairs_as_follower);
  
  for (unsigned k=0; k<where->size(); k++) {
    if ((*where)[k].equals(lfp)) {
      if (lfp.get_lf_delta() < (*where)[k].get_lf_delta())
	(*where)[k]=lfp;
      exists=true;
      break;
    }
  }
  if (!exists) {
    (*where).push_back(lfp);
    return true;
  }
  return false;
}

/*gets the "status" of a bat for a given box boxname
 * Return Values:
  1 - the bat led and occupied this box
  2 - the bat led but did not occuppy this box
  3 - the bat did not lead and did not occupy this box
  4 - the bat did not lead and occupied the box
*/    
short Bat::get_lead_occuppied_status(string boxname) {
  bool occuppied=false, led=false;
  if (find(occuppied_boxes.begin(),occuppied_boxes.end(),boxname) != occuppied_boxes.end())
    occuppied=true;

  for (unsigned i=0; i<my_lfpairs.size(); i++) {
    //sanity check
    if (this->hexid != my_lfpairs[i].leader->hexid)
      cerr<<endl<<"Error in Bat::led_to_box() - bat id and leader id do not match."<<endl;
    if (my_lfpairs[i].box_name == boxname) {
      led=true;
      break;
    }      
  }
  if (led && occuppied)
    return 1;
  else if (led && !occuppied)
    return 2;
  else if (!led && !occuppied)
    return 3;
  else if (!led && occuppied)
    return 4;
  
  return -1;
}

/*gets the "status" of a bat for a given box boxname
 * Return Values:
  1 - the bat followed and occupied this box
  2 - the bat followed but did not occuppy this box
  3 - the bat did not follow and did not occupy this box
  4 - the bat did not follow and occupied the box
*/    
short Bat::get_followed_occuppied_status(string boxname) {
  bool occuppied=false, followed=false;
  if (find(occuppied_boxes.begin(),occuppied_boxes.end(),boxname) != occuppied_boxes.end())
    occuppied=true;

  for (unsigned i=0; i<my_lfpairs_as_follower.size(); i++) {
    //sanity check
    if (this->hexid != my_lfpairs_as_follower[i].follower->hexid)
      cerr<<endl<<"Error in Bat::followed_to_box() - bat id and leader id do not match."<<endl;
    if (my_lfpairs_as_follower[i].box_name == boxname) {
      followed=true;
      break;
    }      
  }
  if (followed && occuppied)
    return 1;
  else if (followed && !occuppied)
    return 2;
  else if (!followed && !occuppied)
    return 3;
  else if (!followed && occuppied)
    return 4;
  
  return -1;
}


void Bat::print_stats(ostream *os) {
  string current_box = "";
  //pair.first nfollowings-total numbers of followers you have led
  //pair.second nleadings - total numbers of leaders you have followed
  map<string, pair<myint,myint> > stats;    
  for (unsigned i=0; i<my_lfpairs.size(); i++)
    stats[my_lfpairs[i].box_name].first++;
  for (unsigned i=0; i<my_lfpairs_as_follower.size(); i++)
    stats[my_lfpairs_as_follower[i].box_name].second++;
  
  for (map<string,pair<myint,myint> >::iterator itr=stats.begin(); itr!=stats.end(); itr++) {
    *os<<hexid<<" "<<itr->first<<" "<<itr->second.first.i<<" "<<itr->second.second.i<<" ";
    *os<<disturbed_in_box[itr->first].custom_boolean<<" ";
    *os<<(std::find(occuppied_boxes.begin(),occuppied_boxes.end(),itr->first) != occuppied_boxes.end() ? 1 : 0)<<endl;
    
  }
  
  
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
  time_duration td = minutes(10); //10 minutes is hard-coded for now
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

int myigraph::calc_centrality(centrality_type *ct , igraph_vector_t* result, int which_graph) {
  if (ct->valid) {
    if (ct->type == 0)
      return get_indegrees(result,which_graph);
    else if (ct->type == 1)
      return eigenvector_centrality(result,which_graph);
    else if (ct->type == 2) 
      return get_second_indegree(result,which_graph, 0.5);
    else
      cerr<<"myigraph::calc_centrality() - Error: Unrecognised centrality type "<<ct->type<<endl;
    return 1;    
  }
  return 1;
}

/*
        igraph_matrix_t lf_adjmatrix; //square matrix
        igraph_matrix_init(&lf_adjmatrix,total_bats_in_lf_events,total_bats_in_lf_events); //init the square matrix
        igraph_matrix_null(&lf_adjmatrix);
	MATRIX(lf_adjmatrix,x,y) = 2.5
	
	
	
	
 */
myigraph::myigraph(igraph_matrix_t* adjmatrix) {
    long nrow = igraph_matrix_nrow(adjmatrix);
    long ncol = igraph_matrix_ncol(adjmatrix);
    //igraph_matrix_init(&weighted_adj_matrix,nrow,ncol);
    //igraph_matrix_null(&weighted_adj_matrix);
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
    rewired_graph = NULL;
    //finally create the graph
    igraph_adjlist(&graph,&adjlist,IGRAPH_OUT,false);  
}

/*calculate the total in-degree/out-degree/total degree*/
int myigraph::get_sum_degrees(igraph_t *g) {
  //get the adjacency matrix of the graph
  int result = 0;
  igraph_integer_t nvertices=0;
  igraph_vs_t vss = igraph_vss_all();
  igraph_vs_size(g,&vss,&nvertices);
  igraph_vs_destroy(&vss);
  igraph_matrix_t m;
  igraph_matrix_init(&m,nvertices,nvertices);
  igraph_matrix_null(&m);
  igraph_get_adjacency(g,&m,IGRAPH_GET_ADJACENCY_BOTH,false);
  for (unsigned r=0; r<nvertices; r++) 
    for (unsigned c=0; c<nvertices; c++) 
      result += MATRIX(m,r,c);
  
  return result;
  	
}

bool myigraph::is_connected(igraph_t *g) {
  igraph_integer_t nvertices=0;
  igraph_vs_t vss = igraph_vss_all();
  igraph_vs_size(g,&vss,&nvertices);
  igraph_vs_destroy(&vss);
  igraph_matrix_t m;
  igraph_matrix_init(&m,nvertices,nvertices);
  igraph_matrix_null(&m);
  igraph_get_adjacency(g,&m,IGRAPH_GET_ADJACENCY_BOTH,false); 
  igraph_vector_t vid_ptr_row, vid_ptr_col;
  igraph_vector_init(&vid_ptr_row,nvertices);
  igraph_vector_null(&vid_ptr_row);
  igraph_vector_init(&vid_ptr_col,nvertices);
  igraph_vector_null(&vid_ptr_col);
  for (unsigned v=0; v<nvertices; v++) { 
    igraph_matrix_get_row(&m,&vid_ptr_row,v);
    igraph_matrix_get_col(&m,&vid_ptr_col,v);
    if (igraph_vector_sum(&vid_ptr_row) == 0 && igraph_vector_sum(&vid_ptr_col) == 0) {
      igraph_matrix_destroy(&m);
      igraph_vector_destroy(&vid_ptr_row);
      igraph_vector_destroy(&vid_ptr_col);
      return false;
    }
  }
  igraph_matrix_destroy(&m);
  igraph_vector_destroy(&vid_ptr_row);
  igraph_vector_destroy(&vid_ptr_col);
  return true;  
}


int myigraph::get_indegrees(igraph_vector_t *result,int which_graph) {
  //igraph_vector_init(result,igraph_matrix_nrow(&weighted_adj_matrix));  
  //result must be initialized with all 0s
  //first check if graph is weakly connected
  if (!is_connected((which_graph==0 ? &graph : rewired_graph))) {
    if (which_graph == 0) {
      cerr<<"Error in myigraph::get_indegrees - Original graph is disconnected"<<endl;  
      //print_adjacency_matrix(0,&cout);
    }
    return 1;
  }  
  //now get the in-degree manually
  igraph_vector_fill(result,0);
  igraph_matrix_t m;
  igraph_matrix_init(&m,igraph_matrix_nrow(&weighted_adj_matrix),igraph_matrix_ncol(&weighted_adj_matrix));
  igraph_matrix_null(&m);
  igraph_get_adjacency((which_graph==0 ? &graph : rewired_graph),&m,IGRAPH_GET_ADJACENCY_BOTH,false);    
  
  for (unsigned column=0; column < igraph_matrix_ncol(&m); column++) 
    for (unsigned row=0; row < igraph_matrix_nrow(&m); row++) 
      VECTOR(*result)[column] +=MATRIX(m,row,column);    
    
  igraph_matrix_destroy(&m);  
  return 0;  
}

int myigraph::eigenvector_centrality(igraph_vector_t* result, int which_graph) {    
    igraph_vector_fill(result,0);  
    //which_graph: 0 for original graph, 1 for the rewired
    /*the idea is to create a new temporary graph from the adjacency matrix of this.graph,
     and calculate the eigenvector centralities from there*/
    //get the adjacency matrix
    //first check if graph is weakly connected
    if (!is_connected((which_graph==0 ? &graph : rewired_graph))) {
      if (which_graph == 0) {
	cerr<<"Error in myigraph::get_indegrees - Original graph is disconnected"<<endl;  
	//print_adjacency_matrix(0,&cout);
      }
      return 1;
    }   
    igraph_matrix_t m;
    igraph_matrix_init(&m,igraph_matrix_nrow(&weighted_adj_matrix),igraph_matrix_ncol(&weighted_adj_matrix));
    igraph_matrix_null(&m);
    igraph_get_adjacency((which_graph==0 ? &graph : rewired_graph),&m,IGRAPH_GET_ADJACENCY_BOTH,false);    
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
	long int row_idx = 0, col_idx = 0;
	while (row_idx == col_idx) {	
	  row_idx = igraph_rng_get_integer(&rng,0,n_vs-1);
	  col_idx = igraph_rng_get_integer(&rng,0,n_vs-1);
	  //cout<<row_idx<<"\t"<<col_idx<<endl;
	}
	MATRIX(rewired_adj_matrix,row_idx,col_idx)++;
	nedges--;
      }
    }
  }
  //if (rewired)
    // igraph_destroy(&rewired_graph);
  if (rewired_graph != NULL) {
    igraph_destroy(rewired_graph);
    delete rewired_graph;  
  }
  rewired_graph = new igraph_t;
  igraph_weighted_adjacency(rewired_graph,&rewired_adj_matrix,IGRAPH_ADJ_DIRECTED,"importance",true);
  igraph_rng_destroy(&rng);
  igraph_matrix_destroy(&rewired_adj_matrix);
  igraph_matrix_destroy(&original_adj_matrix);
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
	long int row_idx = 0, col_idx = 0;
	while (row_idx == col_idx) {
	  row_idx = igraph_rng_get_integer(&rng,0,n_vs-1); //follower
	  col_idx = sample_rnd(probs,&rng);//igraph_rng_get_integer(&rng,0,n_vs-1); //leader
	}
	MATRIX(rewired_adj_matrix,row_idx,col_idx)++;
	nedges--;
      }
    }
  }
  if (rewired_graph != NULL) {
    igraph_destroy(rewired_graph);
    delete rewired_graph;  
  }
  rewired_graph = new igraph_t;
  igraph_weighted_adjacency(rewired_graph,&rewired_adj_matrix,IGRAPH_ADJ_DIRECTED,"importance",true); 
  igraph_rng_destroy(&rng);
  igraph_matrix_destroy(&rewired_adj_matrix);
  igraph_matrix_destroy(&original_adj_matrix);
}

/*preserve out-degree distribution*/
void myigraph::rewire_edges3(unsigned long seed) {  
   //init the random number generators
   igraph_rng_t rng;
   //igraph_rngtype_mt19937 rng_type;
   igraph_rng_init(&rng,&igraph_rngtype_mt19937);
   igraph_rng_seed(&rng,seed);    
   /**/
   //get the adjacency list of the empirical graph
   igraph_adjlist_t adjlist_empirical;
   igraph_adjlist_init(&graph,&adjlist_empirical,IGRAPH_OUT);
   /**/
   long n_vs = igraph_adjlist_size(&adjlist_empirical);
   for (long vid=0; vid<n_vs; vid++) {
    //the neighbours of vid from the adjacency list
      igraph_vector_t *vid_ptr = igraph_adjlist_get(&adjlist_empirical,vid);
      for (long neighbours=0; neighbours<igraph_vector_size(vid_ptr); neighbours++) {
	long random_number = vid;
	while (random_number == vid)
	  random_number=igraph_rng_get_integer(&rng,0,n_vs-1);	
	VECTOR(*vid_ptr)[neighbours] = random_number;
      }      
   }
   /*now init the rewired graph*/   
   //if (rewired) 
     //igraph_destroy(&rewired_graph);
  if (rewired_graph != NULL) {
    igraph_destroy(rewired_graph);
    delete rewired_graph;  
  }  
  rewired_graph = new igraph_t;
  igraph_adjlist(rewired_graph,&adjlist_empirical,IGRAPH_OUT,false);  
  igraph_adjlist_destroy(&adjlist_empirical);  
  igraph_rng_destroy(&rng);
}

/*preserve in-degree distribution*/
void myigraph::rewire_edges4(unsigned long seed) {  
   //init the random number generators
   igraph_rng_t rng;
   //igraph_rngtype_mt19937 rng_type;
   igraph_rng_init(&rng,&igraph_rngtype_mt19937);
   igraph_rng_seed(&rng,seed);    
   /**/
   //get the adjacency list of the empirical graph
   igraph_adjlist_t adjlist_empirical;
   igraph_adjlist_init(&graph,&adjlist_empirical,IGRAPH_IN);
   /**/
   long n_vs = igraph_adjlist_size(&adjlist_empirical);
   for (long vid=0; vid<n_vs; vid++) {
    //the neighbours of vid from the adjacency list
      igraph_vector_t *vid_ptr = igraph_adjlist_get(&adjlist_empirical,vid);
      for (long neighbours=0; neighbours<igraph_vector_size(vid_ptr); neighbours++) {
	long random_number = vid;
	while (random_number == vid)
	  random_number=igraph_rng_get_integer(&rng,0,n_vs-1);
	VECTOR(*vid_ptr)[neighbours] = random_number;
      }      
   }
   /*now init the rewired graph*/   
  if (rewired_graph != NULL) {
    igraph_destroy(rewired_graph);
    delete rewired_graph;  
  }  
  rewired_graph = new igraph_t;
  igraph_adjlist(rewired_graph,&adjlist_empirical,IGRAPH_IN,false);  
  igraph_adjlist_destroy(&adjlist_empirical);  
  igraph_rng_destroy(&rng);
}

/*preserve out-degree distribution with weighted leaders proportionate to activity*/
void myigraph::rewire_edges5(std::vector< double > probs, unsigned long seed) {
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
   //if (rewired)
     //igraph_destroy(&rewired_graph);
  if (rewired_graph != NULL) {
    igraph_destroy(rewired_graph);
    delete rewired_graph;  
  }
  rewired_graph = new igraph_t;   
  igraph_adjlist(rewired_graph,&adjlist,IGRAPH_OUT,false);  
  igraph_rng_destroy(&rng);  
}

/*preserve in-degree and out-degree distribution*/
void myigraph::rewire_edges6(unsigned long seed) {
    bool repeat = true;
    //init the random number generators
    igraph_rng_t rng;
    //igraph_rngtype_mt19937 rng_type;
    igraph_rng_init(&rng,&igraph_rngtype_mt19937);
    igraph_rng_seed(&rng,seed);    
    //get the adjacency list of the empirical graph
    igraph_adjlist_t adjlist_empirical_outdegree, adjlist_empirical_indegree;
    igraph_adjlist_init(&graph,&adjlist_empirical_outdegree,IGRAPH_OUT);
    igraph_adjlist_init(&graph,&adjlist_empirical_indegree,IGRAPH_IN);
    /**/
    long n_vs = igraph_adjlist_size(&adjlist_empirical_outdegree);
    while (repeat) {
      /*auxillary structures*/
      map<long,int> in_degree, out_degree;
      for (long vid=0; vid<n_vs; vid++) {
	//the neighbours of vid from the adjacency list
	igraph_vector_t *vid_ptr = igraph_adjlist_get(&adjlist_empirical_outdegree,vid);
	igraph_vector_t *vid_ptr_indegree = igraph_adjlist_get(&adjlist_empirical_indegree,vid);
	if (igraph_vector_size(vid_ptr) != 0)
	  out_degree[vid] = igraph_vector_size(vid_ptr);
	if (igraph_vector_size(vid_ptr_indegree) != 0)
	  in_degree[vid] = igraph_vector_size(vid_ptr_indegree);
	/*
	for (long neighbours=0; neighbours<igraph_vector_size(vid_ptr); neighbours++) {	  
	  in_degree[VECTOR(*vid_ptr)[neighbours]]++;
	}
	*/
	
      }      
      /*delme*/
      /*
      cout<<"empirical indegrees"<<endl;
      for (map<long,int>::iterator bbb=in_degree.begin(); bbb!=in_degree.end(); bbb++) {
	cout<<bbb->first<<"\t"<<bbb->second<<endl;
      }
      cout<<"empirical outdegrees"<<endl;
      for (map<long,int>::iterator bbb=out_degree.begin(); bbb!=out_degree.end(); bbb++) {
	cout<<bbb->first<<"\t"<<bbb->second<<endl;
      } 
      */
      /*******/
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
	long tmp2 = itr_src->first; //this is the id of the selected vertex 
	map<long,int>::iterator itr_dst;
	//unsuccessful allocation -> start over
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
   if (rewired_graph != NULL) {
    igraph_destroy(rewired_graph);
    delete rewired_graph;  
   }
   rewired_graph = new igraph_t;
   igraph_copy(rewired_graph,&dummy_graph.graph);    
   igraph_rng_destroy(&rng);
   igraph_adjlist_destroy(&adjlist_empirical_outdegree);
   igraph_adjlist_destroy(&adjlist_empirical_indegree);
   //this->print_adjacency_list(1,&cout);
  }

void myigraph::rewire_random_model(short int model, vector<double> *probs) {
  if (model == 1) 
    rewire_edges(rand());  
  else if (model == 2)    
    rewire_edges2(*probs,rand());  
  else if (model == 3)
    rewire_edges3(rand());
  else if (model == 4) 
    rewire_edges4(rand());
  else if (model == 5)     
    rewire_edges5(*probs,rand());
  else if (model == 6) 
    rewire_edges6(rand());
  else {
    cerr<<"Model "<<model<<" is not implemented"<<endl;
    exit(1);
  }
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

/*calculate alpha centrality*/
/*
int myigraph::alpha_centrality(boost::numeric::ublas::vector<double > &result,boost::numeric::ublas::vector<double > *e,double alpha, int which_graph) {
  bool normalize_centralities=false;
  bool clean = true;
  igraph_t *g_ptr = (which_graph == 0 ? &graph : rewired_graph);
  int vc = igraph_vcount(g_ptr);
  //get the adjacenty matrix
  igraph_matrix_t a;
  igraph_matrix_init(&a,vc,vc);
  igraph_get_adjacency(g_ptr,&a,IGRAPH_GET_ADJACENCY_BOTH,false);
  //cout<<"ORIGINAL MATRIX"<<endl;
  //for (unsigned i=0; i<igraph_vcount(g_ptr); i++) {
    //for (unsigned j=0; j<igraph_vcount(g_ptr); j++)
      //cout<<MATRIX(a,i,j)<<",";    
  //}  
  igraph_matrix_transpose(&a);
  //cout<<"TRANSPOSED MATRIX"<<endl;
  //for (unsigned i=0; i<igraph_vcount(g_ptr); i++) {
    //for (unsigned j=0; j<igraph_vcount(g_ptr); j++)
      //cout<<MATRIX(a,i,j)<<" ";
    //cout<<endl;
  //}
  //get the largest eigenvalue
  double lev = -10, tolerance = pow(10,-5);
  unsigned lev_index = 0;
  int info=0; //gives only warning if QR decomposition failes
  igraph_vector_t values_real, values_imag;
  igraph_vector_init(&values_real, 0);
  igraph_vector_init(&values_imag, 0);
  
  igraph_lapack_dgeev(&a,&values_real,&values_imag,NULL,NULL,&info);
  if (info) {
    cerr<<"mygraph::alpha_centrality() - Warning: LAPACK dgeev() with non-zero exit status";
    return (info);
  }  
  //get the index and value of max eigenvalue
  for (unsigned i=0; i<igraph_vector_size(&values_real); i++) {
    if (VECTOR(values_real)[i] > lev) {
      lev = VECTOR(values_real)[i];
      lev_index = i;
    }    
  }
  if (VECTOR(values_imag)[lev_index]) {
    cerr<<"Largest eigenvalue has non-zero imaginary part"<<endl;
    return 1;
  }
  //cout<<lev<<endl;
  if (lev <= tolerance) {
    lev = 0.0;
    igraph_matrix_scale(&a,alpha);
  }
  else 
    igraph_matrix_scale(&a,(alpha < 1.0/lev ? alpha : 1.0/lev-tolerance)); 
  //transform A into boost matrix
  boost::numeric::ublas::matrix<double> A(vc,vc), A_inverse(vc,vc);
  for (unsigned i=0; i<vc; i++) 
    for (unsigned j=0; j<vc; j++) 
      A(i,j) = (i==j ? 1.0 : (-1.0)*MATRIX(a,i,j));     
  //bool invert_flag = InvertMatrix(A,A_inverse); //returns true on success
  bool singular;
  InverseMatrix2<double>(A,A_inverse,singular); //returns true on success
  if (singular) {
    cerr<<"mygraph::alpha_centrality() - Warning: Matrix not invertible"<<endl;
    return 1;    
  }
  if (!e) {
    e = new boost::numeric::ublas::vector<double>(boost::numeric::ublas::scalar_vector<double>(vc,1.0));
    clean = true;
  } 
  //now do the final multiplication
  result = boost::numeric::ublas::prod(A_inverse,*e);  
  //normalize
  if (normalize_centralities) {
    double sum = 0.0;
    for (unsigned i=0; i<result.size(); i++) 
      sum += pow(result[i],2); 
  
    //check if all centralities are 0
    if (sum <= tolerance) {
      cerr<<"mygraph::alpha_centrality() - Warning: All centralities are 0"<<endl;
      return 1;
    }  
    sum = pow(sum,0.5);
    for (unsigned i=0; i<result.size(); i++)  
      result[i] /= sum;  
    }
    
  if (clean)
    delete e;
  
  igraph_matrix_destroy(&a);
  igraph_vector_destroy(&values_imag);
  igraph_vector_destroy(&values_real);
  return 0;
}
*/

int myigraph::get_second_indegree(igraph_vector_t* result, int which_graph, double alpha) {
  igraph_t *g = (which_graph == 0 ? &graph : rewired_graph);
  unsigned vc = igraph_vcount(g);
  /*get the weighted in-degree of all vertices*/
  igraph_vector_t indegree_vector;
  igraph_vector_init(&indegree_vector,vc);
  get_indegrees(&indegree_vector,which_graph);
  
  for (unsigned i=0; i<vc; i++) {     
    double second_degree_sum =0.0;
    igraph_vs_t vertex_selector;
    igraph_vit_t vertex_iterator;
    igraph_vs_adj(&vertex_selector,i,IGRAPH_IN); //get all in-neighbours of vertex i   
    igraph_vit_create(g,vertex_selector,&vertex_iterator);
    while (!IGRAPH_VIT_END(vertex_iterator)) {
      //now get the indegree of this first-degree neighbour
      second_degree_sum += (double) VECTOR(indegree_vector)[IGRAPH_VIT_GET(vertex_iterator)];
      IGRAPH_VIT_NEXT(vertex_iterator);
    }
    VECTOR(*result)[i] = VECTOR(indegree_vector)[i] + alpha * second_degree_sum;    
    igraph_vit_destroy(&vertex_iterator);
    igraph_vs_destroy(&vertex_selector);   
  }  
  igraph_vector_destroy(&indegree_vector);
  return 0;
}


void myigraph::calc_vertex_summary(int which_graph, map<unsigned,pair<int,int> > *result) {
 igraph_t *g = (which_graph == 0 ? &graph: rewired_graph);
 //get the adjacency matrix
 igraph_matrix_t adj;
 igraph_matrix_init(&adj,igraph_vcount(g),igraph_vcount(g));
 igraph_get_adjacency(g,&adj,IGRAPH_GET_ADJACENCY_BOTH,0); 
 for (unsigned i=0; i<igraph_vcount(g); i++) {
    //get the ith row of the matrix
    igraph_vector_t row,col;    
    igraph_vector_init(&row,igraph_vcount(g));
    igraph_vector_init(&col,igraph_vcount(g));
    igraph_matrix_get_row(&adj,&row,i);
    igraph_matrix_get_col(&adj,&col,i);
    pair<int,int> i_summary(igraph_vector_sum(&col),igraph_vector_sum(&row));
    (*result)[i] = i_summary;
    igraph_vector_destroy(&row);
    igraph_vector_destroy(&col);
 }
 igraph_matrix_destroy(&adj);
}

/*check if giant connected component can emerge*/
double myigraph::calc_gcc(map<unsigned, pair<int,int> > *m) {
  struct pair_compare {
    bool operator()(pair<int,int> p1, pair<int,int> p2) {
	return (p1.first==p2.first && p1.second==p2.second);
    }
  };
  //pair<int,int> -> count
  map<pair<int,int>, int > m2;
  
  for (map<unsigned, pair<int,int> >::iterator itr=m->begin(); itr!=m->end(); itr++) {
    if (m2.find(itr->second) == m2.end())
      m2[itr->second] = 1;
    else
      m2[itr->second]++;
  }  
  double gcc = 0.0;
  for (map<pair<int,int>, int >::iterator itr=m2.begin(); itr!=m2.end(); itr++) {
    int j=itr->first.first, k = itr->first.second;
    double pjk = (double) itr->second / (double) m->size();
    gcc += (2.0*j*k-j-k)*pjk;
  }
  return gcc;  
  
}

void myigraph::print_adjacency_list(int which_graph,igraph_neimode_t mode /*IGRAPH_OUT or IGRAPH_IN*/,ostream *out) {
    //0 for original graph, 1 for the rewired   
    igraph_adjlist_t graph_adjlist;
    igraph_adjlist_init((which_graph == 0 ? &graph : rewired_graph),&graph_adjlist,mode);
    
    for (long vid=0; vid < igraph_adjlist_size(&graph_adjlist); vid++) {
      *out<<vid<<"->";
      igraph_vector_t *vid_ptr = igraph_adjlist_get(&graph_adjlist,vid);
      for (long vid_neighbour=0; vid_neighbour<igraph_vector_size(vid_ptr); vid_neighbour++)
	  *out<<VECTOR(*vid_ptr)[vid_neighbour]<<",";
      *out<<endl;      
    }  
    igraph_adjlist_destroy(&graph_adjlist);
}



void myigraph::print_adjacency_matrix(int which_graph, ostream *out) {
    //0 for original graph, 1 for the rewired
    igraph_matrix_t m;
    igraph_matrix_init(&m,igraph_matrix_nrow(&weighted_adj_matrix),igraph_matrix_ncol(&weighted_adj_matrix));
    igraph_matrix_null(&m);
    igraph_get_adjacency((which_graph==0 ? &graph: rewired_graph),&m,IGRAPH_GET_ADJACENCY_BOTH,false);        
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
    if (rewired_graph != NULL) {
      igraph_destroy(rewired_graph);
      delete rewired_graph;
    }
    igraph_adjlist_destroy(&adjlist);
    igraph_matrix_destroy(&weighted_adj_matrix);    
  
}


/* ==================================================================== */

#ifndef _CLASSES_H
#define _CLASSES_H

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/greg_month.hpp>
#include <boost/date_time/gregorian/formatters.hpp>
#include <boost/numeric/ublas/fwd.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/io.hpp>

#include "/usr/local/include/igraph/igraph.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <cctype>
#include <algorithm>


using namespace boost;
using namespace boost::posix_time;
using namespace boost::gregorian;
using namespace std;

class Bat;
class Box;

/* ======================== CLASS DEFINITIONS ========================== */

/*just a wrapper for an int to make sure than an int is initialized to 0*/
class myint {
public:
  int i;
  myint(const myint&other) {
    this->i = other.i;
  }
  myint() {i=0;}
  myint(int j) {i=j;}
  myint& operator++() {
    i++;
    return *this;
  }
  myint& operator++(int) {
    i++;
    return *this;
  }
  myint& operator+=(const myint &rhs) {
    i += rhs.i;
    return *this;
  }  
};
inline bool operator==(const myint& lhs, const myint& rhs){ 
    return lhs.i < rhs.i;
}
inline bool operator!=(const myint& lhs, const myint& rhs){ 
    return lhs.i != rhs.i;
}
inline bool operator<(const myint& lhs, const myint& rhs){ 
    return lhs.i < rhs.i;
}
inline bool operator>(const myint& lhs, const myint& rhs){ 
    return lhs.i > rhs.i;
}
inline bool operator<=(const myint& lhs, const myint& rhs){ 
    return lhs.i <= rhs.i;
}
inline bool operator>=(const myint& lhs, const myint& rhs){ 
    return lhs.i >= rhs.i;
}

enum BoxStatus {DISCOVERED, UNDISCOVERED};

class BatEntry {
public:
    ptime TimeOfEntry; //the time this bat was recorded
    string hexid; //the hexid of the recorded bat
    short occurence; //number of occurences of the bat hexid
    string box_name; //in which box was the bat recorded
    BatEntry(string entry_time, string hex, string name);
    void print(ostream *out) const;
};

/*compare two bat entries based on temporal ordering
  two batentries b1 and b2 are equivalent if
  fun(b1,b2) == false && fun(b2,b1) == false*/
struct batEntryCompare {
    bool operator()(BatEntry b1, BatEntry b2) const {
        if (b1.TimeOfEntry == b2.TimeOfEntry) {
            if (b1.hexid == b2.hexid) //same bat?
                return false; //returns always FALSE, so if the same bat was recorded at the same time in a given box
            //it will not be inserted
            else
                return b1.hexid < b2.hexid; //returns true if two different bats were recorded at the same time, so b1 comes first.
        }
        return (b1.TimeOfEntry < b2.TimeOfEntry);
    }
};


/*like batEntryCompare but first orders by box
 *  and then by time and hexid*/
struct batEntryCompare2 {
    bool operator()(BatEntry b1, BatEntry b2) const {
        if (b1.box_name == b2.box_name) {
            if (b1.TimeOfEntry == b2.TimeOfEntry) {
                if (b1.hexid == b2.hexid) //same bat?
                    return false; //returns always FALSE, so if the same bat was recorded at the same time in a given box
                //it will not be inserted
                else
                    return b1.hexid < b2.hexid; //returns true if two different bats were recorded at the same time, so b1 comes first.
            }
            return (b1.TimeOfEntry < b2.TimeOfEntry);
        }
        else
            return b1.box_name < b2.box_name;
    }
};

/* UNINIT			=	0
 * PERSONAL_UNDISTURBED		=	1
 * PERSONAL_DISTURBED		=	2
 * SOCIAL_UNDISTURBED		=	3
 * SOCIAL_DISTURBED		=	4
 * */
enum lf_flag_types {UNINIT,PERSONAL_UNDISTURBED, PERSONAL_DISTURBED, SOCIAL_UNDISTURBED,SOCIAL_DISTURBED};
class LF_FLAG {
public:
    lf_flag_types this_lf_flag;
    LF_FLAG();
    LF_FLAG(lf_flag_types other);
};
  


class Lf_pair {
public:
    Bat *leader;
    Bat *follower;
    string box_name;
    ptime tleader;
    ptime tfollower;
    bool leader_disturbed;
    bool valid; //valid only if abs(tleader-tfollower) <= "3-minute" rule. true by default
    //is this lf pair a passive leading following event?
    bool is_passive_leading;
    Lf_pair();     
    Lf_pair(Bat *B1, Bat *B2); 
    Lf_pair(Bat *B1, Bat *B2, ptime tB1, ptime tB2,string bname);    
    Lf_pair(Bat *B1, Bat *B2, ptime tB1, ptime tB2,string bname, bool v);
    bool equals(Lf_pair &other);    
    void init(Bat *B1, Bat *B2, ptime tB1, ptime tB2,string bname);     
    void print(ofstream *out);
    time_duration get_lf_delta();
    void validate(time_duration limit);
    string getLeaderId();    
    string getFollowerId();
};

bool Lf_pair_compare(Lf_pair lf1, Lf_pair lf2);
/*just like the normal boolean datatype, but this ensures that a bool is always initted to a token value*/
enum MYBOOLEAN {UNINITIALIZED=2, TRUE=1,FALSE=0};
class mybool {
public:
  MYBOOLEAN custom_boolean;
  mybool() ;
  mybool(MYBOOLEAN b);
};

class event {
public:
  string eventname;//one of DISCOVERY,EXPLORATION,REVISIT,FOLLOW
  Box *box;
  Bat *bat;
  Bat *aux_bat; //used only if this is an lf event to store a pointer to the LEADER
  ptime eventtime;
  bool valid;
  event(string, Bat *,Box *, ptime, Bat * = NULL);
  event();
  void print(ofstream *);
};

class Box {
private:
  struct lfcomp {
    bool operator()(Lf_pair l1, Lf_pair l2) const;
  };  
public:   
    short type; //0-control, 1-minority, 2-majority, 3-all, 4 - heat box
    string name;
    set<BatEntry,batEntryCompare> activity;
    BoxStatus status; //has this box been discovered or not
    pair<string,ptime> discoveredBy; //who discovered the box and when
    ptime occupiedWhen; //when was the box occupied. must be pre-defined
    ptime installedWhen; //when was the box installed
    /*how the knowledge about this box has spread through the colony
      "knowledge" is each discovery, exploration or following*/
    vector<event> information_spread;
    /*sanity check: social_lf_events+personal_lf_events=total_lf_events*/
    unsigned total_lf_events; //total # of lf events to this box
    unsigned social_ud_lf_events; //total # of social undisturbed lf events
    unsigned social_d_lf_events; //total # of social disturbed lf events
    unsigned personal_ud_lf_events; //total # of personal undisturbed lf events 
    unsigned personal_d_lf_events; //total # of personal disturbed lf events 
    set<string> leaders; //all bats who led to this box
    set<string> followers; //all bats who followed to this box
    map<Lf_pair, pair<unsigned,LF_FLAG>,lfcomp> lf_events; //bat_id -> <#lf events,status>
    vector<event> revisiting_bats;
    vector<Bat *> occupyingBats; //pointers to the bats that occupied that box
    vector<pair<ptime, vector<Bat*> > > occupationHistory; //date and bats for all occupation of this box 
    unsigned get_num_occ_bats(); //get the number of occupying bats
    void discovered(string bat_id,ptime when);
    void sort_information_spread();
    /*Makes sure that only a bat follows only once to this box
      typically a bat can follow more than once, if she has participated
      in an lf event with more than 1 leader. Currently, each of the leaders
      will get a separate lf event withi the same follower*/
    void clean_information_spread();
    /*
    returns how many bats who lead or followed (determined by flag)
    occupied the box at the first night of occupation
    also computes how many bats occuppied the box but
    did not take part in an lf event
    prints out a vector {a,b,c,d}, where
    a = #leaders who occupied the box
    b = #followers who occupied the box
    c = #naiive occupators who did not participate in a lf event
    d = #experienced occupators who did not participate in a lf event
    */
    string howmany_leaders_followers();
    //all activities after this date are ignored!
    //if not occupied this time is set to +inf.
    time_duration getOccupiedDiscoveredDelta();
    Box(short Type, string Name, ptime occ, ptime installation);
    Box();
    void print();
    void print_detailed_occupation();
};





//the knowledge of bats per box
enum BatKnowledgeEnum {NAIIVE, EXPERIENCED}; 
enum BatKnowledgeHow {UNDEFINED,SOCIAL,PERSONAL};
class BatKnowledge {
public:
  BatKnowledgeEnum box_knowledge;
  BatKnowledgeHow box_knowledge_how;
  BatKnowledge();
  BatKnowledge(BatKnowledgeEnum b1, BatKnowledgeHow b2);
};


class Bat {
private:
    int internal_check; //safely ignore, used in make_informed for sanity checks
    
    struct movementCompare {
        bool operator() (pair<ptime,Box*> m1, pair<ptime,Box*> m2) {
            //the movement entries are identical if they are for the same box
            //and at the exact same time. Should never happen
            if (m1.second->name == m2.second->name && m1.first == m2.first)
                return false;
            if (m1.second->name == m2.second->name)
                return m1.first<m2.first;

            return m1.second->name < m2.second->name || m1.first<m2.first;
        }
    };
public:
    bool part_of_lf_event; //has this bat taken part in an lf-event? either as a leader
    //or as a follower
    //the cumulative relatedness between all individuals following this bat over time
    double cumulative_relatedness;
    //aggregately store how many daughters have followed this bat so far
    double n_daughters_following;
    //the total number of bats that have followed this one over time
    double total_following;
    string hexid;
    set<pair<ptime, Box*>,movementCompare> movement_history;//time of recording at each box
    //"clean" movement_history by ensuring that no two consequtive events are less than 10 minutes apart
    vector<pair<ptime, Box*> > cleaned_movement_history;
    map<string,BatKnowledge> box_knowledge;//box_name -> knowledge. what bats know about each box
    map<string,mybool> disturbed_in_box; //box_name->0/1. 0 if bat has not been disturbed in that box
				       //1 - otherwise
    //store the time since a bat became informed of a given box
    map<string,ptime> informed_since;
    //the first reading of a bat in a given box
    map<string,ptime> first_reading; 
    vector<string> daughters_hexids;
    vector<Lf_pair> my_lfpairs;
    vector<string> occuppied_boxes; //which boxes have been occupied. takes the name of the box
    vector<event> my_revisits;
    /*last_revisit: misc variable used when identifying the revisits
     box_name -> last_revisit*/
    map<string,ptime> last_revisit; //default constructor is neg_infin
    //amother misc variable, used when identifying the revisits
    ptime last_seen; //default constructor is not_a_date_time
    bool insert_pair(Lf_pair lfp);    //true if pair is inserted, false otherwise
    //bool led_to_box(string boxname); //did this bat ever lead to box boxname
    /*gets the "status" of a bat for a given box boxname
     * Return Values:
     1 - the bat led and occupied this box
     2 - the bat led but did not occuppy this box
     3 - the bat did not lead and did not occupy this box
     4 - the bat did not lead and occupied the box
     -1 - something got fucked
     */    
    short get_lead_occuppied_status(string boxname);
    void add_movement(ptime Time, Box * box_ptr);
    //make the bat informed
    void make_informed(string box_name,ptime informed_time);
    //is the bat informed about a particular box at a given time
    bool is_informed(string box_name, ptime when);
    pair<ptime, Box*> get_movement_history (int idx);
    Bat(string Id);
    Bat();
    void print();
    inline bool operator==(const Bat &other) {
        return (this->hexid == other.hexid);
    }
    void clean_movement_history();
    
};


class assortativity_map {
private:
    map<unsigned, vector<unsigned> > m;
public: 
  assortativity_map();
  void print_all(ofstream *out);  
  void print_average(ofstream *out);
  
  int avg_neighbour_connectivity(igraph_vector_t *indegrees,
			         igraph_t *graph,
			         unsigned nnodes) ;
};



class myigraph {
public:
  igraph_t graph;
  igraph_t *rewired_graph;
  igraph_adjlist_t adjlist; //for the original graph and rewired in models 3,4
  igraph_matrix_t weighted_adj_matrix; //for the original graph
  myigraph(igraph_matrix_t *adjmatrix);
  int eigenvector_centrality(igraph_vector_t *result,int which_graph);
  int get_indegrees(igraph_vector_t *result,int which_graph);
  bool is_connected(igraph_t *g);
  int get_sum_degrees(igraph_t *g);
  void rewire_edges(unsigned long seed);
  void rewire_edges2(vector<double> probs,unsigned long seed);
  void rewire_edges3(unsigned long seed);
  void rewire_edges4(unsigned long seed);
  void rewire_edges5(vector<double> probs,unsigned long seed);
  void rewire_edges6(unsigned long seed);
  //this is just a dispatch method, which calls the right rewiwiring procedure
  void rewire_random_model(short model,vector<double> *probs);
  void print_adjacency_matrix(int which_graph, ostream *out);
  void print_adjacency_list(int which_graph,igraph_neimode_t mode /*IGRAPH_OUT or IGRAPH_IN*/,ostream *out);
  long sample_rnd(vector<double> probs, igraph_rng_t *rnd);
  int alpha_centrality(boost::numeric::ublas::vector<double > &result,boost::numeric::ublas::vector<double> *e,double alpha, int which_graph);  
  ~myigraph();
  
};

#endif
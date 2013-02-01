#ifndef _CLASSES_H
#define _CLASSES_H

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/greg_month.hpp>
#include <boost/date_time/gregorian/formatters.hpp>
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
/* ======================== CLASS DEFINITIONS ========================== */

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
  

class Box {
public:
    short type; //0-control, 1-minority, 2-majority, 3-all
    string name;
    set<BatEntry,batEntryCompare> activity;
    BoxStatus status; //has this box been discovered or not
    pair<string,ptime> discoveredBy; //who discovered the box and when
    ptime occupiedWhen; //when was the box occupied. must be pre-defined
    /*sanity check: social_lf_events+personal_lf_events=total_lf_events*/
    unsigned total_lf_events; //total # of lf events to this box
    unsigned social_ud_lf_events; //total # of social undisturbed lf events
    unsigned social_d_lf_events; //total # of social disturbed lf events
    unsigned personal_ud_lf_events; //total # of personal undisturbed lf events 
    unsigned personal_d_lf_events; //total # of personal disturbed lf events 
    map<string, pair<unsigned,LF_FLAG> > lf_events; //bat_id -> <#lf events,status>
    void discovered(string bat_id,ptime when);
    time_duration getOccupiedDiscoveredDelta();
    //all activities after this date are ignored!
    //if not occupied this time is set to +inf.
    Box(short Type, string Name, ptime occ);
    Box();
    void print();
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
    /*used to reset the roundtrip if the bat is spotted before roundtrip_time expires */
    time_duration roundtrip_timeout;
    
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
    map<string,BatKnowledge> box_knowledge;//box_name -> knowledge. what bats know about each box
    map<string,mybool> disturbed_in_box; //box_name->0/1. 0 if bat has not been disturbed in that box
				       //1 - otherwise
    //store the time since a bat became informed of a given box
    map<string,ptime> informed_since;
    vector<string> daughters_hexids;
    vector<Lf_pair> my_lfpairs;
    bool insert_pair(Lf_pair lfp);    //true if pair is inserted, false otherwise
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

#endif
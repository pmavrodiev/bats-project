#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "standard.yy.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/greg_month.hpp>
#include <boost/date_time/gregorian/formatters.hpp>
#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>
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
#include <stdlib.h>
#include <sqlite3.h>

using namespace std;
using namespace boost;
using namespace boost::posix_time;
using namespace boost::gregorian;

/*TODO - read the number of the reading device in each raw file and if it does not correspond
	 to the reading device in that box, disregard the readings. For now, we have to manually
	 check whether the reading device is the right one for the box. The problem is that
	 with the ascii format.*/

/* ======================== CLASS DECLARATIONS ========================= */
class Box; //represents a bat
class Bat; //represents a box
class BatEntry; //represents an entry of a bat into a box
class Box;
class Event;
/* ===================================================================== */

/* ======================== CLASS DEFINITIONS ========================== */
enum BatKnowledge {NAIIVE, EXPERIENCED}; //the knowledge of bats per box
enum BoxStatus {DISCOVERED, UNDISCOVERED};

class BatEntry {
public:
  ptime TimeOfEntry;
  string hexid; //the hexid of the recorded bat
  short occurence; //number of occurences of the bat hexid
  string box_name; //in which box was the bat recorded
  BatEntry(string entry_time, string hex, string name) {
    TimeOfEntry = ptime(from_iso_string(entry_time));
    hexid = hex;
    occurence = 1;
    box_name = name;
  }
  void print() const {
    cout<<to_simple_string(TimeOfEntry)<<" "<<hexid<<" "<<occurence<<" "<<box_name<<endl;
  }
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


class Box {
public:
  short type; //0-control, 1-minority, 2-majority, 3-all
  string name;
  set<BatEntry,batEntryCompare> activity;
  vector<Event> events; //all events that happened in this box
  BoxStatus status; //has this box been discovered or not
  Box(short Type, string Name) {
    type = Type;
    name = Name;
    this->status = UNDISCOVERED;
  }
  Box() {status = UNDISCOVERED;}
  void print() {
    set<BatEntry,batEntryCompare>::iterator i;
    cout<<"Box "<<name<<" "<<activity.size()<<endl;
    for (i=activity.begin(); i != activity.end(); i++) {
      i->print();
    }
  }
};
class Bat {
private:
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
  //the cumulative relatedness between all individuals following this bat over time
  double cumulative_relatedness;
  //aggregately store how many daughters have followed this bat so far
  double n_daughters_following;
  //the total number of bats that have followed this one over time
  double total_following;
  string hexid;  
  set<pair<ptime, Box*>,movementCompare> movement_history;//time of recording at each box
  map<string,BatKnowledge> box_knowledge;//box_name -> knowledge. what bats know about each box
  vector<string> daughters_hexids;
  void add_movement(ptime Time, Box * box_ptr) {
    pair<ptime,Box*> entry(Time,box_ptr);
    pair<set<pair<ptime, Box*>,movementCompare>::iterator,bool> insert_itr;
    insert_itr = movement_history.insert(entry);
    if (insert_itr.second == false) 
      cout<<"Warning: Duplicate movement entry for bat "<<hexid<<endl;
  }
  
  pair<ptime, Box*> get_movement_history (int idx) {
    int counter = 0;
    set<pair<ptime, Box*>,movementCompare>::iterator i;
    i=movement_history.begin();
    while (counter < idx) {
      i++;
      counter++;
    }
    return *i;    
  }
  
  Bat(string Id) {
    hexid = Id;n_daughters_following=0.0;total_following=0.0;cumulative_relatedness=0.0;
  }
  Bat() {hexid = "";n_daughters_following=0.0;total_following=0.0;cumulative_relatedness=0.0;}
  void print() {
    cout<<"BAT "<<hexid<<endl;
    for (unsigned i=0; i<movement_history.size(); i++) {
      pair<ptime, Box*> m = get_movement_history(i);
      cout<<to_simple_string(m.first)<<" "<<m.second->name<<endl;
    }
  }
};

class Event {
public:
  vector<Bat*> focal_bat; //who were the bats that did the event. for a following event, the focal_bat is the follower.
			  //we may have more than one focal bats responsible for an event. for example when two bats lead 
			  //to a box. in this case we could also have more than one followers
  vector<Bat*> followers; //used only for a LeadFollow event
  ptime time;  //what time did the event occur			  
  Box* box; //in which box did the event happen
  string eventName; //Discovery, Exploration, Revisit or LeadFollow
  Event(vector<Bat*> bat,vector<Bat*> Followers ,ptime Time, Box* b,string event_name) {
    focal_bat=bat;time=Time;box=b;eventName=event_name;followers = Followers;    
  }
  Event(vector<Bat*> bat,ptime Time, Box* b,string event_name) {
    focal_bat=bat;time=Time;box=b;eventName=event_name;
  }
  Event() {box = NULL;}
  void print() {
    if (followers.size() == 0) { //Discovery, Exploration or Revisit    
      for (unsigned i=0; i<focal_bat.size();i++) 
	cout<<focal_bat[i]->hexid<<"\t";      
      cout<<":\t"<<eventName<<"\t"<<to_simple_string(time)<<"\t"<<box->name<<endl; 
    }
    else { //LeadFollow event
      cout<<"{";
      for (unsigned i=0; i<focal_bat.size();i++) {
	cout<<focal_bat[i]->hexid;
	if (i != focal_bat.size()-1) cout<<",";      
      }
      cout<<"} --> {";
      for (unsigned i=0; i<followers.size();i++) {
	cout<<followers[i]->hexid;
	if (i != followers.size()-1) cout<<",";      
      }
      cout<<"}\t:"<<eventName<<"\t"<<to_simple_string(time)<<"\t"<<box->name<<endl;
    }
  }
};

/* ==================================================================== */

/* ======================== GLOBAL DEFINITIONS ======================= */
/*the minimum amount of time between two consequtive recordings above which the recordings are considered disjoint, i.e. analysed
  separately*/
time_duration time_chunk = minutes(3);
//min. time interval for flying between two boxes
time_duration min_time_interval = minutes(0.1);

string bats[] = {
  "00069799E2",
  "0000641775",
  "00065DD1A0",
  "000617A16A",
  "000697B597",
  "00068E2F3D",
  "00068E1B66",
  "00061D4A31",
  "00068E1731",
  "00064407F9",
  "0006011890",
  "00064380ED",
  "000615D7EC",
  "0001E5B8AA",
  "00060D6C05",
  "000697D2F4", //the mother is 00060F5D64 but with a mismatch
  "000697D00F",
  "00065DED81",
  "000697A2BA", //mother is 0000005631 (extracted from poster) but not in the mother daughter files from Nico
  "0006440675",
  "00060F5D64",
  "0006979AC0",
  "0005FE0AF1",
  "0006102122",
  "0005FE29BB",
  "00061D3F49",
  "0000F85C0C",
  "00060F6D0D", //the mother is 0001F7D726 but with a mismatch
  "00065DB1F6",
  "0001F7D726",
  "000658E480",
  "0005FDFD3D",
  "00068E1AC4",
  "00065EA84E",
  "00065D814F",
  "000000586A", //no records for this bat in GB2 2008
  "00000066EE", //no records for this bat in GB2 2008
  "000000CECC", //no records for this bat in GB2 2008
  "000000A3C3", //no records for this bat in GB2 2008
  "000000BCFB", //no records for this bat in GB2 2008
  "000000C99A", //no records for this bat in GB2 2008
  "0000005631", //no records for this bat in GB2 2008
  "0000005574", //no records for this bat in GB2 2008
  "000000909C"  //no records for this bat in GB2 2008 
};
unsigned nbats =  sizeof(bats)/sizeof(string);

/* maps mothers to daughters */
bool multimapcompare (string lhs, string rhs) {return lhs<rhs;}
multimap<string,string,bool (*) (string,string)> mother_daughter(multimapcompare);
/*stores all mothers and all daughters separately
  of course a bat may appear in both vectors*/
vector<string> mothers, daughters;

/* read the relatedness file and store the relatedness data here.
 if the relatedness between A and B is x 
 the map will store AB=x, i.e. A and B concatenated*/
map<string,double> relatedness_map;

string transponders[] = {
  "00068E24FE", 
  "000697D078",
  "000000632D",
  "8E602D0D0F",
  "FFFFFFFFFF",
  "000615D8A9", //this is actually a male bat.
  "8EEEEEEEE8",
  "EEEEEEEEEE",
  "ECEEEEEEEE",
  "88E8E8E88E",
  "EEEEEEEEE6",
  "E8EEEEEEEE",
  "EEEEEE6EEE",
  "EEE0EEEEEE",
  "E0EEEEEEEE",
  "6EEEEEEE0E",
  "8EEEEEEEEE",
  "0EEEEEEEEE",
  "6EEEEEEEEE",
  "EEEEEEE8EE",
  "EEEE8EEEEE",
  "80EEEEEEEE",
  "EEEEEEEE0E"
};
unsigned ntransponders = sizeof(transponders)/sizeof(string);

/*TODO: remove bats_vector, use bats_map instead*/
vector<string> bats_vector (bats, bats + nbats);
vector<string> transponders_vector (transponders, transponders + ntransponders);

/* let's get fancy - init the maps with a function pointer */
bool strcomp (string lhs, string rhs) {return lhs<rhs;}
bool(*fn_pt)(string,string) = strcomp;
bool unsignedcomp (unsigned lhs, unsigned rhs) {return lhs<rhs;}
bool(*fn_pt2)(unsigned,unsigned) = unsignedcomp;

/*all bats in a map keyed by name and indexed from 0*/
map<string,unsigned,bool(*)(string,string)> bats_map(fn_pt);
/*stores a movement history of individual bats*/
map<string,Bat,bool(*)(string,string)> bats_records(fn_pt);
/*stores all boxes with all activity over time*/
map<string,Box,bool(*)(string,string)> boxes(fn_pt);
/*stores boxes names and associated numerical index*/
map<string,unsigned,bool(*)(string,string)> boxes_auxillary(fn_pt);
map<unsigned,string,bool(*)(unsigned,unsigned)> boxes_auxillary_reversed(fn_pt2);

/*store all events*/
struct eventCompare {
  bool operator()(Event e1, Event e2) const {
    return e1.time < e2.time;    
  }
};  

set<Event, eventCompare> all_events;

/*initialised by the flex scanner*/
vector < pair<string,string> >  box_entries;
/*auxillary*/
char *base_dir;
short counter = 0;
string version = "0.1";

/*colors for the nodes in the leading-following graph.
  used topo.colors from R to generate 128 colors */
double red[] = {
0.298039215686275,
0.270588235294118,
0.243137254901961,
0.215686274509804,
0.188235294117647,
0.16078431372549,
0.133333333333333,
0.105882352941176,
0.0784313725490196,
0.0470588235294118,
0.0196078431372549,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0.0235294117647059,
0.0509803921568627,
0.0823529411764706,
0.109803921568627,
0.137254901960784,
0.168627450980392,
0.196078431372549,
0.227450980392157,
0.254901960784314,
0.286274509803922,
0.313725490196078,
0.345098039215686,
0.372549019607843,
0.403921568627451,
0.431372549019608,
0.462745098039216,
0.490196078431373,
0.517647058823529,
0.549019607843137,
0.576470588235294,
0.607843137254902,
0.635294117647059,
0.666666666666667,
0.694117647058824,
0.725490196078431,
0.752941176470588,
0.784313725490196,
0.811764705882353,
0.843137254901961,
0.870588235294118,
0.901960784313726,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1  
};
double green[] = {
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0.00784313725490196,
0.0352941176470588,
0.0627450980392157,
0.0901960784313725,
0.117647058823529,
0.145098039215686,
0.172549019607843,
0.203921568627451,
0.231372549019608,
0.258823529411765,
0.286274509803922,
0.313725490196078,
0.341176470588235,
0.368627450980392,
0.396078431372549,
0.427450980392157,
0.454901960784314,
0.482352941176471,
0.509803921568627,
0.537254901960784,
0.564705882352941,
0.592156862745098,
0.619607843137255,
0.647058823529412,
0.67843137254902,
0.705882352941177,
0.733333333333333,
0.76078431372549,
0.788235294117647,
0.815686274509804,
0.843137254901961,
0.870588235294118,
0.898039215686275,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
0.992156862745098,
0.980392156862745,
0.972549019607843,
0.964705882352941,
0.956862745098039,
0.949019607843137,
0.941176470588235,
0.933333333333333,
0.925490196078431,
0.917647058823529,
0.913725490196078,
0.905882352941176,
0.901960784313726,
0.894117647058824,
0.890196078431372,
0.886274509803922,
0.882352941176471,
0.87843137254902,
0.874509803921569,
0.870588235294118,
0.866666666666667,
0.866666666666667,
0.862745098039216,
0.862745098039216,
0.858823529411765,
0.858823529411765,
0.858823529411765,
0.858823529411765,
0.858823529411765,
0.858823529411765,
0.858823529411765,
0.858823529411765,
0.858823529411765,
0.862745098039216,
0.862745098039216,
0.862745098039216,
0.866666666666667,
0.870588235294118,
0.874509803921569,
0.874509803921569,
0.87843137254902
};
double blue[] = {
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
0.301960784313725,
0.270588235294118,
0.243137254901961,
0.211764705882353,
0.184313725490196,
0.152941176470588,
0.125490196078431,
0.0941176470588235,
0.0666666666666667,
0.0352941176470588,
0.00784313725490196,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0.0156862745098039,
0.0352941176470588,
0.0509803921568627,
0.0666666666666667,
0.0862745098039216,
0.101960784313725,
0.117647058823529,
0.137254901960784,
0.152941176470588,
0.172549019607843,
0.188235294117647,
0.203921568627451,
0.223529411764706,
0.23921568627451,
0.254901960784314,
0.274509803921569,
0.290196078431373,
0.305882352941176,
0.325490196078431,
0.341176470588235,
0.356862745098039,
0.376470588235294,
0.392156862745098,
0.407843137254902,
0.427450980392157,
0.443137254901961,
0.462745098039216,
0.47843137254902,
0.494117647058824,
0.513725490196078,
0.529411764705882,
0.545098039215686,
0.564705882352941,
0.580392156862745,
0.596078431372549,
0.615686274509804,
0.631372549019608,
0.647058823529412,
0.666666666666667,
0.682352941176471,
0.701960784313725
};

/*if we find any of these words in a filename, ignore the file,
  because all activity would be for AFTER the box has been selected as roost*/
string avoid_words[] = {"besiedlung","Besiedlung","besiedelt","_tq","Besiedelt"};
unsigned navoid_words = sizeof(avoid_words)/sizeof(string);

/*should the node colors reflect relatedness or mother-daughter relationship*/
/*0 - mother_daughter 
  1 - relatedness
  2 - indegree*/
int what_node_sizes=2;



bool create_sqlitedb=false; //should we write the recordings of all boxes in a database
string file_sqlitedb = "box_recordings_2008.sqlite";
//callback function for the sqlite query exec
static int callback(void *NotUsed, int argc, char **argv, char **azColName) { 
   int i;
   for(i=0; i<argc; i++){      
      cout<<azColName[i]<<" = ";
      if (argv[i] != NULL)
	cout<<argv[i];
      else 
	cout<<"NULL";
      //printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
   cout<<endl;
   return 0;
}
   


/* ==================================================================== */

/* =========================== GLOBAL FUNCTIONS ======================= */
/* Given "number" find a bin for it in a 128-length array.
   Which array is taken for the binning depends on the flag
   argument. All possible arrays are defined within the function.
   Returns an index to the corresponding color that the given number
   should have*/
int binData(double number, int flag) {
  double mother_daughter_bins[] = {
0,
0.00234375,
0.0046875,
0.00703125,
0.009375,
0.01171875,
0.0140625,
0.01640625,
0.01875,
0.02109375,
0.0234375,
0.02578125,
0.028125,
0.03046875,
0.0328125,
0.03515625,
0.0375,
0.03984375,
0.0421875,
0.04453125,
0.046875,
0.04921875,
0.0515625,
0.05390625,
0.05625,
0.05859375,
0.0609375,
0.06328125,
0.065625,
0.06796875,
0.0703125,
0.07265625,
0.075,
0.07734375,
0.0796875,
0.08203125,
0.084375,
0.08671875,
0.0890625,
0.09140625,
0.09375,
0.09609375,
0.0984375,
0.10078125,
0.103125,
0.10546875,
0.1078125,
0.11015625,
0.1125,
0.11484375,
0.1171875,
0.11953125,
0.121875,
0.12421875,
0.1265625,
0.12890625,
0.13125,
0.13359375,
0.1359375,
0.13828125,
0.140625,
0.14296875,
0.1453125,
0.14765625,
0.15,
0.15234375,
0.1546875,
0.15703125,
0.159375,
0.16171875,
0.1640625,
0.16640625,
0.16875,
0.17109375,
0.1734375,
0.17578125,
0.178125,
0.18046875,
0.1828125,
0.18515625,
0.1875,
0.18984375,
0.1921875,
0.19453125,
0.196875,
0.19921875,
0.2015625,
0.20390625,
0.20625,
0.20859375,
0.2109375,
0.21328125,
0.215625,
0.21796875,
0.2203125,
0.22265625,
0.225,
0.22734375,
0.2296875,
0.23203125,
0.234375,
0.23671875,
0.2390625,
0.24140625,
0.24375,
0.24609375,
0.2484375,
0.25078125,
0.253125,
0.25546875,
0.2578125,
0.26015625,
0.2625,
0.26484375,
0.2671875,
0.26953125,
0.271875,
0.27421875,
0.2765625,
0.27890625,
0.28125,
0.28359375,
0.2859375,
0.28828125,
0.290625,
0.29296875,
0.2953125,
0.29765625,
0.3
};
  double related_bins[]={
-0.3,
-0.2953125,
-0.290625,
-0.2859375,
-0.28125,
-0.2765625,
-0.271875,
-0.2671875,
-0.2625,
-0.2578125,
-0.253125,
-0.2484375,
-0.24375,
-0.2390625,
-0.234375,
-0.2296875,
-0.225,
-0.2203125,
-0.215625,
-0.2109375,
-0.20625,
-0.2015625,
-0.196875,
-0.1921875,
-0.1875,
-0.1828125,
-0.178125,
-0.1734375,
-0.16875,
-0.1640625,
-0.159375,
-0.1546875,
-0.15,
-0.1453125,
-0.140625,
-0.1359375,
-0.13125,
-0.1265625,
-0.121875,
-0.1171875,
-0.1125,
-0.1078125,
-0.103125,
-0.0984375,
-0.09375,
-0.0890625,
-0.084375,
-0.0796875,
-0.075,
-0.0703125,
-0.065625,
-0.0609375,
-0.05625,
-0.0515625,
-0.046875,
-0.0421875,
-0.0375,
-0.0328125,
-0.028125,
-0.0234375,
-0.01875,
-0.0140625,
-0.009375,
-0.0046875,
0,
0.0046875,
0.009375,
0.0140625,
0.01875,
0.0234375,
0.028125,
0.0328125,
0.0375,
0.0421875,
0.046875,
0.0515625,
0.05625,
0.0609375,
0.065625,
0.0703125,
0.075,
0.0796875,
0.084375,
0.0890625,
0.09375,
0.0984375,
0.103125,
0.1078125,
0.1125,
0.1171875,
0.121875,
0.1265625,
0.13125,
0.1359375,
0.140625,
0.1453125,
0.15,
0.1546875,
0.159375,
0.1640625,
0.16875,
0.1734375,
0.178125,
0.1828125,
0.1875,
0.1921875,
0.196875,
0.2015625,
0.20625,
0.2109375,
0.215625,
0.2203125,
0.225,
0.2296875,
0.234375,
0.2390625,
0.24375,
0.2484375,
0.253125,
0.2578125,
0.2625,
0.2671875,
0.271875,
0.2765625,
0.28125,
0.2859375,
0.290625,
0.2953125,
0.3
};
  double indegree_bins[] ={
    0,
0.4375,
0.875,
1.3125,
1.75,
2.1875,
2.625,
3.0625,
3.5,
3.9375,
4.375,
4.8125,
5.25,
5.6875,
6.125,
6.5625,
7,
7.4375,
7.875,
8.3125,
8.75,
9.1875,
9.625,
10.0625,
10.5,
10.9375,
11.375,
11.8125,
12.25,
12.6875,
13.125,
13.5625,
14,
14.4375,
14.875,
15.3125,
15.75,
16.1875,
16.625,
17.0625,
17.5,
17.9375,
18.375,
18.8125,
19.25,
19.6875,
20.125,
20.5625,
21,
21.4375,
21.875,
22.3125,
22.75,
23.1875,
23.625,
24.0625,
24.5,
24.9375,
25.375,
25.8125,
26.25,
26.6875,
27.125,
27.5625,
28,
28.4375,
28.875,
29.3125,
29.75,
30.1875,
30.625,
31.0625,
31.5,
31.9375,
32.375,
32.8125,
33.25,
33.6875,
34.125,
34.5625,
35,
35.4375,
35.875,
36.3125,
36.75,
37.1875,
37.625,
38.0625,
38.5,
38.9375,
39.375,
39.8125,
40.25,
40.6875,
41.125,
41.5625,
42,
42.4375,
42.875,
43.3125,
43.75,
44.1875,
44.625,
45.0625,
45.5,
45.9375,
46.375,
46.8125,
47.25,
47.6875,
48.125,
48.5625,
49,
49.4375,
49.875,
50.3125,
50.75,
51.1875,
51.625,
52.0625,
52.5,
52.9375,
53.375,
53.8125,
54.25,
54.6875,
55.125,
55.5625,
56
  };

  if (flag == 0) {
    unsigned length_mother_daughter_bins = sizeof(mother_daughter_bins)/sizeof(double);
    for (unsigned i=0; i<length_mother_daughter_bins-1; i++) {
      if (mother_daughter_bins[i] <= number && number <= mother_daughter_bins[i+1])
	return i;
    }
    if (number >= mother_daughter_bins[length_mother_daughter_bins-1])
      return 127;
  
    return -1; //to check if number is not in [0,1]  
  }
  else if (flag == 1) {
    unsigned length_related_bins = sizeof(related_bins)/sizeof(double);
    for (unsigned i=0; i<length_related_bins-1; i++) {
      if (related_bins[i] <= number && number <= related_bins[i+1])
	return i;
    }
    if (number >= related_bins[length_related_bins-1])
      return 127;
    else if (number <= related_bins[0])
      return 0;
    
    return -1; //to check if number is not in [0,1]      
  }
  
  else if (flag == 2) {
    unsigned length_indegree_bins = sizeof(indegree_bins)/sizeof(double);
    for (unsigned i=0; i<length_indegree_bins-1; i++) {
      if (indegree_bins[i] <= number && number <= indegree_bins[i+1])
	return i;
    }
    if (number >= indegree_bins[length_indegree_bins-1])
      return 127;
    else if (number <= indegree_bins[0])
      return 0;
    
    return -1; //to check if number is not in [0,1]          
  }
  return -1;
}

/*initialise the boxes based on directory structure*/
void initBoxes(const char* dirname) {
  DIR *dir;
  struct dirent *ent;
  struct stat st;
  string all_box("100"), majority_box("66"),minority_box("33"), control_box("0");
  unsigned count=0;

  dir = opendir (dirname);
  if (dir != NULL) {
    /* print all the files and directories within directory */
    while ((ent = readdir (dir)) != NULL) {
      string node_name(dirname);
      node_name += "/";
      node_name += ent->d_name;      
      lstat(node_name.c_str(), &st);
      if(S_ISDIR(st.st_mode)) {
	string dir_entry(ent->d_name);
	if (dir_entry.find(all_box) != string::npos) {
	  Box b(3,dir_entry);
	  boxes[dir_entry] = b; boxes_auxillary[dir_entry] = count;	  
	  boxes_auxillary_reversed[count++]=dir_entry;
	}
	else if (dir_entry.find(majority_box) != string::npos) {
	  Box b(2,dir_entry);
	  boxes[dir_entry] = b; boxes_auxillary[dir_entry] = count;
	  boxes_auxillary_reversed[count++]=dir_entry;
	}
	else if (dir_entry.find(minority_box) != string::npos) {
	  Box b(1,dir_entry);
	  boxes[dir_entry] = b; boxes_auxillary[dir_entry] = count;
	  boxes_auxillary_reversed[count++]=dir_entry;
	}
	else if (dir_entry.find(control_box) != string::npos) {
	  Box b(0,dir_entry);
	  boxes[dir_entry] = b; boxes_auxillary[dir_entry] = count;
	  boxes_auxillary_reversed[count++]=dir_entry;
	}
      }
    }
    closedir (dir);
  }
  else {
    /* could not open directory */
    perror ("");
  }
}

/*start processing the data directory - read all files.
  at the end of this function the "boxes" vector is filled
  with activity history of each box for all times*/
void processDataDirectory(string dir_name,string box_name) {
  DIR *data_dir;
  struct dirent *ent;

  //cout<<"Entering "<<dir_name<<endl;
  data_dir = opendir (dir_name.c_str());
  short file_counter = 0;
  if (data_dir != NULL) {
    while ((ent = readdir (data_dir)) != NULL) {
      struct stat st;
      string file_name(ent->d_name);
      file_name = dir_name + "/" + file_name;
      lstat(file_name.c_str(), &st);
      if (!S_ISDIR(st.st_mode)) {
	//skip the file if it contains the forbidden words, i.e.
	//the box has already been occupied.
	for (unsigned i=0; i<navoid_words; i++) {
	  if (file_name.find(avoid_words[i]) != string::npos) 
	    cout<<file_name<<" skipped"<<endl;	  	  
	}
	yyin = fopen(file_name.c_str(),"r");
	yylex();
	counter ++;
	file_counter ++;
	/*PROCESS THE CONTENTS OF EACH DATA FILE*/
	Box *targetBox = &boxes[box_name];
	for (unsigned i=0; i<box_entries.size(); i++) {
	  /*is the given transpoder id a bat?*/
	  if (find(bats_vector.begin(),bats_vector.end(),box_entries[i].first) == bats_vector.end()) {
	    if (find(transponders_vector.begin(),transponders_vector.end(),box_entries[i].first) == transponders_vector.end()) {
	      cout<<box_entries[i].first<<" neither a bat nor a transpoder"<<endl;  	      
	    }
	    continue;
	  }
	  /**********************************/
	  //newBatEntry(entry time, hexid of recorded bat, name of the box)
	  BatEntry newBatEntry(box_entries[i].second,box_entries[i].first,targetBox -> name);
	  pair<set<BatEntry,batEntryCompare>::iterator,bool> ret,ret1;
	  
	  ret = targetBox->activity.insert(newBatEntry);
	  if (ret.second == false) { //if the same bat was there at the same time, increase its occurence by 1
	    BatEntry existingBatEntry = *(ret.first); //the bat who had already been insterted	
	    existingBatEntry.occurence += newBatEntry.occurence;
	    targetBox->activity.erase(ret.first);
	    ret1 = targetBox->activity.insert(existingBatEntry);
	    if (ret1.second == false) { //should never happen
	      cout<<"Warning::Duplicate times"<<endl;
	    }  
	  }
	}
	box_entries.clear();
	/*******************************************/
      } //end if
    } //end while
    cout<<"processed "<<file_counter<<" files"<<endl;
  }
  closedir(data_dir); 
}

/* ==================================================================== */


/* ============================   MAIN   ============================== */
/* ===                                                              === */
/* ===                                                              === */
int main(int argc, char**argv) {
  argv++;argc--;

  if (!argc) {
    cout<<"Version: "<<version<<endl<<endl;
    cout<<"Usage: bats <dir>"<<endl<<endl;
    cout<<"<dir>\t full path to the directory containing transponder data files"<<endl<<endl;
    return 0;
  }
  if (argc > 0) {
    base_dir = argv[0];
    initBoxes(base_dir);
    /*this call is in the beginning, as advised in the igraph manual*/
    igraph_i_set_attribute_table(&igraph_cattribute_table);   
     
    //some init stuff
    for (unsigned j=0; j<nbats; j++) 
      bats_map[bats[j]] = j;
    
    /*initialize the relatedness data*/
    string fileName="../data/relatedness.txt";
    ifstream relatedness_file(fileName.c_str(),ios::in);
    if (relatedness_file.fail()) {
      perror(fileName.c_str());
      exit(1);
    }
    while (relatedness_file.good()) {
      string line; 
      getline(relatedness_file,line);
      //A-1st column, B-2nd column, x-3rd column in the file
      char *A = new char[128];char *B = new char[128];
      double x;      
      int tokenize_line=sscanf(line.c_str(),"%s %s %lf",A,B,&x);
      if (tokenize_line == 3) {
	string a=A,b=B;
	stringstream ss; ss<<A<<B;	
	relatedness_map[ss.str()]=x;	
	delete [] A; delete [] B;
      }
    }      
    //cout<<relatedness_map.size()<<endl;
    
    //initialize the mother daughter relationships
    //try to read "bats_mother_daughter_pairs_GB2_noMM.txt" in the current dir.
    //first column are mothers, second column are daughters
    //fills the mother_daughter map
    fileName = "../data/bats_mother_daughter_pairs_GB2_noMM.txt";
    ifstream parentfile(fileName.c_str(),ios::in);
    if (parentfile.fail()) {
      perror(fileName.c_str());
      exit(1);
    }
    while (parentfile.good()) {
      string line,mother,daughter;
      getline(parentfile,line);
      tokenizer<> tok(line);
      for (tokenizer<>::iterator beg=tok.begin(); beg != tok.end(); ) {
	mother = *beg;
	beg++;
	daughter = *beg;
	beg++;
	multimap<string,string>::iterator insert_itr;
	pair<string,string> insert_element(mother,daughter);
	insert_itr = mother_daughter.insert(insert_element);	
      }     
    }
    
    DIR *dir;
    struct dirent *ent;
    string all_box("100"), majority_box("66"),minority_box("33"), control_box("0");   
    
    dir = opendir(base_dir);
    if (dir != NULL) {
      while ((ent = readdir (dir)) != NULL) {
	struct stat st;
	string dir_entry,node_name(ent->d_name);
	dir_entry += base_dir;
	dir_entry += "/";
	dir_entry += node_name;
	lstat(dir_entry.c_str(), &st);

	if(S_ISDIR(st.st_mode)) {
	if (node_name.find(all_box) != string::npos || 
	    node_name.find(majority_box) != string::npos ||
	    node_name.find(minority_box) != string::npos ||
	    node_name.find(control_box) != string::npos) {
	  //cout<<"Opening "<<dir_entry<<" Box "<<node_name<<endl;
	  processDataDirectory(dir_entry,node_name);
	}	
      }
      }
      closedir (dir);
    }
    else {
      /* could not open directory */
      perror ("");
    }
      /*the number of processed files*/
      cout<<"Total processed files "<<counter<<endl;
    
      /*now that the boxes have been filled with transpoder data, start the real work*/
      /*combine the records of all boxes for all times into a multiset*/
      multiset<BatEntry,batEntryCompare2> multibats;
      multiset<BatEntry,batEntryCompare2>::iterator itr;
      map<string,Box,bool(*)(string,string)>::iterator map_itr;

      counter = 0;
      for (map_itr=boxes.begin(); map_itr != boxes.end(); map_itr++) {
	Box currentBox = (*map_itr).second;
	//currentBox.print();
	counter += currentBox.activity.size();
	multibats.insert(currentBox.activity.begin(),currentBox.activity.end());
      }
      cout<<"Total records in all boxes "<<counter<<endl;
      cout<<"\nMULTI BATS "<<multibats.size()<<endl;   
 
      /*generate individual movement history for each bat*/
      //associate a bat name (string) with the corresponding Bat object
      for (itr=multibats.begin(); itr != multibats.end(); itr++) {
	BatEntry current = *itr;
	Bat &ref = bats_records[current.hexid];
	ref.hexid = current.hexid;      
	//which box?
	Box *box_ptr = &boxes[current.box_name];
	ref.add_movement(current.TimeOfEntry,box_ptr);
      }
      //associate mothers to daughters
      for (unsigned i=0; i<nbats; i++) {
	Bat &ref = bats_records[bats[i]];
	//if no records exist for this bat then we need to init also its hexid,
	//because it would be an empty string
	ref.hexid = bats[i];
	string last4letters = bats[i].substr(bats[i].length()-4,4);
	pair<multimap<string,string>::iterator,multimap<string,string>::iterator > p;
	multimap<string,string>::iterator j;
	p=mother_daughter.equal_range(last4letters);
	//bool found=false;
	for (j=p.first; j != p.second;j++) {
	  ref.daughters_hexids.push_back(j->second);
	  mothers.push_back(ref.hexid);
	  daughters.push_back(j->second);
	  //found=true;
	}     
      }
    
      //check to see how many of the bats who have a moving history also have mothers
      //with moving history in the data!
      unsigned total_bats_movinghistory = 0,bats_with_mothers=0;
      for (unsigned j=0; j<nbats; j++) {
	Bat &ref = bats_records[bats[j]];
	if (ref.movement_history.size() > 0) {
	  total_bats_movinghistory++;
	  string last4letters = ref.hexid.substr(ref.hexid.length()-4,4);
	  if (find(daughters.begin(),daughters.end(),last4letters) != daughters.end()) {
	    //found a mother, let's find who she is
	    for (unsigned k=0; k<mothers.size(); k++) {
	      string momlast4letters = mothers[k].substr(mothers[k].length()-4,4);
	      pair<multimap<string,string>::iterator,multimap<string,string>::iterator > p;
	      multimap<string,string>::iterator l;
	      p=mother_daughter.equal_range(momlast4letters);
	      for (l=p.first; l!=p.second;l++) {//found the same mother with several daughters
		if (l->second == last4letters) {
		  //found the mother daughter pair
		  Bat &ref_mom = bats_records[mothers[k]];
		  if (ref_mom.movement_history.size() > 0) bats_with_mothers++;		  	
		}  
	      }
	    }	  
	  }
	}      
      }
      cout<<"Out of "<<total_bats_movinghistory<<" recorded bats, "<<bats_with_mothers<<" have recorded mothers"<<endl;
      //check if a bat has not been found as either mother or a daughter
      for (map<string,Bat>::iterator j=bats_records.begin(); j!=bats_records.end(); j++) {
	vector<string>::iterator mother_itr,daughters_itr;
	string bat = j->second.hexid;
	mother_itr = find(mothers.begin(),mothers.end(),bat);
	daughters_itr = find(daughters.begin(),daughters.end(),bat.substr(bat.length()-4,4));
	if (mother_itr == mothers.end() && daughters_itr == daughters.end()) {
	  cout<<bat<<" not found as either mother or daughter in the mother-daughter file"<<endl;	
	}      
      }
    
      //print the bats history 
      map<string,Bat,bool(*)(string,string)>::iterator bat_itr;
      /*
      for (bat_itr=bats_records.begin(); bat_itr != bats_records.end(); bat_itr++) {
	Bat currentBat = (*bat_itr).second;
	if (currentBat.movement_history.size()>0)
	  currentBat.print();
      }   
      */  
      /*start reading the whole recording history of all boxes and infer social events
	possible events are - discovery, exploration, revisit, leading-following*/        
      multiset<BatEntry,batEntryCompare>::iterator from,to;
      from=multibats.begin();
      ptime prev_instruction_time = multibats.begin()->TimeOfEntry; //kickstart
      //int counter1 = 0;
      Bat *bt = NULL; //use bats_records to get a handle to a bat given its hexid.
      Box *bx = &boxes[multibats.begin()->box_name]; //get the first box in multibats	
	
      //IMPORTANT: DO THE ANALYSIS ONE BOX AT A TIME!!!!    
      vector<time_duration> lf_chunk_sizes; //stores the time duration of the chunk that a lead-follow event was recorded in
      for (to=multibats.begin(); to != multibats.end(); to++) {     
	//get a chunk of instructions. enter if two consequtive instructions are separated 
	//by more than time_chunk OR we have started a new box
	if (to->TimeOfEntry - prev_instruction_time > time_chunk || to->box_name != bx->name) {	
	  //copy the chunk into the vector
	  vector<BatEntry> chunk(from,to);
	  //what is the time span of the chunk?
	  vector<BatEntry>::iterator last_chunk_element = chunk.end();
	  last_chunk_element--; //go to the real last element
	  time_duration t = last_chunk_element->TimeOfEntry - chunk.begin()->TimeOfEntry;
	  //cout<<to_simple_string(t)<<endl;
	  //how many different bats are in the chunk?
	  //the easiest (!=most efficient) way is to insert the bats' hexids into a set<String>
	  set<string> unique_bats;	
	  set<string>::iterator unique_bats_itr;
	  map<string,ptime> eventTime; //used to store the last time a given bat was recorded doind a particular event
				     //for example if a bat discovers a box and circles around it for some time
				     //we will store the first recording in the map	 
	
	  //cout<<"Chunk "<<++counter1<<endl;
	  for (vector<BatEntry>::iterator iter = chunk.begin(); iter != chunk.end(); iter++) {	  
	    //iter->print();
	    unique_bats.insert(iter->hexid);
	    ptime &ptimeref = eventTime[iter->hexid];
	    if (ptimeref.is_not_a_date_time()) ptimeref = iter->TimeOfEntry; //always take the first entry	  
	  }
	  //unique_bats should know contain the number of different bats in the chunk
	  /*determine how many experienced and naiive bats we have in the chunk. possible combinations are:
	   -All NAIIVE + UNDISCOVERED BOX: 1 DISCOVERY EVENT + N-1 EXPLORATORY EVENTS
	   -ALL NAIIVE + DISCOVERED BOX: N EXPLORATORY EVENTS
	   -ALL EXPERIENCED + DISCOVERED BOX: N REVISITS	   
	   -K EXPERIENCED + L NAIIVE + DISCOVERED BOX: LEADFOLLOW(K,L)*/
	  //first count the experience and naiive bats	  
	  unsigned experienced = 0; unsigned naiive = 0;
	  for (unique_bats_itr = unique_bats.begin(); unique_bats_itr != unique_bats.end(); unique_bats_itr++) {
	    //use bats_records to get a handle to a bat given its hexid.
	    bt = &bats_records[*unique_bats_itr];
	    if (bt->box_knowledge[bx->name] == NAIIVE) naiive++;
	    else experienced++;
	  }
	  //ALL NAIIVE
	  if (unique_bats.size() == 0) continue;
	  if (naiive == unique_bats.size()) {
	    //N EXPLORATORY EVENTS
	    if (bx->status == DISCOVERED) {
	      for (unique_bats_itr = unique_bats.begin(); unique_bats_itr != unique_bats.end(); unique_bats_itr++) {
		bats_records[*unique_bats_itr].box_knowledge[bx->name] = EXPERIENCED;
		bt = &bats_records[*unique_bats_itr];
		bt->box_knowledge[bx->name] = EXPERIENCED;
		vector<Bat *> exploring_bat; exploring_bat.push_back(bt);
		Event eevent(exploring_bat,eventTime[bt->hexid],bx,"Exploration");
		//all_explorations.push_back(eevent);		
		bx->events.push_back(eevent);
		all_events.insert(eevent);
	      }     
	    }
	    //1 DISCOVERY EVENT + N-1 EXPLORATORY EVENTS
	    else if (bx->status == UNDISCOVERED) {
	      bx->status = DISCOVERED;
	      //1 DISCOVERY EVENT
	      bt = &bats_records[*unique_bats.begin()]; //the first one is the discoverer
	      bt->box_knowledge[bx->name] = EXPERIENCED;	     
	      vector<Bat *> discovering_bat; discovering_bat.push_back(bt);
	      Event devent(discovering_bat,eventTime[bt->hexid],bx,"Discovery");
	      //all_discoveries.push_back(devent);
	      bx->events.push_back(devent);
	      all_events.insert(devent);
	      //N-1 EXPLORATORY EVENTS
	      for (unique_bats_itr = ++unique_bats.begin(); unique_bats_itr != unique_bats.end(); unique_bats_itr++) {
		bt = &bats_records[*unique_bats_itr];
		bt->box_knowledge[bx->name] = EXPERIENCED;
		vector<Bat *> exploring_bat; exploring_bat.push_back(bt);
		Event eevent(exploring_bat,eventTime[bt->hexid],bx,"Exploration");
		//all_explorations.push_back(eevent);
		bx->events.push_back(eevent);		
		all_events.insert(eevent);
	      }	
	    }
	  }
	  //ALL EXPERIENCED + DISCOVERED BOX: N REVISITS	   
	  else if (experienced == unique_bats.size() && bx->status == DISCOVERED) {
	    for (unique_bats_itr = unique_bats.begin(); unique_bats_itr != unique_bats.end(); unique_bats_itr++) {
		bt = &bats_records[*unique_bats_itr];		
		vector<Bat *> revisiting_bat; revisiting_bat.push_back(bt);
		Event revent(revisiting_bat,eventTime[bt->hexid],bx,"Revisit");
		//all_revisits.push_back(revent);
		bx->events.push_back(revent);
		all_events.insert(revent);
	    }
	  }
	  //K EXPERIENCED + L NAIIVE + DISCOVERED BOX: LEADFOLLOW(K,L)
	  else if (experienced+naiive == unique_bats.size()) {
	    vector<Bat *> leaders, followers;
	    ptime p;
	    for (unique_bats_itr = unique_bats.begin(); unique_bats_itr != unique_bats.end(); unique_bats_itr++) {
	      bt = &bats_records[*unique_bats_itr];
	      if (bt->box_knowledge[bx->name] == NAIIVE) {		
		followers.push_back(bt);
		bt->box_knowledge[bx->name] = EXPERIENCED; //the follower now knows about the box
	      }
	      else if (bt->box_knowledge[bx->name] == EXPERIENCED) { 
		leaders.push_back(bt);
		//take the first time that one of the leaders was recorded as the time of the LeadFollow event
		if (p.is_not_a_date_time()) p = eventTime[bt->hexid]; 
	      }
	      else {cout<<"Sanity checks failed: A bat is neither naiive nor experienced"<<endl;}      
	    }
	    //register a lead follow event only if its in a chunk not longer than 10 minutes
	    if (t <= minutes(10)) {
	      Event lfevent(leaders,followers,p,bx,"LeadFollow");
	      //all_leadfollows.push_back(lfevent);
	      bx->events.push_back(lfevent);
	      all_events.insert(lfevent);
	      //insert as many times as there are leading following events (i.e. edges in the network)
	      //for (unsigned hh=0; hh<leaders.size()*followers.size(); hh++)
	      lf_chunk_sizes.push_back(t);
	    }
	  }
	  else {cout<<"Sanity checks failed: Number of experience plus naiive bats not the same as chunk size"<<endl;} 
	  from = to; //move the 'from' pointer
	  bx = &boxes[to->box_name]; //point the box pointer to the box of the current BatEntry
	}
	prev_instruction_time = to->TimeOfEntry;
      }      
      //print the sizes of the chunks in which all leading-following events were recorded
      for (unsigned k=0; k<lf_chunk_sizes.size(); k++) 
	  cout<<to_simple_string(lf_chunk_sizes[k])<<endl;
      
      for (map<string,Box>::iterator i=boxes.begin(); i!=boxes.end();i++) {      
	pair<string,Box> p = *i;
	//cout<<"BOX "<<p.second.name<<endl;
	for (unsigned j=0; j<p.second.events.size(); j++) {
	  //p.second.events[j].print();	
	}
      }
    
      
      /*store the number of leading events that a leader has taken part of,
     regardless the number of following individuals*/
      map<string,unsigned> bats_lead_stats;
      /*store the number of times a bat has followed*/
      map<string,unsigned> bats_follow_stats; 
      /*aggregately store how many daughters have followed each bat so far*/
      //map<string,int> bats_n_daughters_following;
    
      /*create the cxf file*/
      ofstream cxffile("lead-follow.cxf",ios::trunc);
      //first init a map between hexids and initial node sizes
      map<string,double> bat_nodes;
      double starting_node_size = 9.0;
      for (unsigned i=0; i<nbats; i++) {
	Bat b = bats_records[bats[i]];
	if (b.movement_history.size() == 0) continue;//skip this bat, did not move at all
	bats_lead_stats[bats[i]] = 0;
	bats_follow_stats[bats[i]] = 0;
	bat_nodes[bats[i]] = starting_node_size;
	cxffile<<"node: ("<<i+1<<") size{"<<bat_nodes[bats[i]]<<"} "<<"label{"<<bats[i]<<"} color{0.933,0.796,0.678}"<<endl;
      }
      cxffile.close();
    
      /*create an empty directed igraph with all bats and no edges yet*/
      igraph_matrix_t lf_adjmatrix; //square matrix
    
      igraph_matrix_init(&lf_adjmatrix,nbats,nbats); //init the square matrix
      igraph_matrix_null(&lf_adjmatrix);      
      /*create the cef file*/
      ofstream ceffile("lead-follow.cef",ios::trunc);
      set<Event,eventCompare>::iterator event_itr;
      /*stores a directed edge between Bat1.hexid and Bat2.hexid.
     the edge is encoded as a single string of Bat1.hexid+Bat2.hexid*/
      set<string> edges;
      double edges_starting_width = 1.0;
      unsigned total_following_events = 0;
      map<string,double> edges_width;
      pair<set<string>::iterator, bool> edges_itr;
      for (event_itr=all_events.begin(); event_itr != all_events.end(); event_itr++) {      
      Event e = *event_itr;
      if (e.eventName == "LeadFollow") {
	ceffile<<"["<<endl;
	//create (or update) an edge between each leader and all followers
	for (unsigned leaders=0; leaders<e.focal_bat.size(); leaders++) {
	  string last4letters_leader = e.focal_bat[leaders]->hexid;
	  //always get the last 4 letters of the leaders hexid, because that's what stored in mother_daughter multimap
	  last4letters_leader = last4letters_leader.substr(last4letters_leader.length()-4,4);	  
	  for (unsigned followers=0; followers<e.followers.size(); followers++) {
	    total_following_events++;
	    bats_lead_stats[e.focal_bat[leaders]->hexid]++;
	    e.focal_bat[leaders]->total_following++; //one more bat has followed me
		       				   //we'll see if she's my daughter shortly
	    bats_follow_stats[e.followers[followers]->hexid]++;
	    //check if the follower is a daugther of the leader
	    //first get all children of the mother = focal_bat
	    pair<multimap<string,string>::iterator,multimap<string,string>::iterator > p;
	    multimap<string,string>::iterator k;
	    p = mother_daughter.equal_range(last4letters_leader);
	    string last4letters_follower = e.followers[followers]->hexid;
	    last4letters_follower = last4letters_follower.substr(last4letters_follower.length()-4,4);
	    //cout<<last4letters_leader<<" <<-- "<<last4letters_follower<<endl;
	    //now check if any of the children match our follower
	    for (k=p.first; k!= p.second; k++) {
	      if (k->second == last4letters_follower) {
		e.focal_bat[leaders]->n_daughters_following++; //one more daughter has followed	      
		//cout<<last4letters_follower<<" follows mother "<<last4letters_leader<<endl;		
	      }
	    }	    
	    //concatenate the two bats' hexids to encode a relatedness pair	    
	    stringstream ss2(stringstream::in | stringstream::out);
	    string sledvasht = e.followers[followers]->hexid;
	    string vodach    = e.focal_bat[leaders]->hexid;
	    ss2<<sledvasht.substr(sledvasht.length()-4,4)<<vodach.substr(vodach.length()-4,4);
	    //change the order now. we need both to search in the relatedness map
	    stringstream ss3(stringstream::in | stringstream::out);
	    ss3<<vodach.substr(vodach.length()-4,4)<<sledvasht.substr(sledvasht.length()-4,4);
	    //find the relatedness between these two individuals	    
	    map<string,double>::iterator related_itr;
	    related_itr = relatedness_map.find(ss2.str());	    	    
	    if (related_itr == relatedness_map.end()) { //try the other way around
	      related_itr = relatedness_map.find(ss3.str());
	      if (related_itr != relatedness_map.end()) 
		e.focal_bat[leaders]->cumulative_relatedness += related_itr->second; //found it
	      else cout<<"Warning: No relatedness data between "<<e.focal_bat[leaders]->hexid<<" and "<<e.followers[followers]->hexid<<endl;
	    }
	    else 
	      e.focal_bat[leaders]->cumulative_relatedness += related_itr->second; //found it	    
	    
	    //concatenate the two bats' hexids to encode an edge
	    stringstream ss(stringstream::in | stringstream::out);
	    ss<<e.focal_bat[leaders]->hexid<<e.followers[followers]->hexid;
	    edges_itr = edges.insert(ss.str());	
	    if (edges_itr.second == true) { //the edge did not exist before
					    //create it in the cef file					    
	      edges_width[ss.str()] = edges_starting_width;					    
	      //+1 in the call below, because indexing in the cef file is from 1			   
	      ceffile<<"addEdge: ("<<bats_map[e.followers[followers]->hexid]+1<<",";
	      ceffile<<bats_map[e.focal_bat[leaders]->hexid]+1<<") ";	      
	      ceffile<<"width{"<<edges_starting_width<<"} "<<"weight{"<<edges_starting_width<<"} ";
	      ceffile<<"color{0.803,0.784,0.694}"<<endl;	      	      
	    }
	    else { //the edge exsited. update the edge weight and the size of the leading node
	      edges_width[ss.str()] += 1.0;
	      ceffile<<"editEdge: ("<<bats_map[e.followers[followers]->hexid]+1<<",";
	      ceffile<<bats_map[e.focal_bat[leaders]->hexid]+1<<") ";
	      ceffile<<"width{"<<edges_width[ss.str()]<<"} ";
	      ceffile<<"weight{"<<edges_width[ss.str()]<<"}"<<endl;
	    }
	    /*add/update the edge to the adjacency matrix*/
	    MATRIX(lf_adjmatrix,bats_map[e.followers[followers]->hexid],
		     bats_map[e.focal_bat[leaders]->hexid])++;     	  
	    //calculate the eigenvalue centrality of the graph
	    igraph_t g2;  
	    igraph_weighted_adjacency(&g2,&lf_adjmatrix,IGRAPH_ADJ_DIRECTED,"weight",true);
	    igraph_vector_t eigenvector;
	    igraph_vector_init(&eigenvector,nbats);
	    igraph_vector_null(&eigenvector);	    
	    //igraph_real_t eigenvalue;
	    igraph_arpack_options_t aroptions;
	    igraph_arpack_options_init(&aroptions);
	    igraph_vs_t vs;
	    igraph_vs_all(&vs);
	    //igraph_eigenvector_centrality(&g2,&eigenvector,&eigenvalue,true,false,NULL,&aroptions);	    
	    igraph_pagerank_old(&g2,&eigenvector,vs,true,300,0.001,0.99,false);
            //also increase the size of the leading node
	    //bat_nodes[e.focal_bat[leaders]->hexid] += 0.3;	    
	    //update the size and colors of all nodes
	    for (unsigned j=0; j<nbats; j++) {
	      Bat b = bats_records[bats[j]];
	      if (b.movement_history.size() == 0 || b.total_following == 0) {
		continue; //skit this bat, did not move at all or did not have anyone following it yet!
	      }
	      ceffile<<"editNode: "<<"("<<bats_map[bats[j]]+1<<") ";
	      ceffile<<"size{"<<500.0*VECTOR(eigenvector)[bats_map[bats[j]]]<<"} ";
	      double percentage_following = b.n_daughters_following / b.total_following;
	      double mean_relatedness = b.cumulative_relatedness / b.total_following;
	      int idx=-1;
	      if (what_node_sizes == 1)
		idx=binData(mean_relatedness,1);
	      else if (what_node_sizes == 0)
		idx=binData(percentage_following,0);
	      else if (what_node_sizes == 2)
		idx = binData(b.total_following,2);
	      
      	      if (idx == -1)  {
		cout<<"Error: Invalid index "<<percentage_following<<" or "<<mean_relatedness<<" or "<<b.total_following<<" for binning "<<b.hexid<<endl;
		ceffile<<endl;		
	      }
	      
	      else {
		double red_color = red[idx]; double green_color = green[idx]; double blue_color = blue[idx];		
	        //cout<<percentage_following<<"\t"<<idx<<"\t"<<red_color<<"\t"<<green_color<<"\t"<<blue_color<<endl;
		ceffile<<"color{"<<red_color<<","<<green_color<<","<<blue_color<<"}"<<endl;
	      }
	    }	    
	    //destroy the graph and the vector with page ranks
	    igraph_vector_destroy(&eigenvector);
	    igraph_destroy(&g2);
	  }	  
	}
	ceffile<<"]"<<endl;
      }
      }
      ceffile.close();
      igraph_matrix_destroy(&lf_adjmatrix);
      cout<<"Total LEAD-FOLLOW events: "<<total_following_events<<endl;
    
    
      /*create the .txt file to store lead-follow statistics*/    
      ofstream txtfile("lead-follow.txt",ios::trunc);
      for (unsigned i=0; i<nbats; i++) {
      if (bats_records[bats[i]].movement_history.size() > 0)
	txtfile<<bats[i]<<" "<<bats_lead_stats[bats[i]]<<" "<<bats_follow_stats[bats[i]]<<endl;      
    }
      txtfile.close();
    
    
    
      /*calculate individual frequency of bat movements between boxes*/
      vector<igraph_matrix_t> adjmatrix; //3D array, i.e. each bat has its own matrix
      vector<igraph_t> bat_graphs; //one graph per bat

      for (unsigned j=0; j<nbats; j++) {
      igraph_matrix_t element;
      igraph_matrix_init(&element,boxes.size(),boxes.size());
      igraph_matrix_null(&element);
      adjmatrix.push_back(element);
    }
    
      for (bat_itr=bats_records.begin(); bat_itr !=bats_records.end(); bat_itr++) {
	Bat which_bat = bat_itr->second;
	//skip bats for which no recordings exist
	if (which_bat.movement_history.size() == 0) continue;
	unsigned bat_idx = bats_map[which_bat.hexid];
	string last_visited_box=which_bat.get_movement_history(0).second->name;
	ptime appeared_last = which_bat.get_movement_history(0).first;
	/*loop through this bat's movement_history*/
	for (unsigned j=1; j<which_bat.movement_history.size();j++) {
	  string whereami = which_bat.get_movement_history(j).second->name;
	  ptime when = which_bat.get_movement_history(j).first;
	  if (last_visited_box != whereami) { //appeared in a  new box
	    //enough time elapsed to move to another box
	    if (when-appeared_last > min_time_interval) {
	      //add weight to the edge between last visited box
	      //and current box
	      unsigned last_box_idx=boxes_auxillary[last_visited_box];
	      unsigned current_box_idx=boxes_auxillary[whereami];
	      MATRIX(adjmatrix[bat_idx],last_box_idx,current_box_idx)++;
	      MATRIX(adjmatrix[bat_idx],current_box_idx,last_box_idx)++;
	      appeared_last = when;
	    }
	    last_visited_box = whereami;
	  }
	}
      }   
      /*create the weighted graphs for each bat*/
      for (unsigned j=0; j<nbats; j++) {
	igraph_t g;
	igraph_weighted_adjacency(&g,&adjmatrix[j],IGRAPH_ADJ_UNDIRECTED,"weigth",false);
	bat_graphs.push_back(g);
      }

      /*write all graphs to files*/
      //the necessary vertex iterators and vertex selectors
      igraph_vs_t vs; //vertex selector
      igraph_vit_t vit; //vertex iterator

      for (unsigned j=0; j<nbats; j++) {      
	string bat = bats[j];
	//skip bats for which there are no recordings
	if (bats_records[bat].movement_history.size() == 0) continue;
	unsigned bat_idx = bats_map[bat];
	//just a check
	if (j != bat_idx)
	  cout<<"Warning..."<<endl;
      
	string filename = "graphs/" + bat + ".graph";
	ofstream outfile(filename.c_str(),ios::trunc);
	igraph_t g = bat_graphs[bat_idx];
	for (unsigned t=0; t<boxes.size(); t++) {	
	  igraph_vs_adj(&vs,t,IGRAPH_ALL); //select all adjacent vertices
	  igraph_vit_create(&g,vs,&vit);
	  while (!IGRAPH_VIT_END(vit)) {
	    unsigned v = IGRAPH_VIT_GET(vit);
	    if (v>t)    {
	      outfile<<boxes_auxillary_reversed[t];
	      outfile<<" "<<boxes_auxillary_reversed[v];
	      outfile<<" "<<MATRIX(adjmatrix[j],t,v)<<endl;
	    }
	    IGRAPH_VIT_NEXT(vit);
	  }
	}
	igraph_vit_destroy(&vit);
	igraph_vs_destroy(&vs);
	outfile.close();
      }

      for (unsigned j=0; j<nbats; j++) {
      igraph_matrix_destroy(&adjmatrix[j]);
      igraph_destroy(&bat_graphs[j]);
      }    

   }
   
   if (create_sqlitedb) {
    /* write the box recordings in a database if so configured. see the create_sqlitedb
	flag in the global definitions */
    map<string,Box>::iterator jj;
    sqlite3 *db;
    char *zErrMsg = 0;   
   
    int rc = sqlite3_open(file_sqlitedb.c_str(),&db);
    if (rc) {
	cout<<"Can't open database: "<<sqlite3_errmsg(db)<<endl;
	sqlite3_close(db);
	return 1;
    }
    //create table
    stringstream create_tb_query;
    create_tb_query<<"CREATE TABLE recordings (";
    create_tb_query<<"id INTEGER PRIMARY KEY, TimeOfEntry TEXT, bat_id TEXT, occurences INTEGER, boxname TEXT);";      
    rc=sqlite3_exec(db,create_tb_query.str().c_str(),callback,0,&zErrMsg);      
    if (rc != SQLITE_OK) {
      cout<<"SQL error: "<<zErrMsg<<endl;	
      sqlite3_free(zErrMsg);	
    }   
   
    for (jj = boxes.begin(); jj != boxes.end(); jj++) {
	Box bb = jj->second;
	for (set<BatEntry,batEntryCompare>::iterator j=bb.activity.begin(); j!=bb.activity.end(); j++) {	
	  stringstream insert_tb_query;
	  insert_tb_query<<"INSERT INTO recordings (TimeOfEntry, bat_id, occurences, boxname) values (";
	  string datetime = to_simple_string(j->TimeOfEntry);
	  //replace the ":" and " " in insert_query because sql doesn't like them
	  //replace(datetime.begin(),datetime.end(),':','-');
	  //replace(datetime.begin(),datetime.end(),' ','-');	
	  insert_tb_query<<"\""<<datetime<<"\",\""<<j->hexid<<"\","<<j->occurence<<",\""<<bb.name<<"\");";
	  cout<<insert_tb_query.str().c_str()<<endl;
	  rc = sqlite3_exec(db,insert_tb_query.str().c_str(),callback,0,&zErrMsg);
	  if (rc != SQLITE_OK) {
	    cout<<"SQL error: "<<zErrMsg<<endl;
	    sqlite3_free(zErrMsg);	
	  }
	}
    }
    sqlite3_close(db);
  }
  
  return 0;
}


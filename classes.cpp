
/* ======================== CLASS DECLARATIONS ========================= */
class Box; //represents a box
class Bat; //represents a bat
class BatEntry; //represents an entry of a bat into a box
class Lf_pair; //represents a leading following pair
/* ===================================================================== */

/* ======================== CLASS DEFINITIONS ========================== */
enum BatKnowledge {NAIIVE, EXPERIENCED}; //the knowledge of bats per box
enum BoxStatus {DISCOVERED, UNDISCOVERED};

class BatEntry {
public:
    ptime TimeOfEntry; //the time this bat was recorded
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
    BoxStatus status; //has this box been discovered or not
    pair<string,ptime> discoveredBy; //who discovered the box and then
    ptime occupiedWhen; //when was the box occupied. must be pre-defined
    //all activities after this date are ignored!
    //if not occupied this time is set to +inf.
    Box(short Type, string Name, ptime occ) {
        type = Type;
        name = Name;
        this->status = UNDISCOVERED;
        occupiedWhen = occ;
    }
    Box() {
        status = UNDISCOVERED;
        occupiedWhen = pos_infin;
    }
    void print() {
        set<BatEntry,batEntryCompare>::iterator i;
        cout<<"Box "<<name<<" "<<activity.size()<<" Occ. Date "<<to_simple_string(occupiedWhen)<<endl;
        for (i=activity.begin(); i != activity.end(); i++) {
            i->print();
        }
    }
};

/*just like the normal boolean datatype, but this ensures that a bool is always inited to false*/
class mybool {
public:
  bool custom_boolean;
  mybool() {custom_boolean = false;}
  mybool(bool b) {custom_boolean=b;}
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
    map<string,BatKnowledge> box_knowledge;//box_name -> knowledge. what bats know about each box
    map<string,mybool> disturbed_in_box; //box_name->0/1. 0 if bat has not been disturbed in that box
				       //1 - otherwise
    //store the time since a bat became informed of a given box
    map<string,ptime> informed_since;
    vector<string> daughters_hexids;
    void add_movement(ptime Time, Box * box_ptr) {
        pair<ptime,Box*> entry(Time,box_ptr);
        pair<set<pair<ptime, Box*>,movementCompare>::iterator,bool> insert_itr;
        insert_itr = movement_history.insert(entry);
        if (insert_itr.second == false)
            cerr<<"Warning: Duplicate movement entry for bat "<<hexid<<endl;
    }
    //make the bat informed
    void make_informed(string box_name,ptime informed_time) {
        ptime &ref = informed_since[box_name];
        if (ref.is_not_a_date_time()) {
            ref = informed_time;
        }
        else {
            cerr<<"Warning::Bat "<<hexid<<" informed flag set more than once"<<endl;
        }
    }
    //is the bat informed about a particular box at a given time
    bool is_informed(string box_name, ptime when) {
        if (box_knowledge[box_name] == NAIIVE) return false;
        //sanity check
        if (informed_since[box_name] == not_a_date_time)
            cerr<<"Warning::Bat should have been informed."<<endl;
        if (informed_since[box_name] > when) return false;
        else return true;
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
        hexid = Id;
        n_daughters_following=0.0;
        total_following=0.0;
        cumulative_relatedness=0.0;
        part_of_lf_event=false;
        internal_check=0;
    }
    Bat() {
        hexid = "";
        n_daughters_following=0.0;
        total_following=0.0;
        cumulative_relatedness=0.0;
        part_of_lf_event=false;
        internal_check=0;
    }
    void print() {
        cout<<"BAT "<<hexid<<endl;
        for (unsigned i=0; i<movement_history.size(); i++) {
            pair<ptime, Box*> m = get_movement_history(i);
            cout<<to_simple_string(m.first)<<" "<<m.second->name<<endl;
        }
    }
    inline bool operator==(const Bat &other) {
        return (this->hexid == other.hexid);
    }
};


class Lf_pair {
public:
    Bat *leader;
    Bat *follower;
    string box_name;
    ptime tleader;
    ptime tfollower;  
    bool valid; //valid only if abs(tleader-tfollower) <= "3-minute" rule. true by default
    Lf_pair() {
        valid=true;
        tleader=not_a_date_time;
        tfollower=not_a_date_time;
        box_name="";
    }
    Lf_pair(Bat *B1, Bat *B2) {
        leader = B1;
        follower = B2;
    }
    Lf_pair(Bat *B1, Bat *B2, ptime tB1, ptime tB2,string bname) {
        leader = B1;
        follower = B2;
        tleader = tB1;
        tfollower = tB2;
        box_name = bname;
        valid = true;
    }
    Lf_pair(Bat *B1, Bat *B2, ptime tB1, ptime tB2,string bname, bool v) {
        leader = B1;
        follower = B2;
        tleader = tB1;
        tfollower = tB2;
        box_name = bname;
        valid = v;
    }
    bool equals(Lf_pair &other) {
        if ((other.leader->hexid == leader->hexid && other.follower->hexid == follower->hexid) ||
                (other.leader->hexid == follower->hexid && other.follower->hexid == leader->hexid))
            return true;
        return false;
    }
    void init(Bat *B1, Bat *B2, ptime tB1, ptime tB2,string bname) {
        leader = B1;
        follower = B2;
        tleader = tB1;
        tfollower = tB2;
        box_name = bname;
        valid = true;
    }
    void print(ofstream *out) {
	if (!out->good()) {
	  cerr<<"Cannot print to output file: bad file descriptor"<<endl;
	}
	else
	  (*out)<<leader->hexid<<" "<<follower->hexid<<" "<<to_iso_string(tleader)<<" "<<to_iso_string(tfollower)<<" "<<box_name<<endl;
    }
    time_duration get_lf_delta() {
        time_duration t = tfollower-tleader;
        if (t.is_negative()) t = t.invert_sign();
        return t;
    }
    void validate(time_duration limit) {
        if (get_lf_delta() > limit) valid = false;
	else {
	  leader->part_of_lf_event=true;
	  follower->part_of_lf_event=true;
	}
    }

    string getLeaderId() {
        return leader->hexid;
    }
    string getFollowerId() {
        return follower->hexid;
    }
};

bool Lf_pair_compare(Lf_pair lf1, Lf_pair lf2) {
      return (lf1.tleader < lf2.tleader);    
}

class assortativity_map {
private:
    map<unsigned, vector<unsigned> > m;
public: 
  assortativity_map() {}
  void print_all(ofstream *out) {
    for (map<unsigned, vector<unsigned> >::iterator itr2=m.begin(); itr2 != m.end(); itr2++) {	  
      (*out)<<itr2->first<<" ";
      for (unsigned ttt=0; ttt<itr2->second.size(); ttt++)     
	(*out)<<itr2->second[ttt]<<" ";
      (*out)<<endl;
    }    
  }  
  
  void print_average(ofstream *out) {
    for (map<unsigned, vector<unsigned> >::iterator itr2=m.begin(); itr2 != m.end(); itr2++) {	  
      double sum=0.0;
      
      for (unsigned ttt=0; ttt<itr2->second.size(); ttt++)
	sum += itr2->second[ttt];
      
      (*out)<<itr2->first<<" "<<sum / itr2->second.size() <<endl;	  
    }
  }
  int avg_neighbour_connectivity(igraph_vector_t *indegrees,
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
};











/* ==================================================================== */

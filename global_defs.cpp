/* ======================== GLOBAL DEFINITIONS ======================= */

/*remove this, only temp*/
//set<string> unique_strings;

/*verbose flag*/
bool verbflag=true;
	

map<string,ptime> box_occupation;
map<string,ptime> box_installation;
map<string,string> box_occupation_deadline;
map<string,vector<pair<ptime, vector<string> > > > occupation_history;
map<string,string> monaten; //maps month names to month numbers
map<string,string> short_to_long; //maps short to long bat hex ids
/*the year of the analysis*/
string Year;
/*how is centrality calculated*/
short centrality = -1;
/*should we rewire the random models or not. default is false*/
bool rewire_random_models = false;
/*which colony are we analysing*/
string colony = "GB2";
/*the minimum amount of time between two consequtive recordings above which the recordings are considered disjoint, i.e. analysed
  separately*/
time_duration roundtrip_time;// = minutes(3);
//maximum allowed delay between a leader and a follower
time_duration lf_delay;// = minutes(5);
//min time for revisit
pair<time_duration, time_duration> revisit_interval(minutes(10),hours(23));
//max allowed time distance for a revisit to be counted as leading
time_duration revisit_limit(minutes(2));
/*at what time (we have the dates from the config file) do we start
 *ignoring readings for a given box*/
string occupation_deadline;

string json;

/* maps mothers to daughters */
bool multimapcompare (string lhs, string rhs) {
    return lhs<rhs;
}
multimap<string,string,bool (*) (string,string)> mother_daughter(multimapcompare);
/*stores all mothers and all daughters separately
  of course a bat may appear in both vectors*/
vector<string> mothers, daughters;

/* read the relatedness file and store the relatedness data here.
 if the relatedness between A and B is x
 the map will store AB=x, i.e. A and B concatenated*/
map<string,double> relatedness_map;

/*TODO: remove bats_vector, use bats_map instead*/
unsigned nbats,ntransponders;
//map<string> bats_vector;
//vector<string> bats_vector;
vector<string> transponders_vector;

/* let's get fancy - init the maps with a function pointer */
bool strcomp (string lhs, string rhs) {
    return lhs<rhs;
}
bool(*fn_pt)(string,string) = strcomp;
bool unsignedcomp (unsigned lhs, unsigned rhs) {
    return lhs<rhs;
}
bool(*fn_pt2)(unsigned,unsigned) = unsignedcomp;

bool paircomp(pair<unsigned,unsigned> lhs, pair<unsigned,unsigned> rhs) {
    if (lhs.first == rhs.first) 
      return (lhs.second < rhs.second);   
    else 
      return (lhs.first < rhs.first);
}
bool(*fn_pt3)(pair<unsigned,unsigned>,pair<unsigned,unsigned>) = paircomp;

/*all bats in a map keyed by name and indexed from 0*/
map<string,unsigned,bool(*)(string,string)> bats_map(fn_pt);
unsigned int bats_counter=0;
/*same as above, but valid for the data from all years*/
map<string,unsigned,bool(*)(string,string)> bats_map_all_data(fn_pt);
/*stores a movement history of individual bats*/
map<string,Bat,bool(*)(string,string)> bats_records(fn_pt);
/*same as above, but valid for the data from all years*/
map<string,Bat,bool(*)(string,string)> bats_records_all_data(fn_pt);
/*stores all boxes with all activity over time*/
map<string,Box,bool(*)(string,string)> boxes(fn_pt);
/*stores boxes names and associated numerical index*/
map<string,unsigned,bool(*)(string,string)> boxes_auxillary(fn_pt);
map<unsigned,string,bool(*)(unsigned,unsigned)> boxes_auxillary_reversed(fn_pt2);

/*stores the box programming: box_name->vector of bats programmed for this box*/
map<string,vector<string> > box_programming;
/*stores the occupying bats for each box: box_name->vector of occupying bats*/
map<string,vector<string> > box_occup_bats;

/*initialised by the flex scanner*/
vector < pair<string,string> >  box_entries;
/*auxillary*/
const char *base_dir;
short counter = 0;
string version = "1.0";


/*should the node colors reflect relatedness or mother-daughter relationship*/
/*0 - mother_daughter
  1 - relatedness
  2 - indegree*/
int what_node_sizes=2;

bool create_sqlitedb; //should we write the recordings of all boxes in a database
string file_sqlitedb;
//callback function for the sqlite query exec
static int callback(void *NotUsed, int argc, char **argv, char **azColName) {
    int i;
    for(i=0; i<argc; i++) {
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

/*output files*/
string lf_time_diff,lf_valid_time_diff,lf_pairs_valid_betweenness_preference,disturbed_leader,lf_valid_time_diff_viz;
string social_personal_box_lf,bats_lead_follow_behav,most_detailed,revisits,info_spread,activity_file;
string combined_networks,time_to_occupy,leading_following_statistics,leading_following_statistics_detailed;
string parameter_sweep;
/* ==================================================================== */


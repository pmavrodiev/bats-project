/* ======================== GLOBAL DEFINITIONS ======================= */
/*verbose flag*/
bool verbflag=false;
	
/*start of experiment*/

//tm start_exp = to_tm(ptime(from_iso_string("20080505T000000"))); //05.05.2008
tm end_exp   = to_tm(ptime(from_iso_string("20080926T000000"))); //26.09.2008

map<string,ptime> box_occupation;
map<string,ptime> box_installation;
map<string,string> box_occupation_deadline;
map<string,vector<pair<ptime, vector<string> > > > occupation_history;
map<string,string> monaten; //maps month names to month numbers
map<string,string> short_to_long; //maps short to long bat hex ids
/*the year of the analysis*/
string Year;
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
/*stores a movement history of individual bats*/
map<string,Bat,bool(*)(string,string)> bats_records(fn_pt);
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
char *base_dir;
short counter = 0;
string version = "1.0";

/** @OBSOLETE **/
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
//string avoid_words[] = {"besiedlung","Besiedlung","besiedelt","_tq","Besiedelt"};
//unsigned navoid_words = sizeof(avoid_words)/sizeof(string);

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
string social_personal_box_lf,bats_lead_follow_behav,most_detailed,revisits,info_spread;
/* ==================================================================== */
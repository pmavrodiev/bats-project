#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <list>
#include "config_file.yy.h"
#include "data_file.yy.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/greg_month.hpp>
#include <boost/date_time/gregorian/formatters.hpp>
#include <boost/date_time/gregorian/greg_year.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/exception/all.hpp>
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
#include <zlib.h>

#include "classes.h"

using namespace std;
using namespace boost;
using namespace boost::posix_time;
using namespace boost::gregorian;


/*TODO - read the number of the reading device in each raw file and if it does not correspond
	 to the reading device in that box, disregard the readings. For now, we have to manually
	 check whether the reading device is the right one for the box. The problem is that
	 with the ascii format.*/

/*Run configurations
 1. Single run - /home/pmavrodiev/Documents/bats/data/GB2_2008 /home/pmavrodiev/Documents/bats/result_files/bats_config_2008.txt 3 5 050000
*/


#include "global_defs.cpp"

/* =========================== GLOBAL FUNCTIONS ======================= */

/*searches in a vector of lf_events for a particular event*/
bool lf_exists(vector<Lf_pair> *vec_ptr, Lf_pair *element) {
    for (unsigned i=0; i<vec_ptr->size(); i++) 
        if ((*vec_ptr)[i].equals(*element))
	  return true;    
    return false;
}

/** @OBSOLETE **/
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
    double related_bins[]= {
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
    double indegree_bins[] = {
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
        if (number >= indegree_bins[length_indegree_bins-1]) {
            cout<<"WARNING"<<endl;
            return 127;
        }
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
    string all_box("100"), majority_box("66"),minority_box("33"), control_box("0"),majority_box2("67");
    string heat_box("h");
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
                ptime &ref = box_occupation[dir_entry];
		ptime &ref_installation = box_installation[dir_entry];
		ptime occupation_time = pos_infin;
                if (ref.is_not_a_date_time())
                    ref = pos_infin;
		else {
		    string tmp = to_iso_string(ref);		    
		    string new_date = tmp.substr(0,tmp.length()-6) + box_occupation_deadline[dir_entry];		    
		    //cout<<dir_entry<<"\t"<<ref<<"\t"<<tmp<<"\t"<<new_date<<endl;
		    try {
		      occupation_time = ptime(from_iso_string(new_date));
		    }
		    catch (std::out_of_range exception_bad_datetime) {
		      if (verbflag)
			cout<<"Info: Bad entry date skipped - "<<new_date<<"\t"<<dir_entry<<endl;
		      continue;
		    }
		}
                if (dir_entry.find(heat_box) != string::npos) {
                    Box b(4,dir_entry,occupation_time,ref_installation);		    
		    boxes[dir_entry] = b;
                    boxes_auxillary[dir_entry] = count;
                    boxes_auxillary_reversed[count++]=dir_entry;
                }
                else if (dir_entry.find(majority_box)  != string::npos ||
                         dir_entry.find(majority_box2) != string::npos) {
                    Box b(2,dir_entry,occupation_time,ref_installation);
                    boxes[dir_entry] = b;
                    boxes_auxillary[dir_entry] = count;
                    boxes_auxillary_reversed[count++]=dir_entry;
                }
                else if (dir_entry.find(minority_box) != string::npos) {
                    Box b(1,dir_entry,occupation_time,ref_installation);
                    boxes[dir_entry] = b;
                    boxes_auxillary[dir_entry] = count;
                    boxes_auxillary_reversed[count++]=dir_entry;
                }
                else if (dir_entry.find(all_box) != string::npos) {
                    Box b(3,dir_entry,occupation_time,ref_installation);
                    boxes[dir_entry] = b;
                    boxes_auxillary[dir_entry] = count;
                    boxes_auxillary_reversed[count++]=dir_entry;
                }
                else if (dir_entry.find(control_box) != string::npos) {
                    Box b(0,dir_entry,occupation_time,ref_installation);
                    boxes[dir_entry] = b;
                    boxes_auxillary[dir_entry] = count;
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
    Box *targetBox = &boxes[box_name];
    cout<<"Entering "<<dir_name<<"  ->  box type "<<targetBox->type<<endl;;
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
                bool skip = false;
                //skip excel files
                if (file_name.substr(file_name.length()-3,3) == "xls")
                    skip = true;
                if (skip) continue;
                if (verbflag) cout<<file_name.c_str()<<endl;
                data_in = fopen(file_name.c_str(),"r");
		if (data_in == NULL) {
		  perror(file_name.c_str());
		  exit(1);
		}	
                data_lex();		
                counter ++;
                file_counter ++;
                /*PROCESS THE CONTENTS OF EACH DATA FILE*/
		ptime currentLatestTime = neg_infin;
                for (unsigned i=0; i<box_entries.size(); i++) {
                    /*is the given transpoder id a bat?*/                    
		    if (bats_map.find(box_entries[i].first) == bats_map.end()) {                        
                        if (find(transponders_vector.begin(),transponders_vector.end(),box_entries[i].first) == transponders_vector.end()) 
                          if (verbflag) 
			    cerr<<box_entries[i].first<<" neither a bat nor a transpoder"<<endl;
			  //unique_strings.insert(box_entries[i].first);
                        continue;
                    }
                    /**********************************/       
		    //ignore all entries with malformated recording times
                    ptime currentBoxEntryTime(not_a_date_time);
		    try {
		      currentBoxEntryTime = from_iso_string(box_entries[i].second);		      
		    }
		    catch (std::out_of_range exception_bad_datetime) {
		      //simply skip this entry
		      if (verbflag)
			cout<<"Info: Bad entry date skipped - "<<box_entries[i].first<<"\t"<<box_entries[i].second<<endl;
		      continue;      
		    }
		    /*remember the current latest time we've seen in the data stream*/
		    if (currentBoxEntryTime >= currentLatestTime)
		      currentLatestTime = currentBoxEntryTime;
		    else {
		      if (verbflag)
			cout<<"Info: Decreasing entry date skipped - "<<box_entries[i].first<<"\t"<<box_entries[i].second<<endl;
		      continue;      
		    }
		    /**/		    
		    //disregard all entries that occured AFTER the box has been occupied                    
                    if (currentBoxEntryTime <= targetBox->occupiedWhen) {
                        BatEntry newBatEntry(box_entries[i].second,box_entries[i].first,targetBox -> name);
                        pair<set<BatEntry,batEntryCompare>::iterator,bool> ret,ret1;

                        ret = targetBox->activity.insert(newBatEntry);
                        if (ret.second == false) { //if the same bat was there at the same time, increase its occurence by 1
                            BatEntry existingBatEntry = *(ret.first); //the bat who had already been insterted
                            existingBatEntry.occurence += newBatEntry.occurence;
                            targetBox->activity.erase(ret.first);
                            ret1 = targetBox->activity.insert(existingBatEntry);
                            if (ret1.second == false) { //should never happen
                                cerr<<"Warning::Duplicate times"<<endl;
                            }
                        }
                    } //end if (currentBoxEntryTime <= targetBox->occupiedWhen)
                } //end for (unsigned i=0; i<box_entries.size(); i++) {
                box_entries.clear();
                /*******************************************/
		fclose(data_in);
            } //end if           
        } //end while
        cout<<"processed "<<file_counter<<" files"<<endl;
    } // end if (data_dir != NULL) {
    closedir(data_dir);
}


/* ==================================================================== */
/* ============================   MAIN   ============================== */
/* ===                                                              === */
/* ===
 *                                                                      */
int main(int argc, char**argv) {
    argv++;
    argc--;
    /* argv[1] = config file
     */
    if (argc != 2) {
        cout<<"Version: "<<version<<endl<<endl;
        cout<<"Usage: bats <dir> <config_file>"<<endl<<endl;
        cout<<"<dir>\t full path to the directory containing transponder data files"<<endl;
        cout<<"<config_file>\t full path to the configuration file"<<endl<<endl;
        return 0;
    }
    else  { 
      /*init the months map*/
        monaten["JAN"] = "01";
        monaten["FEB"] = "02";
        monaten["MAR"] = "03";
        monaten["APR"] = "04";
        monaten["MAY"] = "05";
        monaten["JUN"] = "06";
        monaten["JUL"] = "07";
        monaten["AUG"] = "08";
        monaten["SEP"] = "09";
        monaten["OCT"] = "10";
        monaten["NOV"] = "11";
        monaten["DEC"] = "12";
        /********************/
	/*init the box occupation dates from bats_config.txt*/	
        const char* file_name = argv[1];
        config_in = fopen(file_name,"r");
        if (config_in == NULL) {
            perror(file_name);
            exit(1);
        }
        config_lex();	
	fclose(config_in);
	
        /*init the output files*/
        stringstream ss5;
	string outdir="output_files_new_2/"+Year+"_"+colony;
        ss5<<outdir<<"/lf_time_diff_"<<Year<<"_"<<roundtrip_time.minutes()<<"_"<<lf_delay.minutes()<<"_"<<occupation_deadline<<".txt";
        lf_time_diff = ss5.str();
	ss5.str(string());
        ss5<<outdir<<"/lf_valid_time_diff_"<<Year<<"_"<<roundtrip_time.minutes()<<"_"<<lf_delay.minutes()<<"_"<<occupation_deadline<<".txt";
        lf_valid_time_diff = ss5.str();
	ss5.str(string());
	ss5<<outdir<<"/lf_betweenness_preference"<<Year<<"_"<<roundtrip_time.minutes()<<"_"<<lf_delay.minutes()<<"_"<<occupation_deadline<<".txt";
	lf_pairs_valid_betweenness_preference=ss5.str();
	ss5.str(string());
	ss5<<outdir<<"/disturbed_leader"<<Year<<"_"<<roundtrip_time.minutes()<<"_"<<lf_delay.minutes()<<"_"<<occupation_deadline<<".txt";
	disturbed_leader=ss5.str();
	ss5.str(string());
        ss5<<outdir<<"/social-vs-personal-box-lf"<<Year<<"_"<<roundtrip_time.minutes()<<"_"<<lf_delay.minutes()<<"_"<<occupation_deadline<<".txt";
	social_personal_box_lf = ss5.str();
	ss5.str(string());
	ss5<<outdir<<"/bats-lead-follow-behav"<<Year<<"_"<<roundtrip_time.minutes()<<"_"<<lf_delay.minutes()<<"_"<<occupation_deadline<<".txt";
	bats_lead_follow_behav = ss5.str();
	ss5.str(string());
	ss5<<outdir<<"/most-detailed"<<Year<<"_"<<roundtrip_time.minutes()<<"_"<<lf_delay.minutes()<<"_"<<occupation_deadline<<".txt";
	most_detailed = ss5.str();
	ss5.str(string());
	ss5<<outdir<<"/revisits"<<Year<<"_"<<roundtrip_time.minutes()<<"_"<<lf_delay.minutes()<<"_"<<occupation_deadline<<".txt";	
	revisits = ss5.str();
	ss5.str(string());
	ss5<<outdir<<"/info_spread"<<Year<<"_"<<roundtrip_time.minutes()<<"_"<<lf_delay.minutes()<<".txt";	
	info_spread = ss5.str();
	ss5.str(string());
	ss5<<outdir<<"/lf_valid_time_diff_viz_"<<Year<<".json";
	lf_valid_time_diff_viz = ss5.str();
	ss5.str(string());
	ss5<<outdir<<"/time_to_occ_disturbed_"<<Year;
	time_to_occupy = ss5.str();
	ss5.str(string());
	ss5<<"output_files_new_2/combined_networks.txt";
	combined_networks = ss5.str();
	ss5.str(string());
	ss5<<outdir<<"/leading_following_statistics.dat";
	leading_following_statistics = ss5.str();	
	ss5.str(string());
	ss5<<outdir<<"/leading_following_statistics_detailed.dat";
	leading_following_statistics_detailed = ss5.str();
	ss5.str(string());
	/***********************/
        nbats = bats_map.size(); //bats_vector.size();
        ntransponders = transponders_vector.size();	
        base_dir = argv[0];
        initBoxes(base_dir);
        /*this call is in the beginning, as advised in the igraph manual*/
        igraph_i_set_attribute_table(&igraph_cattribute_table);
	/*init the random number generator*/
        srand (time(NULL));
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
            char *A = new char[128];
            char *B = new char[128];
            double x;
            int tokenize_line=sscanf(line.c_str(),"%s %s %lf",A,B,&x);
            if (tokenize_line == 3) {
                string a=A,b=B;
                stringstream ss;
                ss<<A<<B;
                relatedness_map[ss.str()]=x;              
            }
            delete [] A;
            delete [] B;
        }
      
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
        /*print the matched mothers and daughters*/
        /*
        for (multimap<string,string>::iterator tt=mother_daughter.begin(); tt != mother_daughter.end(); tt++) {
          cout<<tt->first<<" <-- "<<tt->second<<endl;
        }
        */

        DIR *dir;
        struct dirent *ent;
        string all_box("100"), majority_box("66"),minority_box("33"), control_box("0"),majority_box2("67");
	string heat_box("h");
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
		  /*the order in the if-then checks is important */
                    if (node_name.find(heat_box) != string::npos ||
                            node_name.find(majority_box)  != string::npos ||
                            node_name.find(majority_box2) != string::npos ||
                            node_name.find(minority_box)  != string::npos ||
                            node_name.find(all_box)       != string::npos ||
		            node_name.find(control_box)   != string::npos) {
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
	
	/*remove this only temp*/
	//for (set<string>::iterator ti=unique_strings.begin(); ti != unique_strings.end(); ti++)
	  //cout<<*ti<<endl;
	/**/
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

         /*fill bat_records with movement information for each bat*/        
        for (itr=multibats.begin(); itr != multibats.end(); itr++) {
            BatEntry current = *itr;
            Bat &ref = bats_records[current.hexid]; //this is init of bats_records
            ref.hexid = current.hexid;
            //which box?
            Box *box_ptr = &boxes[current.box_name];
            ref.add_movement(current.TimeOfEntry,box_ptr);
        }
        /*clean the movement history for each bat*/
	int total_readings = 0;
	for (map<string,Bat>::iterator jj=bats_records.begin(); jj!=bats_records.end();jj++) { 
	  jj->second.clean_movement_history();	  
	  total_readings += jj->second.cleaned_movement_history.size();
	}
        /**********************************************************/
	/*after all bat objects have been created update them with box programming information*/
	set<string> programmed_but_absent_from_data;
	pair<set<string>::iterator, bool> s_itr;
	for (map<string,vector<string> >::iterator itr_box_prog = box_programming.begin(); itr_box_prog != box_programming.end(); itr_box_prog++) {	 
	  string box = itr_box_prog->first;
	  for (unsigned jj=0; jj<itr_box_prog->second.size(); jj++) {
	    string programmed_bat = itr_box_prog->second[jj];	    
	    if (programmed_bat == "all") {
 	      for (map<string,Bat,bool>::iterator bitr=bats_records.begin(); bitr != bats_records.end(); bitr++) {
		mybool bool_true(TRUE); 
		bitr->second.disturbed_in_box[box] = bool_true;
	      }
	    }
	    else if (programmed_bat == "none") {
	      for (map<string,Bat,bool>::iterator bitr=bats_records.begin(); bitr != bats_records.end(); bitr++) {
		mybool bool_false(FALSE);
		bitr->second.disturbed_in_box[box] = bool_false;
	      }
	    }
	    else {
	      Bat &ref = bats_records[programmed_bat];
	      if (ref.hexid != programmed_bat) 
		s_itr = programmed_but_absent_from_data.insert(programmed_bat);					      
	      mybool &bool_ref = ref.disturbed_in_box[box];
	      bool_ref.custom_boolean = TRUE;
	    }
	  }	  
	}
	//each programmed but absent bat is reported only once
        for (set<string>::iterator s_itr2=programmed_but_absent_from_data.begin(); s_itr2 != programmed_but_absent_from_data.end();s_itr2++)
	    cout<<"Info: bat "<<*s_itr2<<" is programmed in a box but no readings exist for her before box occupation"<<endl; //exit(1);
        /***/
        /*after all box objects have been created update the box objects with occupying information*/
	for (map<string,vector<string> >::iterator itr_box_occup = box_occup_bats.begin(); itr_box_occup != box_occup_bats.end(); itr_box_occup++) {	 
	  string box = itr_box_occup->first;
	  for (unsigned jj=0; jj<itr_box_occup->second.size(); jj++) {
	    string occupying_bat = itr_box_occup->second[jj];	    
	    Bat *ptr = &bats_records[occupying_bat];
	    if (ptr->hexid != occupying_bat) {
	      cout<<"Warning: Bat "<<occupying_bat<<" occupied a box, but no readings exist for her before box occupation"<<endl; 	     
	    }
	    boxes[box].occupyingBats.push_back(ptr);
	  }	  
	}
	 /*now add detailed box occupation info to each box*/
	for (map<string,vector<pair<ptime, vector<string> > > >::iterator itr_box = occupation_history.begin(); itr_box != occupation_history.end(); itr_box++) {	 
	  string box = itr_box->first;
	  for (unsigned jj=0; jj<itr_box->second.size(); jj++) {
	    ptime p = itr_box->second[jj].first;
	    vector<Bat *> occupators;
	    for (unsigned kk=0; kk < itr_box->second[jj].second.size(); kk++) {
	      string long_hexid = short_to_long[itr_box->second[jj].second[kk]];
	      occupators.push_back(&bats_records[long_hexid]);
	    }
	    pair<ptime,vector<Bat*> > newpair(p,occupators);	    
	    boxes[box].occupationHistory.push_back(newpair);
	  }	  
	}	 
	/*******/
	/*CALCULATE THE FREQUENCY OF BOX OCCUPATION BY A DISTURBED INDIVIDUAL
	  BASED ON THE TOTAL NUMBER OF OCCUPYING INDIVIDUALS*/
	map<myint,myint> disturbed_occupation;
	for (map<string,Box>::iterator iii=boxes.begin(); iii!=boxes.end(); iii++) {	  
	  if (iii->second.type != 0) { //no control boxes
	    for (unsigned jjj=0; jjj < iii->second.occupationHistory.size(); jjj++) {
	      myint ndisturbed(0);
	      for (unsigned kkk=0; kkk < iii->second.occupationHistory[jjj].second.size(); kkk++) {
		Bat *bb = iii->second.occupationHistory[jjj].second[kkk];
		if (bb->disturbed_in_box[iii->first].custom_boolean == TRUE) {
		  ndisturbed++;		  
		}
	      }
	      disturbed_occupation[myint((int)iii->second.occupationHistory[jjj].second.size())]+=ndisturbed;
	    }	    
	  }
 	}
 	for (map<myint,myint>::iterator mitr = disturbed_occupation.begin(); mitr!=disturbed_occupation.end();mitr++) 
	    cout<<mitr->first.i<<"\t"<<mitr->second.i<<endl;
	
	
        //associate mothers to daughters
        for (map<string,unsigned int>::iterator i=bats_map.begin(); i!=bats_map.end(); i++) {
	    /*Here, if there is a bat with no records in the data from bats_map, it will be added to bats_records*/
            Bat &ref = bats_records[i->first];           
            //if no records exist for this bat then we need to init also its hexid,
            //because it would be an empty string
            ref.hexid = i->first;
            string last4letters = ref.hexid.substr(ref.hexid.length()-4,4);
            pair<multimap<string,string>::iterator,multimap<string,string>::iterator > p;
            multimap<string,string>::iterator j;
            p=mother_daughter.equal_range(last4letters);
            bool found=false;
            bool mothers_flag=true; //used to add a mother only once, when she has more than one daughters
            for (j=p.first; j != p.second; j++) {
                ref.daughters_hexids.push_back(j->second);
                if (mothers_flag) {
                    mothers.push_back(ref.hexid);
                    mothers_flag=false;
                }
                daughters.push_back(j->second);
                found=true;
            }
            if (!found) { //the bat was not found as a mother, let's see if she's someone's daughter
                for (multimap<string,string>::iterator ttt=mother_daughter.begin(); ttt!=mother_daughter.end(); ttt++) {
                    if (ttt->second == last4letters) { //yes, she is someone's daughter
                        daughters.push_back(last4letters);
                        //the mother has not been recorded
                    }
                }
            } //end if (!found)

        }

        //check to see how many of the bats who have a moving history also have mothers
        //with moving history in the data!
        unsigned total_bats_movinghistory = 0,bats_with_mothers=0;
        for (map<string,unsigned int>::iterator j=bats_map.begin(); j!=bats_map.end(); j++) {
            Bat &ref = bats_records[j->first];
            //for (unsigned j=0; j<nbats; j++) {
            //Bat &ref = bats_records[bats_vector[j]];
            if (ref.movement_history.size() > 0) {
                total_bats_movinghistory++;
                string last4letters = ref.hexid.substr(ref.hexid.length()-4,4);
                if (find(daughters.begin(),daughters.end(),last4letters) != daughters.end()) {
                    //found a daughter, let's find who the mother is
                    for (unsigned k=0; k<mothers.size(); k++) {
                        string momlast4letters = mothers[k].substr(mothers[k].length()-4,4);
                        pair<multimap<string,string>::iterator,multimap<string,string>::iterator > p;
                        multimap<string,string>::iterator l;
                        p=mother_daughter.equal_range(momlast4letters);
                        for (l=p.first; l!=p.second; l++) { //found the same mother with several daughters
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
            if (mother_itr == mothers.end() && daughters_itr == daughters.end() && verbflag) {
                cout<<bat<<" not found as either mother or daughter in the mother-daughter file"<<endl;
            }
        }

        multiset<BatEntry,batEntryCompare>::iterator from,to,cur;
        from=multibats.begin();
        Box *bx = &boxes[multibats.begin()->box_name]; //get the first box in multibats
        //store all pairs of experience and naiive bats
        vector<Lf_pair> vec_lfpairs;

        //IMPORTANT: DO THE ANALYSIS ONE BOX AT A TIME!!!!
	//ss_test used for testing 
	stringstream ss_test;
	ss_test<<outdir<<"/test"<<Year;
	ofstream os_test(ss_test.str().c_str(),ios::out);
	if (!os_test.good()) {
	  perror(ss_test.str().c_str());
	  exit(1);
	}	
        for (to=multibats.begin(); to != multibats.end(); to++) {
            //find the boundaries of a box                     
            vector<BatEntry> box_bat_entries; //stores all bat_entries for each box
            map<string,ptime> lastSeen; //when was a given bat_id last recorded in the data
            vector<Lf_pair> current_box_pairs;
            while (to!=multibats.end() && to->box_name == bx->name) to++;
            //now "to" points to the first entry of the new box
            cur=from;
            while (cur != to) {
                box_bat_entries.push_back(*cur);
                cur++;
            }
            /*main pairing loop. */
	    bx->status = DISCOVERED;
	    bx->discovered(box_bat_entries[0].hexid,box_bat_entries[0].TimeOfEntry);
	    bx->information_spread.push_back(event("discovery",&bats_records[box_bat_entries[0].hexid],bx,
						   box_bat_entries[0].TimeOfEntry));	  
            //==================================================================
            /*initialize the map for storing the exploration events.
	     the map gets reset with each box
	     bat hexid -> exploration object*/
	    map<string,event> exploration_objects;
	    //int counter2 = 0;	   
            for (unsigned i=0; i<box_bat_entries.size(); i++) {
                box_bat_entries[i].print(&os_test);                
                Bat *B1 = &bats_records[box_bat_entries[i].hexid];				
		 ptime &first_reading = B1->first_reading[bx->name];
		 if (first_reading.is_not_a_date_time())
		   first_reading = box_bat_entries[i].TimeOfEntry;
		/***/
		ptime &b1_ref = lastSeen[box_bat_entries[i].hexid];
		B1->last_seen = box_bat_entries[i].TimeOfEntry;
		BatKnowledge &b1_know_ref = B1->box_knowledge[bx->name];		
		if (b1_know_ref.box_knowledge_how == UNDEFINED) 
		  b1_know_ref.box_knowledge_how = PERSONAL;
		time_duration t_d(1,0,0,0); //1 hour, dummy value
		if (!b1_ref.is_not_a_date_time()) { //seen it before
		  t_d =  box_bat_entries[i].TimeOfEntry - b1_ref;		  
		  if (t_d > roundtrip_time && !B1->is_informed(bx->name,box_bat_entries[i].TimeOfEntry)) {		    
		    b1_know_ref.box_knowledge = EXPERIENCED;		     
		    B1->make_informed(bx->name,box_bat_entries[i].TimeOfEntry);
		  }
		}
		/*create a provisional exploration object, but NOT for the discovering bat*/
		if (B1->hexid != bx->discoveredBy.first) {
		  event &exploration_ref = exploration_objects[B1->hexid];
		  //if the bat is NAIIVE and no exploration object exists yet then create it		
		  if (!B1->is_informed(bx->name,box_bat_entries[i].TimeOfEntry) && exploration_ref.eventname == "") 
		    exploration_ref = event("exploration",B1,bx,box_bat_entries[i].TimeOfEntry);		
		}
		/**/
                b1_ref = box_bat_entries[i].TimeOfEntry;
                for (unsigned j=(i+1); j<box_bat_entries.size(); j++) {		   
                    Bat *B2 = &bats_records[box_bat_entries[j].hexid];                  
		    BatKnowledge &b2_know_ref = B2->box_knowledge[bx->name];
		    if (b2_know_ref.box_knowledge_how == UNDEFINED)
		      b2_know_ref.box_knowledge_how = PERSONAL;
                    if (*B1 == *B2) {
		      //could be a revisit
		      time_duration tdur = box_bat_entries[j].TimeOfEntry - B2->last_seen;
		      if (tdur > revisit_interval.first) {//&& tdur <= revisit_interval.second) {
 			ptime &lr = B2->last_revisit[bx->name];			
			if (lr.is_not_a_date_time()) lr = neg_infin;
			time_duration time_since_last_revisit = box_bat_entries[j].TimeOfEntry - lr;//B2->last_revisit;
			if (time_since_last_revisit > revisit_interval.first) {
			  event new_revisit("revisit",B2,bx,box_bat_entries[j].TimeOfEntry);
			  B2->my_revisits.push_back(new_revisit);
			  bx->revisiting_bats.push_back(new_revisit);
			  /*B2->last_revisit*/ lr = box_bat_entries[j].TimeOfEntry;
			  bx->information_spread.push_back(new_revisit);
			}
		      }
		      B2->last_seen = box_bat_entries[j].TimeOfEntry;
                      continue; 
		    }
                    //first update this bat's knowledge about the given box, if necessary
                    ptime &ref = lastSeen[B2->hexid];
                    if (ref.is_not_a_date_time()) 
		      ref = pos_infin;
                    time_duration td_update_knowledge = box_bat_entries[j].TimeOfEntry - ref;                    
		    if (td_update_knowledge > roundtrip_time && !B2->is_informed(bx->name,box_bat_entries[j].TimeOfEntry)) {
			b2_know_ref.box_knowledge = EXPERIENCED;			
                        B2->make_informed(bx->name,box_bat_entries[j].TimeOfEntry);
                    }
                    ref = box_bat_entries[j].TimeOfEntry;               
                    //are both bats informed or are both naiive about that box at this time?
                    if ((!B1->is_informed(bx->name,box_bat_entries[i].TimeOfEntry) && !B2->is_informed(bx->name,box_bat_entries[j].TimeOfEntry)) ||
                            (B1->is_informed(bx->name,box_bat_entries[i].TimeOfEntry) && B2->is_informed(bx->name,box_bat_entries[j].TimeOfEntry)))
                        continue;
                    //if the pair is suitable, materialize it only if the time distance between B1 and B2 allows it
                    time_duration tdur= b1_ref -ref;
		    if (tdur.is_negative()) tdur=tdur.invert_sign();
		    if (tdur > lf_delay) continue; //skip the pair if invalid
		    /*both bats B1 and B2 can now be considered to have taken part in an lf-event*/
		    B1->part_of_lf_event = true;
		    B2->part_of_lf_event = true;
	            Lf_pair newPair;
                    if (B1->is_informed(bx->name,b1_ref)) {
			b2_know_ref.box_knowledge_how = SOCIAL;		
                        newPair.init(B1,B2,b1_ref,ref,bx->name);			
			mybool &bool_ref = B1->disturbed_in_box[bx->name];
			if (bool_ref.custom_boolean == UNINITIALIZED) {
			  cerr<<"Error: Something went wrong with reading the box programming information"<<endl;
			  cerr<<"Additional info: box "<<bx->name<<" bat: "<<B1->hexid<<endl;
			  exit(1);
			}
			if (bool_ref.custom_boolean)
			  newPair.leader_disturbed = true;
			//is this passive leading?
			if (t_d > revisit_interval.first) //&& t_d <= revisit_interval.second) //is this lf event passive leading?			 
			  newPair.is_passive_leading = true;			
			bool insert_success = B1->insert_pair(newPair);			
			if (insert_success) {			  
			  LF_FLAG lf;
			  if (b1_know_ref.box_knowledge_how == PERSONAL && !bool_ref.custom_boolean) {
			    bx->personal_ud_lf_events++;lf.this_lf_flag=PERSONAL_UNDISTURBED;
			  }
			  else if (b1_know_ref.box_knowledge_how == PERSONAL && bool_ref.custom_boolean) {
			    bx->personal_d_lf_events++;lf.this_lf_flag=PERSONAL_DISTURBED;
			  }
			  else if (b1_know_ref.box_knowledge_how == SOCIAL && !bool_ref.custom_boolean) {
			    bx->social_ud_lf_events++;lf.this_lf_flag=SOCIAL_UNDISTURBED;
			  }
 			  else if (b1_know_ref.box_knowledge_how == SOCIAL && bool_ref.custom_boolean) {
			    bx->social_d_lf_events++;lf.this_lf_flag=SOCIAL_DISTURBED;
			  }
			  else {
			    cerr<<"Sanity check failed: Box UNINITIALIZED"<<endl;
			    exit(1);
			  }
			  bx->total_lf_events++;
			  pair<unsigned,LF_FLAG> &lf_ref=bx->lf_events[newPair/*B1->hexid*/];
			  lf_ref.second.this_lf_flag = lf.this_lf_flag;
			  if (lf_ref.second.this_lf_flag == UNINIT) {lf_ref.first=1;}
			  else lf_ref.first++;
			}
                    }
                    else if (B2->is_informed(bx->name,ref)) {			
			b1_know_ref.box_knowledge_how = SOCIAL;
                        newPair.init(B2,B1,ref,b1_ref,bx->name);						
			
			mybool &bool_ref = B2->disturbed_in_box[bx->name];
			if (bool_ref.custom_boolean == UNINITIALIZED) {
			  cerr<<"Error: Something went wrong with reading the box programming information!"<<endl;
			  cerr<<"Additional info: box "<<bx->name<<" bat: "<<B2->hexid<<endl;
			  exit(1);
			}
			if (bool_ref.custom_boolean)
			  newPair.leader_disturbed = true;
			//is this passive leading following?
                        //if (td_update_knowledge > revisit_interval.first)// && td_update_knowledge <= revisit_interval.second)						
			  //newPair.is_passive_leading = true;
			bool insert_success = B2->insert_pair(newPair);
			if (insert_success) {			  			
			  LF_FLAG lf;
			  if (b2_know_ref.box_knowledge_how == PERSONAL && !bool_ref.custom_boolean) {
			    bx->personal_ud_lf_events++;lf.this_lf_flag=PERSONAL_UNDISTURBED;
			  }
			  else if (b2_know_ref.box_knowledge_how == PERSONAL && bool_ref.custom_boolean) {
			    bx->personal_d_lf_events++;lf.this_lf_flag=PERSONAL_DISTURBED;
			  }
			  else if (b2_know_ref.box_knowledge_how == SOCIAL && !bool_ref.custom_boolean) {
			    bx->social_ud_lf_events++;lf.this_lf_flag=SOCIAL_UNDISTURBED;
			  }
 			  else if (b2_know_ref.box_knowledge_how == SOCIAL && bool_ref.custom_boolean) {
			    bx->social_d_lf_events++;lf.this_lf_flag=SOCIAL_DISTURBED;
			  }
			  else {
			    cerr<<"Sanity check failed: Box UNINITIALIZED"<<endl;
			    exit(1);
			  }
			  bx->total_lf_events++;
			  pair<unsigned,LF_FLAG> &lf_ref=bx->lf_events[newPair/*B2->hexid*/];
			  lf_ref.second.this_lf_flag = lf.this_lf_flag;
			  if (lf_ref.second.this_lf_flag == UNINIT) {lf_ref.first=1;}			  
			  else lf_ref.first++;
			   
			}
                    }
                    else cout<<"Warning::Sanity checks failed"<<endl;
                    //check if the pair already exists
                    bool exists = false;
		    for (unsigned k=0; k<current_box_pairs.size(); k++) {
                        if (current_box_pairs[k].equals(newPair)) {
                            //if it exists, update the current pair but only if the new pair has lower
                            //time difference between leader and follower
                            if (newPair.get_lf_delta() < current_box_pairs[k].get_lf_delta())
                                current_box_pairs[k]=newPair;
                            exists=true;
                            break;
                        }
                    }
                   //if it doesn't exists simply add it
                   if (!exists) 
                       current_box_pairs.push_back(newPair);		   
		   //add the leader and the follower of this player to the box objects		       
		   pair<set<string>::iterator,bool> set_iterator;
		   set_iterator = bx->leaders.insert(newPair.getLeaderId()); 
		   set_iterator = bx->followers.insert(newPair.getFollowerId());                         
                } //end for (unsigned j=i; j<box_bat_entries.size(); j++)
            } //end for (unsigned j=i; j<box_bat_entries.size(); j++) {
            //==================================================================
            /*check if any of the candidate explorations are in fact following bats in lf events*/
	    vector<string> tmp; //stores the hexids of the bats that need to be removed from the exploration_objects map
	    for (map<string,event>::iterator evnt_itr = exploration_objects.begin(); evnt_itr != exploration_objects.end(); evnt_itr++) {
		string cur_bat = evnt_itr->first;
		bool found = false;
		for (unsigned kk=0; kk<current_box_pairs.size(); kk++) {
		  if (cur_bat == current_box_pairs[kk].follower->hexid)
		    found = true;
		}
		if (found)
		  tmp.push_back(cur_bat);
	    }
	    //now remove the bats who did not explore but were following
	    for (unsigned kk=0; kk<tmp.size(); kk++) 
	      exploration_objects.erase(tmp[kk]);
	    //now assign the remaining exploration objects to the map's information_spread container
	     for (map<string,event>::iterator evnt_itr = exploration_objects.begin(); evnt_itr != exploration_objects.end(); evnt_itr++) 
	       bx->information_spread.push_back(evnt_itr->second);	     
	    /***/  
	    /*now invalidate all revisits which are in fact leading following*/
	    for (unsigned kk=0; kk<bx->information_spread.size();kk++) {
	      if (bx->information_spread[kk].eventname == "revisit") {
		ptime revisit_time = bx->information_spread[kk].eventtime;
		for (unsigned ll=0; ll<current_box_pairs.size(); ll++) {
		  ptime leader_time = current_box_pairs[ll].tleader;
		  time_duration leading_revisit = leader_time - revisit_time;
		  if (leading_revisit.is_negative()) 
		    leading_revisit = leading_revisit.invert_sign();
		  if (leading_revisit <= revisit_limit)
		    bx->information_spread[kk].valid = false;
		}
	      }
	    }
	    /*now take all lf events and create event objects*/
	    for (unsigned kk=0; kk<current_box_pairs.size(); kk++) {  
		Bat *bat =  &bats_records[current_box_pairs[kk].getFollowerId()];
		Bat *fuehrer = &bats_records[current_box_pairs[kk].getLeaderId()];
		if (bat->disturbed_in_box[bx->name].custom_boolean == TRUE) {
		  event leading_following("disturbed-following",bat,bx,current_box_pairs[kk].tfollower,fuehrer);
		  bx->information_spread.push_back(leading_following);	     
		}
		else {
		  event leading_following("undisturbed-following",bat,bx,current_box_pairs[kk].tfollower,fuehrer);
		  bx->information_spread.push_back(leading_following);	     		  
		}
	    }
	    /**/
            vec_lfpairs.insert(vec_lfpairs.end(),current_box_pairs.begin(),current_box_pairs.end());
            to--; //go back to the last entry of the previous box_entries
            from=to;
            from++; //move the "from" pointer to the first entry of the new box
            if (from == multibats.end()) break;
	    bx = &boxes[from->box_name];
            //bx->name = from->box_name; //THIS IS VERY VERY WRONG 
	    
        } //end for (to=multibats.begin(); to != multibats.end(); to++)
		
	/****/
	cout<<"Total readings:\t"<<total_readings<<endl;
	
	/*normalize each bats' activity'*/
	
	for (map<string,Bat>::iterator i=bats_records.begin(); i!=bats_records.end(); i++) {
	    Bat *b = & i->second;
      	      cout<<b->hexid<<"\t"<<b->cleaned_movement_history.size()<<"\t"<<b->movement_history.size()<<"\t";
	      double percentage = (double)b->cleaned_movement_history.size() / (double)total_readings;	    
	      printf("%.15g\n", percentage);   
	}
        
	/****/
	/*
	for (map<string,Bat>::iterator i=bats_records.begin(); i!=bats_records.end(); i++) {
	    Bat *b = & i->second;
	    cout<<b->hexid<<endl;
	    for (map<string,ptime>::iterator j=b->first_reading.begin(); j!=b->first_reading.end(); j++) {
	      cout<<j->first<<"\t"<<to_simple_string(j->second)<<endl;
	    }
	}
	*/
	/****/
	cout<<"Outputting revisit statistics for each box...";
	ofstream os(revisits.c_str(),ios::out);
	if (!os.good()) {
            perror(revisits.c_str());
            exit(1);
        }
        for (map<string,Box>::iterator box_itr=boxes.begin(); box_itr!=boxes.end(); box_itr++) {
	  Box *b = &box_itr->second;
	  os<<b->name<<" "<<endl;
	  for (unsigned i=0; i < b->revisiting_bats.size(); i++) {
	    os<<b->revisiting_bats[i].bat->hexid<<"\t";
	    os<<to_simple_string(b->revisiting_bats[i].eventtime)<<endl;
	  }
	}
        os.close();
	cout<<"DONE"<<endl;	
	cout<<"Outputting detailed box statistics...";
	os.open(most_detailed.c_str(),ios::out);
	if (!os.good()) {
            perror(most_detailed.c_str());
            exit(1);
        }
	for (map<string,Box>::iterator box_itr=boxes.begin(); box_itr!=boxes.end(); box_itr++) {
	  Box *b = &box_itr->second;
	  os<<b->name<<" ";
	  int n_passive_lf = 0;
	  for (map<Lf_pair, pair<unsigned,LF_FLAG> >::iterator lf_itr=b->lf_events.begin(); lf_itr!=b->lf_events.end();lf_itr++) {	    
	    os<<lf_itr->first.leader->hexid<<"/"<<lf_itr->second.first<<"/"<<lf_itr->second.second.this_lf_flag<<"/";
	    os<<lf_itr->first.follower->hexid<<"/"<<((lf_itr->first.is_passive_leading) ? "1":"0")<<"\t";
	    if (lf_itr->first.is_passive_leading)
	      n_passive_lf++;
	  }
	  os<<box_itr->second.personal_d_lf_events+box_itr->second.personal_ud_lf_events<<"\t";
	  os<<box_itr->second.social_d_lf_events+box_itr->second.social_ud_lf_events<<"\t"<<box_itr->second.total_lf_events<<"\t";
	  if (box_itr->second.getOccupiedDiscoveredDelta().is_pos_infinity()) 
	    os<<"NA";
	  else
	    os<<box_itr->second.getOccupiedDiscoveredDelta().total_seconds() / 3600.0;	  
	  os<<"\t"<<(box_itr->second.total_lf_events == 0 ? 0.0 : float(n_passive_lf) / float(box_itr->second.total_lf_events))<<"\t"<<float(box_itr->second.total_lf_events);
	  os<<endl;
	}
	os.close();
	cout<<"DONE"<<endl;	
	
	/*output the lf behaviour of each bat for each of her lf events*/
	cout<<"Outputting lf behaviour for each bat for each of her lf events...";	
	os.open(bats_lead_follow_behav.c_str(),ios::out);	
	if (!os.good()) {
	  perror(bats_lead_follow_behav.c_str());
	  exit(1);
	}	
	for (map<string,Bat>::iterator b_itr=bats_records.begin(); b_itr!=bats_records.end(); b_itr++) {
	  for (unsigned k=0; k<b_itr->second.my_lfpairs.size(); k++) {
	    Lf_pair *lf_ptr = &b_itr->second.my_lfpairs[k];
	    if (!b_itr->second.hexid.compare("0000641775")) lf_ptr->print(&os_test);
	    //sanity check
	    if (b_itr->second.hexid.compare(lf_ptr->getLeaderId()))  //not equal if true
	     {cerr<<"Sanity checks failed: Bats ids do not match"<<endl; exit(1);}
	    os<<b_itr->second.hexid<<" "<<to_iso_extended_string(lf_ptr->tleader)<<" ";
	    //LF_FLAG my_lf_flag;
	    if (!lf_ptr->leader_disturbed && 
		b_itr->second.box_knowledge[lf_ptr->box_name].box_knowledge_how == PERSONAL)
		  os<<PERSONAL_UNDISTURBED<<endl;
	    else if (lf_ptr->leader_disturbed && 
		 b_itr->second.box_knowledge[lf_ptr->box_name].box_knowledge_how == PERSONAL)
		  os<<PERSONAL_DISTURBED<<endl;
	    else if (!lf_ptr->leader_disturbed && 
		 b_itr->second.box_knowledge[lf_ptr->box_name].box_knowledge_how == SOCIAL)
		  os<<SOCIAL_UNDISTURBED<<endl;
	    else if (lf_ptr->leader_disturbed && 
		 b_itr->second.box_knowledge[lf_ptr->box_name].box_knowledge_how == SOCIAL)
		  os<<SOCIAL_DISTURBED<<endl;
	    else {cerr<<"Sanity checks failed: impossible combination"<<endl;}	
	  }
	}
	os.close();	
	cout<<"DONE"<<endl;
	os_test.close();
	/*** OUTPUT THE ASSOCIATION BETWEEN OCUPPYING AND LEADING/FOLLOWING BATS****/
	cout<<"Identifying who the occupying bats are...";
	os_test.open(ss_test.str().c_str(),ios::app);
	os_test<<endl<<endl;
	os_test<<"Box\t#occ dist.leaders(all leaders)\t#occ undist leaders(all leaders)\t#occ. dist.followers(all followers)\t#occ. undist.followers(all followers)\t#naive\t#experienced"<<endl;
	for (map<string,Box>::iterator box_itr=boxes.begin(); box_itr!=boxes.end(); box_itr++) {
	  os_test<<box_itr->second.name<<"\t\t"<<box_itr->second.howmany_leaders_followers()<<endl;
	}
	cout<<"DONE"<<endl;
	os_test.close();
	/*****/
	/*output the information spread for each box*/
	cout<<"Outputting information spread per box ...";	
	os.open(info_spread.c_str(),ios::out);
	if (!os.good()) {
            perror(info_spread.c_str());
            exit(1);
        } 
	for (map<string,Box>::iterator box_itr=boxes.begin(); box_itr!=boxes.end(); box_itr++) {
	  box_itr->second.sort_information_spread();
	  box_itr->second.clean_information_spread();
	  for (unsigned jj=0; jj<box_itr->second.information_spread.size();jj++) 
	    if (box_itr->second.information_spread[jj].valid)
	      box_itr->second.information_spread[jj].print(&os);	
	}	
        os.close();
	cout<<"DONE"<<endl;
	/****/
	/*output the leader_disturbed flag for all valid pairs*/
	os.open(disturbed_leader.c_str(),ios::out);
	if (!os.good()) {
            perror(lf_time_diff.c_str());
            exit(1);
        }
        cout<<"Outputting distrubed/undisturbed status for leaders in all valid lf events...";
        for (unsigned i=0; i<vec_lfpairs.size(); i++) {
            if (vec_lfpairs[i].valid) {
		if (vec_lfpairs[i].leader_disturbed)
		  os<<1<<endl;
		else
		  os<<0<<endl;
	    }
	}   
	os.close();
	cout<<"DONE"<<endl;
        /*output all matched pairs*/
	/*
        os.open(lf_time_diff.c_str(),ios::out);
        if (!os.good()) {
            perror(lf_time_diff.c_str());
            exit(1);
        }
        for (unsigned i=0; i<vec_lfpairs.size(); i++)
	    vec_lfpairs[i].print(&os);
            //os<<to_simple_string(vec_lfpairs[i].get_lf_delta())<<endl;
        os.close();
	*/
        //output only valid pairs, i.e. those which respect the max. allowed delay b/n leader and follower
        os.open(lf_valid_time_diff.c_str(),ios::out);
        if (!os.good()) {
            perror(lf_valid_time_diff.c_str());
            exit(1);
        }
        cout<<"Outputting all valid lf pairs...";
        for (unsigned i=0; i<vec_lfpairs.size(); i++) {
            if (vec_lfpairs[i].valid) 
	        vec_lfpairs[i].print(&os);
                //os<<to_simple_string(vec_lfpairs[i].get_lf_delta())<<endl;
            
        }
        os.close();
	cout<<"DONE"<<endl;
	/*outputting all valid pairs into a form suitable for vizualization*/
	os.open(lf_valid_time_diff_viz.c_str(),ios::out);
        if (!os.good()) {
            perror(lf_valid_time_diff_viz.c_str());
            exit(1);
        }
        cout<<"Outputting all valid lf pairs in a json file for vizualization...";
        os<<"["<<endl;
	unsigned tmp = 0;
	for (map<string,Bat>::iterator b_itr=bats_records.begin(); b_itr!=bats_records.end(); b_itr++) {
	  os<<"{\"name\":\""<<b_itr->second.hexid.substr(b_itr->second.hexid.size()-4,4)<<"\""<<",\"imports\":[";
	  for (unsigned k=0; k<b_itr->second.my_lfpairs.size(); k++) {
	    Lf_pair *lf_ptr = &b_itr->second.my_lfpairs[k];
	    os<<"\""<<lf_ptr->getFollowerId().substr(lf_ptr->getFollowerId().size()-4,4)<<"\"";
	    if (k != (b_itr->second.my_lfpairs.size()-1))
	      os<<",";
	  }
	  os<<"]}";
	  if (++tmp != bats_records.size())
	    os<<",";
	  os<<endl;
	}	
	os<<"]"<<endl;
	os.close();
	cout<<"DONE"<<endl;
	/*output all valid lf pairs, sorted by time of recording the leader. used as input to plot betweenness preference*/
	sort(vec_lfpairs.begin(),vec_lfpairs.end(),Lf_pair_compare);
	os.open(lf_pairs_valid_betweenness_preference.c_str(),ios::out);
        if (!os.good()) {
            perror(lf_pairs_valid_betweenness_preference.c_str());
            exit(1);
        }
        cout<<"Outputting all valid lf pairs reformatted for calculating betweenness preference...";
        os<<"leader follower time_leader time_follower box_name leader-knows-how leader-knowledge"<<endl;
	for (unsigned i=0; i<vec_lfpairs.size(); i++) {
            if (vec_lfpairs[i].valid) {
		vec_lfpairs[i].print(&os);                
            }
        }
	os.close();
	cout<<"DONE"<<endl;
	/***********/
	//cout<<"Outputting time to occupation of all bats...";
	//os.open(time_to_occupy.c_str());
	//if (!os.good()) {
        //    perror(time_to_occupy.c_str());
        //    exit(1);
        //}        
	//for (map<string,Box>::iterator i=boxes.begin(); i!=boxes.end(); i++) {
	//  Box *b = & i->second;
	//  for (unsigned j=0; j < b->occupyingBats.size(); j++) {
	//    double td;
	//    Bat *bb = b->occupyingBats[j];
	//    tm occupying_time = to_tm(b->occupiedWhen);	    
	//    if (!bb->first_reading[b->name].is_not_a_date_time()) {
	//      tm first_reading = to_tm(bb->first_reading[b->name]);	    
	//      //time_duration td = b->occupiedWhen - bb->first_reading[b->name];
	//      td = difftime(mktime(&occupying_time),mktime(&first_reading)) / 60.0; //in minutes
	//    }
	//    else td = -2;
	//    os<<bb->hexid<<"\t"<<td<<"\t"<<bb->disturbed_in_box[i->first].custom_boolean<<"\t"<<b->name<<endl;
	//  }	  
	//}
	//cout<<"DONE"<<endl;
	//os.close();
	/*************/
	
	/***********/
	/*first populate each bat object with the boxes she has occuppied*/
        for (map<string,Box>::iterator i=boxes.begin(); i!=boxes.end(); i++) {
	  Box *B = &(i->second);
	  for (unsigned j=0; j<B->occupyingBats.size(); j++) 	     
	     B->occupyingBats[j]->occuppied_boxes.push_back(i->first);  
	  
	}
        int led_occuppied=0, led_not_occuppied=0,not_led_not_occuppied=0,not_led_occuppied=0;
        for (map<string,Bat>::iterator i=bats_records.begin(); i!=bats_records.end(); i++) {
	  Bat *b = &(i->second);
	  for (map<string,Box>::iterator j=boxes.begin(); j!=boxes.end(); j++) {
	    if (j->second.occupiedWhen.is_pos_infinity()) //box never occuppied
	      continue;	    
	    short bat_status = b->get_lead_occuppied_status(j->first);
	    if (bat_status == 1)
	      led_occuppied++;
	    else if (bat_status == 2)
	      led_not_occuppied++;
	    else if (bat_status == 3)
	      not_led_not_occuppied++;
	    else if (bat_status == 4)
	      not_led_occuppied++;
	    else 
	      cerr<<"Error in parse.cpp - Unrecognised return value from Bat.get_lead_occuppied_status()"<<endl;	    
	  }	  
	}
      	cout<<"Outputting leading and occupying statistics ...";
	os.open(leading_following_statistics.c_str());
	if (!os.good()) {
            perror(leading_following_statistics.c_str());
            exit(1);
        }
        os<<"led_occuppied\t"<<led_occuppied<<endl;
	os<<"led_not_occuppied\t"<<led_not_occuppied<<endl;
	os<<"not_led_not_occuppied\t"<<not_led_not_occuppied<<endl;
	os<<"not_led_occuppied\t"<<not_led_occuppied<<endl;
	os.close();
	os.open(leading_following_statistics_detailed.c_str());
	if (!os.good()) {
            perror(leading_following_statistics_detailed.c_str());
            exit(1);
        }
        os<<"badid    \tled_occuppied  led_not_occuppied  not_led_not_occuppied  not_led_occuppied"<<endl;
        for (map<string,Bat>::iterator i=bats_records.begin(); i!=bats_records.end(); i++) {
	  int led_occuppied2=0, led_not_occuppied2=0,not_led_not_occuppied2=0,not_led_occuppied2=0;
          Bat *b = &(i->second);
	  for (map<string,Box>::iterator j=boxes.begin(); j!=boxes.end(); j++) {
	    if (j->second.occupiedWhen.is_pos_infinity()) //box never occuppied
	      continue;
	    short bat_status = b->get_lead_occuppied_status(j->first);
	    if (bat_status == 1)
	      led_occuppied2++;
	    else if (bat_status == 2)
	      led_not_occuppied2++;
	    else if (bat_status == 3)
	      not_led_not_occuppied2++;
	    else if (bat_status == 4)
	      not_led_occuppied2++;
	    else 
	      cerr<<"Error in parse.cpp - Unrecognised return value from Bat.get_lead_occuppied_status()"<<endl;	    
	  }
	  os<<b->hexid<<"\t\t"<<led_occuppied2<<"\t\t"<<led_not_occuppied2<<"\t\t"<<not_led_not_occuppied2<<"\t\t\t"<<not_led_occuppied2<<endl;
	}
        
	cout<<"DONE"<<endl;
	os.close();
	/*************/
	
	/*****************************************************/
        /*store the number of leading events that a leader has taken part of,
        regardless the number of following individuals*/
        map<string,unsigned> bats_lead_stats;
        /*store the number of times a bat has followed*/
        map<string,unsigned> bats_follow_stats;

	/*compute the nodes sizes and colors, and the edge thickness and colors for the graph, based on the detected lf_pairs*/
	
	//maps the bat_id from bats_map to a consequtive range of ids for lf_adjmatrix
        //we need this if the bats with recordings are fewer than the total #of bats
	map<unsigned,unsigned> bat_id2matrix_id; 
        
        /*stores a directed edge between follower and leader*/
	//maps an edge to its width. an edge is a pair FROM and TO.
        map<pair<unsigned,unsigned>,double,bool(*)(pair<unsigned,unsigned>,pair<unsigned,unsigned>) > edges(fn_pt3); 
	
        map<int,unsigned> nodes; //maps a node_id to its size
        //double starting_node_size = 9.0;
        double edges_starting_width = 1.0;
        int total_valid_pairs = 0;
        unsigned consequtive_ids=0;
	
	/*count how many bats have been part of lf events*/
	unsigned int total_bats_in_lf_events = 0;
	for (map<string,Bat>::iterator i=bats_records.begin(); i != bats_records.end(); i++) 
	  if (i->second.part_of_lf_event)
	    total_bats_in_lf_events++;
        /*create an empty directed igraph with all bats and no edges yet*/
        igraph_matrix_t lf_adjmatrix; //square matrix
        igraph_matrix_init(&lf_adjmatrix,total_bats_in_lf_events,total_bats_in_lf_events); //init the square matrix
        igraph_matrix_null(&lf_adjmatrix);
        //igraph_vector_t final_page_rank;
	
	/*create edges*/
	//os.open(combined_networks.c_str(),ios::app);
	//if (!os.good()) {
          //  perror(combined_networks.c_str());
           // exit(1);
        //}   
        for (unsigned i=0; i<vec_lfpairs.size(); i++) {
            Lf_pair lfpair = vec_lfpairs[i];
            if (lfpair.valid) { //only valid pairs
	        total_valid_pairs++;
                Bat *leader = &bats_records[lfpair.getLeaderId()];
                leader->total_following++; //one more bat has followed me
                string leader_hexid = lfpair.getLeaderId(),follower_hexid=lfpair.getFollowerId();
                bats_lead_stats[leader_hexid]++;
                bats_follow_stats[follower_hexid]++;
                /*map the bats_id from bats_map to consequtive id for lf_adjmatrix*/
                if (bat_id2matrix_id.find(bats_map[leader_hexid]) == bat_id2matrix_id.end()) 
                    bat_id2matrix_id[bats_map[leader_hexid]] = consequtive_ids++;		    
		
                if (bat_id2matrix_id.find(bats_map[follower_hexid]) == bat_id2matrix_id.end()) 
                    bat_id2matrix_id[bats_map[follower_hexid]] = consequtive_ids++;		
                /*****************/
                /*Is the follower a daughter of the leader?*/
                //always get the last 4 letters of the leaders hexid, because that's what stored in mother_daughter multimap
                string last4letters_leader = leader_hexid.substr(leader_hexid.length()-4,4);
                //check if the follower is a daugther of the leader
                //first get all children of the mother = lfpair.leader.hexid
                pair<multimap<string,string>::iterator,multimap<string,string>::iterator > p;
                multimap<string,string>::iterator k;
                p = mother_daughter.equal_range(last4letters_leader);
                string last4letters_follower = lfpair.getFollowerId();
                last4letters_follower = last4letters_follower.substr(last4letters_follower.length()-4,4);
                //now check if any of the children match our follower
                for (k=p.first; k!= p.second; k++) {
                    if (k->second == last4letters_follower) {
                        leader->n_daughters_following++; //one more daughter has followed
                    }
                }
                /***********************************/
                /*Is the follower related to the leader?*/
                stringstream ss2(stringstream::in | stringstream::out);
                //encode a relatedness pair as encoded in relatedness_map
                ss2<<leader_hexid.substr(leader_hexid.length()-4,4);
                ss2<<follower_hexid.substr(follower_hexid.length()-4,4);
                string leader_follower_order = ss2.str();
                //reverse the order now
                stringstream ss3(stringstream::in | stringstream::out);                
		ss3<<follower_hexid.substr(follower_hexid.length()-4,4);
                ss3<<leader_hexid.substr(leader_hexid.length()-4,4);		
                string follower_leader_order = ss3.str();
                //find the relatedness between these two individuals
                map<string,double>::iterator related_itr;
                related_itr = relatedness_map.find(follower_leader_order);
                if (related_itr == relatedness_map.end()) { //try the other way around
                    related_itr = relatedness_map.find(leader_follower_order);
                    if (related_itr != relatedness_map.end())
                        leader->cumulative_relatedness += related_itr->second; //found it
                    else {
		      if (verbflag)
			cout<<"Warning: No relatedness data between "<<leader_hexid<<" and "<<follower_hexid<<endl;
		    }
                }
                else
                    leader->cumulative_relatedness += related_itr->second; //found it
                /*******************************************/
                /*create an edge*/
                pair<unsigned,unsigned> edge_pair(bat_id2matrix_id[bats_map[follower_hexid]],bat_id2matrix_id[bats_map[leader_hexid]]);
                if (edges.find(edge_pair) == edges.end()) //edge does not exist
                    edges[edge_pair] = edges_starting_width;
                else 
                    edges[edge_pair]++ ; //increase the width linearly                   
                /**********************/
		/*add/update the edge to the adjacency matrix*/
		MATRIX(lf_adjmatrix,bat_id2matrix_id[bats_map[follower_hexid]], bat_id2matrix_id[bats_map[leader_hexid]])++;		
		//os<<follower_hexid<<"\t"<<leader_hexid<<"\t"<<Year<<endl;
            }
        }
        //os.close();
	
        /****************************************/	
	myigraph my_graph(&lf_adjmatrix);	 
	//my_graph.print_adjacency_matrix(0,&cout);
	//my_graph.print_adjacency_list(0,IGRAPH_IN,&cout);
	//exit(1);
	/*compute the alpha-centrality*/
	boost::numeric::ublas::vector<double > result(igraph_matrix_nrow(&lf_adjmatrix));
	cout<<my_graph.alpha_centrality(result,0,1.0,0);
	for (map<string,unsigned>::iterator i=bats_map.begin(); i!=bats_map.end(); i++) {
	  Bat b = bats_records[i->first];
	  if (!b.part_of_lf_event) continue;//skip this bat, if she hasn't led or followed at all    
	      cout<<i->first<<"\t"<<result[bat_id2matrix_id[i->second]]<<endl;	    
	}
	exit(1);
	igraph_vector_t centralities, centralities2;
	igraph_vector_init(&centralities,igraph_matrix_nrow(&lf_adjmatrix));
	igraph_vector_fill(&centralities,0);
	igraph_vector_init(&centralities2,igraph_matrix_nrow(&lf_adjmatrix));
	igraph_vector_fill(&centralities2,0);

	string message;
	stringstream centr;
	if (centrality == 0) {
	  message = "Outputting in-degree centrality ...";
          centr<<outdir<<"/indegree_original_"<<Year<<".dat";	
	  my_graph.get_indegrees(&centralities,/*rewired=*/0);
	}
	else if (centrality == 1){
	  message = "Outputting eigenvector centrality ...";
	  centr<<outdir<<"/eigenvector_original_"<<Year<<".dat";	
	  my_graph.eigenvector_centrality(&centralities,/*rewired=*/0);
	}
	else {
	  message = "Outputting both eigenvector and in-degree centrality in the graphml file";	 
	  my_graph.get_indegrees(&centralities,/*rewired=*/0);
	  my_graph.eigenvector_centrality(&centralities2,/*rewired=*/0);
	}
	cout<<message;	
	if (centrality != 2) {
	  ofstream centrfile(centr.str().c_str(),ios::out);
	  if (!centrfile.good()) {
	    perror(centr.str().c_str());
	    exit(1);
	  }	
	  for (map<string,unsigned>::iterator i=bats_map.begin(); i!=bats_map.end(); i++) {
		Bat b = bats_records[i->first];
		if (!b.part_of_lf_event) continue;//skip this bat, if she hasn't led or followed at all    
		centrfile<<i->first<<"\t"<<VECTOR(centralities)[bat_id2matrix_id[i->second]]<<endl;
		//cout<<i->first<<"\t"<<bat_id2matrix_id[i->second]<<"\t"<<VECTOR(centralities)[bat_id2matrix_id[i->second]]<<endl;
	  }
	  //centrfile<<my_graph.get_sum_degrees(&my_graph.graph)<<endl;
	  centrfile.close();
	}
	cout<<"DONE"<<endl; vector<double> probs(total_bats_in_lf_events,0);
	ofstream graphfile;
	cout<<"Writing graph to file ...";
	stringstream graph_outputfile;
	graph_outputfile<<outdir<<"/graph_"<<Year<<".csv"; //this was for Nico
	graphfile.open(graph_outputfile.str().c_str(),ios::out);
	my_graph.print_adjacency_matrix(0,&graphfile);
	graphfile.close();
	graphfile.open(graph_outputfile.str().c_str(),ios::app);
	graphfile<<endl<<endl;
	//os.open(combined_networks.c_str(),ios::app);
	//if (!os.good()) {
          //perror(combined_networks.c_str());
          //exit(1);
        //}   
	for (map<string,unsigned>::iterator i=bats_map.begin(); i!=bats_map.end(); i++) {
	  Bat b = bats_records[i->first];
	  if (!b.part_of_lf_event) continue;//skip this bat, if she hasn't led or followed at all    
	      graphfile<<i->first<<";"<<VECTOR(centralities)[bat_id2matrix_id[i->second]]<<";"<<bat_id2matrix_id[i->second]<<endl;	    
              probs[bat_id2matrix_id[i->second]] = (double)b.cleaned_movement_history.size() / (double)total_readings;
	      //os<<i->first<<" "<<(double)b.cleaned_movement_history.size()<<" "<<Year<<endl;	      	     
	}
	 //os.close();
	 graphfile.close();
	 cout<<"DONE"<<endl;
	 /***/	
	 /*create the graph_ml file*/
	 cout<<"Outputting the lf network ...";        
	 stringstream graphml;
	 graphml<<outdir<<"/graphml_lf_network_"<<Year<<".graphml";
	 ofstream graphmlfile(graphml.str().c_str(),ios::trunc);
	 //add the header
	 graphmlfile<<"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"<<endl;
	 graphmlfile<<"<graphml xmlns=\"http://graphml.graphdrawing.org/xmlns\""<<endl;
	 graphmlfile<<"xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\""<<endl;
	 graphmlfile<<"xsi:schemaLocation=\"http://graphml.graphdrawing.org/xmlns"<<endl;
	 graphmlfile<<"http://graphml.graphdrawing.org/xmlns/1.0/graphml.xsd\">"<<endl;
	 graphmlfile<<"<!-- Created by Pavlin Mavrodiev -->"<<endl;	
	 graphmlfile<<"<key id=\"d0\" for=\"node\" attr.name=\"Label\" attr.type=\"string\"/>"<<endl;
	 graphmlfile<<"<key id=\"d2\" for=\"node\" attr.name=\"centrality\" attr.type=\"double\"/>"<<endl;
	 graphmlfile<<"<key id=\"centrality2\" for=\"node\" attr.name=\"centrality2\" attr.type=\"double\"/>"<<endl;
	 graphmlfile<<"<key id=\"d1\" for=\"edge\" attr.name=\"importance\" attr.type=\"double\"/>"<<endl;
	 graphmlfile<<"<graph id=\"G\" edgedefault=\"directed\">"<<endl;
	 //normalize the eigenvector centralities
	 //double max = igraph_vector_max(&centralities);
	 //for (unsigned h=0; h<igraph_vector_size(&centralities); h++)
	  //VECTOR(centralities)[h] /= max;
	 //add the nodes
	 for (map<string,unsigned>::iterator i=bats_map.begin(); i!=bats_map.end(); i++) {
	     Bat b = bats_records[i->first];
	     if (!b.part_of_lf_event) continue;//skip this bat, if she hasn't led or followed at all    
	     graphmlfile<<"<node id = \"n"<<bat_id2matrix_id[i->second]<<"\">"<<endl<<"<data key=\"d0\">"<<endl;
	     graphmlfile<<"<Label>"<<i->first.substr(i->first.length()-4,4)<<"</Label>"<<endl;
	     graphmlfile<<"</data>"<<endl;
	     graphmlfile<<"<data key=\"d2\">"<<endl;	    
	     graphmlfile<<"<centrality>"<<VECTOR(centralities)[bat_id2matrix_id[i->second]]<<"</centrality>"<<endl;	    
	     graphmlfile<<"</data>"<<endl;
    	     if (centrality == 2) {
	       graphmlfile<<"<data key=\"centrality2\">"<<endl;	    
	       graphmlfile<<"<centrality2>"<<VECTOR(centralities2)[bat_id2matrix_id[i->second]]<<"</centrality2>"<<endl;	    
	       graphmlfile<<"</data>"<<endl;
	     }
	     graphmlfile<<"</node>"<<endl;
	 }
	 //add the edges
	 for (map<pair<unsigned,unsigned>, double>::iterator itr2=edges.begin(); itr2 != edges.end(); itr2++) {
	   graphmlfile<<"<edge source=\"n"<<itr2->first.first<<"\" target=\"n"<<itr2->first.second<<"\">"<<endl;
	   graphmlfile<<"<data key=\"d1\">\n<importance>"<<itr2->second<<"</importance>\n</data>\n</edge>\n";
	 }
	 graphmlfile<<"</graph>\n</graphml>"<<endl;
	 graphmlfile.close();
	 igraph_vector_destroy(&centralities);	
	 cout<<"DONE"<<endl;
	
	 /***************************/	
	 /*rewrire the graph 10000 times*/
	 cout<<"Rewiring in progress ... ";
	 /**/
	   for (map<string,unsigned>::iterator i=bats_map.begin(); i!=bats_map.end(); i++) {
	     Bat b = bats_records[i->first];
	     if (!b.part_of_lf_event) continue;//skip this bat, if she hasn't led or followed at all    	      
	     cout<<b.hexid<<"\t"<<probs[bat_id2matrix_id[i->second]]<<endl;
	   }
	  /**/
	 if (rewire_random_models) {
	  gzFile centrfile_shuffled;
	  igraph_vector_t rewired_centralities;
	  igraph_vector_init(&rewired_centralities,igraph_matrix_nrow(&lf_adjmatrix));
	  for (unsigned kk=0; kk<100000 && (centrality !=2); kk++) {	    
	     stringstream centr_shuffled;
	     /*MODEL 1*/	     
	     my_graph.rewire_random_model(1,&probs);
	     if (!centrality) {
		centr_shuffled<<outdir<<"/indegree_shuffled_"<<Year<<"_model-1"<<".dat.gz";
		while (my_graph.get_indegrees(&rewired_centralities,1)) {		  
		  my_graph.rewire_random_model(1,&probs);
		}
	     }
	      else {
		centr_shuffled<<outdir<<"/eigenvector_shuffled_"<<Year<<"_model-1"<<".dat.gz";	
		while (my_graph.eigenvector_centrality(&rewired_centralities,1)) {		  
		  my_graph.rewire_random_model(1,&probs);
		}
	      }              
	      centrfile_shuffled = gzopen(centr_shuffled.str().c_str(),"a9");
	      centr_shuffled.str("");     
	      //get the edge attributes
	      for (map<string,unsigned>::iterator i=bats_map.begin(); i!=bats_map.end(); i++) {
		  Bat b = bats_records[i->first];
		  if (!b.part_of_lf_event) continue;//skip this bat, if she hasn't led or followed at all    
		  //centrfile_shuffled<<i->first<<"\t"<<VECTOR(rewired_centralities)[bat_id2matrix_id[i->second]]<<endl;		
		  gzprintf(centrfile_shuffled,"%s\t%10f\n",i->first.c_str(),VECTOR(rewired_centralities)[bat_id2matrix_id[i->second]]);		  
	      }
	      gzclose(centrfile_shuffled);
	      /*MODEL 2*/	      
	      my_graph.rewire_random_model(2,&probs);
	      if (!centrality) {
		centr_shuffled<<outdir<<"/indegree_shuffled_"<<Year<<"_model-2"<<".dat.gz";
		while (my_graph.get_indegrees(&rewired_centralities,1)) {		 
		  my_graph.rewire_random_model(2,&probs);
		}
	      }
	      else {
		centr_shuffled<<outdir<<"/eigenvector_shuffled_"<<Year<<"_model-2"<<".dat.gz";	
		while (my_graph.eigenvector_centrality(&rewired_centralities,1)) {		
		  my_graph.rewire_random_model(2,&probs);
		}
	      }              
	      centrfile_shuffled = gzopen(centr_shuffled.str().c_str(),"a9");
	      centr_shuffled.str("");	     
	      //get the edge attributes
	      for (map<string,unsigned>::iterator i=bats_map.begin(); i!=bats_map.end(); i++) {
		  Bat b = bats_records[i->first];
		  if (!b.part_of_lf_event) continue;//skip this bat, if she hasn't led or followed at all    		  
		  gzprintf(centrfile_shuffled,"%s\t%10f\n",i->first.c_str(),VECTOR(rewired_centralities)[bat_id2matrix_id[i->second]]);
	      }
	      gzclose(centrfile_shuffled);	     
	      /*MODEL 3*/	      
	      my_graph.rewire_random_model(3,&probs);
	      if (!centrality) {
		centr_shuffled<<outdir<<"/indegree_shuffled_"<<Year<<"_model-3"<<".dat.gz";
		while (my_graph.get_indegrees(&rewired_centralities,1)) {		 
		  my_graph.rewire_random_model(3,&probs);
		}
	      }
	      else {
		centr_shuffled<<outdir<<"/eigenvector_shuffled_"<<Year<<"_model-3"<<".dat.gz";	
		while (my_graph.eigenvector_centrality(&rewired_centralities,1)) {		 
		  my_graph.rewire_random_model(3,&probs);
		}
	      }
	      centrfile_shuffled = gzopen(centr_shuffled.str().c_str(),"a9");
	      centr_shuffled.str("");
	      //get the edge attributes
	      for (map<string,unsigned>::iterator i=bats_map.begin(); i!=bats_map.end(); i++) {
		  Bat b = bats_records[i->first];
		  if (!b.part_of_lf_event) continue;//skip this bat, if she hasn't led or followed at all    		
		  gzprintf(centrfile_shuffled,"%s\t%10f\n",i->first.c_str(),VECTOR(rewired_centralities)[bat_id2matrix_id[i->second]]);	      		
	      }	      
	      gzclose(centrfile_shuffled);
	      /*MODEL 4*/
	      /*Since this preserves indegree, it makes no sense to calculate in-degree centrality*/	      
	      if (centrality == 1) {
		my_graph.rewire_random_model(4,&probs);
		centr_shuffled<<outdir<<"/eigenvector_shuffled_"<<Year<<"_model-4"<<".dat.gz";	
		while (my_graph.eigenvector_centrality(&rewired_centralities,1)) {		 
		  my_graph.rewire_random_model(4,&probs);
		}	      
		centrfile_shuffled = gzopen(centr_shuffled.str().c_str(),"a9");
		centr_shuffled.str("");	    
		//get the edge attributes
		for (map<string,unsigned>::iterator i=bats_map.begin(); i!=bats_map.end(); i++) {
		  Bat b = bats_records[i->first];
		  if (!b.part_of_lf_event) continue;//skip this bat, if she hasn't led or followed at all    		  
		  gzprintf(centrfile_shuffled,"%s\t%10f\n",i->first.c_str(),VECTOR(rewired_centralities)[bat_id2matrix_id[i->second]]);
		}	      
		gzclose(centrfile_shuffled);
	      }
	      /*MODEL 5*/	     
	      my_graph.rewire_random_model(5,&probs); 	   
	      if (!centrality) {
		centr_shuffled<<outdir<<"/indegree_shuffled_"<<Year<<"_model-5"<<".dat.gz";
		while (my_graph.get_indegrees(&rewired_centralities,1)) {  
		  my_graph.rewire_random_model(5,&probs);
		}
	      }
	      else {
		centr_shuffled<<outdir<<"/eigenvector_shuffled_"<<Year<<"_model-5"<<".dat.gz";	
		while (my_graph.eigenvector_centrality(&rewired_centralities,1)) {		  
		  my_graph.rewire_random_model(5,&probs);
		}
	      }            
	      centrfile_shuffled = gzopen(centr_shuffled.str().c_str(),"a9");
	      centr_shuffled.str("");
	      //get the edge attributes
	      for (map<string,unsigned>::iterator i=bats_map.begin(); i!=bats_map.end(); i++) {
		  Bat b = bats_records[i->first];
		  if (!b.part_of_lf_event) continue;//skip this bat, if she hasn't led or followed at all    
		  gzprintf(centrfile_shuffled,"%s\t%10f\n",i->first.c_str(),VECTOR(rewired_centralities)[bat_id2matrix_id[i->second]]);
	      }
	      gzclose(centrfile_shuffled);
	      /*MODEL 6*/	      
 	      /*Since this preserves indegree, it makes no sense to calculate in-degree centrality*/
	      if (centrality == 1) {
		my_graph.rewire_random_model(6,&probs);
		centr_shuffled<<outdir<<"/eigenvector_shuffled_"<<Year<<"_model-6"<<".dat.gz";	
		while (my_graph.eigenvector_centrality(&rewired_centralities,1)) {		  
		  my_graph.rewire_random_model(6,&probs);
		}	      
		centrfile_shuffled = gzopen(centr_shuffled.str().c_str(),"a9");
		centr_shuffled.str("");
		//get the edge attributes
		for (map<string,unsigned>::iterator i=bats_map.begin(); i!=bats_map.end(); i++) {
		  Bat b = bats_records[i->first];
		  if (!b.part_of_lf_event) continue;//skip this bat, if she hasn't led or followed at all    
		  gzprintf(centrfile_shuffled,"%s\t%10f\n",i->first.c_str(),VECTOR(rewired_centralities)[bat_id2matrix_id[i->second]]);
		}
		gzclose(centrfile_shuffled);
	      }
	    }
	    igraph_vector_destroy(&rewired_centralities);	    
	    cout<<"DONE"<<endl;
	  }
	  /***/
	  /*calculate assortativity*/
	  stringstream assortativity;
	  assortativity<<"assortativity/"<<Year<<".txt";
	  ofstream assort(assortativity.str().c_str(),ios::trunc);	
	  igraph_vector_t indegree,outdegree;
	  igraph_real_t res1;
	  igraph_vector_init(&indegree,total_bats_in_lf_events);
	  igraph_vector_init(&outdegree,total_bats_in_lf_events);	
	  igraph_degree(&my_graph.graph,&indegree,igraph_vss_all(),IGRAPH_IN,/*count self-loops=*/0);
	  igraph_degree(&my_graph.graph,&outdegree,igraph_vss_all(),IGRAPH_OUT,/*count self-loops=*/0);
	  igraph_assortativity(&my_graph.graph,&outdegree,&indegree,&res1,/*directed=*/1);	
	  assort<<res1<<endl;	
	  assort.close();
	  /*******************************************/
	  stringstream assortativity_shuffled;
	  assortativity_shuffled<<"assortativity/"<<Year<<"_shuffled.txt";
	  ofstream assort_shuffled(assortativity_shuffled.str().c_str(),ios::trunc);	
	  for (unsigned tt=1000; tt<1000; tt++) {
	      my_graph.rewire_edges3(rand());	    
	      //for the 2007 data use igraph_rewire_edges
	      //igraph_rewire_edges(&shuff_graph,1,false,true);
	      igraph_vector_fill(&indegree,0);
	      igraph_vector_fill(&outdegree,0);
	      igraph_degree(my_graph.rewired_graph,&indegree,igraph_vss_all(),IGRAPH_IN,/*count self-loops=*/0);
	      igraph_degree(my_graph.rewired_graph,&outdegree,igraph_vss_all(),IGRAPH_OUT,/*count self-loops=*/0);
	      //igraph_vector_add_constant(&indegree, -1);
	      igraph_assortativity(my_graph.rewired_graph,&outdegree,&indegree,&res1,/*directed=*/1);
	      assort_shuffled<<res1<<endl;	    
	    //reshuffled_graph.avg_neighbour_connectivity(&indegree,&g2,total_bats_in_lf_events);
	  }
	  //reshuffled_graph.print_all(&assort_shuffled);
	  assort_shuffled.close();	  
	  
	  igraph_vector_destroy(&indegree);
	  igraph_vector_destroy(&outdegree);
	
	  
	/*DESTROY STUFF*/          
	
	//igraph_vector_destroy(&res3);igraph_vector_destroy(&res4);        
        igraph_matrix_destroy(&lf_adjmatrix);  		
    }
	
    if (create_sqlitedb) {
            /* write the box recordings in a database if so configured. see the create_sqlitedb
            flag in the global definitions */
            map<string,Box>::iterator jj;
            sqlite3 *db;
            char *zErrMsg = 0;
            file_sqlitedb = "box_recordings_"+Year+".sqlite";
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


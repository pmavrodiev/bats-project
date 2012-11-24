#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <list>
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

#include "classes.cpp"
#include "global_defs.cpp"

/* =========================== GLOBAL FUNCTIONS ======================= */

/*searches in a vector of lf_events for a particular event*/
bool lf_exists(vector<Lf_pair> *vec_ptr, Lf_pair *element) {
    for (unsigned i=0; i<vec_ptr->size(); i++) {
        if ((*vec_ptr)[i].equals(*element))
            return true;
    }
    return false;
}


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
                if (ref.is_not_a_date_time())
                    ref = pos_infin;
                if (dir_entry.find(all_box) != string::npos) {
                    Box b(3,dir_entry,ref);
                    boxes[dir_entry] = b;
                    boxes_auxillary[dir_entry] = count;
                    boxes_auxillary_reversed[count++]=dir_entry;
                }
                else if (dir_entry.find(majority_box) != string::npos ||
                         dir_entry.find(majority_box2) != string::npos) {
                    Box b(2,dir_entry,ref);
                    boxes[dir_entry] = b;
                    boxes_auxillary[dir_entry] = count;
                    boxes_auxillary_reversed[count++]=dir_entry;
                }
                else if (dir_entry.find(minority_box) != string::npos) {
                    Box b(1,dir_entry,ref);
                    boxes[dir_entry] = b;
                    boxes_auxillary[dir_entry] = count;
                    boxes_auxillary_reversed[count++]=dir_entry;
                }
                else if (dir_entry.find(control_box) != string::npos) {
                    Box b(0,dir_entry,ref);
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
                bool skip = false;
                //skip excel files
                if (file_name.substr(file_name.length()-3,3) == "xls")
                    skip = true;
                if (skip) continue;
                cout<<file_name.c_str()<<endl;
                yyin = fopen(file_name.c_str(),"r");
                yylex();
                counter ++;
                file_counter ++;
                /*PROCESS THE CONTENTS OF EACH DATA FILE*/
                Box *targetBox = &boxes[box_name];
                for (unsigned i=0; i<box_entries.size(); i++) {
                    /*is the given transpoder id a bat?*/
                    if (bats_map.find(box_entries[i].first) == bats_map.end()) {
                        //if (find(bats_vector.begin(),bats_vector.end(),box_entries[i].first) == bats_vector.end()) {
                        if (find(transponders_vector.begin(),transponders_vector.end(),box_entries[i].first) == transponders_vector.end()) {
                            cerr<<box_entries[i].first<<" neither a bat nor a transpoder"<<endl;
                        }
                        continue;
                    }
                    /**********************************/
                    //disregard all entries that occured AFTER the box has been occupied
                    ptime currentBoxEntryTime = from_iso_string(box_entries[i].second);
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
    if (argc < 5 || argc >=6) {
        cout<<"Version: "<<version<<endl<<endl;
        cout<<"Usage: bats <dir> <config_file>"<<endl<<endl;
        cout<<"<dir>\t full path to the directory containing transponder data files"<<endl;
        cout<<"<config_file>\t full path to the configuration file"<<endl<<endl;
        return 0;
    }
    if (argc == 5) {
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
        char* file_name = argv[1];
        yyin = fopen(file_name,"r");
        if (yyin == NULL) {
            perror(file_name);
            exit(1);
        }
        yylex();
        nbats = bats_map.size(); //bats_vector.size();
        ntransponders = transponders_vector.size();
        /* remove this later (1)*/
        /* argv[2] = knowledge_delay
         * argv[3] = lf_delay
         * argv[4] = occupation_deadline
         */
        stringstream foo,moo,goo;
        foo<<argv[2];
        moo<<argv[3];
        goo<<argv[4];
        int tt,gg;
        foo>>tt;
        moo>>gg;
        knowledge_delay = minutes(tt);
        lf_delay = minutes(gg);
        occupation_deadline=goo.str();
        /*******************/
        /*init the output files*/
        stringstream ss5,ss6;
        ss5<<"output_files_new_2/lf_time_diff_"<<Year<<"_"<<knowledge_delay.minutes()<<"_"<<lf_delay.minutes()<<"_"<<occupation_deadline<<".txt";
        lf_time_diff = ss5.str();
        ss6<<"output_files_new_2/lf_valid_time_diff_"<<Year<<"_"<<knowledge_delay.minutes()<<"_"<<lf_delay.minutes()<<"_"<<occupation_deadline<<".txt";;
        lf_valid_time_diff = ss6.str();
        /***********************/
        base_dir = argv[0];
        initBoxes(base_dir);
        /*remove that later (2)*/
        //change the occupation deadline of the boxes
        for (map<string,Box>::iterator kk=boxes.begin(); kk!=boxes.end(); kk++) {
            if (!kk->second.occupiedWhen.is_not_a_date_time() && !kk->second.occupiedWhen.is_pos_infinity()) {
                string existingDate = to_iso_string(kk->second.occupiedWhen);
                string newDate = existingDate.substr(0,existingDate.length()-6) + occupation_deadline;
                ptime occupation_time(from_iso_string(newDate));
                kk->second.occupiedWhen = occupation_time;
            }
        }
        /*********************/
        /*this call is in the beginning, as advised in the igraph manual*/
        igraph_i_set_attribute_table(&igraph_cattribute_table);

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
                delete [] A;
                delete [] B;
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
        /*print the matched mothers and daughters*/
        /*
        for (multimap<string,string>::iterator tt=mother_daughter.begin(); tt != mother_daughter.end(); tt++) {
          cout<<tt->first<<" <-- "<<tt->second<<endl;
        }
        */

        DIR *dir;
        struct dirent *ent;
        string all_box("100"), majority_box("66"),minority_box("33"), control_box("0"),majority_box2("67");

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
                            node_name.find(majority_box2) != string::npos ||
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
            Bat &ref = bats_records[current.hexid]; //this is init of bats_records
            ref.hexid = current.hexid;
            //which box?
            Box *box_ptr = &boxes[current.box_name];
            ref.add_movement(current.TimeOfEntry,box_ptr);
        }
        //associate mothers to daughters
        for (map<string,unsigned int>::iterator i=bats_map.begin(); i!=bats_map.end(); i++) {
            //for (unsigned i=0; i<nbats; i++) {
            Bat &ref = bats_records[i->first];
            //Bat &ref = bats_records[bats_vector[i]];
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
            if (mother_itr == mothers.end() && daughters_itr == daughters.end()) {
                cout<<bat<<" not found as either mother or daughter in the mother-daughter file"<<endl;
            }
        }

        multiset<BatEntry,batEntryCompare>::iterator from,to,cur;
        from=multibats.begin();
        Box *bx = &boxes[multibats.begin()->box_name]; //get the first box in multibats
        //store all pairs of experience and naiive bats
        vector<Lf_pair> vec_lfpairs;

        //IMPORTANT: DO THE ANALYSIS ONE BOX AT A TIME!!!!
        for (to=multibats.begin(); to != multibats.end(); to++) {
            //find the boundaries of a box
            cout<<"Box: "<<bx->name<<endl;
            //cout<<to->box_name<<endl;
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
            /*main pairing loop. no enforcement of max. leader-follower time difference*/
            //the first bat is the discoverer, i.e. she is automatically experienced
            bats_records[box_bat_entries[0].hexid].box_knowledge[bx->name]=EXPERIENCED;
            bats_records[box_bat_entries[0].hexid].informed_since[bx->name]=box_bat_entries[0].TimeOfEntry;
            boxes[bx->name].status = DISCOVERED;
            //==================================================================
            //int counter2 = 0;
            for (unsigned i=0; i<box_bat_entries.size(); i++) {
                //cout<<++counter2<<". Comparing "<<box_bat_entries[i].hexid<<" with ";
                Bat *B1 = &bats_records[box_bat_entries[i].hexid];
                lastSeen[box_bat_entries[i].hexid] = box_bat_entries[i].TimeOfEntry;
                for (unsigned j=(i+1); j<box_bat_entries.size(); j++) {
                    Bat *B2 = &bats_records[box_bat_entries[j].hexid];
                    //cout<<box_bat_entries[j].hexid<<endl;
                    if (*B1 == *B2) {
                        /*cout<<"\t"<<"identical"<<endl;*/continue;   //no self lf events
                    }
                    //first update this bat's knowledge about the given box, if necessary
                    ptime &ref = lastSeen[box_bat_entries[j].hexid];
                    if (ref.is_not_a_date_time()) ref = pos_infin;
                    time_duration td_update_knowledge = box_bat_entries[j].TimeOfEntry - ref;
                    if (td_update_knowledge > knowledge_delay && !B2->is_informed(bx->name,box_bat_entries[j].TimeOfEntry)) {
                        B2->box_knowledge[bx->name] = EXPERIENCED;
                        B2->make_informed(bx->name,box_bat_entries[j].TimeOfEntry);
                    }
                    ref = box_bat_entries[j].TimeOfEntry;
                    //cout<<B1->hexid<<": "<<B1->informed(bx->name,box_bat_entries[i].TimeOfEntry)<<endl;
                    //cout<<B2->hexid<<": "<<B2->informed(bx->name,box_bat_entries[j].TimeOfEntry)<<endl;
                    //are both bats informed or are both naiive about that box at this time?
                    if ((!B1->is_informed(bx->name,box_bat_entries[i].TimeOfEntry) && !B2->is_informed(bx->name,box_bat_entries[j].TimeOfEntry)) ||
                            (B1->is_informed(bx->name,box_bat_entries[i].TimeOfEntry) && B2->is_informed(bx->name,box_bat_entries[j].TimeOfEntry)))
                        continue;
                    //if the pair is suitable, materialize it disregarding the time distance between B1 and B2
                    Lf_pair newPair;
                    if (B1->is_informed(bx->name,box_bat_entries[i].TimeOfEntry)) {
                        newPair.init(B1,B2,box_bat_entries[i].TimeOfEntry,box_bat_entries[j].TimeOfEntry,bx->name);
                    }
                    else if (B2->is_informed(bx->name,box_bat_entries[j].TimeOfEntry)) {
                        newPair.init(B2,B1,box_bat_entries[j].TimeOfEntry,box_bat_entries[i].TimeOfEntry,bx->name);
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
                } //end for (unsigned j=i; j<box_bat_entries.size(); j++)
            } //end for (unsigned j=i; j<box_bat_entries.size(); j++) {
            //==================================================================
            vec_lfpairs.insert(vec_lfpairs.end(),current_box_pairs.begin(),current_box_pairs.end());
            to--; //go back to the last entry of the previous box_entries
            from=to;
            from++; //move the "from" pointer to the first entry of the new box
            if (from == multibats.end()) break;
            bx->name = from->box_name;

        } //end for (to=multibats.begin(); to != multibats.end(); to++)

        //invalidate those pairs that violate lf_delay
        for (unsigned h=0; h<vec_lfpairs.size(); h++)
            vec_lfpairs[h].validate(lf_delay);

        /*output all matched pairs*/
        ofstream os(lf_time_diff.c_str(),ios::out);
        if (!os.good()) {
            perror(lf_time_diff.c_str());
            exit(1);
        }
        for (unsigned i=0; i<vec_lfpairs.size(); i++)
            os<<to_simple_string(vec_lfpairs[i].get_lf_delta())<<endl;

        os.close();
        //output only valid pairs, i.e. those which respect the max. allowed delay b/n leader and follower
        os.open(lf_valid_time_diff.c_str(),ios::out);
        if (!os.good()) {
            perror(lf_valid_time_diff.c_str());
            exit(1);
        }
        for (unsigned i=0; i<vec_lfpairs.size(); i++) {
            if (vec_lfpairs[i].valid) {
                os<<to_simple_string(vec_lfpairs[i].get_lf_delta())<<endl;
            }
        }
        os.close();
        /*store the number of leading events that a leader has taken part of,
        regardless the number of following individuals*/
        map<string,unsigned> bats_lead_stats;
        /*store the number of times a bat has followed*/
        map<string,unsigned> bats_follow_stats;
        /*compute the nodes sizes and colors, and the edge thickness and colors for the graph, based on the detected lf_pairs*/
        map<unsigned,unsigned> bat_id2matrix_id; //maps the bat_id from bats_map to a consequtive range of ids for lf_adjmatrix
        //we need this if the bats with recordings are fewer than the total #of bats
        /*stores a directed edge between follower and leader*/
        map<pair<unsigned,unsigned>,double,bool(*)(pair<unsigned,unsigned>,pair<unsigned,unsigned>) > edges(fn_pt3); //maps an edge to its width. an edge is a pair FROM and TO.
        map<int,unsigned> nodes; //maps a node_id to its size
        double starting_node_size = 9.0;
        double edges_starting_width = 1.0;
        int total_valid_pairs = 0;
        unsigned consequtive_ids=0;
        for (unsigned i=0; i<vec_lfpairs.size(); i++) {
            Lf_pair lfpair = vec_lfpairs[i];
            if (lfpair.valid) { //only valid pairs
                Bat *leader = &bats_records[lfpair.getLeaderId()];
                leader->total_following++; //one more bat has followed me
                total_valid_pairs++;
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
                    else cout<<"Warning: No relatedness data between "<<leader_hexid<<" and "<<follower_hexid<<endl;
                }
                else
                    leader->cumulative_relatedness += related_itr->second; //found it
                /*******************************************/
                /*create an edge*/
                pair<unsigned,unsigned> edge_pair(bats_map[follower_hexid],bats_map[leader_hexid]);
                if (edges.find(edge_pair) == edges.end()) //edge does not exist
                    edges[edge_pair] = edges_starting_width;
                else 
                    edges[edge_pair]++ ; //increase the width linearly
                   
                /**********************/
            }
        }
        /*create the cxf file*/
        stringstream cxf;
        cxf<<"output_files_new_2/lead_follow_"<<Year<<"_"<<knowledge_delay.minutes()<<"_"<<lf_delay.minutes()<<"_"<<occupation_deadline<<".cxf";
        ofstream cxffile(cxf.str().c_str(),ios::trunc);
	//add the nodes
	for (map<string,unsigned>::iterator i=bats_map.begin(); i!=bats_map.end(); i++) {
            Bat b = bats_records[i->first];
            if (!b.part_of_lf_event) continue;//skip this bat, if she hasn't led or followed at all    
	    //add + 1 to the size of the node, because otherwise there can be nodes with size 0, i.e. bats who've never been followed
            cxffile<<"node: ("<<i->second+1<<") size{"<<b.total_following+1<<"} "<<"label{"<<i->first<<"} color{0.933,0.796,0.678}"<<endl;
        }
        //add the edges
        for (map<pair<unsigned,unsigned>, double>::iterator itr2=edges.begin(); itr2 != edges.end(); itr2++) {
	  cxffile<<"edge: ("<<itr2->first.first+1<<","<<itr2->first.second+1<<") width{"<<itr2->second<<"} ";
	  cxffile<<" color{0.54,0.54,0.54}"<<endl;
	}
	/***************************/
	cxffile.close();
	exit(0);
	
        /*create an empty directed igraph with all bats and no edges yet*/
        igraph_matrix_t lf_adjmatrix; //square matrix
        igraph_matrix_init(&lf_adjmatrix,total_bats_movinghistory,total_bats_movinghistory); //init the square matrix
        igraph_matrix_null(&lf_adjmatrix);
        igraph_vector_t final_page_rank;


        //first init a map between hexids and initial node sizes
        map<string,double> bat_nodes;

        

        /*add/update the edge to the adjacency matrix*/
        //MATRIX(lf_adjmatrix,bats_map[lfpair.getFollowerId()], bats_map[lfpair.getLeaderId()])++;
        //calculate the pagerank of the graph
        igraph_t g2;
        igraph_weighted_adjacency(&g2,&lf_adjmatrix,IGRAPH_ADJ_DIRECTED,"weight",true);
        igraph_vector_t eigenvector;
        igraph_vector_init(&eigenvector,nbats);
        igraph_vector_null(&eigenvector);
        igraph_arpack_options_t aroptions;
        igraph_arpack_options_init(&aroptions);
        igraph_vs_t vs;
        igraph_vs_all(&vs);
        igraph_pagerank_old(&g2,&eigenvector,vs,true,300,0.001,0.99,false);
        igraph_vector_copy(&final_page_rank,&eigenvector);  //save the final page rank vector
        //update the size and colors of all nodes
        for (map<string,unsigned>::iterator j=bats_map.begin(); j!=bats_map.end(); j++) {
            //for (unsigned j=0; j<nbats; j++) {
            Bat b1 = bats_records[j->first];
            if (!b1.part_of_lf_event) continue; //skip this bat, if not part of any lf event
            double percentage_following = (b1.total_following == 0) ? 0 :
                                          b1.n_daughters_following / b1.total_following;
            double mean_relatedness = (b1.total_following == 0) ? 0 :
                                      b1.cumulative_relatedness / b1.total_following;
            int idx=-1;
            if (what_node_sizes == 1)
                idx=binData(mean_relatedness,1);
            else if (what_node_sizes == 0)
                idx=binData(percentage_following,0);
            else if (what_node_sizes == 2)
                idx = binData(b1.total_following,2);
            if (idx == -1)  {
                cout<<"Error: Invalid index "<<percentage_following<<" or "<<mean_relatedness<<" or "<<b1.total_following<<" for binning "<<b1.hexid<<endl;
                //ceffile<<endl;
            }
        } //end for (unsigned j=0; j<nbats; j++) {
        //destroy the graph and the vector with page ranks
        igraph_vector_destroy(&eigenvector);
        igraph_destroy(&g2);
        //end if (lfpair.valid)

        cxffile.close();
        igraph_matrix_destroy(&lf_adjmatrix);

        /*write the final pagerank (fpr) to a file*/
        stringstream fpr;
        fpr<<"output_files_new_2/lead_follow_fpr_"<<Year<<"_"<<knowledge_delay.minutes()<<"_"<<lf_delay.minutes()<<"_"<<occupation_deadline<<".txt";
        ofstream fprfile(fpr.str().c_str(),ios::trunc);
        if (!fprfile.good()) {
            perror(fpr.str().c_str());
            exit(1);
        }

        for (map<string,unsigned>::iterator j=bats_map.begin(); j!=bats_map.end(); j++) {
            fprfile<<j->first<<" "<<VECTOR(final_page_rank)[j->second]<<" ";
            fprfile<<bats_records[j->first].part_of_lf_event<<endl;
        }
        igraph_vector_destroy(&final_page_rank);
        fprfile<<total_valid_pairs<<endl;
        fprfile.close();
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


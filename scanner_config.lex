%option noyywrap
%option nounput
%option prefix="config_"
%{
#include <iostream>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <vector>
#include <utility>
#include <cctype>
#include <algorithm>
#include <map>
#include <sstream>
#include <boost/date_time/gregorian/greg_month.hpp>

using namespace std;
using namespace boost::posix_time;
using namespace boost::gregorian;

string box_detailed_occupation; 
ptime detailed_occupation_time;
vector<string> detailed_box_occupation_bats;
vector<pair<ptime, vector<string> > > box_detailed_occupation_vector;
char *pch;

extern string Year; //the year of the analysis
extern map<string,string> short_to_long; //maps short to long bat hex ids
extern map<string,ptime> box_occupation; //maps box names to their dates of occupation
					 //if a box has not been occupied, this date
					 //is set to not_a_date_time
					 
extern map<string,ptime> box_installation; //maps box names to their dates of installation

/*stores the box programming: box_name->vector of bats programmed for this box*/
extern map<string,vector<string> > box_programming; 

/*stores the occupying bats for each box*/
extern map<string,vector<string> > box_occup_bats;

/*maps box names to box-specific occupation deadline hour*/
extern map<string,string> box_occupation_deadline;

extern map<string,vector<pair<ptime, vector<string> > > > occupation_history;
string box_name, box_date,current_programmed_box,current_occup_box,box_installation_name,box_installation_date;

extern map<string,unsigned,bool(*)(string,string)> bats_map;
extern unsigned int bats_counter;
extern vector<string> transponders_vector;
extern time_duration roundtrip_time;
extern time_duration lf_delay;
extern string occupation_deadline;
extern short centrality;
extern bool create_sqlitedb, rewire_random_models;
extern string colony;
int comment_caller;
%}

DIGIT [0-9]
HEXDIGIT [a-fA-F0-9]
BOXDIGIT [a-zA-Z0-9]
LETTER [a-zA-Z]
%x BOX_DETAILED_OCCUPATION
%x INSIDE_DETAILED_BOX_OCCUPATION
%x BOX_OCCUPATION
%x BOX_OCCUPATION_DEADLINE
%x BOX_INSTALLATION
%x BOX_OCCUP_BATS
%x COMMENT
%x BATS
%x TRANSPONDERS
%x BOX_PROGRAMMING
%x INSIDE_BOX_PROGRAMMING
%x INSIDE_BOX_OCCUP_BATS  
%x BATUPDATE
%x LFDELAY
%x OCCUPATIONDEADLINE
%x YEAR
%x EXPORTDATABASE
%x CENTRALITY
%x REWIRE
%x COLONY
%%
"begin{detailed_box_occupation}"	BEGIN(BOX_DETAILED_OCCUPATION);
"begin{exportdatabase}"	BEGIN(EXPORTDATABASE);
"begin{year}"	BEGIN(YEAR);
"begin{occupation_deadline}"	BEGIN(OCCUPATIONDEADLINE);
"begin{box_occupation_deadline}"	BEGIN(BOX_OCCUPATION_DEADLINE);
"begin{lf_delay}"	BEGIN(LFDELAY);
"begin{bat_update}"	BEGIN(BATUPDATE);
"begin{transponders}"	BEGIN(TRANSPONDERS);
"begin{bats}"	BEGIN(BATS);
"begin{box_occupation}"	BEGIN(BOX_OCCUPATION);
"begin{box_installation}"	BEGIN(BOX_INSTALLATION);
"begin{box_programming}" {current_programmed_box="";BEGIN(BOX_PROGRAMMING);}
"begin{box_occup_bats}" {current_occup_box="";BEGIN(BOX_OCCUP_BATS);}
"begin{centrality}"	BEGIN(CENTRALITY);
"begin{rewiring}"	BEGIN(REWIRE);
"begin{colony}"		BEGIN(COLONY);
<*>"end{detailed_box_occupation}" BEGIN(INITIAL);
<EXPORTDATABASE>"end{exportdatabase}" BEGIN(INITIAL);
<YEAR>"end{year}" BEGIN(INITIAL);
<BATS>"end{bats}" BEGIN(INITIAL);
<BOX_PROGRAMMING>"end{box_programming}" BEGIN(INITIAL);  
<BOX_OCCUP_BATS>"end{box_occup_bats}" BEGIN(INITIAL);
<BOX_OCCUPATION>"end{box_occupation}" BEGIN(INITIAL);
<BOX_OCCUPATION_DEADLINE>"end{box_occupation_deadline}" BEGIN(INITIAL);
<BOX_INSTALLATION>"end{box_installation}" BEGIN(INITIAL);
<TRANSPONDERS>"end{transponders}" BEGIN(INITIAL);
<BATUPDATE>"end{bat_update}" BEGIN(INITIAL);
<LFDELAY>"end{lf_delay}" BEGIN(INITIAL);
<OCCUPATIONDEADLINE>"end{occupation_deadline}" BEGIN(INITIAL);
<CENTRALITY>"end{centrality}"  BEGIN(INITIAL);
<REWIRE>"end{rewiring}"	BEGIN(INITIAL);
<COLONY>"end{colony}"	BEGIN(INITIAL);

<COLONY>{LETTER}+{DIGIT}* {
  pch = strtok(config_text,".");
  colony = pch;
}


<EXPORTDATABASE>{DIGIT}{1} {
  string ss;
  pch=strtok(config_text,".");
  ss=pch;
  if (ss == "0") create_sqlitedb=false;
  else if (ss == "1") create_sqlitedb=true;
  else {
    printf("Error: Unrecognized value for exportdatabase in config file\n");
    exit(1);
  }
}

<BOX_DETAILED_OCCUPATION>{DIGIT}+{LETTER}+ {
  pch=strtok(config_text,".");
  current_occup_box = pch;
  //cout<<"1:"<<current_occup_box<<endl;
  box_detailed_occupation_vector.clear(); 
  occupation_history[current_occup_box]=box_detailed_occupation_vector;
}

<BOX_DETAILED_OCCUPATION>{DIGIT}{4}-{DIGIT}{2}-{DIGIT}{2} {
  string local_day, local_month, local_year;
  pch = strtok(config_text,"-");  
  local_year = pch;
  pch = strtok(NULL,"-");  
  local_month = pch;
  pch = strtok(NULL,"-");  
  local_day = pch;
  box_detailed_occupation = local_year+local_month+local_day+"T000000";   
  //cout<<"2:"<<box_detailed_occupation<<endl;
  detailed_occupation_time = ptime(from_iso_string(box_detailed_occupation));   
  BEGIN(INSIDE_DETAILED_BOX_OCCUPATION);
}

<INSIDE_DETAILED_BOX_OCCUPATION>{HEXDIGIT}{4} {
  pch = strtok(config_text,".");  
  detailed_box_occupation_bats.push_back(pch);
}


<BOX_OCCUP_BATS>{BOXDIGIT}{1,5} {
  pch=strtok(config_text,":");
  current_occup_box = pch; 
  BEGIN(INSIDE_BOX_OCCUP_BATS);
}

<INSIDE_BOX_OCCUP_BATS>{HEXDIGIT}{10} {
  pch=strtok(config_text,".");
  string current_occupying_bat = pch;  
  if (current_occup_box == "") {
    printf("Error: Something went wrong while processing the box programming from the config file\n");
    exit(1);
  }  
  vector<string> &ref = box_occup_bats[current_occup_box];
  ref.push_back(current_occupying_bat);  
}

<BOX_PROGRAMMING>{BOXDIGIT}{1,5} {
  pch=strtok(config_text,":");
  current_programmed_box = pch;
  BEGIN(INSIDE_BOX_PROGRAMMING);
}

<INSIDE_BOX_PROGRAMMING>{HEXDIGIT}{10}|"none"|"all" {
  pch=strtok(config_text,".");
  string current_programmed_bat = pch;
  if (current_programmed_box == "") {
    printf("Error: Something went wrong while processing the box programming from the config file\n");
    exit(1);
  }  
  vector<string> &ref = box_programming[current_programmed_box];  
  ref.push_back(current_programmed_bat);  
}

<CENTRALITY>{DIGIT}{1} {
  pch=strtok(config_text,".");
  stringstream ss; ss<<pch;
  ss>>centrality;
}

<REWIRE>{DIGIT}{1} {
  pch=strtok(config_text,".");
  string temp = pch;
  if (!temp.compare("0")) {
    rewire_random_models = false;
  }
  else if (!temp.compare("1")) {
    rewire_random_models = true;
  }
  else {
    cerr<<"Unrecognized value in rewiring block: "<<temp<<". Default value of 0 assumed."<<endl;
    rewire_random_models = false;
  }
}


<YEAR>{DIGIT}{4} {
  pch=strtok(config_text,".");
  Year = pch;
}

<OCCUPATIONDEADLINE>{DIGIT}{6} {
  pch=strtok(config_text,".");
  occupation_deadline = pch;
}

<LFDELAY>{DIGIT}+ {
  int digit;
  stringstream ss;
  pch = strtok(config_text,".");
  //printf("%s\n",pch);
  ss<<pch; ss>>digit;
  lf_delay = minutes(digit);
}


<BATUPDATE>{DIGIT}+ {
  int digit;
  stringstream ss;
  pch = strtok(config_text,".");  
  ss<<pch; ss>>digit;
  roundtrip_time = minutes(digit);
}

<TRANSPONDERS>{HEXDIGIT}{10} {  
  string transponders_hexid;
  pch = strtok(config_text,".");
  transponders_hexid = pch;
  transponders_vector.push_back(transponders_hexid);
}


<BATS>{HEXDIGIT}{10} {  
  string bat_hexid;
  pch = strtok(config_text,".");
  bat_hexid = pch;  
  //only if we haven't seen the bat already
  if (bats_map.find(bat_hexid) == bats_map.end()) 
    bats_map[bat_hexid] = bats_counter++;
  string last4 = bat_hexid.substr(bat_hexid.size()-4,4);  
  short_to_long[last4] = bat_hexid;
  //bats_vector.push_back(bat_hexid);
}
 
<BOX_OCCUPATION_DEADLINE>{BOXDIGIT}{1,5} {  
  pch = strtok(config_text,".");  
  box_name = pch;  
} 
 
<BOX_OCCUPATION_DEADLINE>{DIGIT}{6} {
  pch = strtok(config_text,".");
  string str = pch;
  box_occupation_deadline[box_name] =  str;
}
 
 

<BOX_OCCUPATION>{BOXDIGIT}{1,5} {  
  pch = strtok(config_text,".");
  box_name = pch;
  //printf("box: %s\n",box_name.c_str());
}

<BOX_OCCUPATION>{DIGIT}{2}.{DIGIT}{2}.{DIGIT}{4} {
  string local_day, local_month, local_year;
  pch = strtok(config_text,".");
  local_day = pch;
  pch = strtok(NULL,".");
  local_month = pch;
  pch = strtok(NULL,".");
  local_year = pch;
  box_date = local_year+local_month+local_day+"T"+occupation_deadline; 
  //printf("Time: %s\n",box_date.c_str());
  ptime occupation_time(from_iso_string(box_date));
  //printf("Time: %s\n",to_simple_string(occupation_time).c_str());
  box_occupation[box_name] =  occupation_time;
}

<BOX_INSTALLATION>{BOXDIGIT}{1,5} {  
  pch = strtok(config_text,".");
  box_installation_name = pch;
  //printf("box: %s\n",box_name.c_str());
}

<BOX_INSTALLATION>{DIGIT}{2}.{DIGIT}{2}.{DIGIT}{4} {
  string local_day, local_month, local_year;
  pch = strtok(config_text,".");
  local_day = pch;
  pch = strtok(NULL,".");
  local_month = pch;
  pch = strtok(NULL,".");
  local_year = pch;
  box_installation_date = local_year+local_month+local_day+"T000000"; 
  //printf("Time: %s\n",box_installation_date.c_str());
  ptime installation_time(from_iso_string(box_installation_date));  
  box_installation[box_installation_name] =  installation_time;
}


<*>"/*" {
comment_caller = YY_START;
BEGIN(COMMENT);
}
<*>"*/" BEGIN(comment_caller);

<INSIDE_DETAILED_BOX_OCCUPATION>\n|\r|\r\n {
  pair<ptime,vector<string> > newpair(detailed_occupation_time,detailed_box_occupation_bats);
  box_detailed_occupation_vector.push_back(newpair);
  occupation_history[current_occup_box] = box_detailed_occupation_vector;  
  detailed_box_occupation_bats.clear();
  //cout<<"4:"<<endl;
  BEGIN(BOX_DETAILED_OCCUPATION);
}
<INSIDE_BOX_PROGRAMMING>\n|\r|\r\n {BEGIN(BOX_PROGRAMMING);}
<INSIDE_BOX_OCCUP_BATS>\n|\r|\r\n {BEGIN(BOX_OCCUP_BATS);}

<*>\n|\r|\r\n //{printf("%s",config_text);}/*ignore this token in any start condition*/

<*>";;;" /* ignore this token */
<*>";;" /* ignore this token */
<*>";" /* ignore this token */
<*>"," /*ignore this token*/


<*>. //{printf("%s",config_text);}/* ignore this token in any start condition*/


%%



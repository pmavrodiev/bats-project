%option noyywrap
%option nounput
%option prefix="data_"
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

string Tid; string Date; string Time; string HexId; 
char *pch2;

pair<string,string> entry_pair;
extern vector < pair<string,string> >  box_entries; 
extern string Year; //the year of the analysis
extern map<string,string> monaten; //maps months to their numerical value
bool date_flag=false, time_flag=false, hexid_flag=false,skip_line=false;
%}

DIGIT [0-9]
HEXDIGIT [a-fA-F0-9]
BOXDIGIT [a-zA-Z0-9]
LETTER [a-zA-Z]
%x DATA_ENTRY_NORMAL
%x DATA_ENTRY_TROVAN
%%

<INITIAL>{DIGIT}{2}"."{DIGIT}{2}	{
  string local_month, local_day;
  if (!date_flag) {
    pch2 = strtok(data_text,".");
    local_day = pch2;
    pch2 = strtok(NULL,".");
    local_month = pch2;

    Date = Year + local_month + local_day; //20080131
    //printf("A date: %s ",data_text);
    date_flag = true;
    BEGIN(DATA_ENTRY_NORMAL);
  }
}
<INITIAL>{DIGIT}{1,2}"-"{LETTER}{3} {
  //this is how non-Trovian files start in 2007
  string local_month, local_day;
  stringstream ss;
  if (!date_flag) {
    pch2 = strtok(data_text,"-");
    local_day = pch2;
    if (local_day.length() == 1)
      local_day = "0" + local_day;
    pch2 = strtok(NULL,"-");
    local_month = pch2;
    transform(local_month.begin(), local_month.end(),local_month.begin(),::toupper);
  
    ss<<Year<<monaten[local_month]<<local_day;    
    Date = ss.str(); //20080131
    date_flag=true;
    BEGIN(DATA_ENTRY_NORMAL);
    //printf("A date: %s ",Date.c_str());
  }
}

<INITIAL>{DIGIT}+	BEGIN(DATA_ENTRY_TROVAN);


<DATA_ENTRY_NORMAL,DATA_ENTRY_TROVAN>{HEXDIGIT}{6,10} {
  //6 to 10, because the stupid .txt file for 2007 sometimes clipps the 
  //leading zeros :-(
  if (!hexid_flag) {
    HexId = data_text;
  /*   //cout<<data_text<<endl; */
    if (HexId.length() != 10) 
      HexId = string(10-HexId.length(),'0') + HexId;
    transform( HexId.begin(), HexId.end(),HexId.begin(),::toupper);
    if (HexId == "000697B587")
      HexId = "000697B597";
    hexid_flag = true;
  }
}


<DATA_ENTRY_NORMAL,DATA_ENTRY_TROVAN>{DIGIT}{1,2}":"{DIGIT}{2}":"{DIGIT}{2} {
  //time 
  //cout<<"*"<<data_text<<endl;
  if (!time_flag) {
    pch2 = strtok(data_text,":");  
    Time = pch2;
    if (Time.length() == 1)
      Time = "0" + Time;
    pch2 = strtok(NULL,":");
    Time += pch2;
    pch2 = strtok(NULL,":");
    Time += pch2;

    Date += "T" + Time; //20020131T235959  
    time_flag = true;
  }
}

<DATA_ENTRY_TROVAN>{DIGIT}{2}"."{DIGIT}{2}"."{DIGIT}{4} {
  if (!date_flag) {
    string local_month, local_day, local_year;
    pch2 = strtok(data_text,".");
    local_day = pch2;
    pch2 = strtok(NULL,".");
    local_month = pch2;
    pch2 = strtok(NULL,".");
    local_year = pch2;
  
    Date = local_year + local_month + local_day;//20020131
    date_flag = true;
    //printf("Year: %s ",Date.c_str());
  }
}

<DATA_ENTRY_TROVAN>{DIGIT}{4}"-"{DIGIT}{2}"-"{DIGIT}{2} {
  if (!date_flag) {
    //these are the TROVIAN files from 2007
    string local_month, local_day, local_year;  
    pch2 = strtok(data_text,"-");
    local_year = pch2;
    pch2 = strtok(NULL,"-");
    local_month = pch2;
    pch2 = strtok(NULL,"-");
    local_day = pch2;  
    Date = local_year + local_month + local_day;//20020131
    date_flag=true;
    //printf("Year: %s ",Date.c_str());
  }
}

<DATA_ENTRY_TROVAN>{DIGIT}{2}"-"{DIGIT}{2}"-"{DIGIT}{2} {
  if (!date_flag) {
    //these are the TROVIAN files from 2007
    string local_month, local_day, local_year;  
    pch2 = strtok(data_text,"-");
    local_year = pch2;
    pch2 = strtok(NULL,"-");
    local_month = pch2;
    pch2 = strtok(NULL,"-");
    local_day = pch2;  
    Date = local_year + local_month + local_day;//20020131
    date_flag=true;
    //printf("Year: %s ",Date.c_str());
  }
}



<DATA_ENTRY_TROVAN,DATA_ENTRY_NORMAL>[;]+ /* ignore this token */
<DATA_ENTRY_TROVAN,DATA_ENTRY_NORMAL>"OK" /* ignore this token */

<DATA_ENTRY_TROVAN,DATA_ENTRY_NORMAL>"Checksum error" {
   skip_line=true;
}

<DATA_ENTRY_TROVAN,DATA_ENTRY_NORMAL>[\n]+|[\r]+|[\r\n]+ {
  if (date_flag && time_flag && hexid_flag && !skip_line) {
    //cout<<HexId<<"\t"<<Date<<endl;
    entry_pair.first = HexId;
    entry_pair.second = Date;
    box_entries.push_back(entry_pair);
  }
  //else {//something went wrong in the fucking line - ignore
    //cout<<"\t skipped"<<endl;
  //}
  Date = "";
  Time = "";
  HexId = "";
  date_flag = false;
  time_flag = false;
  hexid_flag = false;
  skip_line = false;

  BEGIN(INITIAL);
}


<INITIAL>\n|\r|\r\n /*ignore*/

<*>. //{printf("%s",data_text);}/* ignore this token in any start condition*/


%%



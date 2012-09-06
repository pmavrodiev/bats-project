%{
#include <iostream>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <vector>
#include <utility>
#include <cctype>
#include <algorithm>

using namespace std;

string Tid; string Date; string Time; string HexId;
char *pch;
pair<string,string> entry_pair;
extern vector < pair<string,string> >  box_entries; 
%}


DIGIT [0-9]
HEXDIGIT [a-fA-F0-9]

%%
{DIGIT}{2}"."{DIGIT}{2}"."{DIGIT}{4} {
  string local_month, local_day, local_year;
  
  pch = strtok(yytext,".");
  local_day = pch;
  pch = strtok(NULL,".");
  local_month = pch;
  pch = strtok(NULL,".");
  local_year = pch;
  
  Date = local_year + local_month + local_day;//20020131
  //printf("Year: %s ",Date.c_str());
}

{DIGIT}{2}"."{DIGIT}{2} {
  string local_month, local_day;
  
  pch = strtok(yytext,".");
  local_day = pch;
  pch = strtok(NULL,".");
  local_month = pch;

  Date = "2008" + local_month + local_day; //20080131
  //printf("A date: %s ",yytext);
}

";;" /* ignore this token */
";" /* ignore this token */

{DIGIT}{2}":"{DIGIT}{2}":"{DIGIT}{2}  {
  pch = strtok(yytext,":");
  Time = pch;
  pch = strtok(NULL,":");
  Time += pch;
  pch = strtok(NULL,":");
  Time += pch;

  Date += "T" + Time; //20020131T235959

  if (!HexId.empty()) { //Trovan Unique files
    //HexId should've been already scanned
    if (HexId == "000697B587")
      HexId = "000697B597";
    entry_pair.first = HexId;
    entry_pair.second = Date;
    box_entries.push_back(entry_pair);
    Date = "";
    Time = "";
    HexId = "";
  }
  //printf("Time: %s ",yytext);
}

{HEXDIGIT}{10} {
  HexId = yytext;
  transform( HexId.begin(), HexId.end(),HexId.begin(),::toupper);

  if (!Date.empty()) { //non-Trovan unique files
    if (HexId == "000697B587")
      HexId = "000697B597";
    entry_pair.first = HexId;
    entry_pair.second = Date;
    box_entries.push_back(entry_pair);
    Date = "";
    Time = "";
    HexId = "";
  }
  //printf("TID: %s\n",yytext);
}

"OK" /* ignore this token */

"Checksum error" {
  box_entries.pop_back();
  Date = "";
  Time = "";
  HexId = "";
}

. /* ignore this token */

\n|\r|\r\n /*ignore this token*/

%%

%option noyywrap
%option nounput
%{
#include <iostream>
#include <vector>
#include <utility>
#include <cctype>
#include <algorithm>
#include <map>
#include <sstream>
#include "classes.h"


using namespace std;

extern map<string,bat> bats_to_ids;
extern vector<pair<string,string> > lfpairs;
extern double activities_sum;

int line_num = 1, year, activity=0;
char *pch;
int counter = 0;
string follower, leader,individual;
bool activity_flag=false;

int comment_caller;
%}

DIGIT [0-9]
HEXDIGIT [a-fA-F0-9]
LETTER [a-zA-Z]
%x LFPAIRS
%x ACTIVITY
%x COMMENT
%%
"begin{lfpairs}"	BEGIN(LFPAIRS);
"begin{activity}"	BEGIN(ACTIVITY);
<LFPAIRS>"end{lfpairs}" BEGIN(INITIAL);
<ACTIVITY>"end{activity}" BEGIN(INITIAL);


<LFPAIRS>{HEXDIGIT}{10} {
  pch=strtok(yytext,".");
  if (follower != "")
    leader = pch;
  else 
    follower = pch;
}

<LFPAIRS>{DIGIT}{4} {
  pch=strtok(yytext,".");
  if (leader != "" && follower != "") {
    stringstream ss; ss<<pch; ss>>year;   
    if (bats_to_ids.find(leader) == bats_to_ids.end()) 
      bats_to_ids[leader] = bat(leader,counter++);
    else
      bats_to_ids[leader].add_year(&year);
    if (bats_to_ids.find(follower) == bats_to_ids.end()) 
      bats_to_ids[follower] = bat(follower,counter++);
    else
    bats_to_ids[follower].add_year(&year);
    lfpairs.push_back(pair<string,string>(follower,leader));
  }
  else {
    printf("Error from scanner.lex: something is wrong at line %d\n",line_num);
    exit(1);
  }
  leader = "";
  follower="";
}


<ACTIVITY>{HEXDIGIT}{10} {
  pch=strtok(yytext,".");
  individual = pch;
}

<ACTIVITY>{DIGIT}{4} {
  if (activity_flag)
    activity_flag=false;
}

<ACTIVITY>{DIGIT}+ {
  if (!activity_flag) {
    pch=strtok(yytext,".");
    stringstream ss; ss<<pch; ss>>activity;
    if (individual == "") {
      printf("Error from scanner.lex: something is wrong at line %d\n",line_num);
      exit(1);
    }    
    map<string,bat>::iterator itr;
    itr = bats_to_ids.find(individual);
    if (itr == bats_to_ids.end()) {
      printf("Error from scanner.lex: bat has not id at line %d\n",line_num);
      exit(1);
    }
    itr->second.add_activity(&activity);
    activities_sum += activity;
  }
  activity_flag = true;
}


<*>"/*" {
comment_caller = YY_START;
BEGIN(COMMENT);
}
<*>"*/" BEGIN(comment_caller);


<*>. //{printf("%s",yytext);}/* ignore this token in any start condition*/
<*>\n|\r|\r\n {line_num++;}
%%



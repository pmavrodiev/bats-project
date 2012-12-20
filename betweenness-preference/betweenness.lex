
%{
#include <iostream>
#include <vector>
#include <utility>
#include <cctype>
#include <algorithm>
#include <map>
#include <sstream>
#include "betweenness.h"

using namespace std;


char *pch;
extern custom_edge new_edge;
extern time_unfolded_temporal_network tutn;
int comment_caller;
bool nnodes_flag=false; 
bool ntimesteps_flag=false;
unsigned time_counter = 0;

%}
BINARY_DIGIT [0-1]
DIGIT [0-9]

%x NNODES
%x NTIMESTEPS
%x COMMENT
%x TUTN
%x INSIDE_TUTN
%%

"begin{nnodes}"  BEGIN(NNODES);
"begin{ntimesteps}"  BEGIN(NTIMESTEPS);
"begin{tutn}" BEGIN(TUTN);
<NNODES>"end{nnodes}" BEGIN(INITIAL);
<NTIMESTEPS>"end{ntimesteps}" BEGIN(INITIAL);
<TUTN>"{" BEGIN(INSIDE_TUTN);
<TUTN>"end{tutn}" BEGIN(INITIAL);


<NNODES>{DIGIT}+ {
  unsigned n;
  stringstream ss;
  pch=strtok(yytext,".");
  ss<<pch;ss>>n;
  //cout<<n<<endl;
  tutn.nnodes = n;
  nnodes_flag=true;
}

<NTIMESTEPS>{DIGIT}+ {
  unsigned n;
  stringstream ss;
  pch=strtok(yytext,".");
  ss<<pch;ss>>n;
  //cout<<n<<endl;
  tutn.ntimesteps = n;
  ntimesteps_flag=true;
}

<INSIDE_TUTN>{DIGIT},{DIGIT},{DIGIT} {
  unsigned n1,n2,type;
  stringstream ss1,ss2,ss3;
  pch=strtok(yytext,",");
  ss1<<pch; ss1>>n1;
  if (n1 > tutn.nnodes) {
    cerr<<"Node id "<<n1<<" larger than the total number of nodes "<<tutn.nnodes<<" in config file"<<endl;
    return 1;
  }
  pch=strtok(NULL,",");
  ss2<<pch; ss2>>n2;
  if (n2 > tutn.nnodes) {
    cerr<<"Node id "<<n2<<" larger than the total number of nodes "<<tutn.nnodes<<" in config file"<<endl;
    return 1;
  }
  pch=strtok(NULL,",");
  ss3<<pch; ss3>>type;
  new_edge.from = n1; new_edge.to = n2; new_edge.type=type;  
  tutn.temporal_network[time_counter].push_back(new_edge);
}

<INSIDE_TUTN>\n|\r|\r\n {
  time_counter++;
  BEGIN(TUTN);
}


<*>"/*" {
comment_caller = YY_START;
BEGIN(COMMENT);
}

<*>"*/" BEGIN(comment_caller);


<*>. //{printf("%s",yytext);}/* ignore this token in any start condition*/

<*>\n|\r|\r\n //{printf("%s",yytext);}/*ignore this token in any start condition*/

<<EOF>> {
  if ((time_counter+1) != tutn.ntimesteps) {
    cerr<<"The supplied temporal network("<<time_counter<<") and the expected #time steps("<<tutn.ntimesteps<<") do not match in config file"<<endl;
    return 1;
  }
  return 0;
}


%%



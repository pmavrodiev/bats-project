#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "pstream.h"
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <cctype>


#include <stdio.h>
#include <stdlib.h>
#include <boost/concept_check.hpp>



using namespace std;

string box_occupation_deadline[] = {
"020000",
"030000",
"050000",
"080000"  
};
unsigned sizeof_box_occupation_deadline = sizeof(box_occupation_deadline) /
					  sizeof(string);

vector<string> vbox_occupation_deadline(box_occupation_deadline,					
					box_occupation_deadline+sizeof_box_occupation_deadline);

string lf_delay[] = {"1","2","3","5","7","9","12","15","20","25"};
unsigned sizeof_lf_delay = sizeof(lf_delay) / sizeof(string);
vector<string> vlf_delay(lf_delay,lf_delay+sizeof_lf_delay);

string knowledge_update[] = {"2","3","5","7","9"};
unsigned sizeof_knowledge_update= sizeof(knowledge_update) / sizeof(string);
vector<string> vknowledge_update(knowledge_update,knowledge_update+sizeof_knowledge_update);


string first_part = "\n \
begin{exportdatabase}\n\
0\n\
end{exportdatabase}\n\
\n\
begin{colony}\n\
BS\n\
end{colony}\n\
\n\
begin{year}\n\
2007\n\
end{year}\n\
\n\
begin{centrality}\n\
2\n\
end{centrality}\n\
\n\
begin{rewiring}\n\
0\n\
end{rewiring}\n";

string second_part = "\n\
begin{box_occupation}\n\
0a,\n\
0b,   08.08.2007\n\
0c,   04.06.2007\n\
0d,   07.07.2007\n\
0e,\n\
0f,\n\
33a,\n\
33b,  09.06.2007\n\
33c,  04.07.2007\n\
33d,\n\
67a,\n\
67b,  12.06.2007\n\
67c,\n\
67d,\n\
100a,\n\
100b,\n\
100c,\n\
h1,   13.06.2007\n\
h2,   04.06.2007\n\
h3,   07.06.2007\n\
h4,\n\
h5,   19.06.2007\n\
h6,   24.07.2007\n\
h7,   25.08.2007\n\
h8,\n\
end{box_occupation}\n\
begin{box_installation}\n\
0a,   02.06.2007\n\
0b,   02.06.2007\n\
0c,   02.06.2007\n\
0d,   06.06.2007\n\
0e,   14.07.2007\n\
0f,   16.07.2007\n\
33a,  02.06.2007\n\
33b,  02.06.2007\n\
33c,  02.06.2007\n\
33d,  08.07.2007\n\
67a,  02.06.2007\n\
67b,  02.06.2007\n\
67c,  11.06.2007\n\
100a, 02.06.2007\n\
100b, 02.06.2007\n\
100c, 28.06.2007\n\
h1,   02.06.2007\n\
h2,   02.06.2007\n\
h3,   02.06.2007\n\
h4,   06.06.2007\n\
h5,   09.06.2007\n\
h6,   19.06.2007\n\
h7,   18.07.2007\n\
h8,   27.07.2007\n\
end{box_installation}\n\
begin{bats}\n\
  00001F5C08,\n\
  000064BEA6,\n\
  0006242035,\n\
  0000F8007E,\n\
  0000624F85,\n\
  0001F7D573,\n\
  00061D466F,\n\
  00061791E7,\n\
  00060F600D,\n\
  000644514C,\n\
  00068E1A46,\n\
  000697C596,\n\
  000697D368,\n\
  00068E19F7,\n\
  00065EB019,\n\
  00065EC1B3\n\
end{bats}\n\
begin{box_programming}\n\
0a:none\n\
0b:none\n\
0c:none\n\
0d:none\n\
0e:none\n\
0f:none\n\
0g:none\n\
33a:0006242035,00061D466F,00001F5C08,000064BEA6,000697C596\n\
33b:00068E19F7,00068E1A46,0000624F85,000644514C,00060F600D,0001F7D573\n\
33c:0000F8007E,00061791E7,00065EB019,00065EC1B3,000697D368\n\
33d:00068E19F7,00060F600D,00061791E7,000697D368,0001F7D573\n\
67a:0006242035,0000F8007E,00068E1A46,0000624F85,000644514C,00001F5C08,00060F600D,00061791E7,00065EB019,00065EC1B3,000697D368\n\
67b:0000F8007E,00068E19F7,00061D466F,0000624F85,000644514C,00060F600D,00065EB019,000064BEA6,00065EC1B3,000697C596,0001F7D573\n\
67c:0006242035,00068E19F7,00068E1A46,00061D466F,00001F5C08,00061791E7,000064BEA6,000697C596,000697D368,0001F7D573\n\
100a:all\n\
100b:all\n\
100c:all\n\
h1:all\n\
h2:all\n\
h3:all\n\
h4:all\n\
h5:all\n\
h6:all\n\
h7:all\n\
h8:all\n\
end{box_programming}\n\
begin{box_occup_bats}\n\
0b:00001F5C08,000064BEA6,0006242035,0000624F85,0001F7D573,00060F600D,000697C596,000697D368,00068E19F7\n\
0c:000064BEA6,0000F8007E,0000624F85,000697C596,000697D368,00065EC1B3\n\
0d:00001F5C08,0006242035,0000F8007E,0000624F85,0001F7D573,00061D466F,00061791E7,00060F600D,000644514C,00068E1A46,000697D368,00068E19F7,00065EB019,00065EC1B3\n\
33b:0000F8007E,0001F7D573,00061D466F,00061791E7,000644514C,00068E1A46,000697C596,000697D368,00068E19F7,00065EB019\n\
33c:00061791E7,00068E19F7,00065EC1B3\n\
67b:000064BEA6,0006242035,0000F8007E,0001F7D573,00061791E7,00060F600D,000644514C,00068E1A46,000697C596,000697D368,00065EB019,00065EC1B3\n\
h1:00001F5C08,000064BEA6,0006242035,0000F8007E,0000624F85,0001F7D573,00061D466F,00061791E7,00060F600D,000644514C,00068E1A46,000697C596,000697D368,00068E19F7,00065EB019,00065EC1B3\n\
h2:00001F5C08,000064BEA6,0000624F85,00061D466F,000644514C,00068E1A46,000697C596,000697D368,00068E19F7,00065EB019,00065EC1B3\n\
h3:00001F5C08,000064BEA6,0006242035,0000F8007E,0000624F85,0001F7D573,00061D466F,00061791E7,00060F600D,000644514C,00068E1A46,000697C596,000697D368,00068E19F7,00065EB019,00065EC1B3\n\
h5:00001F5C08,000064BEA6,0000624F85,0001F7D573,00061D466F,00060F600D,000644514C,00068E1A46,000697C596,000697D368,00068E19F7,00065EB019,00065EC1B3\n\
h6:00001F5C08,0006242035,0000F8007E,0000624F85,0001F7D573,00061D466F,00060F600D,000644514C,000697D368,00068E19F7,00065EC1B3\n\
h7:0006242035,000644514C,000697D368\n\
end{box_occup_bats}\n\
begin{transponders}\n\
000697B566,\n\
000624514F,\n\
00068E0CFF,\n\
00065E632D,\n\
00068EA92B,\n\
6C065E632D,\n\
FFFFFFFFFF,\n\
8E602D0D0F,\n\
00068E24FE,\n\
00065EC57E,\n\
000697B2A6,\n\
000697A117,\n\
0006979C67,\n\
000615D774,\n\
00060FA2CA,\n\
0006444F30,\n\
0005FE0832,\n\
00065D7D8B,\n\
000697D078,\n\
000697B05B,\n\
0006160637,\n\
00064427F2,\n\
00065DD7F6,\n\
00065D81D3,\n\
000697B0DE,\n\
00061D3DC5,\n\
00060D7AE7\n\
end{transponders}\n\
";



int main() {
  
ofstream log("run-log.log",ios::out);  
if (!log.good()) {
  perror("run-log.log");
  exit(1);
}



stringstream remaining_part;
remaining_part<<first_part;


  for (unsigned j=0; j<vbox_occupation_deadline.size(); j++) {   
      remaining_part<<"\nbegin{occupation_deadline}\n"<<vbox_occupation_deadline[j];
      remaining_part<<"\nend{occupation_deadline}\n";     

  for (unsigned i=0; i<vknowledge_update.size(); i++) {
    remaining_part<<"\nbegin{bat_update}\n"<<vknowledge_update[i]<<"\nend{bat_update}\n";            
    for (unsigned k=0; k<vlf_delay.size();k++) {
      remaining_part<<"\nbegin{lf_delay}\n"<<vlf_delay[k]<<"\nend{lf_delay}\n";
      remaining_part<<"\nbegin{box_occupation_deadline} \n\
0a,\t"<<vbox_occupation_deadline[j]<<"\n\
0b,\t"<<vbox_occupation_deadline[j]<<"\n\
0c,\t"<<vbox_occupation_deadline[j]<<"\n\
0d,\t"<<vbox_occupation_deadline[j]<<"\n\
0e,\t"<<vbox_occupation_deadline[j]<<"\n\
0f,\t"<<vbox_occupation_deadline[j]<<"\n\
33a,\t"<<vbox_occupation_deadline[j]<<"\n\
33b,\t"<<vbox_occupation_deadline[j]<<"\n\
33c,\t"<<vbox_occupation_deadline[j]<<"\n\
33d,\t"<<vbox_occupation_deadline[j]<<"\n\
67a,\t"<<vbox_occupation_deadline[j]<<"\n\
67b,\t"<<vbox_occupation_deadline[j]<<"\n\
67c,\t"<<vbox_occupation_deadline[j]<<"\n\
67d,\t"<<vbox_occupation_deadline[j]<<"\n\
100a,\t"<<vbox_occupation_deadline[j]<<"\n\
100b,\t"<<vbox_occupation_deadline[j]<<"\n\
100c,\t"<<vbox_occupation_deadline[j]<<"\n\
h1,\t"<<vbox_occupation_deadline[j]<<"\n\
h2,\t"<<vbox_occupation_deadline[j]<<"\n\
h3,\t"<<vbox_occupation_deadline[j]<<"\n\
h4,\t"<<vbox_occupation_deadline[j]<<"\n\
h5,\t"<<vbox_occupation_deadline[j]<<"\n\
h6,\t"<<vbox_occupation_deadline[j]<<"\n\
h7,\t"<<vbox_occupation_deadline[j]<<"\n\
h8,\t"<<vbox_occupation_deadline[j]<<"\n end{box_occupation_deadline}\n";      
      
      remaining_part<<second_part;
     
      /*create temporary file*/
      char buffer [L_tmpnam];
      string temp_file = tmpnam (buffer);
      ofstream os_temp(temp_file.c_str(),ios::out);
      if (!os_temp.good()) {
	perror(temp_file.c_str());
	exit(1);
      }
      os_temp<<remaining_part.str();
      os_temp.close();
      string command = "./bats /home/pmavrodiev/Documents/bats/data/Blutsee2007/BS_2008_sortiert-von-Daniela/ ";
      command = command + temp_file;
      log<<"RUNNING"<<endl;
      log<<command<<endl;
      redi::ipstream proc(command, redi::pstreams::pstderr);
      std::string line;
      // read child's stdout
      while (std::getline(proc.out(), line))
	log<< "stdout: "<<line<<endl;
      // read child's stderr
      while (std::getline(proc.err(), line))
	log<< "stderr: "<<line<<endl;      
    }
  }

}
  log.close();
  return 0;
}
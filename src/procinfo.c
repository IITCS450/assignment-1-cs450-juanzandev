#include "common.h"
#include <ctype.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
static void usage(const char *a){fprintf(stderr,"Usage: %s <pid>\n",a); exit(1);}
static int isnum(const char*s){for(;*s;s++) if(!isdigit(*s)) return 0; return 1;}
int main(int c,char**v){
 if(c!=2||!isnum(v[1])) usage(v[0]);
 // printf("TODO: implement procinfo\n");
 int pid = atoi(v[1]);
 printf("PID:%d\n", pid);

 // stat proc (/proc/<pid>/stat)
 
 //file path
 char path[64];
 FILE *fp;
 
 snprintf(path, sizeof(path), "/proc/%d/stat", pid);
 fp = fopen(path, "r");
 if (!fp) {
    perror("stat");
    return 1;
 }
 // getting status from file
 char state;
 // retrieving character from field 3 (<PID> <COMM> <STATUS> <PPID>)
 // %*d skips the pid
 // then %*[^)] reads everything until the parenthesis from comm
 // %*c skips the parenthesis
 // %c reads the status character
 //fscanf(fp, "%*d %*[^)] %*c %c", &state);
 

 // same logic with PPID except that we skip the state character and we read %d
 int ppid;
 //fscanf(fp, "%*d %*[^)] %*c %*c %d", &ppid); 
 

 // stime and utime are in fields 14 and 15
 // so we need to skip all fields before that
 unsigned long utime, stime;
 
 //fscanf(fp, "%*d %*[^)]%*c %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %lu %lu", &utime, &stime);
 
 fscanf(fp,
    "%*d %*[^)]%*c %c %d"
    "%*d %*d %*d %*d %*d %*u %*u %*u %*u %*u"
    "%lu %lu",
    &state, &ppid, &utime, &stime);


 fclose(fp);

 
 // for cmdline we need to access /proc/%d/cmdline. The first part of the file should have the name of the cmdline
 snprintf(path, sizeof(path), "/proc/%d/cmdline", pid);
 fp = fopen(path, "r");
 if (!fp) {
    perror("cmdline");
    return 1;
 }

 char cmd[4096];
 size_t len = fread(cmd, 1, sizeof(cmd) -1, fp);
 fclose(fp);

 if (len == 0){
    printf("Cmd:\n");
    return 0;
 }
 for (size_t i = 0; i < len; i++){
    if (cmd[i] == '\0'){
        cmd[i] = ' ';
    }
 }
 cmd[len] = '\0';

 // now toget cpu_seconds we need to add utime and stime, get the ticks per second and then do cpu_ticks / ticks per sec
 unsigned long cpu_ticks = utime + stime;
 long ticks_per_sec = sysconf(_SC_CLK_TCK);
 double cpu_seconds = (double)cpu_ticks / ticks_per_sec;

 // Finally, to get VmRSS we need to access proc/%d/status which is the resident memory in kilobytes in this case
 
 snprintf(path, sizeof(path), "/proc/%d/status", pid);
 fp = fopen(path, "r");
 if (!fp) {
     perror("status");
     return 1;
 }
 int vmrss = 0;
 char line[256];

 while (fgets(line, sizeof(line), fp)) {
    if (strncmp(line, "VmRSS:", 6) == 0) {
        sscanf(line, "VmRSS: %d", &vmrss);
        break;
    }
 }
 fclose(fp);



 printf("State:%c\n", state);
 printf("PPID:%d\n", ppid); 
 printf("Cmd: %s\n", cmd);
 printf("CPU:%lu %.3f\n", cpu_ticks, cpu_seconds); 
 printf("VmRSS: %d\n", vmrss);
 return 0;
}

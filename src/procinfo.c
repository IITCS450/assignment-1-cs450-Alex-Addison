#include "common.h"
#include <ctype.h>
#include <stdio.h>
#include <unistd.h>

static void usage(const char *a){
	fprintf(stderr,"Usage: %s <pid>\n",a); 
	exit(1);
}

static int isnum(const char*s){
	for(;*s;s++) 
		if(!isdigit(*s)) 
			return 0; 
	return 1;
}

// Acounts for non-numeric PID, PID not found, and permission errors
int main(int c,char**v){
	// non-numeric PID
	if(c!=2||!isnum(v[1])) 
		usage(v[0]);

	// build the /proc/<pid>/stat path using the provided pid argument
	char * pid = v[1];
	char proc_path[64];
	snprintf(proc_path, sizeof(proc_path), "/proc/%s/stat", pid);

	FILE *procStat = fopen(proc_path, "r");

	snprintf(proc_path, sizeof(proc_path), "/proc/%s/status", pid);
	FILE *procStatus = fopen(proc_path, "r");

	snprintf(proc_path, sizeof(proc_path), "/proc/%s/cmdline", pid);
	FILE *procCmdline = fopen(proc_path, "r");

	// PID does not exist and permission denied
	if (!procStat){
		if (errno == EACCES) {
        	fprintf(stderr, "Permission denied reading %s\n", proc_path);
        	return 1;
    	}
		printf("PID does not exist.\n");
			return 1;
	}

	char line[256];
	char state;
	unsigned long utime, stime;
	int ppid;
	unsigned long rss;
	char cmdline[256];

	if (fgets(line, sizeof(line), procStat) != NULL) {
		// Parse the required fields from the /proc/<pid>/stat file
		sscanf(line, "%*d %*s %c %*d %d %*d %*d %*u %*u %*u %*u %*u %lu %lu",
			   &state, &ppid, &utime, &stime);
	}
	else {
		printf("Failed to read procStat\n");
		fclose(procStat);
		return 1;
	}

	// Read the command line from /proc/<pid>/cmdline
	// fread does whole file since args are \0 separated
	size_t n = fread(cmdline, 1, sizeof(cmdline) - 1, procCmdline);
	if (n == 0) {
		// empty cmdline
		cmdline[0] = '\0';
	} else {
		// Replace \0 with spaces until size n
		for (size_t i = 0; i < n; i++) {
			if (cmdline[i] == '\0') cmdline[i] = ' ';
		}
		cmdline[n] = '\0';
	}
	fclose(procCmdline);

	if (fgets(line, sizeof(line), procStatus) != NULL) {
		// Parse VmRSS field from /proc/<pid>/status
		char field[256];
		while (fgets(line, sizeof(line), procStatus) != NULL) {
			if (sscanf(line, "%s %lu", field, &rss) == 2 && 
				strcmp(field, "VmRSS:") == 0) {
				break;
			}
		}
	}
	else {
		printf("Failed to read procStatus\n");
		fclose(procStatus);
		return 1;
	}

	// Print the extracted information
	printf("Process State: %c\n", state);
	printf("Parent PID: %d\n", ppid);
	printf("Command Line: %s\n", cmdline);
	// sum stime and utime for total CPU time and divide by sysconf(_SC_CLK_TCK) to convert to seconds
	printf("CPU Time: %lu\n", (utime+stime)/sysconf(_SC_CLK_TCK));
	printf("Resident Memory Usage: %lu kB\n", rss);
	fclose(procStat);

 	return 0;
}

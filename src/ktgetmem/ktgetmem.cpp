#include <stdio.h>
#include <string>

#include <sys/sysinfo.h>


void getfreemem()
{
	struct sysinfo sys_info;
    sysinfo(&sys_info);
    sysinfo(&sys_info);
    int memlimit = sys_info.freeram / 1024 * sys_info.mem_unit;
    int slack = memlimit / 20;
    memlimit = memlimit - slack;
    printf ("%d\n",memlimit);
    return;
	
}

void getcgroupLimit(int core_count)
{
	char line[1024];
	FILE *fin = fopen("/sys/fs/cgroup/memory/memory.limit_in_bytes","rb");
	if (fin == NULL) {
		fprintf(stderr,"Open failed");
		return;
	}	
	fgets(line, sizeof(line), fin);

	std::string s = line;
	s.erase(s.length()-1);	// strip new line
	if (s == "9223372036854771712") {
		getfreemem();
	}else {
		long long int memlimit;
		if (sscanf(line, "%lld", &memlimit) != 1){
	           fprintf(stderr, "Invalid data : %s\n", line);
	           return ;
	    }

	    fclose(fin);

	    memlimit = memlimit / 1024;	   	// convert to kilobytes

	    long long int slack = memlimit / 20;	// leave 5 percent of memory
	    if (slack < 4096) {
	    	slack =4096;
	    }
	    memlimit = memlimit - slack;		// remove 5 % from mem from limit	
	    memlimit = memlimit / core_count;
		printf("%lld",memlimit);		// ouput memory limit - 4 meg		
	}
}


int main(int argc, char *argv[])
{
	int core_count = atoi(argv[1]);
	getcgroupLimit(core_count);	
}
#include <stdio.h>

void getcgroupLimit()
{
	char line[1024];
	FILE *fin = fopen("/sys/fs/cgroup/memory/memory.limit_in_bytes","rb");
	if (fin == NULL) {
		fprintf(stderr,"Open failed");
		return;
	}
	fgets(line, sizeof(line), fin);
	printf(line);
	fclose(fin);
}


int main()
{
	getcgroupLimit();
}
#include <stdio.h>
#include <string.h>

int main() {
	int i = system("ls -l | wc -l");
	printf("%d\n", i);
	return 0;
}
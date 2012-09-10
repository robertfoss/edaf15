#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct {
	double				zero;
	int				i;
	char				s[6];
	char				t[6];
	unsigned long long		x;
	unsigned long long		y;
} a = { 0, 1756, "mozart", "???", 8243124871055238688ull };

size_t strlen(const char* s)
{
	int		i;
	
	i = 0;

	while (s[i] != 0)
		i++;

	return i;
}
	
int main()
{
	int	length;

	strncpy(a.t, a.s, 6);
	
	length = strlen(a.t);

	printf("a.t is %s with length %d\n", a.t, length);

	return 0;
}

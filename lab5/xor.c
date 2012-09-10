#define N	(10)

int	n = N;

int	a = 0xedaf15;
int	b = 0x970;
int	s;

int main(void)
{
	int	i;
	int	j;

	for (i = 0; i < 2; ++i)
		for (j = 0; j < n; ++j)
			s = s ^ a * b;
}

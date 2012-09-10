#define N	(10)

int	n = N;

float	a = 0xedaf15;
float	b = 970;
float	s;

int main(void)
{
	int	i;
	int	j;

	for (i = 0; i < 2; ++i)
		for (j = 0; j < n; ++j)
			s = s + a * b;
}

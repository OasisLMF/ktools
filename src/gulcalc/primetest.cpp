
// code from http://stackoverflow.com/questions/4424374/determining-if-a-number-is-prime

#include <inttypes.h>

int64_t power(int a, int n, int mod)
{
	int64_t power = a, result = 1;

	while (n)
	{
		if (n & 1)
			result = (result*power) % mod;
		power = (power*power) % mod;
		n >>= 1;
	}
	return result;
}

bool witness(int a, int n)
{
	int t, u, i;
	int64_t prev, curr;

	u = n / 2;
	t = 1;
	while (!(u & 1))
	{
		u /= 2;
		++t;
	}

	prev = power(a, u, n);
	for (i = 1; i <= t; ++i)
	{
		curr = (prev*prev) % n;
		if ((curr == 1) && (prev != 1) && (prev != n - 1))
			return true;
		prev = curr;
	}
	if (curr != 1)
		return true;
	return false;
}

bool isPrime(int number)
{
	if (((!(number & 1)) && number != 2) || (number < 2) || (number % 3 == 0 && number != 3))
		return (false);

	if (number < 1373653)
	{
		for (int k = 1; 36 * k*k - 12 * k < number; ++k)
			if ((number % (6 * k + 1) == 0) || (number % (6 * k - 1) == 0))
				return (false);

		return true;
	}

	if (number < 9080191)
	{
		if (witness(31, number)) return false;
		if (witness(73, number)) return false;
		return true;
	}


	if (witness(2, number)) return false;
	if (witness(7, number)) return false;
	if (witness(61, number)) return false;
	return true;
}

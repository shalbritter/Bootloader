/*
 * Bootloader.c
 *
 * Created: 08.06.2017 12:45:00
 * Author : x10mi
 */ 

//include//////////////////////////////////////////////////////////////////////////

//prototypes///////////////////////////////////////////////////////////////////////
uint16_t atoi_hex(const char *__s);
char* reverse(char *__s, int __len);
char* itoa(int __val, char *__s, int __radix);



//main/////////////////////////////////////////////////////////////////////////////
int main(void)
{
    /* Replace with your application code */
    while (1) 
    {
    }
}

//functions////////////////////////////////////////////////////////////////////////
uint16_t atoi_hex(const char *__s) {
	uint16_t ret = 0;
	while (*(__s+1))
	{
		if ('0' <= *__s && *__s <= '9')
		{
			ret += (*__s - '0');
		}
		else if ('A' <= *__s && *__s <= 'F')
		{
			ret += (*__s - 'A' + 10);
		}
		ret *= 16;
		__s++;
	}
	if ('0' <= *__s && *__s <= '9')
	{
		ret += (*__s - '0');
	}
	else if ('A' <= *__s && *__s <= 'F')
	{
		ret += (*__s - 'A' + 10);
	}
	return ret;
}

char* reverse(char *__s, int __len) {
	int start = 0, end = __len - 1;
	while (start < end)
	{
		char temp = __s[start];
		__s[start] = __s[end];
		__s[end] = temp;
		start++;
		end--;
	}
	return __s;
}


char* itoa(int __val, char *__s, int __radix) {
	int i = 0;
	if (__val == 0)
	{
		__s[i++] = '0';
		__s[i] = '\0;';
		return __s;
	}
	while (__val != 0)
	{
		int rem = __val % __radix;
		__s[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
		__val = __val / __radix;
	}

	__s[i] = '\0';
	__s = reverse(__s, i);
	return __s;
}

#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

#define HEAP_SIZE	128

static char heap[HEAP_SIZE];

static char* _cur_brk = heap;

void sendchar(char ch);
int recvchar(void);

int _read_r(struct _reent* r, int file, char* ptr, int len)
{
	int i, ch;

	for (i = 0; i < len; i++)
	{
		ch = recvchar();
		if (ch < 0) break;
		if (ch == '\r') ch = '\n';
		sendchar(ch);
		ptr[i] = (char)ch;
	}

	return i;
}

int _write_r(struct _reent* r, int file, char* ptr, int len)
{
	int i;

	for (i = 0; i < len; i++)
	{
		if (ptr[i] == '\n')
		{
			sendchar('\r');
		}
		sendchar(ptr[i]);
	}

	return len;
}

int _lseek_r(struct _reent* r, int file, int ptr, int dir)
{
	return 0;
}

int _close_r(struct _reent* r, int file)
{
	return -1;
}

caddr_t _sbrk_r(struct _reent* r, int incr)
{
	char* _old_brk = _cur_brk;

	if ((_cur_brk + incr) > (heap + HEAP_SIZE))
	{
		errno = ENOMEM;
		return (void *)-1;
	}

	_cur_brk += incr;

	return (caddr_t)_old_brk;
}

int _fstat_r(struct _reent* r, int file, struct stat* st)
{
	st->st_mode = S_IFCHR;
	return 0;
}

int _isatty_r(struct _reent* r, int fd)
{
	return 1;
}

void _exit(int rc)
{
	for (;;);
}

int _kill(int pid, int sig)
{
	errno = EINVAL;
	return -1;
}

int _getpid(void)
{
	return 1;
}

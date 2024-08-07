/*------------------------------------------------------------------------
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 ------------------------------------------------------------------------*/

#include "core.h"
#include "timer.h"
#include "socket.h"

extern int SERVER_TYPE;
static void (*term_func)(void) = NULL;
void set_termfunc(void (*termfunc)(void))
{
	term_func = termfunc;
}

void debug_output(char *string, ...)
{
	va_list ap;
	char *p, * sval;
	int ival;
	long lval;
	double dval;
	struct timeval tv;
	char tmpstr[24];
	gettimeofday(&tv, NULL);
	strftime(tmpstr, 24, "%d/%m/%Y %I:%M:%S", localtime(&(tv.tv_sec)));
	FILE *fp = NULL;

	if (SERVER_TYPE == 1) {
		if (!(fp = fopen("logs/account_tracer.log", "rt"))) {
			fclose(fp);
			fp = fopen("logs/account_tracer.log", "a+");
			fprintf(fp, "REPORTED TIME\t   FUNCTION CALL\t   MESSAGE INFO\n");
		}
		else
			fp = fopen("logs/account_tracer.log", "a+");

	}
	else if (SERVER_TYPE == 2) {
		if (!(fp = fopen("logs/character_tracer.log", "rt"))) {
			fclose(fp);
			fp = fopen("logs/character_tracer.log", "a+");
			fprintf(fp, "REPORTED TIME\t   FUNCTION CALL\t   MESSAGE INFO\n");
		}
		else
			fp = fopen("logs/character_tracer.log", "a+");

	}
	else if (SERVER_TYPE == 3) {
		if (!(fp = fopen("logs/zone_tracer.log", "rt"))) {
			fclose(fp);
			fp = fopen("logs/zone_tracer.log", "a+");
			fprintf(fp, "REPORTED TIME\t  FUNCTION CALL\t   MESSAGE INFO\n");
		}
		else
			fp = fopen("logs/zone_tracer.log", "a+");

	}
	fprintf(fp, "%s: ", tmpstr);
	va_start(ap, string);
	if (fp) {
		for (p = string; *p; p++) {
			if (*p != '%') {
				putc(*p, fp);
				continue;
			}
			switch (*++p) {
			case 'l':
				lval = va_arg(ap, long);
				fprintf(fp, "%ld", lval);
				break;
			case 'd':
				ival = va_arg(ap, int);
				fprintf(fp, "%d", ival);
				break;
			case 'f':
				dval = va_arg(ap, double);
				fprintf(fp, "%f", dval);
				break;
			case 's':
				for (sval = va_arg(ap, char *); *sval; sval++)
					putc(*sval, fp);
				break;
			default:
				putc(*p, fp);
				break;
			}
		}
		fclose(fp);
		va_end(ap);
	}
	else {
		fprintf(stderr, "debug_output: Fail to open file stream.\n");
		exit(0);
	}
}

static void sig_proc(int sn)
{
	unsigned int i;

	switch (sn) {
	case SIGINT:
	case SIGABRT:
	case SIGFPE:
	case SIGSEGV:
	case SIGTERM:
		if (term_func)
			term_func();

		for (i = 0; i < fd_max; i++) {
			if (!session[i])
				continue;

			close(i);
		}
		do_exit();
		exit(0);
		break;
	}
}

char *strtolower(char *s)
{
	while (*s != '\0') {
		if (*s >= 65 && *s <= 90)
			*s = *s + 32;

		s++;
	}
	return s;
}

void filecopy(FILE *from, FILE *to)
{
	int i;

	while ((i = getc(from)) != EOF)
		putc(i, to);

}

int randN(int n)
{
	int i;

	while ((i = (int)((double)rand() / (double)RAND_MAX * n)) == n);
	return i;
}

int main()
{
	time_t seconds;
	register int next;

	signal(SIGPIPE, SIG_IGN);
	signal(SIGABRT, sig_proc);
	signal(SIGFPE,  sig_proc);
	signal(SIGINT,  sig_proc);
	signal(SIGSEGV, sig_proc);
	signal(SIGTERM, sig_proc);

	do_socket();

	time(&seconds);
	srand((unsigned)seconds);

	if (do_init() != 0) {
		debug_output("main: 'do_init()' function call fail.\n");
		return 0;
	}
	for (;;) {
		next = do_timer(gettick());
		do_sendrecv(next);
		do_parsepacket();
	}
	return 0;
}

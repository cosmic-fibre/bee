#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <string.h>
#include <errno.h>

#include "unit.h"
#include <fiob.h>
#include <say.h>
#include <stdarg.h>
#include <errno.h>
#include <stdlib.h>



#define PLAN		67

#define ITEMS		7


const char *
catfile(const char *a, const char *b)
{
	size_t la = strlen(a);

	static char r[4096];

	strcpy(r, a);

	if (a[la - 1] == '/' && b[0] == '/') {
		strcat(r, b + 1);
		return r;
	}

	if (a[la - 1]== '/') {
		strcat(r, b);
		return r;
	}
	r[la] = '/';
	r[la + 1] = 0;
	strcat(r, b);
	return r;
}


void sayf(int level, const char *filename, int line, const char *error,
                          const char *format, ...)
{
	const char *dbg = getenv("DEBUG");
	if (!dbg)
		return;
	if (strcmp(dbg, "1") != 0)
		return;

	printf("# ");
	va_list ap;
	va_start(ap, format);
	vprintf(format, ap);
	va_end(ap);
	printf("\n#\tat %s line %d\n", filename, line);
	if (error)
		printf("#\t%s\n", error);
}
sayfunc_t _say = sayf;

int
main(void)
{
	plan(PLAN);

	/* don't create test files in /tmp - tmpfs doesn't support O_DIRECT */
	char *td = mkdtemp(strdup("./fiob.XXXXXX"));
	isnt(td, NULL, "tempdir is created");
	if (td == 0) {
		diag("Can't create temporary dir: %s", strerror(errno));
		return -1;
	}

	static char buf[4096];

	{

		FILE *f = fiob_open(catfile(td, "t0"), "w+d");
		isnt(f, NULL, "common open");
		size_t done = fwrite("Hello, world", 1, 12, f);
		is(done, 12, "Hello world is written (%zu bytes)", done);

		is(ftell(f), 12, "current position");
		is(fseek(f, 0L, SEEK_SET), 0, "set new position");
		is(ftell(f), 0, "current position %li", ftell(f));


		done = fread(buf, 1, 12, f);
		is(done, 12, "Hello world is read (%zu bytes)", done);
		is(memcmp(buf, "Hello, world", 12), 0, "data");

		is(fseek(f, 7L, SEEK_SET), 0, "set odd position");
		is(ftell(f), 7L, "check odd position");
		is(fread(buf, 1, 4096, f), 5, "read from odd position (size)");
		is(memcmp(buf, "world", 5), 0, "read from odd position (data)");

		is(fseek(f, 0L, SEEK_SET), 0, "set start position");
		is(ftell(f), 0L, "check start position");

		is(fseek(f, 0L, SEEK_END), 0, "set eof position");
		is(ftell(f), 12L, "check eof position");
		is(fread(buf, 1, 4096, f), 0, "read from eof position (size)");
		ok(feof(f), "feof");
		is(fread(buf, 1, 4096, f), 0, "read from eof position (size)");
		ok(feof(f), "feof");

		is(fseek(f, -1L, SEEK_END), 0, "set -1 position");
		is(ftell(f), 11L, "check -1 position");
		is(fread(buf, 1, 4096, f), 1, "read from -1 position (size)");
		is(memcmp(buf, "d", 1), 0, "read from -1 position (data)");
		ok(feof(f), "feof");

		is(fread(buf, 1, 4096, f), 0, "read from eof position (size)");

		is(fseek(f, 0L, SEEK_SET), 0, "set new position");
		done = fread(buf + 1, 1, 12, f);
		is(done, 12, "Hello world is read (%zu bytes)", done);
		is(memcmp(buf + 1, "Hello, world", 12), 0, "data");


		is(fseek(f, 0L, SEEK_SET), 0, "set new position");
		fwrite("ololo ololo ololo", 1, 17, f);
		is(fseek(f, 1L, SEEK_SET), 0, "set new position");

		done = fread(buf + 1, 1, 12, f);
		is(done, 12, "data is read");
		is(memcmp(buf + 1, "lolo ololo ololo", 12), 0, "data is read");

		is(fclose(f), 0, "fclose");

		f = fopen(catfile(td, "t0"), "r");
		isnt(f, NULL, "reopened file");
		is(fseek(f, 0L, SEEK_END), 0, "move pos at finish");
		is(ftell(f), 17, "file size");
		is(fclose(f), 0, "fclose");

		f = fiob_open(catfile(td, "t0"), "w+x");
		is(f, NULL, "common open: O_EXCL");
	}

	{
		FILE *f = fiob_open(catfile(td, "t1"), "w+");
		isnt(f, NULL, "common open");
		size_t done = fwrite("Hello, world", 1, 12, f);
		is(done, 12, "Hello world is written (%zu bytes)", done);

		is(fseek(f, 1, SEEK_SET), 0, "move pos");
		done = fwrite("Hello, world", 1, 12, f);
		is(done, 12, "Hello world is written (%zu bytes)", done);

		is(fseek(f, 2, SEEK_SET), 0, "move pos");
		done = fread(buf, 1, 12, f);
		is(done, 11, "read 11 bytes");
		is(memcmp(buf, "ello, world", 11), 0, "content was read");

		is(fclose(f), 0, "fclose");
	}

	{
		FILE *f = fiob_open(catfile(td, "tm"), "wxd");
		isnt(f, NULL, "open big file");
		size_t done = fwrite("Hello, world\n", 1, 13, f);
		is(done, 13, "Hello world is written (%zu bytes)", done);

		size_t i;

		for (i = 0; i < 1000000; i++) {
			done += fwrite("Hello, world\n", 1, 13, f);
		}
		is(done, 13 + 13 * 1000000, "all bytes were written");
		is(fclose(f), 0, "fclose");

		f = fopen(catfile(td, "tm"), "r");
		isnt(f, NULL, "reopen file for reading");
		done = 0;
		for (i = 0; i < 1000000 + 1; i++) {
			buf[0] = 0;
			char *res = fgets(buf, 4096, f);
			(void) res;
			if (strcmp(buf, "Hello, world\n") == 0)
				done++;
		}
		is(done, 1000000 + 1, "all records were written properly");

		is(fgets(buf, 4096, f), NULL, "eof");
		isnt(feof(f), 0, "feof");
		is(fclose(f), 0, "fclose");
	}
	{
		FILE *f = fiob_open(catfile(td, "tm"), "w+d");
		setvbuf(f, NULL, _IONBF, 0);
		isnt(f, NULL, "open big file");
		size_t done = fwrite("Hello, world\n", 1, 13, f);
		is(done, 13, "Hello world is written (%zu bytes)", done);

		size_t i;

		for (i = 0; i < 1000000; i++) {
			done += fwrite("Hello, world\n", 1, 13, f);
		}
		is(done, 13 + 13 * 1000000, "all bytes were written");
		is(fclose(f), 0, "fclose");

		f = fopen(catfile(td, "tm"), "r");
		isnt(f, NULL, "reopen file for reading");
		done = 0;
		for (i = 0; i < 1000000 + 1; i++) {
			memset(buf, 0, 4096);
			char *res = fgets(buf, 4096, f);
			(void) res;
			if (strcmp(buf, "Hello, world\n") == 0)
				done++;
#if 0
			else {
				fprintf(stderr, "#   wrong line %zu: %s",
					i, buf);
			}
#endif
		}
		is(done, 1000000 + 1, "all records were written properly");

		is(fgets(buf, 4096, f), NULL, "eof");
		isnt(feof(f), 0, "feof");
		is(fclose(f), 0, "fclose");
	}

	{
		FILE *f = fiob_open("/dev/full", "wd");
		if (f) {
			errno = 0;
			fputs("test", f);
			/* flush buffer && close file */
			int r = fclose(f);
			is(errno, ENOSPC, "fwrite failed");
			is(r, EOF, "fwrite failed");
		} else {
			/* System doesn't have /dev/full */
			ok(1, "fwrite failed");
			ok(1, "fwrite failed")
		}
	}

	if (fork() == 0)
		execl("/bin/rm", "/bin/rm", "-fr", td, NULL);

	free(td);
	return check_plan();
}

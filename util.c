/* sxiv: util.c
 * Copyright (c) 2013 Bert Muennich <be.muennich at gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#define _POSIX_C_SOURCE 200112L

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "util.h"

enum { BUFLEN = 32 };

void cleanup();

void warn(const char *fmt, ...) {
	va_list args;

	if (!fmt)
		return;

	va_start(args, fmt);
	fprintf(stderr, "physlock: warning: ");
	vfprintf(stderr, fmt, args);
	fprintf(stderr, "\n");
	va_end(args);
}

void die(const char *fmt, ...) {
	va_list args;

	if (!fmt)
		return;

	va_start(args, fmt);
	fprintf(stderr, "physlock: error: ");
	vfprintf(stderr, fmt, args);
	fprintf(stderr, "\n");
	va_end(args);

	cleanup();
	exit(1);
}

/*
 * Read a file to a buffer.
 * The buffer is ensured to be NULL-terminated.
 * The call always succeeds (it dies() on failure).
 * Returns the number of characters read.
 */
static size_t read_file(const char *path, char *buf, size_t len) {
	FILE *ctl_file;
	int nread;

	assert(path != NULL);

	ctl_file = fopen(path, "r");
	if (ctl_file == NULL)
		die("could not open file: %s: %s", path, strerror(errno));

	nread = fread(buf, 1, len - 1, ctl_file);
	if (ferror(ctl_file))
		die("could not read file: %s: %s", path, strerror(errno));

	if (fclose(ctl_file) != 0)
		die("could not close file: %s: %s", path, strerror(errno));

	buf[nread] = '\0';

	return nread;
}

/*
 * Write a buffer into a file.
 * The call always succeeds (it dies() on failure).
 * Returns the number of characters written.
 */
static size_t write_file(const char *path, char *buf, size_t len) {
	FILE *ctl_file;
	int nwritten;

	assert(path != NULL);

	ctl_file = fopen(path, "w+");
	if (ctl_file == NULL)
		die("could not open file: %s: %s", path, strerror(errno));

	nwritten = fwrite(buf, 1, len, ctl_file);
	if (ferror(ctl_file))
		die("could not write file: %s: %s", path, strerror(errno));

	if (fclose(ctl_file) != 0)
		die("could not close file: %s: %s", path, strerror(errno));

	return nwritten;
}

/*
 * Read integer from file, and ensure the next character is as expected.
 * The call always succeeds (it dies() on failure).
 */
static int read_int_from_file(const char *path, char ending_char) {
	char buf[BUFLEN], *end;
	int value;

	read_file(path, buf, BUFLEN);

	value = strtol(buf, &end, 0);
	if (*end && *end != ending_char)
		die("invalid file content: %s: %s", path, buf);

	return value;
}

/*
 * Write integer to file.
 * The call always succeeds (it dies() on failure).
 */
static void write_int_to_file(const char *path, int value) {
	char buf[BUFLEN];

	snprintf(buf, BUFLEN, "%d\n", value);
	write_file(path, buf, strlen(buf));
}

int get_sysrq_state(const char *path) {
	return read_int_from_file(path, '\n');
}

void set_sysrq_state(const char *path, int new_state) {
	write_int_to_file(path, new_state);
}

int get_printk_console(const char *path) {
	return read_int_from_file(path, '\t');
}

void set_printk_console(const char *path, int new_level) {
	write_int_to_file(path, new_level);
}

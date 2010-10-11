/*
 * Copyright (c) 2010, Bertrand Janin <tamentis@neopulsar.org>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <Python.h>

#define SAMPLE_SIZE	10000

static PyObject *RecLoadError;

char *fgetln(FILE *, size_t *);
void *memrchr(const void *, int, size_t);

/**
 * Read lines from the given file pointer and returns when a line of the right
 * type was found and parsed. It returns the length of the line, if 0 then
 * we reached the end of the file.
 */
size_t
parse_next_valid_line(FILE *fp, time_t *ts, float *gallons)
{
	char *buf, *p;
	size_t len;

	while ((buf = fgetln(fp, &len))) {
		if (buf == NULL)
			break;

		/* Invalid or wrong record type */
		if (len == 0 || buf[0] != 'r')
			continue;

		/* Finish him! */
		if (buf[len - 1] == '\n')
			len--;
		buf[len] = '\0';

		/* Extract timestamp and accept no failures! */
		*ts = atoi(buf + 2);
		if (*ts == 0)
			continue;

		/* Extract gallons */
		p = (char *)memrchr(buf, ':', len);
		*gallons = atof(p + 1);

		return len;
	}

	return 0;
}

/**
 * Add a new record to the record set typle.
 */
void
add_record(PyObject *timestamps, PyObject *levels, unsigned int idx, time_t ts,
		float gallons)
{
	PyList_SetItem(timestamps, idx, PyInt_FromLong(ts));
	PyList_SetItem(levels, idx, PyFloat_FromDouble(gallons));
}

static PyObject *
recload_read(PyObject *self, PyObject *args)
{
	unsigned int i, j, sample_step;
	time_t min_ts, ts, sts, now;
	time_t ts_keys[SAMPLE_SIZE];
	const char *filename;
	float gallons = 0.0;
	FILE *fp;
	PyObject *record_set = NULL;
	PyObject *timestamps = NULL;
	PyObject *levels = NULL;

	if (!PyArg_ParseTuple(args, "si", &filename, &min_ts))
		return NULL;

	now = time(NULL);

	/* First create a list of requested samples (targets) */
	sample_step = (now - min_ts) / SAMPLE_SIZE;
	for (i = min_ts, j = 0; j < SAMPLE_SIZE; i += sample_step, j++)
		ts_keys[j] = i;

	fp = fopen(filename, "rb");
	if (fp == NULL) {
		PyErr_SetFromErrnoWithFilename(PyExc_OSError, (char *)filename);
		return NULL;
	}

	/* Parse first valid line to obtain the oldest timestamp available. */
	if (parse_next_valid_line(fp, &ts, &gallons) == 0) {
		PyErr_SetString(PyExc_ValueError,
				"Data file does not contain any valid record.");
		return NULL;
	}

	/* Create the output structures */
	timestamps = PyList_New(SAMPLE_SIZE);
	levels = PyList_New(SAMPLE_SIZE);
	record_set = PyTuple_New(2);
	PyTuple_SetItem(record_set, 0, timestamps);
	PyTuple_SetItem(record_set, 1, levels);

	/*
	 * As long as the target timestamps are lower than the first timestamp
	 * in the file, fill up the records with zeroes.
	 */
	for (i = 0; i < SAMPLE_SIZE; i++) {
		sts = ts_keys[i];
		if (sts >= ts)
			break;
		add_record(timestamps, levels, i, sts, 0.0);
	}

	/* 
	 * Go through actual non-zero records. As long as the next file
	 * timestamp is greater or equal to the current target, keep adding
	 * records.
	 */
	for (; i < SAMPLE_SIZE;) {
		if (ts >= ts_keys[i]) {
			add_record(timestamps, levels, i, ts_keys[i], gallons);
			i++;
			continue;
		}

		if (parse_next_valid_line(fp, &ts, &gallons) == 0)
			break;
	}

	/* 
	 * At this point if we are missing data, just fill it up with zeroes,
	 * just to avoid breaking something.
	 */
	for (; i < SAMPLE_SIZE; i++) {
		add_record(timestamps, levels, i, ts_keys[i], 0.0);
	}

	fclose(fp);

	return record_set;
}


static PyMethodDef RecLoadMethods[] = {
	{"read",  recload_read, METH_VARARGS, 
		"Get a tuple of tupes representing the set of records within "
		"a file, sampled since a given timestamp."},
	{NULL, NULL, 0, NULL}
};


PyMODINIT_FUNC
initrecload(void)
{
	PyObject *m;

	m = Py_InitModule("recload", RecLoadMethods);
	if (m == NULL)
		return;

	RecLoadError = PyErr_NewException("recload.error", NULL, NULL);
	Py_INCREF(RecLoadError);
	PyModule_AddObject(m, "error", RecLoadError);
}


/*
 * logwrite.cpp
 *
 *  Created on: Jul 1, 2010
 *      Author: pushkar
 */

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <aio_writer.h>
#include "log.h"

aio_writer* aio_w;
size_t data_size;
char* data_w;

void
logwrite_init(const char* filename, mesa_t frame)
{
	size_t sz = frame.cloud_size + frame.img_size * 3;
	data_size = sz;
	data_w = (char*) malloc (sz);

	aio_w = new aio_writer(sz, 10);
	if(1 == aio_w->fopen(filename))
		fprintf(stderr, "Opened %s for logging\n", filename);
}

void
logwrite_logframe(mesa_t frame)
{
	size_t szc = frame.cloud_size/3;
	memcpy(data_w, frame.x, szc);
	memcpy(data_w+szc, frame.y, szc);
	memcpy(data_w+2*szc, frame.z, szc);
	memcpy(data_w+3*szc, frame.amplitude, frame.img_size);
	memcpy(data_w+3*szc+frame.img_size, frame.distance, frame.img_size);
	memcpy(data_w+3*szc+2*frame.img_size, frame.confidence, frame.img_size);
	aio_w->fwrite(data_w, data_size);
}

void
logwrite_close()
{
	aio_w->fclose();
}

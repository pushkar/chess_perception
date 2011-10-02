/*
 * logread.cpp
 *
 *  Created on: Jul 1, 2010
 *      Author: pushkar
 */

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <aio_reader.h>
#include "log.h"

aio_reader* aio_r;
size_t _data_size;
char* data_r;

void
logread_init(const char* filename, mesa_t frame)
{
	size_t sz = frame.cloud_size + frame.img_size * 3;
	_data_size = sz;
	data_r = (char*) malloc (sz);

	aio_r = new aio_reader(sz, 10);
	if(1 == aio_r->fopen(filename))
		fprintf(stderr, "Opened %s for reading\n", filename);

	aio_r->freadfullbuffer();
}

void
logread_logframe(mesa_t* frame)
{
	size_t szc = frame->cloud_size/3;
	aio_r->fetchnew();
	size_t sz;
	aio_r->fread(data_r, &sz);
	assert(sz == _data_size);

	memcpy(frame->x, data_r, szc);
	memcpy(frame->y, data_r+szc, szc);
	memcpy(frame->z, data_r+2*szc, szc);
	memcpy(frame->amplitude, data_r+3*szc, frame->img_size);
	memcpy(frame->distance, data_r+3*szc+frame->img_size, frame->img_size);
	memcpy(frame->confidence, data_r+3*szc+2*frame->img_size, frame->img_size);
}

void
logread_close()
{
	aio_r->fclose();
}

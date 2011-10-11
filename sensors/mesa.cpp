#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <definesSR.h>
#include <libMesaSR.h>

#include "mesa.h"

static SRCAM _srcam;
static int _srccam_init = 0;

static const char *imgtype_string(ImgEntry::ImgType i) {
	switch (i) {
	case ImgEntry::IT_DISTANCE: return "DISTANCE";
	case ImgEntry::IT_AMPLITUDE: return "AMPLITUDE";
	case ImgEntry::IT_INTENSITY: return "INTENSITY";
	case ImgEntry::IT_TAP0: return "TAP0";
	case ImgEntry::IT_TAP1: return "TAP1";
	case ImgEntry::IT_TAP2: return "TAP2";
	case ImgEntry::IT_TAP3: return "TAP3";
	case ImgEntry::IT_SUM_DIFF: return "SUM_DIFF";
	case ImgEntry::IT_CONF_MAP: return "CONF_MAP";
	case ImgEntry::IT_UNDEFINED: return "UNDEFINED";
	case ImgEntry::IT_LAST: return "LAST";
	}
	return "Unknown";
}

static const char *datatype_string(ImgEntry::DataType i) {
	switch (i) {
	case ImgEntry::DT_NONE: return "NONE";
	case ImgEntry::DT_UCHAR: return "UCHAR";
	case ImgEntry::DT_CHAR: return "CHAR";
	case ImgEntry::DT_USHORT: return "USHORT";
	case ImgEntry::DT_SHORT: return "SHORT";
	case ImgEntry::DT_UINT: return "UINT";
	case ImgEntry::DT_INT: return "INT";
	case ImgEntry::DT_FLOAT: return "FLOAT";
	case ImgEntry::DT_DOUBLE: return "DOUBLE";
	}
	return "Unknown";
}

static const char *modfrq_to_string(ModulationFrq i) {
	switch (i) {
	case MF_40MHz: return "40MHz";
	case MF_30MHz: return "30MHz";
	case MF_21MHz: return "21MHz";
	case MF_20MHz: return "20MHz";
	case MF_19MHz: return "19MHz";
	case MF_60MHz: return "60MHz";
	case MF_15MHz: return "15MHz";
	case MF_10MHz: return "10MHz";
	case MF_29MHz: return "29MHz";
	case MF_31MHz: return "31MHz";
	case MF_14_5MHz: return "14_5MHz";
	case MF_15_5MHz: return "15_5MHz";
	case MF_LAST: return "LAST";
	}
	return "Unknown";
}

void mesa_csv(const char* str, mesa_t frame) {
	FILE *fptr;
	fptr = fopen(str, "w");
	for (int i = 0; i < frame.len; i++) {
		fprintf(fptr, "%.6f %.6f %.6f\n", frame.x[i], frame.y[i], frame.z[i]);
	}
	fclose(fptr);
}

void mesa_info() {
	assert(_srcam);
	// library version
	unsigned short version[4];
	SR_GetVersion(version);
	fprintf(stderr, "Library Version: %d.%d.%d.%d\n", version[0], version[1], version[2], version[3]);

	// device string
	size_t len = 512;
	char buf[len];
	memset(buf, 0, len);
	SR_GetDeviceString(_srcam, buf, len);
	fprintf(stderr, "Device String: %s\n", buf);

	// Acquire Mode
	int mode = SR_GetMode(_srcam);
	fprintf(stderr, "Acquire Mode: 0x%x \n ", mode);

	// dimensions
	fprintf(stderr, "Size (r x c): %d x %d\n", SR_GetRows(_srcam), SR_GetCols(_srcam));

	ImgEntry *imgent;
	int r = SR_GetImageList(_srcam, &imgent);
	fprintf(stderr, "Images available: %d\n", r);
	for (int i = 0; i < r; i++) {
		fprintf(stderr,
				"Image %d -- Type: %s, DataType: %s, W: %d, H: %d\n", i,
				imgtype_string(imgent[i].imgType),
				datatype_string(imgent[i].dataType),
				imgent[i].width, imgent[i].height);
	}

	// ModFrq
	fprintf(stderr, "Modulation Frequency: %s\n", modfrq_to_string(SR_GetModulationFrequency(_srcam)));

	// integration time
	fprintf(stderr, "Integration time code: %d\n", SR_GetIntegrationTime(_srcam));

	// amplitude threshold
	fprintf(stderr, "Amplitude Threshold: %d\n", SR_GetAmplitudeThreshold(_srcam));

	// Distance Offset
	// Deprecated: Only for SR2
	// fprintf(stderr, "Distance Offset: %d\n", SR_GetDistanceOffset(_srcam));
}

void
mesa_set_modes(int verbose)
{
	int mode = SR_GetMode(_srcam);
	SR_SetMode(_srcam, mode | AM_CONF_MAP);

	mode = SR_GetMode(_srcam);
	SR_SetMode(_srcam, mode | AM_RESERVED1);

	int freq = SR_GetModulationFrequency(_srcam);
	SR_SetModulationFrequency(_srcam, MF_30MHz);
	freq = SR_GetModulationFrequency(_srcam);

	int time = SR_GetIntegrationTime(_srcam);
	SR_SetIntegrationTime(_srcam, 30);

	if(verbose)
	{
		fprintf(stderr, "Setting Confidence Map capture... 0x%x\n", mode);
		fprintf(stderr, "Modulation Frequency is %d\n", freq);
		fprintf(stderr, "Integration Time is %d\n", time);
	}
}

// 1 on sucesss
int
mesa_init_device(const char* host)
{
	if(SR_OpenETH(&_srcam, host) == 1)
	{
		_srccam_init = 1;
		return 1;
	}
	return 0;
}

// 0 on sucess
// -1 wrong device
// -2 cannot release interface
// -3 cannot close device
int
mesa_close_device()
{
	if(_srccam_init == 1)
		return SR_Close(_srcam);
	return 0;
}

mesa_t*
mesa_dup_frame(mesa_t* dest, mesa_t src) {
	size_t sz_frame = src.len * sizeof(float);
	memcpy(dest->x, src.x, sz_frame);
	memcpy(dest->y, src.y, sz_frame);
	memcpy(dest->z, src.z, sz_frame);
	memcpy(dest->amplitude, src.amplitude, src.img_size);
	memcpy(dest->confidence, src.confidence, src.img_size);
	memcpy(dest->distance, src.distance, src.img_size);
	return dest;
}

mesa_t
mesa_init_frame() {

	mesa_t frame;
	if(_srccam_init == 1) {
		frame.rows = SR_GetRows(_srcam);
		frame.cols = SR_GetCols(_srcam);
	}
	else {
		frame.rows = 144;
		frame.cols = 176;
	}

	frame.len = frame.rows * frame.cols;

	size_t sz_frame = frame.len * sizeof(float);
	frame.x = (float*) malloc (sz_frame);
	frame.y = (float*) malloc (sz_frame);
	frame.z = (float*) malloc (sz_frame);
	frame.img_size = frame.len * sizeof(uint16_t);
	frame.cloud_size = sz_frame * 3;

//	if(_srccam_init == 0) {
		frame.distance = (unsigned char*) malloc(frame.img_size);
		frame.amplitude = (unsigned char*) malloc(frame.img_size);
		frame.confidence = (unsigned char*) malloc(frame.img_size);
//	}

	assert(frame.x);
	assert(frame.y);
	assert(frame.z);

	mesa_set_modes(0);

	return frame;
}

void
mesa_release_frame(mesa_t *frame)
{
	if(frame->x != NULL) free(frame->x);
	if(frame->y != NULL) free(frame->y);
	if(frame->z != NULL) free(frame->z);
}


void mesa_grab_frame(mesa_t *frame) {
	assert(frame->distance != NULL);
	assert(frame->amplitude != NULL);
	assert(frame->confidence != NULL);
	// get images
	int r = SR_Acquire(_srcam);
	if (r < 0) {
		fprintf(stderr, "Error getting data from the camera: %d\n", r);
		mesa_close_device();
		exit(1);
	}

	frame->distance = (unsigned char*) SR_GetImage(_srcam, 0);
	frame->amplitude = (unsigned char*) SR_GetImage(_srcam, 1);
	frame->confidence = (unsigned char*) SR_GetImage(_srcam, 2);
	SR_CoordTrfFlt(_srcam, frame->x, frame->y, frame->z, sizeof(float),
			sizeof(float), sizeof(float));
}


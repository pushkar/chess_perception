/*
 * mesa.h
 *
 *  	Desc: API to the MESA Swiss Ranger 4000
 */
#ifndef MESA_H_
#define MESA_H_

#define SRD_LEN_DEFAULT 25344
#define SRD_ROWS_DEFAULT 144
#define SRD_COLS_DEFAULT 176

typedef struct {
	int rows;
	int cols;
	int len;
	float* x;
	float* y;
	float* z;
	size_t img_size;
	size_t cloud_size;
	unsigned char* distance;
	unsigned char* amplitude;
	unsigned char* confidence;
} mesa_t;

int mesa_init_device(const char* host);
void mesa_set_modes();
int mesa_close_device();
void mesa_grab_frame(mesa_t* frame);

mesa_t* mesa_dup_frame(mesa_t* dest, mesa_t src);
mesa_t mesa_init_frame();
void mesa_info();
void mesa_retrieve_frame();
void mesa_release_frame(mesa_t* frame);

#endif /* MESA_H_ */

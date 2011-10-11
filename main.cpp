/*
 * main.cpp
 *
 *  Created on: Jun 30, 2010
 *      Author: pushkar
 */

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include "sensors/mesa.h"
#include "log/log.h"
#include "chess.h"

#define LIVE 0
#define LOG 1
#define READ 2
#define FILENAME "/home/bluebot/Desktop/exp1.dat"
#define MESA_IP "192.168.1.30"
#define MESA_IP_2 "192.168.1.42"
#define WC 0.057 // width of chessboard square

int state = LIVE;
double alpha = 0.1f;  // value near 0 for less noise
double len_max = 1.203;
double len_min = 1.12;
double bx_max, bx_min, by_max, by_min;
int _init_chessboard_done = 0;
int opt_images = 0;
int opt_c = 1;
int opt_v = 1;
int opt_b = 1;
int opt_B = 0;

int count_loop = 0;
const int min_weight = 4;
const int frame_last = 5;

mesa_t frame_init;
mesa_t frame[frame_last];
IplImage* img_distance;
IplImage* img_amplitude;
IplImage* img_confidence;
IplImage* img_amplitude_l;
IplImage* img_amplitude_L;

piece_type_t board_state_type[64];
int obstacles = 0; // anything less than len_min is an obstacle

void
_end()
{
	usleep(1000);
	for(int i = 0; i < frame_last; i++)
		mesa_release_frame(&frame[i]);
	cvReleaseImage(&img_distance);
	cvReleaseImage(&img_amplitude);
	cvReleaseImage(&img_confidence);
	cvReleaseImage(&img_amplitude_l);
	cvReleaseImage(&img_amplitude_L);
	switch (state) {
		case LIVE:
		printf("Closing Mesa Ranger...\n");
		if(0 == mesa_close_device())
			printf("Done\n");
		break;
		case LOG:
			logwrite_close();
			break;
		case READ:
			logread_close();
			break;
	}
	ach_close();
	ach_crafty_close();
	ach_move_close();
}

void
_reset() {
	// printf("reset\n");
	chessboard_clear();
	for(int i = 0; i < 64; i++) {
		board_state[i] = 0;
		board_state_type[i].reset();
	}
}

// Temp Variables
IplImage* img;
static int _count = 0;
char str[10] = "";
void
_keyboard(unsigned char key)
{
	switch (key) {
	case 'F':
		opt_images = !opt_images;
		printf("images now %d\n", opt_images);
		break;
	case 'f':
		_count++;
		if(opt_images == 0) {
			printf("Images are switched off...\n");
			break;
		}
		sprintf(str, "img_amp_%d.jpg", _count);
		img = cvCreateImage(cvSize(frame[0].cols, frame[0].rows), IPL_DEPTH_8U, 1);
		for(int j = 0; j < frame[0].len; j++)
			memcpy(img->imageData+j, frame[0].amplitude+j*2+1, 1);
		printf("Saving image\n");
		cvSaveImage(str, img);
		cvReleaseImage(&img);
		break;
	case 'p':
		alpha += 0.01f;
		if(alpha > 0.9) alpha = 0.9f;
		printf("alpha is now %f\n", alpha);
		break;
	case 'o':
		alpha -= 0.01f;
		if(alpha < 0.01) alpha = 0.01f;
		printf("alpha is now %f\n", alpha);
		break;
	case 'k':
		len_max += 0.001f;
		printf("len_max is now %f\n", len_max);
		break;
	case 'l':
		len_max -= 0.001f;
		printf("len_max is now %f\n", len_max);
		break;
	case 'K':
		len_min += 0.01f;
		printf("len_min is now %f\n", len_min);
		break;
	case 'L':
		len_min -= 0.01f;
		printf("len_min is now %f\n", len_min);
		break;
	case 'n':
		_init_chessboard_done = !_init_chessboard_done;
		printf("chessboard is now init %d\n", _init_chessboard_done);
		break;
	case 'c':
		opt_c = !opt_c;
		printf("c: %d\n", opt_c);
		break;
	case 'v':
		opt_v = !opt_v;
		printf("v: %d\n", opt_v);
		break;
	case 'b':
		opt_b = !opt_b;
		printf("b: %d\n", opt_b);
		break;
	case 'B':
		opt_B = !opt_B;
		printf("t: %d\n", opt_B);
		break;
	case '?':
		printf("\nHelp\n");
		printf("------------\n");
		printf("F: Images on/off\n");
		printf("p/o: alpha for filtering\n");
		printf("k/l: len_max\n");
		printf("K/L: len_min\n");
		printf("n: To initialize chess board\n");
		printf("c: cloud on/off\n");
		printf("v: chessboard means on/off\n");
		printf("b: other means on/off\n");
		printf("B: other means info\n");
		printf("q: quit\n");
		printf("\n");
		break;
	case 'g':
		break;
	}
}

void get_frame(mesa_t *fr) {
	switch (state) {
	case LIVE:
		mesa_grab_frame(fr);
		break;
	case LOG:
		mesa_grab_frame(fr);
		logwrite_logframe(*fr);
		break;
	case READ:
		logread_logframe(fr);
		break;
	}
}

// This is literally hard coded. Hopefully it makes sense.
// This function tries to initialize the centers of each chessboard square
// Input is the center of chess board
void _init_chessboard(double x, double y, double z) {
	//if(_init_chessboard_done == 1) return;

	for(int i = -4; i < 4; i++) {
		double xi = i*WC;
		for(int j = -4; j < 4; j++) {
			double yi = j*WC;
			int n = (i+4) * 8 + (j+4);
			_point_t p;
			p.x = x + xi + WC/2.0;
			p.y = y + yi + WC/2.0;
			p.z = z + 0;
			chessboard[n].set_mean(p, 0.1);
			chessboard_original[n].filter(p, 0.1);
		}
	}
	bx_max = x + 4.5*WC;
	bx_min = x - 4.5*WC;
	by_max = y + 4.5*WC;
	by_min = y - 4.5*WC;
}

int skip = 0;
void
_draw() {

	// glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_POINT_SMOOTH);
	glPointSize(1.0f);

	get_frame(&frame_init);

	for(int i = frame_last - 1; i > 0; i--) {
		mesa_dup_frame(&frame[i], frame[i-1]);
	}

	mesa_dup_frame(&frame[0], frame_init);

	ach_crafty_read();

	// smooth and show images
	if(opt_images) {
		img_distance->imageData = (char*) frame[0].distance;
		img_amplitude->imageData = (char*) frame[0].amplitude;
		img_confidence->imageData = (char*) frame[0].confidence;
		cvSmooth(img_distance, img_distance, CV_MEDIAN, 3, 0, 0, 0);

		for(int j = 0; j < frame[0].len-1; j++) {
			memcpy(img_amplitude_l->imageData+j, frame[0].amplitude+j*2+1, 1);
		}

		cvResize(img_amplitude_l, img_amplitude_L, CV_INTER_CUBIC);
		cvDrawLine(img_amplitude_L, cvPoint(0, img_amplitude_L->height/2), cvPoint(img_amplitude_L->width, img_amplitude_L->height/2), CV_RGB(255, 255, 255), 1);
		cvDrawLine(img_amplitude_L, cvPoint(img_amplitude_L->width/2, 0), cvPoint(img_amplitude_L->width/2, img_amplitude_L->height), CV_RGB(255, 255, 255), 1);

		cvShowImage("Amplitude", img_amplitude_L);
		cvWaitKey(100);
	}

	chessboard_clear();

	int i = frame[0].rows/2.0;
	int j = frame[0].cols/2.0;
	int n = i * frame[0].cols + j;
	_init_chessboard(frame[0].x[n], frame[0].y[n], len_max);

	glPushMatrix();
	glColor3f(0.3f, 0.3f, 0.3f);

	glBegin(GL_POINTS);
	n = 0;
	obstacles = 0;

	for(int i = 0; i < frame[0].rows; i++) {
		for(int j = 0; j < frame[0].cols; j++) {
			n = i * frame[0].cols + j;
			frame[0].x[n] = alpha * frame[0].x[n] + (1 - alpha) * frame[1].x[n];
			frame[0].y[n] = alpha * frame[0].y[n] + (1 - alpha) * frame[1].y[n];
			frame[0].z[n] = alpha * frame[0].z[n] + (1 - alpha) * frame[1].z[n];
		}
	}

	for (int i = 0; i < frame_last; i++) {
		for (int j = 0; j < frame[i].rows; j++) {
			for (int k = 0; k < frame[i].cols; k++) {
				n = j * frame[i].cols + k;
				double len = frame[i].z[n];

				float c = (len - len_min) / (len_max - len_min);
				glColor3f(0.0f, c, 0.1f);
				skip = 0;

				if (frame[i].z[n] < len_min) {
					obstacles++;
					glColor3f(0.0f, 0.0f, 0.0f);
					skip = 1;
					continue;
				}
				if (frame[i].z[n] > len_max) continue;
				if (frame[i].x[n] > bx_max) continue;
				if (frame[i].x[n] < bx_min) continue;
				if (frame[i].y[n] > by_max) continue;
				if (frame[i].y[n] < by_min) continue;

				if (skip == 0) chessboard_add_point(frame[i].x[n], frame[i].y[n], frame[i].z[n]);

				if (opt_c) cloud_drawpoint(frame[i].x[n], frame[i].y[n], frame[i].z[n]);
			}
		}
	}
	glEnd();
	glPopMatrix();

	chessboard_drawboard();

	glPushMatrix();
	glColor3f(1.0f, 1.0f, 1.0f);

	if(skip == 0) chessboard_evaluate();
	int c = 0;

	for(uint i = 0; i < 64; i++) {
		c = 63-i;
		board_state[c] = 0;
		if(chessboard[i].weight() < min_weight) continue;

		offset[c*4+0] = (chessboard[i].mean.y - chessboard_original[i].y);
		offset[c*4+1] = -(chessboard[i].mean.x - chessboard_original[i].x);
		offset[c*4+2] = (chessboard[i].mean.z - chessboard_original[i].z);
		offset[c*4+3] = chessboard[i].height();

		board_state[c] = 1;
		if(near(chessboard[i].height(), 0.031, 0.001)) board_state[c] = 1;
		if(near(chessboard[i].height(), 0.032, 0.001)) board_state[c] = 2;
		if(near(chessboard[i].height(), 0.033, 0.001)) board_state[c] = 3;
		if(near(chessboard[i].height(), 0.034, 0.001)) board_state[c] = 4;
		if(near(chessboard[i].height(), 0.045, 0.003)) board_state[c] = 5;
		if(near(chessboard[i].height(), 0.050, 0.005)) board_state[c] = 6;
		if(board_state[c] > 1 && chessboard[i].ratio() > 1.4) board_state[c] = 3;
	}

	for(uint i = 0; i < piece.size(); i++) {
		c = 64+i;
		board_state[c] = 1;
		offset[c*4+0] = piece[i].mean.y;
		offset[c*4+1] = -piece[i].mean.x;
		offset[c*4+2] = piece[i].mean.z;
		offset[c*4+2] = piece[i].angle();
	}

	board_state[c] = 0; // To denote the last piece
	board_state[c+1] = 0; // To denote the last piece

	ach_write();
	glPopMatrix();

	//compare_with_crafty(min_weight);
	if (opt_B) {
		printf("Total pieces: %d\n", chessboard_number_of_pieces(min_weight));
		for (int i = 0; i < 64; i++) {
			if (board_state[i] > 0)
				printf("%c%c is %Ld: %.2lf, %.2lf\n", chess_to_column(i),
						chess_to_row(i), board_state[i], offset[i * 4 + 0]
								* 1000.0f, offset[i * 4 + 1] * 1000.0f);
		}
		printf("\n");
		printf("\n");
	}

	glPointSize(1.0f);
	// cloud_findorientation_and_draw();

	if(opt_b) cloud_draw_piece_means();
	if(opt_v) cloud_draw_chessboard_means();
	if(opt_B) { cloud_pout(); opt_B = 0; }

	count_loop++;
	if(obstacles > 3000) _reset();
}


int
main(int argc, char* argv[])
{
	ach_move_init();
	ach_crafty_initialize();
	ach_initialize();
	switch (state) {
		case LIVE:
			if(1 == mesa_init_device(MESA_IP))
					printf("Mesa Ranger initialized at %s\n", MESA_IP);
			else {
				mesa_init_device(MESA_IP_2);
				printf("Mesa Ranger initialized at %s\n", MESA_IP_2);
			}

			for(int i = 0; i < frame_last; i++) {
				frame[i] = mesa_init_frame();
			}

			break;
		case LOG:
			if(1 == mesa_init_device(MESA_IP))
					printf("Mesa Ranger initialized at %s\n", MESA_IP);
			else {
				mesa_init_device(MESA_IP_2);
				printf("Mesa Ranger initialized at %s\n", MESA_IP_2);
			}

			for(int i = 0; i < frame_last; i++) {
				frame[i] = mesa_init_frame();
				logwrite_init(FILENAME, frame[i]);
			}

		break;
		case READ:
			for(int i = 0; i < frame_last; i++) {
				frame[i] = mesa_init_frame();
				logread_init(FILENAME, frame[i]);
			}
			break;
	}

	img_distance = cvCreateImage(cvSize(frame[0].cols, frame[0].rows), IPL_DEPTH_16U, 1);
	img_amplitude = cvCreateImage(cvSize(frame[0].cols, frame[0].rows), IPL_DEPTH_16U, 1);
	img_confidence = cvCreateImage(cvSize(frame[0].cols, frame[0].rows), IPL_DEPTH_16U, 1);
	img_amplitude_l = cvCreateImage(cvSize(frame[0].cols, frame[0].rows), IPL_DEPTH_8U, 1);
	img_amplitude_L = cvCreateImage(cvSize(frame[0].cols*3, frame[0].rows*3), IPL_DEPTH_8U, 1);
	cvNamedWindow("Amplitude", 1);

	frame_init = mesa_init_frame();
	get_frame(&frame_init);

	for(int i = 0; i < frame_last; i++) {
		printf("%d\n", i);
		mesa_dup_frame(&frame[i], frame_init);
		usleep(100);
	}

	printf("Entering MainLoop\n");
	gl_init(argc, argv, "Chess Perception", 800, 600);
	glutMainLoop();
	_end();

	return 0;
}


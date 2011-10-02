/*
 * chessd.h
 *
 *  Created on: Sep 2, 2010
 *      Author: pushkar
 */

#ifndef CHESSD_H_
#define CHESSD_H_

#define CHANNEL_NAME "chess_perception"

#define BOARDSTATE_SIZE sizeof(int64_t)*120

#define OFFSET_SIZE sizeof(double)*120*4

#define TOKEN_CHANNEL_DATA_SIZE BOARDSTATE_SIZE+OFFSET_SIZE

extern uint64_t board_state[];
extern double offset[];

typedef struct _piece_type_t {
	int t[6];
	int max() {
		int r = 0;
		int max = 0;
		for(int i = 0; i < 6; i++) {
			if(max < t[i]) {
				max = t[i];
				r = i;
			}
		}
		return r+1;
	}

	void reset() {
		for(int i = 0; i < 6; i++)
			t[i] = 0;
	}
} piece_type_t;

extern piece_type_t board_state_type[64];

int ach_initialize();
void ach_write();
void ach_read();
void ach_close();

void ach_debug_arr(char* str, double* arr);

#endif /* CHESSD_H_ */

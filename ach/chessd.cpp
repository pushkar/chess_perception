/*
 * chessd.cpp
 *
 *  Created on: Sep 2, 2010
 *      Author: pushkar
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>

#include <somatic.h>
#include <ach.h>
#include <somatic/util.h>
#include <somatic.pb-c.h>

#include "chessd.h"

static const char *token_chan_name = CHANNEL_NAME;
Somatic__Token* token_message;
ach_channel_t *token_chan;

uint64_t board_state[120];
double offset[120*4];

// Initialize ACH Channel and all data
int ach_initialize() {
  // Channels
  int r = 0;
  printf("ACHT: Creating Channel\n");
  ach_create_attr_t attr;
  ach_create_attr_init(&attr);
  r = ach_create((char*)token_chan_name, 100, TOKEN_CHANNEL_DATA_SIZE, &attr);
  printf("ACHT: %s\n", ach_result_to_string((ach_status_t)r));

  printf("ACHT: Opening Channel\n");
  token_chan = (ach_channel_t*) malloc (sizeof(ach_channel_t));
  if(token_chan == NULL) return 0;
  r = ach_open(token_chan, token_chan_name, NULL);

  if(r != ACH_OK) {
	  printf("ACHT: %s\n", ach_result_to_string((ach_status_t)r));
	  return 0;
  }

  ach_chmod(token_chan, SOMATIC_CHANNEL_MODE );

  r = ach_flush(token_chan);

  if(r != ACH_OK) {
	  printf("ACHT: %s\n", ach_result_to_string((ach_status_t)r));
	  return 0;
  }

  // Data
  token_message = (Somatic__Token*) malloc(sizeof(Somatic__Token));
  if(token_message == NULL) return 0;
  somatic__token__init(token_message);

  token_message->type = (char*) malloc(100);
  token_message->iattr = (Somatic__Ivector*)malloc(sizeof(Somatic__Ivector));
  somatic__ivector__init(token_message->iattr);
  token_message->iattr->data = (int64_t*) malloc(sizeof(int64_t)*120);
  token_message->iattr->n_data = 120;

  token_message->fattr = (Somatic__Vector*) malloc(sizeof(Somatic__Vector));
  somatic__vector__init(token_message->fattr);
  token_message->fattr->data = (double*) malloc(sizeof(double)*120*4);
  token_message->fattr->n_data = 120*4;

  printf("ACHT: Info\n"
		 "ACHT: Name: %s\n"
		 "ACHT: Size: %d\n", token_chan_name, somatic__token__get_packed_size(token_message));

  // Output data

  return 1;
}

void ach_write() {
  memcpy(token_message->iattr->data, board_state, BOARDSTATE_SIZE);
  memcpy(token_message->fattr->data, offset, OFFSET_SIZE);
  int r = SOMATIC_PACK_SEND(token_chan, somatic__token, token_message);
  if(r != ACH_OK)
	  printf("ACH: Writing data to channel returned %s\n", ach_result_to_string((ach_status_t)r));
}

void ach_read() {
  int r = 0;
  Somatic__Token *msg =
		  SOMATIC_GET_LAST_UNPACK(r, somatic__token, &protobuf_c_system_allocator, TOKEN_CHANNEL_DATA_SIZE , token_chan);
  if(r != ACH_OK)
	  printf("ACH: Reading data from channel returned %s\n", ach_result_to_string((ach_status_t)r));
  for(int i = 0; i < 64; i++) {
	  printf("%4Ld\t", msg->iattr->data[i]);
	  if((i+1) % 8 == 0) printf("\n");
  }
  printf("\n");

  somatic__token__free_unpacked(msg, &protobuf_c_system_allocator);
}

void ach_close() {
	printf("ACHT: Closing ACH\n");
	int r = ach_close(token_chan);
	if(r != ACH_OK)
	  printf("ACHT: %s\n", ach_result_to_string((ach_status_t)r));
	free(token_message);
}

void ach_debug_arr(char* str, double* arr) {
	for(int i = 0; i < 64; i++) {
		printf("%s %d: %.2lf \t %.2lf \t %.2lf \n", str, i,
				arr[i*3],
				arr[i*3+1],
				arr[i*3+2]);
	}
}

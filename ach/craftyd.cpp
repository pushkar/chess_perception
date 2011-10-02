/*
 * craftyd.cpp
 *
 *  Created on: Sep 6, 2010
 *      Author: bluebot
 */

#include <argp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>

#include <somatic.h>
#include <ach.h>
#include <somatic/util.h>
#include <somatic.pb-c.h>

#include "craftyd.h"

static const char *crafty_chan_name = CRAFTY_CHANNEL_NAME;
Somatic__Token* crafty_message;
ach_channel_t *crafty_chan;

uint64_t crafty_board_state[64];

// Initialize ACH Channel and all data
int ach_crafty_initialize() {
  // Channels
  int r = 0;
  printf("ACHT: Creating Channel\n");
  ach_create_attr_t attr;
  ach_create_attr_init(&attr);
  r = ach_create((char*)crafty_chan_name, 100, CRAFTY_CHANNEL_DATA_SIZE, &attr);
  printf("ACHT: %s\n", ach_result_to_string((ach_status_t)r));

  printf("ACHT: Opening Channel\n");
  crafty_chan = (ach_channel_t*) malloc (sizeof(ach_channel_t));
  if(crafty_chan == NULL) return 0;
  r = ach_open(crafty_chan, crafty_chan_name, NULL);

  if(r != ACH_OK) {
	  printf("ACHT: %s\n", ach_result_to_string((ach_status_t)r));
	  return 0;
  }

  ach_chmod(crafty_chan, SOMATIC_CHANNEL_MODE );

  r = ach_flush(crafty_chan);

  if(r != ACH_OK) {
	  printf("ACHT: %s\n", ach_result_to_string((ach_status_t)r));
	  return 0;
  }

  // Data
  crafty_message = (Somatic__Token*)malloc(sizeof(Somatic__Token));
  if(crafty_message == NULL) return 0;
  somatic__token__init(crafty_message);

  crafty_message->type = (char*) malloc(100);
  crafty_message->iattr = (Somatic__Ivector*) malloc(sizeof(Somatic__Ivector));
  somatic__ivector__init(crafty_message->iattr);
  crafty_message->iattr->data = (int64_t*) malloc(sizeof(int64_t)*68);
  crafty_message->iattr->n_data = 68;

  crafty_message->fattr = (Somatic__Vector*) malloc(sizeof(Somatic__Vector));
  somatic__vector__init(crafty_message->fattr);
  crafty_message->fattr->data = (double*) malloc(sizeof(double)*64);
  crafty_message->fattr->n_data = 64;

  printf("ACHT: Info\n"
		 "ACHT: Name: %s\n"
		 "ACHT: Size: %d\n", crafty_chan_name, somatic__token__get_packed_size(crafty_message));

  // Output data

  return 1;
}

void ach_crafty_read() {
  int r = 0;
  int i = 0;
  struct timespec abstime = aa_tm_now();
  Somatic__Token *msg =
		  SOMATIC_WAIT_LAST_UNPACK(r, somatic__token,  &protobuf_c_system_allocator, CRAFTY_CHANNEL_DATA_SIZE , crafty_chan, &abstime);

  if(r == ACH_OK) {
	  printf("New board state is");
	  for(i = 0; i < 64; i++) {
		  crafty_board_state[i] = (int)msg->iattr->data[i+4];
		  if((i) % 8 == 0) printf("\n");
		  printf("%2Ld  ", crafty_board_state[i]);
	  }
	  printf("\n");
	  somatic__token__free_unpacked(msg, &protobuf_c_system_allocator);
  }
  //else
  //  printf("ACHT: Reading from crafty channel returned %s\n", ach_result_to_string((ach_status_t)r));

}

void ach_crafty_close() {
  printf("ACHT: Closing ACH\n");
  int r = ach_close(crafty_chan);
  if(r != ACH_OK)
	  printf("ACHT: %s\n", ach_result_to_string((ach_status_t)r));
  free(crafty_message);
}

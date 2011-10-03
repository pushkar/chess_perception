/*
 * chessdout.cpp
 *
 *  Created on: Sep 10, 2010
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
#include "chessdout.h"

static const char *craftyo_chan_name = CRAFTY_MOVE_CHANNEL_NAME;
Somatic__Crafty* craftyo_message;
ach_channel_t *craftyo_chan;

int ach_move_init() {
  // Channels
  int r = 0;
  printf("ACH: Creating Channel\n");
  ach_create_attr_t attr;
  ach_create_attr_init(&attr);
  r = ach_create((char*)craftyo_chan_name, 100, IBOARDSTATE_SIZE+IMOVESTRING_SIZE, &attr);
  printf("ACH: %s\n", ach_result_to_string((ach_status_t)r));

  printf("ACH: Opening Channel\n");
  craftyo_chan = (ach_channel_t*) malloc (sizeof(ach_channel_t));
  if(craftyo_chan == NULL) return 0;
  r = ach_open(craftyo_chan, craftyo_chan_name, NULL);
  if(r != ACH_OK) {
	  printf("ACH: %s\n", ach_result_to_string((ach_status_t)r));
	  return 0;
  }

  ach_chmod(craftyo_chan, SOMATIC_CHANNEL_MODE );

  r = ach_flush(craftyo_chan);

  if(r != ACH_OK) {
	  printf("ACH: %s\n", ach_result_to_string((ach_status_t)r));
	  return 0;
  }

  // Data
  craftyo_message = (Somatic__Crafty*)malloc(sizeof(Somatic__Crafty));
  if(craftyo_message == NULL) return 0;
  somatic__crafty__init(craftyo_message);
  //craftyo_message->boardstate.data = (uint8_t*) malloc(128);
  //craftyo_message->has_boardstate = 1;
  //craftyo_message->boardstate.len = 128;
  craftyo_message->move = (char*)malloc(10);

  printf("ACH: Info\n"
		 "ACH: Name: %s\n"
		 "ACH: Size: %d\n", craftyo_chan_name, somatic__crafty__get_packed_size(craftyo_message));

  return 1;
}

void ach_move_write(char* move) {
  memcpy(craftyo_message->move, move, 10);
  int r = SOMATIC_PACK_SEND(craftyo_chan, somatic__crafty, craftyo_message);
  if(r != ACH_OK)
	  printf("ACH: Writing to channel returned %s\n", ach_result_to_string((ach_status_t)r));
}

void ach_move_close() {
  printf("ACH: Closing ACH\n");
  int r = ach_close(craftyo_chan);
  if(r != ACH_OK)
	  printf("ACH: %s\n", ach_result_to_string((ach_status_t)r));
  //free(craftyo_message->boardstate.data);
  free(craftyo_message->move);
  free(craftyo_message);
}

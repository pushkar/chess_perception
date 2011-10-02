/*
 * craftyd.h
 *
 *  Created on: Sep 6, 2010
 *      Author: bluebot
 */

#ifndef CRAFTYD_H_
#define CRAFTYD_H_

#define CRAFTY_CHANNEL_NAME "crafty"

#define CRAFTY_CHANNEL_DATA_SIZE sizeof(int)*68+sizeof(double)*64+100

extern uint64_t crafty_board_state[];

int ach_crafty_initialize();
void ach_crafty_read();
void ach_crafty_close();

#endif /* CRAFTYD_H_ */

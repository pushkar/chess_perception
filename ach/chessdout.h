/*
 * chessdout.h
 *
 *  Created on: Sep 10, 2010
 *      Author: bluebot
 */

#ifndef CHESSDOUT_H_
#define CHESSDOUT_H_

#define CRAFTY_MOVE_CHANNEL_NAME "craftygeneric-out"
#define IBOARDSTATE_SIZE 128
#define IMOVESTRING_SIZE 10


int ach_move_init();

void ach_move_write(char* move);

void ach_move_close();

#endif /* CHESSDOUT_H_ */

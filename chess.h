/*
 * chess.h
 *
 *  Created on: Sep 8, 2010
 *      Author: bluebot
 */

#ifndef CHESS_H_
#define CHESS_H_

#include <stdio.h>
#include "gl/view.h"
#include "gl/main.h"
#include "ach/chessd.h"
#include "ach/craftyd.h"
#include "ach/chessdout.h"
#include "knn_types.h"
#include "knn.h"

extern double len_max;

extern vector<_cloud_t> piece;
extern _cloud_t chessboard[64];
extern _point_t chessboard_original[64];

int chess_to_i(const char* position);
char chess_to_row(int position);
char chess_to_column(int position);
int near(double x, double mean, double ep);
void cloud_markpoint(double x, double y, double z);
void cloud_drawpoint(double x, double y, double z);

void cloud_markpoint(_point_t p);
void cloud_drawpoint(_point_t p);

int crafty_number_of_pieces();
int chessboard_number_of_pieces(int min_weight);

void compare_with_crafty(int min_weight);

#endif /* CHESS_H_ */

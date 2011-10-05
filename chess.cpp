/*
 * chess.cpp
 *
 *  Created on: Sep 9, 2010
 *      Author: bluebot
 */

#include "chess.h"

int chess_to_i(const char* position) {
	int col = position[0] - 96;
	int row = position[1] - 48;
	return (col*8 + row);
}

char chess_to_row	(int position) {
	if(position < 0) return ' ';
	return (49 + position / 8);
}

char chess_to_column(int position) {
	if(position < 0) return ' ';
	return (97 + position % 8);
}

int near(double x, double mean, double ep) {
	if(fabs(x-mean) < ep) return 1;
	return 0;
}


void cloud_markpoint(double x, double y, double z) {
	glBegin(GL_LINES);
	glVertex3f(z-0.005, x, y);
	glVertex3f(z+0.005, x, y);
	glEnd();
	glBegin(GL_LINES);
	glVertex3f(z, x-0.005, y);
	glVertex3f(z, x+0.005, y);
	glEnd();
	glBegin(GL_LINES);
	glVertex3f(z, x, y-0.005);
	glVertex3f(z, x, y+0.005);
	glEnd();
}

void cloud_markpoint(_point_t p) {
	cloud_markpoint(p.x, p.y, p.z);
}

void cloud_drawpoint(double x, double y, double z) {
	glVertex3f(z, x, y);
}

void cloud_drawpoint(_point_t p) {
	glVertex3f(p.z, p.x, p.y);
}

int crafty_number_of_pieces() {
	int sum = 0;
	for(int i = 0; i < 64; i++) {
		if(crafty_board_state[i] != 0)
			sum++;
	}
	return sum;
}

int chessboard_number_of_pieces(int min_weight) {
	int sum = 0;
	for(uint i = 0; i < 64; i++) {
		if(chessboard[i].weight() > min_weight) {
			printf("no: %c%c\n", chess_to_column(i), chess_to_row(i));
			sum++;
		}
	}
	// sum+=piece.size();
	return sum;
}

void compare_with_crafty(int min_weight) {
	int miss = -1;
	int n_miss = 0;
	int newp = -1;
	int n_newp = 0;

	if(abs(crafty_number_of_pieces() - chessboard_number_of_pieces(min_weight)) > 2)
			return;

	for(int n = 0; n < 64; n++) {
		int c = 63-n;
		 if(crafty_board_state[c] != 0 && chessboard[n].weight() > min_weight) {
			 printf(" %2Ld  ", crafty_board_state[c]);
		 }
		 else if(crafty_board_state[c] != 0 && chessboard[n].weight() < min_weight) {
			 miss = c;
			 n_miss++;
			 printf(" %2Ld| ", crafty_board_state[c]);
		 }
		 else if(crafty_board_state[c] == 0 && chessboard[n].weight() > min_weight) {
			 newp = c;
			 n_newp++;
			 printf(" %2c  ", 65+23);
		 }
		 else printf(" %2d  ", 0);
		 if((n+1) % 8 == 0) printf("\n");
	}

	printf("\n");
	printf("\n");

	if(n_miss < 2 && n_newp < 2) {
		printf("%c%c to %c%c\n", chess_to_column(miss), chess_to_row(miss), chess_to_column(newp), chess_to_row(newp));
	}
}

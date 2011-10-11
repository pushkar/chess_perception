/*
 * knn.cpp
 *
 *  Created on: Sep 9, 2010
 *      Author: bluebot
 */

#include "chess.h"

vector<_cloud_t> piece;
_cloud_t chessboard[64];
_point_t chessboard_original[64];

// --------------------------------------------------------------------------------------------
// Chessboard KNN stuff
// knn: juggad version of knn routine, adds point to a chessboard square
void chessboard_add_point(float x, float y, float z) {
	_point_t p(x, y, z);
	uint i = 0;
	for(i = 0; i < 64; i++) {
		if(chessboard[i].near_pointcloud(p, 0.055/2.0)) {
		//if(p.distance(chessboard[i].mean) < 0.045) {
			//if(crafty_board_state[i] == 0) continue;
			chessboard[i].add_point(p, len_max);
			return;
		}
	}

	// Otherwise it is a random piece
	// Find if we know that piece
	for(i = 0; i < piece.size(); i++) {
		if(piece[i].distance_to(p) < 0.05) {
			piece[i].add_point(p, len_max);
			return;
		}
	}

	// If there is no random piece we know of
	_cloud_t c;
	c.set_mean(p, 0);
	piece.push_back(c);
}

void chessboard_evaluate() {
	for(uint i = 0; i < 64; i++) {
		chessboard[i].find_orientation();
	}
	for(uint i = 0; i < piece.size(); i++) {
		piece[i].find_orientation();
	}
}

void chessboard_clear() {
	for(uint i = 0; i < 64; i++) {
		chessboard[i].reset();
	}
	for(uint i = 0; i < piece.size(); i++) {
		piece[i].reset();
	}
	piece.clear();
}

void chessboard_drawboard() {
	glPushMatrix();
	glColor3f(0.1f, 0.1f, 0.1f);
	for(uint i = 0; i < 63; i++) {
		if((i+1) % 8 != 0) {
			glBegin(GL_LINES);
			glVertex3f(chessboard_original[i].z, chessboard_original[i].x, chessboard_original[i].y);
			glVertex3f(chessboard_original[i+1].z, chessboard_original[i+1].x, chessboard_original[i+1].y);
			glEnd();
		}

		if (i < 56) {
			glBegin( GL_LINES);
			glVertex3f(chessboard_original[i].z, chessboard_original[i].x, chessboard_original[i].y);
			glVertex3f(chessboard_original[i + 8].z, chessboard_original[i + 8].x, chessboard_original[i + 8].y);
			glEnd();
		}
	}
	glPopMatrix();
}

void cloud_draw_chessboard_means() {
	glPushMatrix();
	glColor3f(1.0f, 1.0f, 1.0f);
	for(uint i = 0; i < 64; i++) {
		if(chessboard[i].weight() < 5) continue;
		cloud_markpoint(chessboard[i].mean);
	}
	glPopMatrix();
}

void cloud_draw_piece_means() {
	glPushMatrix();
	glColor3f(1.0f, 1.0f, 1.0f);
	for(uint i = 0; i < piece.size(); i++) {
		if(piece[i].weight() < 5) continue;
		cloud_markpoint(piece[i].mean);
	}
	glPopMatrix();
}

void cloud_findorientation_and_draw() {
	glPushMatrix();
	glBegin(GL_POINTS);
	glColor3f(1.0f, 0.0f, 0.0f);
	for(uint i = 0; i < piece.size(); i++) {
		if(piece[i].weight() < 25) continue;
		piece[i].find_orientation();
		if(piece[i].angle() == 0.0f) continue;
		for(uint j = 0; j < piece[i].cloud.size(); j++) {
			cloud_drawpoint(piece[i].cloud[j]);
		}
	}
	glEnd();
	glPopMatrix();
}

void cloud_pout() {
	uint i = 0;
	for(uint i = 0; i < piece.size() && piece[i].weight() > 5; i++) {
		printf("Point %.2f, %.2f, %.2f with weight %d and orientation %lf\n",
				piece[i].mean.x, piece[i].mean.y, piece[i].mean.z, piece[i].weight(), piece[i].angle());
	}
	printf("Cloud size is %d\n", i);
}

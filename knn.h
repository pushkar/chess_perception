
#ifndef KNN_H_
#define KNN_H_

#include <vector>
#include <math.h>
#include "chess.h"
using namespace std;

// --------------------------------------------------------------------------------------------
// Chessboard KNN stuff
// knn: juggad version of knn routine, adds point to a chessboard square
void chessboard_add_point(float x, float y, float z);
void chessboard_evaluate();
void chessboard_clear();

void chessboard_drawboard();

void cloud_draw_piece_means();
void cloud_draw_chessboard_means();
void cloud_findorientation_and_draw();
void cloud_pout();


#endif /* KNN_H_ */


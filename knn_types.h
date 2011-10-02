/*
 * knn_types.h
 *
 *  Created on: Sep 7, 2010
 *      Author: bluebot
 */

#ifndef KNN_TYPES_H_
#define KNN_TYPES_H_

#include <vector>
#include <sys/types.h>
#include <math.h>
#include "chess.h"

class _point_t {
public:
	float x, y, z;
	int weight;
	float height;

	_point_t(float _x, float _y, float _z) {
		x = _x;
		y = _y;
		z = _z;
		weight = 0;
		height = 0.0f;
	}
	_point_t() {
		x = 0;
		y = 0;
		z = 0;
		weight = 0;
		height = 0.0f;
	}

	~_point_t() {}

	double distance(_point_t p) {
		return sqrt((x-p.x)*(x-p.x) + (y-p.y)*(y-p.y));
	}

	double distance(double _x, double _y, double _z) {
		return sqrt((x-_x)*(x-_x) + (y-_y)*(y-_y));
	}

	_point_t average(_point_t p) {
		_point_t r;
		r.x = (p.x + x)/2.0f;
		r.y = (p.y + y)/2.0f;
		r.z = (p.z + z)/2.0f;
		return r;
	}

	void filter(_point_t p, double alpha) {
		x = (1-alpha)*p.x + alpha*x;
		y = (1-alpha)*p.y + alpha*y;
		z = (1-alpha)*p.z + alpha*z;
	}
};

class _cloud_t {
public:
	_point_t mean;
	std::vector<_point_t> cloud;
	double _angle;
	double _ratio;
	double _height;
	int type;

	void reset() {
		type = 0;
		set_mean(_point_t(0, 0, 0), 0);
		cloud.clear();
		_angle = _ratio = _height = 0.0f;
	}

	_cloud_t() {
		reset();
	}

	~_cloud_t() {
		reset();
	}

	void set_mean(_point_t p, double alpha) {
		mean.filter(p, alpha);
	}

	void calculate_mean() {
		_point_t r;
		for(uint i = 0; i < cloud.size(); i++) {
			r.x += cloud[i].x;
			r.y += cloud[i].y;
			r.z += cloud[i].z;
		}
		mean.x = r.x/cloud.size();
		mean.y = r.y/cloud.size();
		mean.z = r.z/cloud.size();
	}

	double distance_to(_point_t p) {
		return sqrt((mean.x - p.x)*(mean.x - p.x)
				+ (mean.y - p.y)*(mean.y - p.y));
	}

	void add_point(_point_t p, double len_max) {
		cloud.push_back(p);
		float h_new = fabs(len_max - p.z);
		if(h_new > _height) _height = h_new;
		calculate_mean();
	}

	int near_pointcloud(_point_t p, double limit) {
		if(mean.distance(p) < limit) return 1;
		for(uint i = 0; i < cloud.size(); i++) {
			if(cloud[i].distance(p) < limit) return 1;
		}
		return 0;
	}

	int weight() {
		return cloud.size();
	}

	void translate(int opt, _point_t p) {
		for(uint i = 0; i < cloud.size(); i++) {
			cloud[i].x += (opt*p.x);
			cloud[i].y += (opt*p.y);
			cloud[i].z += (opt*p.z);
		}
	}

	void rotate(double angle) {
		for(uint i = 0; i < cloud.size(); i++) {
			double x = cloud[i].x;
			double y = cloud[i].y;
			cloud[i].x = x*cos(angle) - y*sin(angle);
			cloud[i].y = y*cos(angle) + x*sin(angle);
		}
	}

	void rotate(double angle, float *x_max, float *x_min, float *y_max, float *y_min) {
		*x_max = -100.0f;
		*x_min = 100.0f;
		*y_max = -100.0f;
		*y_min = 100.0f;
		for(uint i = 0; i < cloud.size(); i++) {
			double x = cloud[i].x;
			double y = cloud[i].y;
			cloud[i].x = x*cos(angle) - y*sin(angle);
			cloud[i].y = y*cos(angle) + x*sin(angle);
			if(cloud[i].x < *x_min) *x_min = cloud[i].x;
			if(cloud[i].x > *x_max) *x_max = cloud[i].x;
			if(cloud[i].y < *y_min) *y_min = cloud[i].y;
			if(cloud[i].y > *y_max) *y_max = cloud[i].y;
		}
	}

	void find_orientation() {
		translate(-1, mean);
		float x_max, x_min, y_max, y_min;
		_ratio = 0.0f;
		_angle = 0.0f;
		for(double r = 0; r < 3.14; r+=0.1f) {
			rotate(0.1f, &x_max, &x_min, &y_max, &y_min);
			float x = fabs(x_max - x_min);
			float y = fabs(y_max - y_min);
			if(x == 0.0f) continue;
			if(_ratio < y/x) {
				_ratio = y/x;
				_angle = r;
			}
		}
		rotate(_angle);
		translate(1, mean);
	}

	float angle() {
		return _angle;
	}

	float ratio() {
		return _ratio;
	}

	float height() {
		return _height;
	}

};

#endif /* KNN_TYPES_H_ */

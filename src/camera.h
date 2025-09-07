#ifndef CAMERA_H
#define CAMERA_H

#include "vector.h"

typedef struct {
	Vec3 pos;
	Vec3 target;
	Vec3 up;
} Camera;

#endif
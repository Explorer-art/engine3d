#ifndef MATRIX_H
#define MATRIX_H

#include "vector.h"

typedef struct {
	float m[4][4];
} Mat4;

Mat4 mat4_identity(void);
Mat4 mat4_translation(Vec3 t);
Vec4 mat4_mul_vec4(Mat4 m, Vec4 v);

#endif 

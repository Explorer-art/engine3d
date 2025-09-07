#include "matrix.h"

Mat4 mat4_identity(void) {
    Mat4 m = {0};

    m.m[0][0] = 1.0f;
    m.m[1][1] = 1.0f;
    m.m[2][2] = 1.0f;
    m.m[3][3] = 1.0f;

    return m;
}

Mat4 mat4_translation(Vec3 t) {
    Mat4 m = mat4_identity();

    m.m[0][3] = t.x;
    m.m[1][3] = t.y;
    m.m[2][3] = t.z;

    return m;
}

Vec4 mat4_mul_vec4(Mat4 m, Vec4 v) {
	Vec4 r;
    r.x = v.x * m.m[0][0] + v.y * m.m[0][1] + v.z * m.m[0][2] + v.w * m.m[0][3];
    r.y = v.x * m.m[1][0] + v.y * m.m[1][1] + v.z * m.m[1][2] + v.w * m.m[1][3];
    r.z = v.x * m.m[2][0] + v.y * m.m[2][1] + v.z * m.m[2][2] + v.w * m.m[2][3];
    r.w = v.x * m.m[3][0] + v.y * m.m[3][1] + v.z * m.m[3][2] + v.w * m.m[3][3];
    return r;
}
#ifndef VECTOR_H
#define VECTOR_H

typedef struct {
	int x, y;
} Vec2;

typedef struct {
	float x, y, z;
} Vec3;

typedef struct {
	float x, y, z, w;
} Vec4;

Vec3 vec3_add(Vec3 a, Vec3 b);
Vec3 vec3_sub(Vec3 a, Vec3 b);
float vec3_dot(Vec3 a, Vec3 b);
Vec3 vec3_div(Vec3 v, float s);
float vec3_length(Vec3 v);
Vec3 vec3_normalize(Vec3 v);
Vec3 vec3_cross(Vec3 a, Vec3 b);
Vec3 vec3_neg(Vec3 v);

#endif 

#include "vector.h"
#include <math.h>

Vec3 vec3_add(Vec3 a, Vec3 b) {
	return (Vec3){a.x + b.x, a.y + b.y, a.z + b.z};
}

Vec3 vec3_sub(Vec3 a, Vec3 b) {
	return (Vec3){a.x - b.x, a.y - b.y, a.z - b.z};
}

float vec3_dot(Vec3 a, Vec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

float vec3_length(Vec3 v) {
	return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

Vec3 vec3_cross(Vec3 a, Vec3 b) {
	return (Vec3){
		a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x
	};
}
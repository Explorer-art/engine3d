#include "vector.h"

Vec3 vec3_add(Vec3 a, Vec3 b) {
	return (Vec3){a.x + b.x, a.y + b.y, a.z + b.z};
}

Vec3 vec3_sub(Vec3 a, Vec3 b) {
	return (Vec3){a.x - b.x, a.y - b.y, a.z - b.z};
}
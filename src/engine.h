#ifndef ENGINE_H
#define ENGINE_H

#include "vector.h"

typedef struct {
	Vec3 pos;
	Vec3 target;
	Vec3 up;
} Camera;

typedef struct {
	Vec3 pos;
	Vec3 size;
	Vec3 angle;
	unsigned int v_size;
	unsigned int f_size;
	Vec3 *v;
	unsigned int *v_ids;
	int (*f)[3];
	unsigned int *f_ids;
} Object3D;

int E3DInit(Camera *cam);
void E3DEnd(void);
void E3DUpdate(void);
Vec3* E3DGetVertexes(void);
int (*E3DGetFaces(void))[3];
unsigned int E3DAddVertex(Vec3 vertex);
void E3DDelVertex(unsigned int v_id);
unsigned int E3DAddFace(int face[3]);
void E3DDelFace(unsigned int f_id);
Object3D* E3DNewBox(Vec3 pos, Vec3 size);
void E3DAddObject3D(Object3D *object);
void E3DDelObject3D(Object3D *obj);

#endif
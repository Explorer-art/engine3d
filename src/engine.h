#ifndef ENGINE_H
#define ENGINE_H

#include "vector.h"

int E3DInit(void);
void E3DEnd(void);
void E3DProcess(void);
Vec3* E3DGetVertexes(void);
int* E3DGetFaces(void);
int E3DAddVertex(Vec3 vertex);
void E3DDelVertex(unsigned int v_id);
int E3DAddFace(int face[3]);
void E3DDelFace(unsigned int f_id);
int E3DAddBox(Vec3 pos, Vec3 size);

#endif 

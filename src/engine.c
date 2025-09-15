#include <ncurses.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include "engine.h"
#include "vector.h"
#include "matrix.h"

float terminal_aspect_correction = 0.5f;
static int screen_h = 0;
static int screen_w = 0;
static unsigned int v_size = 0;
static unsigned int f_size = 0;
static Camera *camera = NULL;
static unsigned int objects_count = 0;
static Object3D **objects = NULL;

Vec3 *v = NULL;

int (*f)[3] = NULL;

void draw_line(Vec2 v1, Vec2 v2) {
    int dx = abs(v2.x - v1.x), sx = v1.x < v2.x ? 1 : -1;
    int dy = -abs(v2.y - v1.y), sy = v1.y < v2.y ? 1 : -1;
    int err = dx + dy;
    while (1) {
        mvaddch(v1.y, v1.x, '#');
        if (v1.x == v2.x && v1.y == v2.y) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; v1.x += sx; }
        if (e2 <= dx) { err += dx; v1.y += sy; }
    }
}

Mat4 perspective(float fov_deg, float aspect, float near, float far) {
    float fov_rad = fov_deg * M_PI / 180.0f;
    float f = 1.0f / tanf(fov_rad / 2.0f);
    Mat4 m = {0};

    m.m[0][0] = f / aspect;
    m.m[1][1] = f;
    m.m[2][2] = (far + near) / (near - far);
    m.m[2][3] = (2 * far * near) / (near - far);
    m.m[3][2] = -1.0f;
    return m;
}

Mat4 look_at(Vec3 pos, Vec3 target, Vec3 up) {
    Vec3 f = vec3_sub(target, pos);
    f = vec3_normalize(f);  // Вектор взгляда (forward)

    Vec3 s = vec3_cross(f, up);
    s = vec3_normalize(s);  // Вектор вправо (side)

    Vec3 u = vec3_cross(s, f);  // Пересчитанный вектор вверх (up)

    Mat4 view = {0};
    view.m[0][0] = s.x;
    view.m[1][0] = s.y;
    view.m[2][0] = s.z;

    view.m[0][1] = u.x;
    view.m[1][1] = u.y;
    view.m[2][1] = u.z;

    view.m[0][2] = -f.x;
    view.m[1][2] = -f.y;
    view.m[2][2] = -f.z;

    view.m[3][3] = 1.0f;

    view.m[0][3] = -vec3_dot(s, pos);
    view.m[1][3] = -vec3_dot(u, pos);
    view.m[2][3] = vec3_dot(f, pos);

    return view;
}

Vec2 project_point(Vec3 v, Mat4 proj, Mat4 view, int w, int h) {
    Vec4 v4 = {v.x, v.y, v.z, 1.0f};
    Vec4 view_space = mat4_mul_vec4(view, v4);
    Vec4 projected = mat4_mul_vec4(proj, view_space);

    if (projected.w == 0) projected.w = 0.0001f; // защита от деления на 0

    // Перспективное деление
    projected.x /= projected.w;
    projected.y /= projected.w;
    projected.z /= projected.w;

    // Теперь x, y, z ∈ [-1, 1]
    // Перевод в координаты экрана
    int sx = (projected.x + 1.0f) * 0.5f * screen_w;
    int sy = (1.0f - projected.y) * 0.5f * screen_h;

    return (Vec2){sx, sy};
}

Vec3 triangle_normal(Vec3 a, Vec3 b, Vec3 c) {
    Vec3 ab = vec3_sub(b, a);
    Vec3 ac = vec3_sub(c, a);

    Vec3 normal = vec3_cross(ab, ac);

    float length = vec3_length(normal);

    if (length == 0.0f)
        return (Vec3){0.0f, 0.0f, 0.0f};

    return (Vec3){normal.x / length, normal.y / length, normal.z / length};
}

Vec3 rotate_point(Vec3 v, float angleX, float angleY) {
    float cosX = cos(angleX);
    float sinX = sin(angleX);
    float y1 = v.y * cosX - v.z * sinX;
    float z1 = v.y * sinX + v.z * cosX;

    float cosY = cos(angleY);
    float sinY = sin(angleY);
    float x2 = v.x * cosY + z1 * sinY;
    float z2 = -v.x * sinY + z1 * cosY;

    return (Vec3){x2, y1, z2};
}

Vec3* E3DGetVertexes(void) {
    return v;
}

int (*E3DGetFaces(void))[3] {
    return f;
}

unsigned int E3DAddVertex(Vec3 vertex) {
    v = realloc(v, (v_size + 1) * sizeof(Vec3));
    if (!v) {
        fprintf(stderr, "Memory re-allocated failed\n");
        return -1;
    }

    v[v_size] = vertex;
    v_size++;
    return v_size - 1;
}

void E3DDelVertex(unsigned int v_id) {
    for (int i = v_id; i < v_size - 1; i++) {
        v[i] = v[i + 1];
    }
    v_size--;

    v = realloc(v, v_size * sizeof(Vec3));
    if (!v) {
        fprintf(stderr, "Memory re-allocated failed\n");
    }
}

unsigned int E3DAddFace(int face[3]) {
    f = realloc(f, (f_size + 1) * sizeof(int[3]));
    if (!f) {
        fprintf(stderr, "Memory re-allocated failed\n");
        return -1;
    }

    f[f_size][0] = face[0];
    f[f_size][1] = face[1];
    f[f_size][2] = face[2];
    f_size++;
    return f_size - 1;
}

void E3DDelFace(unsigned int f_id) {
    for (int i = f_id; i < f_size - 1; i++) {
        f[i][0] = f[i + 1][0];
        f[i][1] = f[i + 1][1];
        f[i][2] = f[i + 1][2];
    }
    f_size--;

    f = realloc(f, f_size * sizeof(int[3]));
    if (!f) {
        fprintf(stderr, "Memory re-allocated failed\n");
    }
}

Object3D* E3DNewBox(Vec3 pos, Vec3 size, Vec3 angle) {
    Object3D *box = malloc(sizeof(Object3D));
    box->pos = pos;
    box->size = size;
    box->angle = angle;
    box->enable = 0;

    Vec3 *v = malloc(8 * sizeof(Vec3));
    if (!v) {
        fprintf(stderr, "Memory allocated failed\n");
        return NULL;
    }

    v[0] = (Vec3){pos.x - (size.x / 2), pos.y - (size.y / 2), pos.z - (size.z / 2)};
    v[1] = (Vec3){pos.x + (size.x / 2), pos.y - (size.y / 2), pos.z - (size.z / 2)};
    v[2] = (Vec3){pos.x + (size.x / 2), pos.y + (size.y / 2), pos.z - (size.z / 2)};
    v[3] = (Vec3){pos.x - (size.x / 2), pos.y + (size.y / 2), pos.z - (size.z / 2)};
    v[4] = (Vec3){pos.x - (size.x / 2), pos.y - (size.y / 2), pos.z + (size.z / 2)};
    v[5] = (Vec3){pos.x + (size.x / 2), pos.y - (size.y / 2), pos.z + (size.z / 2)};
    v[6] = (Vec3){pos.x + (size.x / 2), pos.y + (size.y / 2), pos.z + (size.z / 2)};
    v[7] = (Vec3){pos.x - (size.x / 2), pos.y + (size.y / 2), pos.z + (size.z / 2)};

    box->v = v;
    box->v_size = 8;

    int (*f)[3] = malloc(12 * sizeof(int[3]));
    if (!f) {
        free(v);
        fprintf(stderr, "Memory allocated failed\n");
        return NULL;
    }

    f[0][0] = 0; f[0][1] = 1; f[0][2] = 2;
    f[1][0] = 0; f[1][1] = 2; f[1][2] = 3;
    f[2][0] = 1; f[2][1] = 5; f[2][2] = 6;
    f[3][0] = 1; f[3][1] = 6; f[3][2] = 2;
    f[4][0] = 5; f[4][1] = 4; f[4][2] = 7;
    f[5][0] = 5; f[5][1] = 7; f[5][2] = 6;
    f[6][0] = 4; f[6][1] = 0; f[6][2] = 3;
    f[7][0] = 4; f[7][1] = 3; f[7][2] = 7;
    f[8][0] = 3; f[8][1] = 2; f[8][2] = 6;
    f[9][0] = 3; f[9][1] = 6; f[9][2] = 7;
    f[10][0] = 4; f[10][1] = 5; f[10][2] = 1;
    f[11][0] = 4; f[11][1] = 1; f[11][2] = 0;

    box->f = f;
    box->f_size = 12;

    objects = realloc(objects, (objects_count + 1) * sizeof(Object3D));
    if (!objects) {
        fprintf(stderr, "Memory re-allocated failed\n");
        return NULL;
    }

    objects[objects_count] = box;
    box->id = objects_count;
    objects_count++;

    return box;
}

void E3DAddObject3D(Object3D *obj) {
    unsigned int *v_ids = malloc(obj->v_size * sizeof(unsigned int));
    if (!v_ids) {
        fprintf(stderr, "Memory allocated failed\n");
        return;
    }

    for (unsigned int i = 0; i < obj->v_size; i++) {
        v_ids[i] = E3DAddVertex(obj->v[i]);
    }

    obj->v_ids = v_ids;

    unsigned int *f_ids = malloc(obj->f_size * sizeof(unsigned int));
    if (!f_ids) {
        fprintf(stderr, "Memory allocated failed\n");
        return;
    }

    for (unsigned int i = 0; i < obj->f_size; i++) {
        f_ids[i] = E3DAddFace((int[3]){v_ids[obj->f[i][0]], v_ids[obj->f[i][1]], v_ids[obj->f[i][2]]});
    }

    obj->f_ids = f_ids;
    obj->enable = 1;
}

void E3DDelObject3D(Object3D *obj) {
    for (unsigned int i = obj->id; i < objects_count - 1; i++) {
        objects[i] = objects[i + 1];
    }

    objects_count--;
    objects = realloc(objects, (objects_count + 1) * sizeof(Object3D));

    if (!objects) {
        fprintf(stderr, "Memory re-allocated failed\n");
        return;
    }

    for (unsigned int i = 0; i < obj->v_size; i++) {
        E3DDelVertex(obj->v_ids[i]);
    }

    free(obj->v_ids);

    for (unsigned int i = 0; i < obj->f_size; i++) {
        E3DDelFace(obj->f_ids[i]);
    }

    free(obj->f_ids);
    free(obj->v);
    free(obj->f);
}

int E3DInit(Camera *cam) {
    initscr();
    noecho();
    curs_set(0);

    getmaxyx(stdscr, screen_h, screen_w);

    camera = cam;

    return 1;
}

void E3DEnd(void) {
    if (v) {
        free(v);
        v = NULL;
    }

    if (f) {
        free(f);
        f = NULL;
    }
    endwin();
}

void E3DUpdateObject3D(Object3D *obj) {
    for (unsigned int i = 0; i < obj->v_size; i++) {
        Vec3 local = vec3_sub(v[obj->v_ids[i]], obj->pos);
        Vec3 rotated_local = rotate_point(local, obj->angle.x, obj->angle.y);
        v[i] = vec3_add(rotated_local, obj->pos);
    }
}

void E3DUpdate(void) {
    float aspect = ((float)screen_w / screen_h) * terminal_aspect_correction;
    Mat4 proj = perspective(90.0f, aspect, 0.1f, 100.0f);
    Mat4 view = look_at(camera->pos, camera->target, camera->up);

    Vec3 *rotated = malloc(v_size * sizeof(Vec3));
    if (!rotated) {
        fprintf(stderr, "Memory allocated failed\n");
        return;
    }

    Vec2 *projected = malloc(v_size * sizeof(Vec2));
    if (!projected) {
        fprintf(stderr, "Memory allocated failed\n");
        return;
    }

    for (unsigned int i = 0; i < objects_count; i++) {
        if (objects[i]->enable == 1) {
            for (unsigned int j = 0; j < objects[i]->v_size; j++) {
                unsigned int global_v_id = objects[i]->v_ids[j];
                
                Vec3 local = vec3_sub(v[global_v_id], objects[i]->pos);
                Vec3 rotated_local = rotate_point(local, objects[i]->angle.x, objects[i]->angle.y);
                rotated[global_v_id] = vec3_add(rotated_local, objects[i]->pos);
            }
        }
    }

    // Project
    for (int i = 0; i < v_size; i++) {
        projected[i] = project_point(rotated[i], proj, view, screen_w, screen_h);
    }

    #ifdef DEBUG
    mvprintw(0, 0, "Vertexes size: %d", v_size);
    mvprintw(1, 0, "Faces size: %d", f_size);

    for (int i = 0; i < v_size; i++) {
        mvprintw(3 + i, 0, "Vec3 (%f, %f, %f)", v[i].x, v[i].y, v[i].z);
    }

    for (int i = 0; i < f_size; i++) {
        mvprintw(15 + i, 0, "%d, %d, %d", f[i][0], f[i][1], f[i][2]);
    }
    #endif

    for (int i = 0; i < f_size; i++) {
        int *t = f[i];

        // Переводим вершины треугольника в view space
        Vec4 v0_view4 = mat4_mul_vec4(view, (Vec4){rotated[t[0]].x, rotated[t[0]].y, rotated[t[0]].z, 1.0f});
        Vec4 v1_view4 = mat4_mul_vec4(view, (Vec4){rotated[t[1]].x, rotated[t[1]].y, rotated[t[1]].z, 1.0f});
        Vec4 v2_view4 = mat4_mul_vec4(view, (Vec4){rotated[t[2]].x, rotated[t[2]].y, rotated[t[2]].z, 1.0f});

        // Переводим в Vec3
        Vec3 v0_view = {v0_view4.x, v0_view4.y, v0_view4.z};
        Vec3 v1_view = {v1_view4.x, v1_view4.y, v1_view4.z};
        Vec3 v2_view = {v2_view4.x, v2_view4.y, v2_view4.z};

        Vec3 center = vec3_div(vec3_add(vec3_add(v0_view, v1_view), v2_view), 3.0f);
        Vec3 normal = triangle_normal(v0_view, v1_view, v2_view);
        Vec3 view_dir = vec3_normalize(vec3_neg(center));

        float dp = vec3_dot(normal, view_dir);

        if (dp < 0.0f) {
            Vec2 v0 = projected[t[0]];
            Vec2 v1 = projected[t[1]];
            Vec2 v2 = projected[t[2]];

            draw_line(v0, v1);
            draw_line(v1, v2);
            draw_line(v2, v0);
        }

        // draw_line(v0, v1);
        // draw_line(v1, v2);
        // draw_line(v2, v0);
    }

    free(rotated);
    free(projected);
}
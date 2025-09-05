#include <ncurses.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include "vector.h"
#include "matrix.h"

static float distance = 10.0;
float terminal_aspect_correction = 0.5f;
static int screen_h = 0;
static int screen_w = 0;
static int v_count = 8;
static int f_count = 12;
static float angleX = 0;
static float angleY = 0;

Vec3 v[] = {
    {-1, -1, -1 - 3},
    {1, -1, -1 - 3},
    {1, 1, -1 - 3},
    {-1, 1, -1 - 3},
    {-1, -1, 1 - 3},
    {1, -1, 1 - 3},
    {1, 1, 1 - 3},
    {-1, 1, 1 - 3}
};

int f[][3] = {
    // Передняя грань (z = -4)
    {0, 1, 2},
    {0, 2, 3},

    // Правая грань (x = +1)
    {1, 5, 6},
    {1, 6, 2},

    // Задняя грань (z = -2)
    {5, 4, 7},
    {5, 7, 6},

    // Левая грань (x = -1)
    {4, 0, 3},
    {4, 3, 7},

    // Верхняя грань (y = +1)
    {3, 2, 6},
    {3, 6, 7},

    // Нижняя грань (y = -1)
    {4, 5, 1},
    {4, 1, 0}
};


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

Vec2 project_point(Vec3 v, Mat4 proj, int w, int h) {
	Vec4 v4 = {v.x, v.y, v.z, 1.0f};
    Vec4 projected = mat4_mul_vec4(proj, v4);

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

// float angle_camera_and_normal(Vec3 camera_pos, Vec3 triangle_point, Vec3 normal) {
//     Vec3 to_camera = vec3_sub(camera_pos, triangle_point);

//     float length = vec3_length(to_camera);

//     Vec3 dir = {0.0f, 0.0f, 0.0f};

//     if (length != 0.0f) {
//         dir.x = to_camera.x / length;
//         dir.y = to_camera.y / length;
//         dir.z = to_camera.z / length;
//     }

//     float cos_angle = vec3_dot(normal, dir);
//     if (cos_angle > 1.0f) cos_angle = 1.0f;
//     if (cos_angle < -1.0f) cos_angle = -1.0f;

//     return acosf(cos_angle);
// }

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

void E3DInit(void) {
	initscr();
	noecho();
	curs_set(0);

	getmaxyx(stdscr, screen_h, screen_w);
}

void E3DEnd(void) {
	endwin();
}

void E3DProcess(void) {
    while (1) {
        clear();

        Vec3 *rotated = malloc(v_count * sizeof(Vec3));
        Vec2 *projected = malloc(v_count * sizeof(Vec2));
        float aspect = ((float)screen_w / screen_h) * terminal_aspect_correction;
        Mat4 proj = perspective(90.0f, aspect, 0.1f, 100.0f);

        Vec3 center = {0, 0, -3};

        // Rotate around center
        for (int i = 0; i < v_count; i++) {
            Vec3 local = vec3_sub(v[i], center);
            Vec3 rotated_local = rotate_point(local, angleX, angleY);
            rotated[i] = vec3_add(rotated_local, center);
        }

        // Project
        for (int i = 0; i < v_count; i++) {
            projected[i] = project_point(rotated[i], proj, screen_w, screen_h);
        }

        // Draw faces
        for (int i = 0; i < f_count; i++) {
            int *t = f[i];
            Vec2 v0 = projected[t[0]];
            Vec2 v1 = projected[t[1]];
            Vec2 v2 = projected[t[2]];

            Vec3 normal = triangle_normal(rotated[t[0]], rotated[t[1]], rotated[t[2]]);
            Vec3 to_camera = vec3_sub((Vec3){0.0f, 0.0f, 0.0f}, rotated[t[0]]);
            float dot = vec3_dot(normal, to_camera);

            if (dot < 0.0f) {
                draw_line(v0, v1);
                draw_line(v1, v2);
                draw_line(v2, v0);
            }
        }

        free(projected);
        free(rotated);

        refresh();
        usleep(30000);

        angleX += 0.07f;
        angleY += 0.07f;
    }
}
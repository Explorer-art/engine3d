#include <stdlib.h>
#include <ncurses.h>
#include <math.h>
#include <unistd.h>

#define SCALE 0.3

float cube[8][3] = {
    {-1 * SCALE, -1 * SCALE, -1 * SCALE},
    { 1 * SCALE, -1 * SCALE, -1 * SCALE},
    { 1 * SCALE,  1 * SCALE, -1 * SCALE},
    {-1 * SCALE,  1 * SCALE, -1 * SCALE},
    {-1 * SCALE, -1 * SCALE,  1 * SCALE},
    { 1 * SCALE, -1 * SCALE,  1 * SCALE},
    { 1 * SCALE,  1 * SCALE,  1 * SCALE},
    {-1 * SCALE,  1 * SCALE,  1 * SCALE}
};

int triangles[12][3] = {
    {0,1,2}, {0,2,3}, // задняя грань
    {4,5,6}, {4,6,7}, // передняя
    {0,3,7}, {0,7,4}, // левая
    {1,2,6}, {1,6,5}, // правая
    {3,2,6}, {3,6,7}, // верхняя
    {0,1,5}, {0,5,4}  // нижняя
};

void rotate_point(float x, float y, float z, float angleX, float angleY, float *outX, float *outY, float *outZ) {
    float cosX = cos(angleX);
    float sinX = sin(angleX);
    float y1 = y * cosX - z * sinX;
    float z1 = y * sinX + z * cosX;

    float cosY = cos(angleY);
    float sinY = sin(angleY);
    float x2 = x * cosY + z1 * sinY;
    float z2 = -x * sinY + z1 * cosY;

    *outX = x2;
    *outY = y1;
    *outZ = z2;
}

void project_point(float x, float y, float z, int w, int h, int *sx, int *sy) {
    float distance = 10.0;
    float factor = distance / (z + distance);
    *sx = (int)(x * factor * w / 2 + w / 2);
    *sy = (int)(-y * factor * h / 2 + h / 2);
}

void draw_line(int x0, int y0, int x1, int y1) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;
    while (1) {
        mvaddch(y0, x0, '#');
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

int main() {
    initscr();
    noecho();
    curs_set(FALSE);

    float angleX = 0;
    float angleY = 0;

    while (1) {
        clear();
        int h, w;
        getmaxyx(stdscr, h, w);

        float rotated[8][3];
        int projected[8][2];

        for (int i = 0; i < 8; i++) {
            rotate_point(cube[i][0], cube[i][1], cube[i][2], angleX, angleY,
                         &rotated[i][0], &rotated[i][1], &rotated[i][2]);
            project_point(rotated[i][0], rotated[i][1], rotated[i][2], w, h,
                          &projected[i][0], &projected[i][1]);
        }

        for (int i = 0; i < 12; i++) {
            int *t = triangles[i];
            int x0 = projected[t[0]][0], y0 = projected[t[0]][1];
            int x1 = projected[t[1]][0], y1 = projected[t[1]][1];
            int x2 = projected[t[2]][0], y2 = projected[t[2]][1];

            // рисуем треугольник (пока только границы)
            draw_line(x0, y0, x1, y1);
            draw_line(x1, y1, x2, y2);
            draw_line(x2, y2, x0, y0);
        }

        refresh();
        usleep(30000); // ~30 FPS
        angleX += 0.03;
        angleY += 0.05;
    }

    endwin();
    return 0;
}
 

#include <ncurses.h>
#include <unistd.h>
#include "engine.h"

int main() {
    Camera camera = {
        (Vec3){0.0f, 0.0f, 4.0f},
        (Vec3){0.0f, 0.0f, 0.0f},
        (Vec3){0.0f, 1.0f, 0.0f}
    };

    int status = E3DInit(&camera);
    if (!status) {
        return 1;
    }

    Object3D *cube = E3DNewBox((Vec3){0, 0, 0}, (Vec3){2, 2, 2}, (Vec3){0.0f, 0.0f, 0.0f});
    E3DAddObject3D(cube);

    while (1) {
        clear();

        E3DUpdate();

        refresh();
        usleep(30000); // ~30 FPS

        cube->angle.x += 0.05f;
        cube->angle.y += 0.05f;
    }
    
    E3DDelObject3D(cube);
    E3DEnd();
    
    return 0;
}
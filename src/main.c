#include "engine.h"
#include "vector.h"

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

    E3DAddBox((Vec3){0, 0, 0}, (Vec3){2, 2, 2});

    E3DProcess();
    E3DEnd();
    
    return 0;
}
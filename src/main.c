#include "engine.h"
#include "vector.h"

int main() {
    int status = E3DInit();
    if (!status) {
        return 1;
    }

    E3DAddBox((Vec3){0, 0, -3}, (Vec3){2, 2, 2});

    E3DProcess();
    E3DEnd();
    
    return 0;
}
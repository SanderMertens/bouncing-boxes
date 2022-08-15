#include <bouncing_boxes.h>
#define BOX_BRIGHTNESS (1.5)
#define BOX_X_COUNT (400.0)
#define BOX_Y_COUNT (400.0)
#define BOX_SIZE (1.5)

bool dice(float chance) {
    return (float)rand() > ((float)RAND_MAX / chance);
}

float randf() {
    return (float)rand() / (float)RAND_MAX;
}

void Bounce(ecs_iter_t *it) {
    EcsPosition3 *p = ecs_field(it, EcsPosition3, 1);
    EcsVelocity3 *v = ecs_field(it, EcsVelocity3, 2);
    EcsRgb *c = ecs_field(it, EcsRgb, 3);

    /* Fade out colors, and move squares back into position */
    for (int i = 0; i < it->count; i ++) {
        float y = p[i].y;
        float lt = y < 0;
        p[i].y = lt * (y + it->delta_time * v[i].y);
        v[i].y += 20 * it->delta_time;

        c[i].r = glm_max(0.96 * c[i].r, 0.0);
        c[i].g = glm_max(0.93 * c[i].g, 0.0);
        c[i].b = glm_max(0.99 * c[i].b, 0.0);
    }

    /* Bounce random squares. Lower threshold to increase number of bounces */
    int threshold = RAND_MAX - RAND_MAX / 1000.0;

    for (int i = 0; i < it->count; i ++) {
        if (rand() > threshold) {
            p[i].y = -0.01;
            v[i].y = -1.0 - 5 * randf();

            bool d = dice(1.1);
            c[i].r = BOX_BRIGHTNESS * (d);
            c[i].b = BOX_BRIGHTNESS * (1.0);
            c[i].g = BOX_BRIGHTNESS * (1.0 - d * 0.2);
        }
    }
}

int main(int argc, char *argv[]) {
    ecs_world_t *world = ecs_init();

    ECS_IMPORT(world, FlecsComponentsTransform);
    ECS_IMPORT(world, FlecsComponentsGeometry);
    ECS_IMPORT(world, FlecsComponentsPhysics);
    ECS_IMPORT(world, FlecsComponentsGraphics);
    ECS_IMPORT(world, FlecsComponentsGui);
    ECS_IMPORT(world, FlecsSystemsSokol);
    ECS_IMPORT(world, FlecsGame);

    ECS_SYSTEM(world, Bounce, EcsOnUpdate,
        flecs.components.transform.Position3,
        flecs.components.physics.Velocity3,
        flecs.components.graphics.Rgb);

    ecs_entity_t camera = ecs_set(world, 0, EcsCamera, {
        .lookat = {0.0, 0.0, 5.0},
        .up = {0.0, -1.0, 0.0},
        .fov = 20,
        .near_ = 1.0, .far_ = 1000.0
    });

    ecs_add(world, camera, EcsCameraController);
    ecs_set(world, camera, EcsPosition3, {0, -10, 250});
    ecs_set(world, camera, EcsRotation3, {0.7});

    ecs_set(world, 0, EcsCanvas, {
        .title = "Flecs Bouncing Boxes",
        .width = 1400, .height = 1000,
        .background_color = {0, 0, 0},
        .camera = camera,
        .fog_density = 5.0
    });

    /* Create strip of squares */
    for (int x = 0; x < BOX_X_COUNT; x ++) {
        for (int z = 0; z < BOX_Y_COUNT; z ++) {
            ecs_entity_t e = 
            ecs_set(world, 0, EcsBox, {BOX_SIZE, BOX_SIZE * 4.0, BOX_SIZE});
            ecs_set(world, e, EcsRgb, {0.0, 0.0, 0.0});
            ecs_set(world, e, EcsPosition3, {
                x * BOX_SIZE - (BOX_X_COUNT / 2.0) * BOX_SIZE, 
                0.0, 
                z * BOX_SIZE});
            ecs_set(world, e, EcsVelocity3, {0.0, 0.0, 0.0});
        }
    }   

    return ecs_app_run(world, &(ecs_app_desc_t){ .target_fps = 60 });
}

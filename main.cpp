#include "raylib.h"
#include <raymath.h>
#include <functional>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <random>

Color BACKGROUND = BLACK;
Color FOREGROUND = RAYWHITE;
//resizing isnt changing this
int SCREEN_WIDTH = 800;
int SCREEN_HEIGHT = 600;

float INITIAL_SPEED = 0.10;
int SWARM_SIZE = 200;
float INERTIA_WEIGHT = 0.5;
float COGNITIVE_COEFFICIENT = 2;
float SOCIAL_COEFFICIENT = 2;
float TOLERANCE = 1e-10f;
Color SWARM_COLOUR = (Color) {255, 0, 255, 150};
Color ANTIPODE_COLOUR = BLUE;
Color PATH_COLOUR = GREEN;

// minimap constants.
int MINIMAP_WIDTH = 200;
int MINIMAP_HEIGHT = 200;
int MINIMAP_STEPS = 100;
int MINIMAP_MARGIN_LEFT = 10;
int MINIMAP_MARGIN_BOTTOM = 10;
Color MINIMAP_COLOUR = LIGHTGRAY;



Vector2 f(Vector3 coords) {
   // defines a function over S^2

   return (Vector2) {coords.x - coords.y, tan(coords.z)};
}

float loss(Vector3 coords, Vector2 (*func)(Vector3)) {
    return Vector2Length(Vector2Subtract((*func)(coords), (*func)(Vector3Scale(coords, -1.0))));

}

std::tuple<std::vector<Vector3>, std::vector<Vector3>, std::vector<Vector3>, Vector3> init_particle_swarm(int swarm_size, Vector2 (*func)(Vector3), std::mt19937 gen) {
    // the simplex stuff was too hard and my brain is smooth.
    // We will do particle swarm optimisation instead.

    // set up random stuff
    //std::mt19937 gen(rd()); 
    std::uniform_real_distribution<float> dis(-1.0f, 1.0f);

    // init swarm (swarm_size random unit vectors.)
    std::vector<Vector3> swarm(swarm_size); 
    std::vector<Vector3> best_known_pos(swarm_size);
    std::vector<Vector3> velocities(swarm_size);
    Vector3 global = {1,0,0};
    for (int i = 0; i < swarm_size; i++) {
        Vector3 rand_vec = {dis(gen), dis(gen), dis(gen)};
        rand_vec = Vector3Normalize(rand_vec);
        //printf("%f,%f,%f", rand_vec.x, rand_vec.y, rand_vec.z);
        swarm[i] = rand_vec;
        //printf("%f,%f,%f", swarm[i].x, swarm[i].y, swarm[i].z);
        best_known_pos[i] = rand_vec;
        if (loss(rand_vec, func) < loss(global,func)) {
            global = rand_vec;
        }
        
        Vector3 rand_velocity = {dis(gen), dis(gen), dis(gen)};
        rand_velocity = Vector3Scale(Vector3Normalize(rand_velocity), INITIAL_SPEED);
        velocities[i] =  rand_velocity;
    }
   

    return {swarm, best_known_pos, velocities, global};
} 

// we define a graph on the edges instead of a dual graph.
// it should map 1e to a {{e,e}, {e,e}} and we can the previous edge to see which face we came from.
int* generate_weighted_graph(Mesh mesh) {
    // maybe make this a dictionary instead.
    int* vertex_labels = new int[mesh.vertexCount];

    int totalIndices = mesh.triangleCount * 3;

    for (int i = 0; i < totalIndices; i += 3) {
        unsigned short idx0 = mesh.indices[i];
        unsigned short idx1 = mesh.indices[i + 1];
        unsigned short idx2 = mesh.indices[i + 2];

        // remember to not include the triangle if its in the wrong hemisphere.
        Vector3 v0 = (Vector3){ mesh.vertices[idx0 * 3], mesh.vertices[idx0 * 3 + 1], mesh.vertices[idx0 * 3 + 2] };
        Vector3 v1 = (Vector3){ mesh.vertices[idx1 * 3], mesh.vertices[idx1 * 3 + 1], mesh.vertices[idx1 * 3 + 2] };
        Vector3 v2 = (Vector3){ mesh.vertices[idx2 * 3], mesh.vertices[idx2 * 3 + 1], mesh.vertices[idx2 * 3 + 2] };

        
    }
}
// compute a path on the sphere and its mapping to the plane.

std::tuple<std::vector<Vector3>, std::vector<Vector2>> compute_path(Vector3 global, Vector3 axis, int steps, Vector2 (*func)(Vector3)) {
    std::vector<Vector3> path(steps + 1);
    path[0] = global;
    for (int i = 0; i < steps; i++) {
        path[i + 1] = Vector3RotateByAxisAngle(global, axis, PI / steps * (i + 1));
    }

    std::vector<Vector2> values(steps + 1);
    for (int i = 0; i < steps + 1; i++) {
        values[i] = (*func)(path[i]);
    }

    return {path, values};
}


int main(void) {
    //boilder plates
    
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Borsuk-Ulam");

    // setting up the camera
    Camera3D camera = { 0 };
    camera.position = (Vector3){ 0.0f, 4.0f, 6.0f }; // Camera position in 3D space
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };   // Look at the center of the world
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };       // Camera up vector (Y-axis is up)
    camera.fovy = 45.0f;                             // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;          // Camera projection type


    // man I just want my camera to move around with WASD.
    float cam_radius = 5.0f; // Distance from the sphere target
    float yaw = 0.0f;  // Horizontal angle (yaw)
    float pitch = 0.5f;   // Vertical angle (pitch)
    float rotation_speed = 2.0f;  // Speed of rotation

    // this is the mesh (idk how to edit size on the fly and parameterise it.)
    Mesh sphereMesh = GenMeshSphere(1.0f, 100, 100);
    Model sphereModel = LoadModelFromMesh(sphereMesh);

    sphereModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = RED;

    Vector3 modelPosition = { 0.0f, 0.0f, 0.0f };

    // the texture for the minimap.
    RenderTexture2D target = LoadRenderTexture(MINIMAP_WIDTH, MINIMAP_HEIGHT);

    // initialise the swarm
    std::random_device rd;
    std::mt19937 gen(rd()); 

    float min = 0.0f;
    float max = 1.0f;
    std::uniform_real_distribution<float> dis(min, max);



    // PCO constants
    auto [positions, best_known_pos, velocities, global] = init_particle_swarm(SWARM_SIZE, f, gen);
    float path_angle = 0;
    Vector3 axis = Vector3Perpendicular(global);
    auto [path_sphere, path_plane] = compute_path(global, axis, MINIMAP_STEPS, f);
    bool global_changed = true;
    bool path_angle_changed = true;

    SetTargetFPS(120); 

    bool swarm_running = false;
    bool result = false;
    // main loop
    int iterations = 0;
    while (!WindowShouldClose()) {
        //update
        float dt = GetFrameTime();

        //mvoing things around
        if (IsKeyDown(KEY_A)) yaw -= rotation_speed * dt; // Orbit left
        if (IsKeyDown(KEY_D)) yaw += rotation_speed * dt; // Orbit right
        if (IsKeyDown(KEY_W)) pitch += rotation_speed * dt;  // Orbit up
        if (IsKeyDown(KEY_S)) pitch -= rotation_speed * dt;  // Orbit down
        if (IsKeyPressed(KEY_SPACE)) swarm_running = !swarm_running;
        // Clamp the vertical angle so the camera doesn't flip upside down at the poles
        if (pitch >  1.5f) pitch =  1.5f;
        if (pitch < -1.5f) pitch = -1.5f;

        // 3. Mathematical conversion from Angles -> 3D position vector
        camera.position.x = camera.target.x + cam_radius * cosf(pitch) * sinf(yaw);
        camera.position.y = camera.target.y + cam_radius * sinf(pitch);
        camera.position.z = camera.target.z + cam_radius * cosf(pitch) * cosf(yaw);
        
        //compute the swarm
        if (swarm_running && iterations % 5 == 0) {

            // random movement
            for (int i = 0; i < SWARM_SIZE; i++) {
                Vector3 rand_p = {dis(gen), dis(gen), dis(gen)};
                Vector3 rand_g = {dis(gen), dis(gen), dis(gen)};
                
                // updating the velocities using the formula
                velocities[i] = {
                    INERTIA_WEIGHT * velocities[i].x + COGNITIVE_COEFFICIENT * rand_p.x * (best_known_pos[i].x - positions[i].x) + SOCIAL_COEFFICIENT * rand_g.x * (global.x - positions[i].x),
                    INERTIA_WEIGHT * velocities[i].y + COGNITIVE_COEFFICIENT * rand_p.y * (best_known_pos[i].y - positions[i].y) + SOCIAL_COEFFICIENT * rand_g.y * (global.y - positions[i].y),
                    INERTIA_WEIGHT * velocities[i].z + COGNITIVE_COEFFICIENT * rand_p.z * (best_known_pos[i].z - positions[i].z) + SOCIAL_COEFFICIENT * rand_g.z * (global.z - positions[i].z)
                };
                
                positions[i] = Vector3Normalize(Vector3Add(positions[i], velocities[i]));

                // updating best knowns and global
                if (loss(positions[i], f) < loss(best_known_pos[i], f)) {
                    best_known_pos[i] = positions[i];
                    
                    if (loss(best_known_pos[i], f) < loss(global, f)) {
                        global = best_known_pos[i];
                        global_changed = true;
                    }
                }

                if (loss(global, f) < TOLERANCE) {
                    swarm_running = false;
                    result = true;

                    TraceLog(LOG_INFO, "Result found, %f,%f,%f, with loss %f", best_known_pos[i].x, best_known_pos[i].y, best_known_pos[i].z, loss(best_known_pos[i], f));                
                    TraceLog(LOG_INFO, "Antipodal values: (%f, %f), (%f, %f)", f(global).x, f(global).y, f(Vector3Scale(global, -1)).x, f(Vector3Scale(global,-1)).y);                
                }

            }
                TraceLog(LOG_INFO, "Current Global best, %f,%f,%f, with loss %f", global.x, global.y, global.z, loss(global, f));                
        }


        //raylib boilerplate
        BeginDrawing();
            ClearBackground(BACKGROUND);

            BeginMode3D(camera);

                DrawModel(sphereModel, modelPosition, 1.0f, FOREGROUND);

                DrawModelWires(sphereModel, modelPosition, 1.0f, (Color) {107, 7, 0, 55});

                //DrawGrid(10, 1.0f);


           
            //for (int i = 0; i < SWARM_SIZE; i++) {
            //       printf("%f,%f,%f", positions[i].x, positions[i].y, positions[i].z);
            //}

            // drwa the swarm
            for (int i = 0; i < SWARM_SIZE; i++) {
                DrawSphere(positions[i], 0.05f, SWARM_COLOUR);
            }

            // draw the global as it moves around.
            DrawSphere(global, 0.05f, ANTIPODE_COLOUR);

            DrawSphere(Vector3Scale(global,-1), 0.05f, ANTIPODE_COLOUR);

            //DrawSphere(axis, 0.05f, PATH_COLOUR);
            // draw the path on the sphere 
            // TODO: draw an arc


            EndMode3D();

            // recalculate the path if anything has changed.
            if (global_changed || path_angle_changed){
                axis = Vector3Perpendicular(global);
                axis = Vector3RotateByAxisAngle(axis, global, path_angle);
                auto [path_sphere, path_plane] = compute_path(global, axis, MINIMAP_STEPS, f);
                
                global_changed = false;
                path_angle_changed = false;

                // compute new minimap texture 
                BeginTextureMode(target);
                    ClearBackground(MINIMAP_COLOUR);
                    // scale the stuff to the target.
                    // So we want to take this path_plane 
                    // and scale it to fit in a 200x200 box with origin in the top left.
                    // kv + w , we can calculate k by taking max of length of path_plane,
                    // k = 100/max
                    // set w to be ((100, 100) - k * f(global)), then f(global) gets put in the middle.
                    Vector2 fixed_point = f(global);

                    // compute maxlength
                    float k = 0;
                    for (int i = 0; i <  path_plane.size(); i++) { 
                        if (Vector2Length(path_plane[i]) > k) {
                            k = Vector2Length(path_plane[i]);
                        }
                    }
                    k = 100/k;

                    // scale the path
                    Vector2 path_plane_scaled[path_plane.size()];
                    for (int i = 0; i <  path_plane.size(); i++) { 
                        path_plane_scaled[i] = Vector2Add(Vector2Scale(Vector2Subtract(path_plane[i], fixed_point), k), (Vector2){100,100});
                    }
                    // this doesnt work
                    DrawSplineLinear(path_plane_scaled, path_plane.size(), 0.5, PATH_COLOUR);
                    DrawText(TextFormat("global_changed: %d", iterations), 50 ,50, 10, ANTIPODE_COLOUR);
                EndTextureMode();
            }
        
            // draw texture onto the screen
            DrawTextureRec(target.texture, (Rectangle){ 0, 0, target.texture.width, -target.texture.height }, (Vector2){ MINIMAP_MARGIN_LEFT, SCREEN_HEIGHT - MINIMAP_HEIGHT - MINIMAP_MARGIN_BOTTOM}, MINIMAP_COLOUR);


            DrawText(TextFormat("Current Global best, %f,%f,%f, with loss %f", global.x, global.y, global.z, loss(global, f)), 10, 10, 20, ANTIPODE_COLOUR);
            DrawFPS(10, 40);

        EndDrawing();
        iterations++;
    }

    
    UnloadModel(sphereModel); 
    CloseWindow();             
    return 0;
}
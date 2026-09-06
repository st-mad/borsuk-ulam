#include "raylib.h"
#include <raymath.h>
#include <functional>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <random>

Vector2 f(Vector3 coords) {
   // defines a function over S^2

   return (Vector2) {0.0f, 0.0f};
}

float loss(Vector3 coords, Vector2 (*func)(Vector3)) {
    return Vector2Length(Vector2Subtract((*func)(coords), (*func)(Vector3Scale(coords, -1.0))));

}

std::vector<std::vector<Vector3>> init_particle_swarm(int swarm_size, Vector2 (*func)(Vector3)) {
    // the simplex stuff was too hard and my brain is smooth. We will do particle swarm optimisation instead.
    std::random_device rd;
    std::mt19937 gen(rd()); 

    float min = 0.0f;
    float max = 1.0f;
    std::uniform_real_distribution<float> dis(min, max);


    // init swarm (swarm_size random unit vectors.)
    std::vector<Vector3> swarm(swarm_size); 
    std::vector<Vector3> best_known_pos(swarm_size);
    std::vector<Vector3> velocities(swarm_size);
    Vector3 global = {0,0,0};
    for (int i = 0; i < swarm_size; i++) {
        Vector3 rand_vec = {dis(gen) *2 - 1, dis(gen) *2 - 1, dis(gen) *2 - 1};
        rand_vec = Vector3Normalize(rand_vec);
        swarm.push_back(rand_vec);
        best_known_pos.push_back(rand_vec);
        if (loss(rand_vec,f) < loss(global,f)) {
            global = rand_vec;
        }
        
        Vector3 rand_velocity = {dis(gen)*2 - 1, dis(gen) *2 - 1, dis(gen) *2 - 1};
        velocities.push_back(rand_velocity);
    }

    return {swarm, best_known_pos, velocities};
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

int main(void) {
    //boilder plates
    const int screenWidth = 800;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "raylib [models] - sphere triangulation");

    // setting up the camera
    Camera3D camera = { 0 };
    camera.position = (Vector3){ 0.0f, 4.0f, 6.0f }; // Camera position in 3D space
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };   // Look at the center of the world
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };       // Camera up vector (Y-axis is up)
    camera.fovy = 45.0f;                             // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;          // Camera projection type


    // man I just want my camera to move around with WASD.
    float radius = 8.0f; // Distance from the sphere target
    float alpha = 0.0f;  // Horizontal angle (yaw)
    float beta = 0.5f;   // Vertical angle (pitch)
    float speed = 2.0f;  // Speed of rotation

    // this is the mesh (idk how to edit size on the fly and parameterise it.)
    Mesh sphereMesh = GenMeshSphere(2.0f, 16, 16);
    Model sphereModel = LoadModelFromMesh(sphereMesh);

    // AI SLOP
    // Set the rendering mode to wireframe if you want to visually see the triangles
    // By default, it will be a flat color.
    // To see the triangles: sphereModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = RED;

    Vector3 modelPosition = { 0.0f, 0.0f, 0.0f };

    // initialise the swarm
    std::vector<std::vector<Vector3>> swarm = init_particle_swarm(100, f);

    SetTargetFPS(60); 

    bool swarm_running = true;
    // main loop
    while (!WindowShouldClose()) {
        //update
        float dt = GetFrameTime();

        //mvoing things around
        if (IsKeyDown(KEY_A)) alpha -= speed * dt; // Orbit left
        if (IsKeyDown(KEY_D)) alpha += speed * dt; // Orbit right
        if (IsKeyDown(KEY_W)) beta += speed * dt;  // Orbit up
        if (IsKeyDown(KEY_S)) beta -= speed * dt;  // Orbit down

        // Clamp the vertical angle so the camera doesn't flip upside down at the poles
        if (beta >  1.5f) beta =  1.5f;
        if (beta < -1.5f) beta = -1.5f;

        // 3. Mathematical conversion from Angles -> 3D position vector
        camera.position.x = camera.target.x + radius * cosf(beta) * sinf(alpha);
        camera.position.y = camera.target.y + radius * sinf(beta);
        camera.position.z = camera.target.z + radius * cosf(beta) * cosf(alpha);
        
        //compute the swarm
        if (swarm_running) {

        }


        // Draw
        BeginDrawing();
            ClearBackground(RAYWHITE);

            // Enter 3D Mode using our camera setup
            BeginMode3D(camera);

                // Option A: Render a solid shaded sphere model
                DrawModel(sphereModel, modelPosition, 1.0f, LIGHTGRAY);

                // Option B: Render the wireframe triangles on top so you can see the triangulation layout
                DrawModelWires(sphereModel, modelPosition, 1.0f, DARKGRAY);

                // Draw a reference grid underneath the sphere
                DrawGrid(10, 1.0f);

            EndMode3D();

            DrawText("Triangulated Sphere Demo", 10, 10, 20, DARKGRAY);
            DrawFPS(10, 40);

        EndDrawing();
    }

    // 5. De-Initialization and Cleanup
    UnloadModel(sphereModel); // This automatically unloads the inner mesh data from GPU VRAM
    CloseWindow();            // Close window and OpenGL context

    return 0;
}
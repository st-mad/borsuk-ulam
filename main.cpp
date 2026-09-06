#include "raylib.h"
#include <math.h>

Vector2 f(Vector2 coords) {
   // defines a function over S^2

   return (Vector2) {0.0f, 0.0f}
}



int main(void) {
    // 1. Initialize Window and Context
    const int screenWidth = 800;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "raylib [models] - sphere triangulation");

    // 2. Define the 3D Camera
    Camera3D camera = { 0 };
    camera.position = (Vector3){ 0.0f, 4.0f, 6.0f }; // Camera position in 3D space
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };   // Look at the center of the world
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };       // Camera up vector (Y-axis is up)
    camera.fovy = 45.0f;                             // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;          // Camera projection type


    // Camera orbital parameters
    float radius = 8.0f; // Distance from the sphere target
    float alpha = 0.0f;  // Horizontal angle (yaw)
    float beta = 0.5f;   // Vertical angle (pitch)
    float speed = 2.0f;  // Speed of rotation

    // 3. Generate the Sphere Mesh and Load it into a Model
    // Parameters: radius = 2.0f, rings = 16, slices = 16 (low values make triangulation obvious)
    Mesh sphereMesh = GenMeshSphere(2.0f, 16, 16);
    Model sphereModel = LoadModelFromMesh(sphereMesh);

    // Set the rendering mode to wireframe if you want to visually see the triangles
    // By default, it will be a flat color.
    // To see the triangles: sphereModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = RED;

    Vector3 modelPosition = { 0.0f, 0.0f, 0.0f };

    SetTargetFPS(60); 

    // 4. Main Game Loop
    while (!WindowShouldClose()) {
        // Update
        // moving camera around with WASD

        float dt = GetFrameTime();

        // 2. Handle WASD Inputs to adjust angles instead of position vectors
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
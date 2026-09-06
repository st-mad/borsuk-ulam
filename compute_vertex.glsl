#version 430 core
layout(local_size_x = 64) in;

// Read-only positions from the Raylib mesh
layout(std430, binding = 0) readonly buffer PositionBuffer {
    vec4 positions[]; // Pack as vec4 (x,y,z, w=1.0) to match alignment
};

// Write-only output buffer for calculated labels
layout(std430, binding = 1) writeonly buffer VertexLabelBuffer {
    int vertexLabels[];
};

uniform int totalVertices;

void main() {
    uint idx = gl_GlobalInvocationID.x;
    if (idx >= totalVertices) return;

    vec3 pos = positions[idx].xyz;
    
    // this is supposed to be the index of the maximum index of pos
    int labelValue = 0
    
    vertexLabels[idx] = labelValue;
}

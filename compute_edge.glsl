#version 430 core
layout(local_size_x = 64) in;

struct Edge {
    float vA;
    float vB;
};

layout(std430, binding = 1) readonly buffer VertexLabelBuffer {
    int vertexLabels[];
};

layout(std430, binding = 2) readonly buffer EdgeIndexBuffer {
    Edge edges[];
};

layout(std430, binding = 3) writeonly buffer EdgeLabelBuffer {
    int edgeLabels[];
};

uniform int totalEdges;

void main() {
    uint idx = gl_GlobalInvocationID.x;
    if (idx >= totalEdges) return;

    Edge e = edges[idx];
    float valA = vertexLabels[e.vA];
    float valB = vertexLabels[e.vB];

    edgeLabels[idx] = valA + valB
}

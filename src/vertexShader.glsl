#version 330 core            // Minimal GL version support expected from the GPU

layout(location=0) in vec3 vPosition;
layout(location=1) in vec3 vColor;
layout(location=2) in vec3 vNormal; // Now, the 2nd input attribute is the vertex normal.
out vec3 fNormal;
uniform mat4 viewMat, projMat;
out vec3 fColor;

void main() {
        gl_Position = projMat * viewMat * vec4(vPosition, 1.0); // mandatory to rasterize properly
        fColor = vColor; // will be passed to the next stage
        fNormal = vNormal;

}

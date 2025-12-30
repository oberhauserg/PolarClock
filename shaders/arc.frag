#version 300 es
precision highp float;

uniform vec3 u_colorBase;

out vec4 fragColor;

void main() {
    fragColor = vec4(u_colorBase, 1.0);
}

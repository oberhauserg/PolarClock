#version 300 es
precision highp float;

in vec2 v_texCoord;

uniform sampler2D u_fontTexture;
uniform vec3 u_textColor;
uniform float u_alpha;

out vec4 fragColor;

void main() {
    float alpha = texture(u_fontTexture, v_texCoord).r;
    fragColor = vec4(u_textColor, alpha * u_alpha);
}

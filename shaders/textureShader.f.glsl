#version 410 core

// uniform inputs
// TODO #E - add uniform
uniform sampler2D texMap;

// varying inputs
// TODO #D - add varying
in vec2 texCoord;


// fragment outputs
out vec4 fragColorOut;

void main() {
    // TODO #F - get texel
    vec4 texel = texture(texMap, texCoord);


    // TODO #G - set texel
    fragColorOut = texel;
}
#version 410 core

// uniform inputs
// TODO #E - add uniform
uniform sampler2D texMap;

// varying inputs
// TODO #D - add varying
in vec2 texCoord;


// fragment outputs
out vec4 fragColorOut;

vec3 rgb2hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main() {
    // TODO #F - get texel
    vec4 texel = texture(texMap, texCoord);

    vec3 shadedColor = vec3(texel.x,texel.y,texel.z);


    vec3 shadedHSV = rgb2hsv(shadedColor);

    vec3 toonHSV = shadedHSV;
    float toonV = toonHSV.z;

    int accuracy = 8;

    toonV = pow(toonV, 0.5);
    toonV *= accuracy;
    toonV = floor(toonV);
    toonV = toonV/accuracy;
    toonV = pow(toonV, 2);

    toonHSV.z = toonV;

    vec3 toonColor = hsv2rgb(toonHSV);

    fragColorOut = vec4(toonColor ,1.0);
}
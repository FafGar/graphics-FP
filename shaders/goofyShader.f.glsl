#version 410 core

// uniform inputs
uniform vec3 lightColor;                // light color
uniform vec3 materialAmbColor;          // the material ambient color
uniform vec3 materialDiffColor;
uniform vec3 materialSpecColor;     
uniform float materialShininess; 

layout(location = 0) in vec3 color;     // interpolated color for this fragment
layout(location = 1) in vec3 normalVec; 
layout(location = 2) in vec3 badDirLightVec; 
layout(location = 3) in vec3 badSpotLightVec; 
layout(location = 4) in vec3 badDirHalfwayVec; 
layout(location = 5) in vec3 badSpotHalfwayVec; 

// outputs
out vec4 fragColorOut;// color to apply to this fragment

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
    // if we are looking at the front face of the fragment
    if(gl_FrontFacing) {
        vec3 newNormal = normalize(normalVec);

        vec3 dirLightVec = normalize(badDirLightVec);
        vec3 spotLightVec = normalize(badSpotLightVec);
        vec3 dirHalfwayVec = normalize(badDirHalfwayVec);
        vec3 spotHalfwayVec = normalize(badSpotHalfwayVec);
        float specAngle = max(dot(dirHalfwayVec, newNormal), 0.0);
        float specular = pow(specAngle, materialShininess);

        vec3 diffColor = lightColor  * max( dot(newNormal, dirLightVec), 0.0 );

        if(1==1){ //Use this for spot light stuff later
            specAngle = max(dot(spotHalfwayVec, newNormal), 0.0);
            specular += pow(specAngle, materialShininess);

            diffColor += lightColor  * max( dot(newNormal, spotLightVec), 0.0 );
        }

        vec3 shadedColor = materialAmbColor+materialDiffColor * diffColor+materialSpecColor * specular;

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
}
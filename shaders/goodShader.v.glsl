#version 410 core

// uniform inputs
uniform mat4 mvpMatrix;                 // the precomputed Model-View-Projection Matrix
uniform mat4 modelMatrix;               // just the model matrix
uniform mat3 normalMtx;                 // normal matrix
uniform vec3 eyePos;                    // eye position in world space
uniform vec3 spotLightPos;                  // spot light position in world space
uniform vec3 spotLightDir;             // direction of the spot light
uniform vec3 dirLightDir;             // direction of the directional light
uniform float lightCutoff;              // angle of our spotlight
uniform vec3 lightColor;                // light color
uniform vec3 materialDiffColor;         // the material diffuse color
uniform vec3 materialSpecColor;         // the material specular color
uniform float materialShininess;        // the material shininess value
uniform vec3 materialAmbColor;          // the material ambient color
// uniform uint lightType;                 // 0 - point light, 1 - directional light, 2 - spotlight

// attribute inputs
layout(location = 0) in vec3 vPos;      // the position of this specific vertex in object space
layout(location = 1) in vec3 vNormal;   // the normal of this specific vertex in object space

// varying outputs
layout(location = 0) out vec3 color;    // color to apply to this vertex
layout(location = 1) out vec3 normalVec; 
layout(location = 2) out vec3 dirLightVec; 
layout(location = 3) out vec3 spotLightVec; 
layout(location = 4) out vec3 dirHalfwayVec; 
layout(location = 5) out vec3 spotHalfwayVec; 

//global variable because I'm lazy
float redOffset;


void main() {
    // transform & output the vertex in clip space
    gl_Position = mvpMatrix * vec4(vPos,1);

    // transform vertex information into world space
    vec3 vPosWorld = (modelMatrix * vec4(vPos, 1.0)).xyz;
    vec3 nVecWorld = normalMtx * vNormal;
    
    color = materialSpecColor;

    normalVec = nVecWorld;
    dirLightVec = dirLightDir ;
    spotLightVec = (spotLightPos - vPosWorld);
    dirHalfwayVec = (dirLightVec + eyePos);
    spotHalfwayVec = (spotLightVec + eyePos);
}
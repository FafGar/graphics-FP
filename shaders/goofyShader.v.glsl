#version 410 core

// uniform inputs
uniform vec3 hitVec; 
uniform float timeSince; 
uniform mat4 mvpMatrix;                 // the precomputed Model-View-Projection Matrix
uniform mat4 modelMatrix;               // just the model matrix
uniform mat3 normalMtx;                 // normal matrix
uniform vec3 eyePos;                    // eye position in world space
uniform vec3 lightPos;                  // light position in world space
uniform vec3 lightDir;                  // light direction in world space
uniform float lightCutoff;              // angle of our spotlight
uniform vec3 lightColor;                // light color
uniform vec3 materialDiffColor;         // the material diffuse color
uniform vec3 materialSpecColor;         // the material specular color
uniform float materialShininess;        // the material shininess value
uniform vec3 materialAmbColor;          // the material ambient color
uniform uint lightType;                 // 0 - point light, 1 - directional light, 2 - spotlight

// attribute inputs
layout(location = 0) in vec3 vPos;      // the position of this specific vertex in object space
layout(location = 1) in vec3 vNormal;   // the normal of this specific vertex in object space

// varying outputs
layout(location = 0) out vec4 color;    // color to apply to this vertex

//global variable because I'm lazy
float redOffset;

// compute the diffuse color using lambertian diffuse reflectance
vec3 diffuseColor(vec3 vertexPosition, vec3 vertexNormal) {
    vec3 lightVector;

    // directional light
    if(lightType == 1) {
        lightVector = normalize( -lightDir );
    }
    // spotlight or point light
    else {
        lightVector = normalize(lightPos - vertexPosition);
    }

    vec3 diffColor = lightColor * materialDiffColor * max( dot(vertexNormal, lightVector), 0.0 );

    // spotlight - check if within cone
    if(lightType == 2) {
        float theta = dot(normalize(lightDir), -lightVector);
        if( theta <= lightCutoff ) {
            diffColor = vec3(0.0, 0.0, 0.0);
        }
    }

    return diffColor;
}

// compute the specular color using Blinn-Phong specular reflectance
vec3 specularColor(vec3 vertexPosition, vec3 vertexNormal) {
    vec3 lightVector;

    // directional light
    if(lightType == 1) {
        lightVector = normalize( -lightDir );
    }
    // spotlight or point light
    else {
        lightVector = normalize(lightPos - vertexPosition);
    }

    vec3 viewVector = normalize(eyePos - vertexPosition);
    vec3 halfwayVector = normalize(viewVector + lightVector);

    vec3 specColor = lightColor * materialSpecColor * pow(max( dot(vertexNormal, halfwayVector), 0.0 ), 4.0*materialShininess);

    // spotlight - check if within cone
    if(lightType == 2) {
        float theta = dot(normalize(lightDir), -lightVector);
        if( theta <= lightCutoff ) {
            specColor = vec3(0.0, 0.0, 0.0);
        }
    }

    return specColor;
}

vec4 deform(){
    if(pow(0.8,timeSince) > 0.1){
        float dotProd = dot(hitVec, vNormal);
        if(dotProd > 0){
            redOffset = 0.2*pow(dotProd,5);
            vec3 offsetVec = pow(0.8,timeSince)*sin(timeSince)*hitVec*pow(dotProd,5);
            return vec4(vPos - offsetVec,1.0);
        }
    }
    redOffset = 0;
    return vec4(vPos,1.0);
}

void main() {
    // transform & output the vertex in clip space
    gl_Position = mvpMatrix * deform();

    // transform vertex information into world space
    vec3 vPosWorld = (modelMatrix * vec4(vPos, 1.0)).xyz;
    vec3 nVecWorld = normalize( normalMtx * vNormal );

    // compute each component of the Phong Illumination Model
    vec3 diffColor = diffuseColor(vPosWorld, nVecWorld);
    vec3 specColor = specularColor(vPosWorld, nVecWorld);
    vec3 ambColor = lightColor * materialAmbColor;

    // assign the final color for this vertex
    diffColor.r+=redOffset;
    color = vec4(diffColor + specColor + ambColor, 1.0f);
}
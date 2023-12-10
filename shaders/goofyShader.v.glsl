#version 410 core

// uniform inputs
uniform vec3 hitVec; 
uniform float timeSince; 
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
layout(location = 2) in vec2 inTexCoord;

// varying outputs
layout(location = 0) out vec3 color;    // color to apply to this vertex
layout(location = 1) out vec3 normalVec; 
layout(location = 2) out vec3 dirLightVec; 
layout(location = 3) out vec3 spotLightVec; 
layout(location = 4) out vec3 dirHalfwayVec; 
layout(location = 5) out vec3 spotHalfwayVec;
layout(location = 6) out vec2 texCoord;

//global variable because I'm lazy
float redOffset;

// compute the diffuse color using lambertian diffuse reflectance
vec3 diffuseColor(vec3 vertexPosition, vec3 vertexNormal) {

    // directional light
    vec3 dirLightVector = normalize( -dirLightDir );
    // spotlight light
    vec3 spotLightVector = normalize(vertexPosition- spotLightPos);

    vec3 diffColor = lightColor  * max( dot(vertexNormal, dirLightVector), 0.0 );

    // spotlight - check if within cone
    float theta = dot(normalize(spotLightDir), - spotLightVector);
    if( theta <= lightCutoff ) {
        diffColor += vec3(0.0, 0.0, 0.0);
    } else{
        diffColor += lightColor * max( dot(vertexNormal, spotLightVector), 0.0 );
    }

    return diffColor* materialDiffColor;
}

// compute the specular color using Blinn-Phong specular reflectance
vec3 specularColor(vec3 vertexPosition, vec3 vertexNormal) {
    // directional light
    vec3 dirLightVector = normalize( -dirLightDir );
    // spotlight light
    vec3 spotLightVector = normalize(spotLightPos - vertexPosition);

    vec3 viewVector = normalize(eyePos - vertexPosition);
    vec3 dirHalfwayVector = reflect(spotLightVector, vertexNormal);

    vec3 specColor = lightColor  * pow(max( dot(viewVector, dirHalfwayVector), 0.0 ), 2*materialShininess);

    // spotlight - check if within cone
    float theta = dot(normalize(spotLightDir), - spotLightVector);
    if( theta <= lightCutoff ) {
        specColor += vec3(0.0, 0.0, 0.0);
    }else{
        vec3 spotHalfwayVector = reflect(spotLightVector, vertexNormal);
        specColor += lightColor * pow(max( dot(viewVector, spotHalfwayVector), 0.0 ), 2*materialShininess);
    }

    return specColor* materialSpecColor;
}

vec4 deform(){
    if(pow(0.8,timeSince) > 0.1){
        float dotProd = dot(hitVec, vNormal);
        if(dotProd > 0){
            redOffset = 0.2*pow(dotProd,5);
            vec3 offsetVec = pow(0.8,timeSince)*sin(timeSince*3)*hitVec*pow(dotProd,5);
            return vec4(vPos - offsetVec,1.0);
        }
    }
    redOffset = 0;
    return vec4(vPos,1.0);
}

void main() {
    texCoord = inTexCoord;
    // transform & output the vertex in clip space
    gl_Position = mvpMatrix * deform();

    // transform vertex information into world space
    vec3 vPosWorld = (modelMatrix * vec4(vPos, 1.0)).xyz;
    vec3 nVecWorld = normalMtx * vNormal;

    // // compute each component of the Phong Illumination Model
    // vec3 diffColor = diffuseColor(vPosWorld, nVecWorld);
    // vec3 specColor = specularColor(vPosWorld, nVecWorld);
    // vec3 ambColor = lightColor * materialAmbColor;

    // assign the final color for this vertex
    // diffColor.r+=redOffset;
    color = materialSpecColor;

    normalVec = nVecWorld;
    dirLightVec = dirLightDir;
    spotLightVec = (spotLightPos - vPosWorld);
    dirHalfwayVec =(dirLightVec + eyePos);
    spotHalfwayVec = (spotLightVec + eyePos);
}
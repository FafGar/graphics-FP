#version 410 core

// uniform inputs
uniform vec3 lightColor;                // light color
uniform vec3 materialAmbColor;          // the material ambient color
uniform vec3 materialDiffColor;
uniform vec3 materialSpecColor;     
uniform float materialShininess; 

// varying inputs
layout(location = 0) in vec3 color;     // interpolated color for this fragment
layout(location = 1) in vec3 normalVec; 
layout(location = 2) in vec3 dirLightVec; 
layout(location = 3) in vec3 spotLightVec; 
layout(location = 4) in vec3 dirHalfwayVec; 
layout(location = 5) in vec3 spotHalfwayVec; 

// outputs
out vec4 fragColorOut;                  // color to apply to this fragment

void main() {
    // if we are looking at the front face of the fragment
    if(gl_FrontFacing) {
        vec3 newNormal = normalize(normalVec);

        float specAngle = max(dot(dirHalfwayVec, newNormal), 0.0);
        float specular = pow(specAngle, materialShininess);

        vec3 diffColor = lightColor  * max( dot(newNormal, dirLightVec), 0.0 );

        if(1==1){ //Use this for spot light stuff later
            specAngle = max(dot(spotHalfwayVec, newNormal), 0.0);
            specular += pow(specAngle, materialShininess);

            diffColor += lightColor  * max( dot(newNormal, spotLightVec), 0.0 );
        }
        
        fragColorOut = vec4(materialAmbColor+
                            materialDiffColor * diffColor+
                            materialSpecColor * specular ,1.0
                            );
    }    else {
        fragColorOut = vec4(0);
    }
}
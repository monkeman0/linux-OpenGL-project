#version 330 core

out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform mat4 view;
uniform int post;

void main()
{ 
    vec4 baseColor = texture(screenTexture, TexCoords);
    switch(post){
    case 0: FragColor = baseColor;
        break;
    case 1: {
        float kernelSize = 15.0f;
        int halfSize = int(kernelSize) / 2;
        float coefficient = 1.0f / kernelSize;
        vec2 dx = vec2(0.004f, 0.0f);

        FragColor = vec4(0.0f);
        for(int x = -halfSize; x <= halfSize; x++){
            vec4 color = texture(screenTexture, TexCoords + x * dx);
            color = vec4(1.15f - color.x, 1.05f - color.y, 1.0f - color.z, (color.a * 1.4f));
            FragColor += coefficient * color;
        }
    } break;
    case 2: {
        float kernelSize = 15.0f;
        int halfSize = int(kernelSize) / 2;
        float coefficient = 1.0f / kernelSize;
        vec2 dx = vec2(0.004f, 0.0f);
        if(baseColor.a == 0.0f) discard;

        FragColor = baseColor;
        for(int x = -halfSize; x <= halfSize; x++){
            vec4 color = texture(screenTexture, TexCoords + x * dx);
            if(color.a == 0.0f) FragColor = vec4(FragColor.x - coefficient * 0.1f, FragColor.y - coefficient * 0.2f, FragColor.z - coefficient * 0.2f, FragColor.a);
        }
    } break;
    default: break;
    }
}
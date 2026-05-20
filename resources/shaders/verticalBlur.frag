#version 460 core

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
        vec2 dy = vec2(0.0f, 0.004f);

        FragColor = vec4(0.0f);
        for(int y = -halfSize; y <= halfSize; y++){
            vec4 color = texture(screenTexture, TexCoords + y * dy);
            color = vec4(1.15f - color.x, 1.05f - color.y, 1.0f - color.z, (color.a * 1.4f));
            FragColor += coefficient * color;
        }
    } break;
    case 2: {
        float kernelSize = 15.0f;
        int halfSize = int(kernelSize) / 2;
        float coefficient = 1.0f / kernelSize;
        vec2 dy = vec2(0.0f, 0.004f);
        if(baseColor.a == 0.0f) discard;

        FragColor = baseColor;
        for(int y = -halfSize; y <= halfSize; y++){
            vec4 color = texture(screenTexture, TexCoords + y * dy);
            if(color.a == 0.0f) FragColor = vec4(FragColor.x - coefficient * 0.1f, FragColor.y - coefficient * 0.2f, FragColor.z - coefficient * 0.2f, FragColor.a);
        }
    } break;
    default: break;
    }
}
#version 460 core
out vec4 FragColor;


uniform sampler2D aTexture;
uniform int skipped;

void main()
{

    switch(skipped){
        case 0: FragColor = vec4(vec3(1.0f, 0.0f, 0.0f), 0.9f); 
            break;
        case 1: FragColor = vec4(vec3(0.0f, 0.8f, 1.0f), 0.9f); 
            break;
        case 2: FragColor = vec4(vec3(0.3f, 0.3f, 0.3f), 0.9f); 
            break;
        default:
            break;
    }
}
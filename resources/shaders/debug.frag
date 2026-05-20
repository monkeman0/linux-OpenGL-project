#version 460 core
out vec4 FragColor;


uniform sampler2D aTexture;
uniform bool skipped;

void main()
{

    if(skipped){
        FragColor = vec4(vec3(0.0f, 0.0f, 1.0f), 0.5f);
    }else{
        FragColor = vec4(vec3(1.0f, 0.0f, 0.0f), 0.9f);
    }
}
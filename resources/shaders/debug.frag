#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D aTexture;
uniform bool skipped;

void main()
{
    if(texture(aTexture, TexCoord).a < 0.02f)   discard;

    if(skipped){
        FragColor = vec4(vec3(0.0f, 0.0f, 1.0f), texture(aTexture, TexCoord).a);
    }else{
        FragColor = vec4(vec3(texture(aTexture, TexCoord)), texture(aTexture, TexCoord).a);
    }
}
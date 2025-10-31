#version 330 core

out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform mat4 view;
uniform bool rays;

void main()
{ 
    vec4 colors = texture(screenTexture, TexCoords);
    if(rays){
        if(colors.w == 0.0f){
            colors = vec4(1.0f, 1.0f, 1.0f, 1.0f);
        }else{
            colors = vec4(0.0f, 0.0f, 0.0f, 1.0f);
        }
        vec2 lightPos = vec2(0.5, 0.5); // Center of screen, or make this a uniform
        vec2 dir = lightPos - TexCoords;
        float samples = 16;
        float decay = 0.96f;
        float exposure = 0.065f;
        float weight = 0.54f;
        float illuminationDecay = 1.224f;
        vec4 sum = vec4(0.0);

        vec2 step = dir / samples;
        vec2 coord = TexCoords;

        for (float i = 0.0; i < samples; i++) {
            coord += step;
            vec4 sampleColor = texture(screenTexture, coord);
            if (sampleColor.w == 0.0f) {
                sampleColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
            } else {
                sampleColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
            }
            sampleColor *= illuminationDecay * weight;
            sum += sampleColor;
            illuminationDecay *= decay;
        }

        vec4 finalColor = sum * exposure + colors * 0.5;
        finalColor.w *= 0.4f;
        finalColor.w += 0.1f;
        FragColor = finalColor;
    }else{
        FragColor = colors;
    }
}
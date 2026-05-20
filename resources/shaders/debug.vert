#version 460 core

layout (location = 0) in float aData;

out vec2 TexCoord;

uniform mat4 pv;
uniform mat4 model;
uniform bool skipped;

void main()
{
    int corner = int(aData);
    vec3 pos = vec3(0.0f, 0.0f, 0.0f);

    switch(corner){
	case 0: pos = vec3(0.0f, 0.0f, 40.0f);
	break;
	case 1: pos = vec3(40.0f, 0.0f, 40.0f);
	break;
	case 2: pos = vec3(40.0f, 0.0f, 0.0f);
	break;
	case 3: pos = vec3(0.0f, 0.0f, 0.0f);
	break;
	case 4: pos = vec3(0.0f, 40.0f, 40.0f);
	break;
	case 5: pos = vec3(40.0f, 40.0f, 40.0f);
	break;
	case 6: pos = vec3(40.0f, 40.0f, 0.0f);
	break;
	case 7: pos = vec3(0.0f, 40.0f, 0.0f);
	break;
	default: break;
	}

	if(skipped) pos*=0.9f;

    gl_Position = pv * model * vec4(pos, 1.0);
}
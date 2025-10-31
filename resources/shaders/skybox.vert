#version 330 core

layout (location = 0) in float aCorner;

out vec3 texCoords;

uniform mat4 pv;

void main(){
	vec3 pos = vec3(0.0f, 0.0f, 0.0f);
	int corner = int(aCorner);

	switch(corner){
	case 0: pos = vec3(-1.0f, -1.0f, 1.0f);
	break;
	case 1: pos = vec3(1.0f, -1.0f, 1.0f);
	break;
	case 2: pos = vec3(1.0f, -1.0f, -1.0f);
	break;
	case 3: pos = vec3(-1.0f, -1.0f, -1.0f);
	break;
	case 4: pos = vec3(-1.0f, 1.0f, 1.0f);
	break;
	case 5: pos = vec3(1.0f, 1.0f, 1.0f);
	break;
	case 6: pos = vec3(1.0f, 1.0f, -1.0f);
	break;
	case 7: pos = vec3(-1.0f, 1.0f, -1.0f);
	break;
	default: break;
	}

	vec4 Pos = pv * vec4(pos, 1.0f);
	gl_Position = vec4(Pos.x, Pos.y, Pos.w, Pos.w);
	texCoords = vec3(pos.x, pos.y, -pos.z);
}
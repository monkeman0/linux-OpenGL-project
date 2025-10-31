#version 330 core

layout (location = 0) in uint aAppearance;

out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;

vec3 unpack2(uint data) {
	float texX = ((((data & 63u) * 32.0f) + (data >> 6 & 31u)) / 1024.0f);
	float texY = ((((data >> 11 & 63u) * 32.0f) + (data >> 17 & 31u)) / 1024.0f);
	float material = data >> 22 & 1023u;
	return vec3(texX, texY, material);
}

uniform mat4 pv;
uniform mat4 model;
uniform float utime;

const float pi = 3.141592;

void main()
{
	float time = utime;
    vec3 appearance = unpack2(aAppearance);
	vec3 normal = vec3(0.0, 1.0, 0.0);
	TexCoord = vec2(appearance.x, appearance.y);

	float verticesPerLeaf = 0.0f;
	float pitch = 0.0f;
	float yaw = 0.0f;
	float bendStrength = 0.0f;
	int prevID = 0;
	float widthFactor = 1.0f;
	float scatter = 0.0f;
	float height = 0.0f;
	float freq = 0.0f;
	float shift = 0.0f;
	float tall = 2.0f;
	if(appearance.z == 0){
		verticesPerLeaf = 12.0f;
		bendStrength = 0.12f;
		pitch = 2.0f;
		yaw = 0.0f;
		prevID = 0;
	}else if(appearance.z == 1){
		verticesPerLeaf = 10.0f;
		bendStrength = 0.45f;
		pitch = 0.75f;
		yaw = 0.5f;
		prevID = 72;
	}else if(appearance.z == 2){
		verticesPerLeaf = 8.0f;
		bendStrength = 2.0f;
		pitch = pi / 2.0f;
		yaw = 0.0f;
		prevID = 132;
		widthFactor = 0.35f;
		scatter = 0.1f;
		height = 3.200f;
		freq = 3.5f;
		shift = -0.120f;
		tall = 3.5f;
	}

	float leafID = floor(float(gl_VertexID - prevID) / verticesPerLeaf);
	float ID = gl_VertexID - 1 - prevID;
	if(ID - leafID * verticesPerLeaf == -1) ID++;
	if(ID - leafID * verticesPerLeaf == verticesPerLeaf - 2) ID--;
	ID -= leafID * verticesPerLeaf;

	float yawOffset = 0.0f;
	yawOffset = yaw - leafID * 0.1f + (leafID * 0.333333f) * pi;
	if(appearance.z == 2) yawOffset = yaw - leafID * 0.1f * pi;


	vec3 position = vec3(0.0);
	float width = mod(ID, 2) - 0.5;
	float distance = floor(ID / 2) / tall;

	time -= distance * 0.3;
	time -= leafID;
	float wind = sin(time - leafID) * 0.15f - sin((time - leafID) * 1.4) * 0.15f - sin((time - leafID) * 2) * 0.15f - sin((time - leafID) * 3.7) * 0.15f;

	float bendPitch = pitch + distance * bendStrength * wind;

	position.x = ((cos(yawOffset) * -width + cos(bendPitch) * distance * sin(yawOffset)) * widthFactor) + (height * sin((leafID + shift) * freq) + sin(leafID)) * scatter;
	position.y = sin(bendPitch) * distance;
	position.z = ((sin(yawOffset) * width + cos(bendPitch) * distance * cos(yawOffset)) * widthFactor) + (height * cos((leafID + shift) * freq) + sin(leafID)) * scatter;
	
	Normal = mat3(transpose(inverse(model))) * normal * (sin(bendPitch) * distance);

	FragPos = vec3(model * vec4(position, 1.0));
	gl_Position = pv * vec4(FragPos, 1.0);
}
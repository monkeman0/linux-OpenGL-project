#version 330 core

layout(location = 0) in uint aData;

out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;

uniform float LODstep;

vec3 unpackSingle(uint data, int returnType) {
vec3 toReturn = vec3(0.0f, 0.0f, 0.0f);
if (returnType == 0) {
	uint index = data >> 16 & 65535u;
	uint corner = data & 7u;
	float y = float(index / (40u * 40u)) * (0.25f) - 5.0f + (0.125f * LODstep);
	float z = float((index / 40u) % 40u) * (0.25f) - 5.0f + (0.125f * LODstep);
	float x = float(index % 40u) * (0.25f) - 5.0f + (0.125f * LODstep);
	switch (corner) {
	case 0u:{
		x -= 0.125f * LODstep;
		y -= 0.125f * LODstep;
		z -= 0.125f * LODstep;
	}break;
	case 1u: {
		x += 0.125f * LODstep;
		y -= 0.125f * LODstep;
		z -= 0.125f * LODstep;
	}break;
	case 2u: {
		x += 0.125f * LODstep;
		y -= 0.125f * LODstep;
		z += 0.125f * LODstep;
	}break;
	case 3u: {
		x -= 0.125f * LODstep;
		y -= 0.125f * LODstep;
		z += 0.125f * LODstep;
	}break;
	case 4u: {
		x -= 0.125f * LODstep;
		y += 0.125f * LODstep;
		z -= 0.125f * LODstep;
	}break;
	case 5u: {
		x += 0.125f * LODstep;
		y += 0.125f * LODstep;
		z -= 0.125f * LODstep;
	}break;
	case 6u: {
		x += 0.125f * LODstep;
		y += 0.125f * LODstep;
		z += 0.125f * LODstep;
	}break;
	case 7u: {
		x -= 0.125f * LODstep;
		y += 0.125f * LODstep;
		z += 0.125f * LODstep;
	}break;
	default: break;
	}
	toReturn = vec3(x, y, z);
}
if (returnType == 1) {
	uint corner = data & 7u;
	uint normal = data >> 3 & 7u;
	uint texture = data >> 6 & 1023u;

	// Calculate base texture coordinates (tile in atlas)
	float tileSize = 32.0f / 1024.0f;
	float texX = float(texture % 32u) * tileSize;
	float texY = float(texture / 32u) * tileSize;

	// Local face UVs for each corner (0-3: bottom, 4-7: top)
	vec2 faceUVs[8];
	faceUVs[0] = vec2(0.02f, 0.02f);
	faceUVs[1] = vec2(0.98f, 0.02f);
	faceUVs[2] = vec2(0.98f, 0.98f);
	faceUVs[3] = vec2(0.02f, 0.98f);
	faceUVs[4] = vec2(0.02f, 0.02f);
	faceUVs[5] = vec2(0.98f, 0.02f);
	faceUVs[6] = vec2(0.98f, 0.98f);
	faceUVs[7] = vec2(0.02f, 0.98f);

	// Map faceUVs to the correct orientation based on normal
	vec2 uv = vec2(0.0f, 0.0f);
	if (normal == 0u || normal == 1u) { // Y axis (top/bottom)
		uv = faceUVs[corner % 4u];
		if (normal == 1u) uv = faceUVs[4u + (corner % 4u)];
	}
	if (normal == 2u) {
		if (corner == 7u) uv = faceUVs[3];
		if (corner == 4u) uv = faceUVs[2];
		if (corner == 0u) uv = faceUVs[1];
		if (corner == 3u) uv = faceUVs[0];
	}
	if (normal == 3u) {
		if (corner == 5u) uv = faceUVs[3];
		if (corner == 6u) uv = faceUVs[2];
		if (corner == 2u) uv = faceUVs[1];
		if (corner == 1u) uv = faceUVs[0];
	}
	if (normal == 4u) {
		if (corner == 4u) uv = faceUVs[3];
		if (corner == 5u) uv = faceUVs[2];
		if (corner == 1u) uv = faceUVs[1];
		if (corner == 0u) uv = faceUVs[0];
	}
	if (normal == 5u) {
		if (corner == 6u) uv = faceUVs[3];
		if (corner == 7u) uv = faceUVs[2];
		if (corner == 3u) uv = faceUVs[1];
		if (corner == 2u) uv = faceUVs[0];
	}

	// Scale and offset into the atlas
	texX += uv.x * tileSize;
	texY += uv.y * tileSize;

	toReturn = vec3(texX, texY, 0.0f);
}
if (returnType == 2) {
	uint normal = data >> 3 & 7u;
	switch (normal) {
	case 0u: toReturn = vec3(0.0f, -1.0f, 0.0f);
		break;
	case 1u: toReturn = vec3(0.0f, 1.0f, 0.0f);
		break;
	case 2u: toReturn = vec3(-1.0f, 0.0f, 0.0f);
		break;
	case 3u: toReturn = vec3(1.0f, 0.0f, 0.0f);
		break;
	case 4u: toReturn = vec3(0.0f, 0.0f, -1.0f);
		break;
	case 5u: toReturn = vec3(0.0f, 0.0f, 1.0f);
		break;
	default: break;
	}
}
return toReturn;
}



vec3 unpack1(uint data) {
	float x = ((data & 511u) * 0.01f);
	if ((data & 134217728u) == 134217728u) x *= -1.0f;
	float y = ((data >> 9 & 511u) * 0.01f);
	if ((data & 268435456u) == 268435456u) y *= -1.0f;
	float z = ((data >> 18 & 511u) * 0.01f);
	if ((data & 536870912u) == 536870912u) z *= -1.0f;
	vec3 toReturn = vec3(x, y, z);
	return toReturn;
}

vec3 unpack2(uint data) {
	float texX = ((((data & 63u) * 32.0f) + (data >> 6 & 31u)) / 1024.0f);
	float texY = ((((data >> 11 & 63u) * 32.0f) + (data >> 17 & 31u)) / 1024.0f);
	float material = data >> 22 & 1023u;
	return vec3(texX, texY, material);
}

vec3 unpack3(uint data) {
	float x = ((data & 511u) / 511.0f);
	if ((data & 134217728u) == 134217728u) x *= -1.f;
	float y = ((data >> 9 & 511u) / 511.0f);
	if ((data & 268435456u) == 268435456u) y *= -1.f;
	float z = ((data >> 18 & 511u) / 511.0f);
	if ((data & 536870912u) == 536870912u) z *= -1.f;
	return vec3(x, y, z);
}

uniform mat4 pv;
uniform mat4 model;

void main()
{
	vec3 pos = unpackSingle(aData, 0);
	vec3 appearance = unpackSingle(aData, 1);
	vec3 normal = unpackSingle(aData, 2);

	FragPos = vec3(model * vec4(pos, 1.0));
	TexCoord = vec2(appearance.x, appearance.y);
	Normal = mat3(transpose(inverse(model))) * normal;

	gl_Position = pv * vec4(FragPos, 1.0);
}
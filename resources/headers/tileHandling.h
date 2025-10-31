#include <bitset>

unsigned int DATA;
unsigned int test[] = { 1, 5, 2, 9, 0, 0, 3, 2, 6, 7 };

template<typename T>
unsigned int elementsSum(T array[], unsigned int index1, unsigned int index2) {
	unsigned int total = 0;
	for (unsigned int i = index1; i < index2; i++) {
		total += array[i];
	}
	return total;
}

//uses binary search, unsigned ints only
bool arrayContains(unsigned int array[], unsigned int element, unsigned int arrayLength) {
	unsigned int* tempArray = new unsigned int[arrayLength];
	std::copy(array, array + arrayLength, tempArray);
	std::sort(tempArray, tempArray + arrayLength);
	bool contains = std::binary_search(tempArray, tempArray + arrayLength, element);
	delete[] tempArray;
	if(contains) return 1;
	return 0;
}

template<typename T>
T difference(T num1, T num2) {
	if (num1 > num2) {
		return num1 - num2;
	}
	else {
		return num2 - num1;
	}
	return 0;
}

unsigned short clampShort(unsigned short min, unsigned short value, unsigned short max) {
	if (value > max) {
		return max;
	}
	else if (value < min) {
		return min;
	}
	return value;
}

unsigned int clampUint(unsigned int min, unsigned int value, unsigned int max) {
	if (value > max) {
		return max;
	}
	else if (value < min) {
		return min;
	}
	return value;
}

float clampFloat(float min, float value, float max) {
	if (value > max) {
		return max;
	}
	else if (value < min) {
		return min;
	}
	return value;
}

void flipBit(short place) {
	 DATA ^= (1u << place);
}

void bitOne(short place) {
	DATA |= (1u << place);
}

void bitZero(short place) {
	DATA &= ~(1u << place);
}

//Pack Vertex Data Voxel, for chunk data not available yet put 69420
unsigned int PVDV(unsigned int corner, unsigned int texture, unsigned int normal, unsigned int chunkPlace) {
	DATA = 0x00000000000000000000000000000000;
	corner = clampShort(0, corner, 7);
	texture = clampShort(0, texture, 1023);
	normal = clampShort(0, normal, 5);
	DATA |= corner;
	DATA |= normal << 3;
	DATA |= texture << 6;
	if (chunkPlace != 69420) {
		DATA |= chunkPlace << 16;
	}
	return DATA;
}
//example: unpackSingle(PVDV(0, 992, 4, 8720), 0);
glm::vec3 unpackSingle(unsigned int Data, int returnType) {
	glm::vec3 toReturn = glm::vec3(0.0f, 0.0f, 0.0f);
	if (returnType == 0) {
		unsigned int index = Data >> 16 & 65535u;
		unsigned int corner = Data & 7u;
		unsigned int widths = static_cast<unsigned int>(12);
		float y = ((index / (widths * widths)));
		float z = (((index / widths) % widths));
		float x = (index % widths);
		switch (corner) {
		case 0u: {
			x -= 0.5f;
			y -= 0.5f;
			z -= 0.5f;
		}break;
		case 1u: {
			x += 0.5f;
			y -= 0.5f;
			z -= 0.5f;
		}break;
		case 2u: {
			x += 0.5f;
			y -= 0.5f;
			z += 0.5f;
		}break;
		case 3u: {
			x -= 0.5f;
			y -= 0.5f;
			z += 0.5f;
		}break;
		case 4u: {
			x -= 0.5f;
			y += 0.5f;
			z -= 0.5f;
		}break;
		case 5u: {
			x += 0.5f;
			y += 0.5f;
			z -= 0.5f;
		}break;
		case 6u: {
			x += 0.5f;
			y += 0.5f;
			z += 0.5f;
		}break;
		case 7u: {
			x -= 0.5f;
			y += 0.5f;
			z += 0.5f;
		}break;
		default: break;
		}
		toReturn = glm::vec3(x, y, z);
	}
	if (returnType == 1) {
		unsigned int corner = Data & 7u;
		unsigned int normal = Data >> 3 & 7u;
		unsigned int texture = Data >> 6 & 1023u;
		bool offsetX = true;
		bool offsetY = false;
		switch (corner) {
		case 0u: {
			if (normal == 0u) {
				offsetX = false;
				offsetY = true;
			}
			if (normal == 4u) {
				offsetX = false;
				offsetY = false;
			}
			if (normal == 2u) {
				offsetX = true;
				offsetY = false;
			}
		} break;
		case 1u: {
			if (normal == 0u) {
				offsetX = true;
				offsetY = true;
			}
			if (normal == 4u) {
				offsetX = true;
				offsetY = false;
			}
			if (normal == 3u) {
				offsetX = false;
				offsetY = false;
			}
		} break;
		case 2u: {
			if (normal == 0u) {
				offsetX = true;
				offsetY = false;
			}
			if (normal == 3u) {
				offsetX = false;
				offsetY = false;
			}
			if (normal == 5u) {
				offsetX = false;
				offsetY = false;
			}
		} break;
		case 3u: {
			if (normal == 0u) {
				offsetX = false;
				offsetY = false;
			}
			if (normal == 2u) {
				offsetX = false;
				offsetY = true;
			}
			if (normal == 5u) {
				offsetX = true;
				offsetY = false;
			}
		} break;
		case 4u: {
			if (normal == 4u) {
				offsetX = false;
				offsetY = true;
			}
			if (normal == 2u) {
				offsetX = true;
				offsetY = true;
			}
			if (normal == 1u) {
				offsetX = false;
				offsetY = false;
			}
		} break;
		case 5u: {
			if (normal == 4u) {
				offsetX = true;
				offsetY = true;
			}
			if (normal == 3u) {
				offsetX = false;
				offsetY = true;
			}
			if (normal == 1u) {
				offsetX = true;
				offsetY = false;
			}
		} break;
		case 6u: {
			if (normal == 1u) {
				offsetX = true;
				offsetY = true;
			}
			if (normal == 5u) {
				offsetX = false;
				offsetY = true;
			}
			if (normal == 3u) {
				offsetX = true;
				offsetY = true;
			}
		} break;
		case 7u: {
			if (normal == 1u) {
				offsetX = false;
				offsetY = true;
			}
			if (normal == 2u) {
				offsetX = false;
				offsetY = true;
			}
			if (normal == 5u) {
				offsetX = true;
				offsetY = true;
			}
		} break;
		default: break;
		}
		float texY = (float((texture / 32u) * 32.0f) / 1024.f) + 0.015625f;
		float texX = (float(texture % 32u * 32.0f) / 1024.f) + 0.015625f;
		if (offsetX) {
			texX += 0.015625f;
		}
		else {
			texX -= 0.015625f;
		}
		if (offsetY) {
			texY += 0.015625f;
		}
		else {
			texY -= 0.015625f;
		}
		toReturn = glm::vec3(texX, texY, 0.0f);
	}
	if (returnType == 2) {
		unsigned int normal = Data >> 3 & 7u;
		switch (normal) {
		case 0u: toReturn = glm::vec3(0.0f, -1.0f, 0.0f);
			break;
		case 1u: toReturn = glm::vec3(0.0f, 1.0f, 0.0f);
			break;
		case 2u: toReturn = glm::vec3(-1.0f, 0.0f, 0.0f);
			break;
		case 3u: toReturn = glm::vec3(1.0f, 0.0f, 0.0f);
			break;
		case 4u: toReturn = glm::vec3(0.0f, 0.0f, -1.0f);
			break;
		case 5u: toReturn = glm::vec3(0.0f, 0.0f, 1.0f);
			break;
		default: break;
		}
	}
	return toReturn;
}












// x, y, z values must be -5.11 to 5.11, is unit cube
unsigned int PVD1(float x, float y, float z) {
	DATA = 0x00000000000000000000000000000000;
	x = clampFloat(-5.11f, x, 5.11f);
	y = clampFloat(-5.11f, y, 5.11f);
	z = clampFloat(-5.11f, z, 5.11f);
	x *= 100.0f;
	y *= 100.0f;
	z *= 100.0f;
	if (x < 0) 	bitOne(27);
	if (y < 0)	bitOne(28);
	if (z < 0) 	bitOne(29);
	x = abs(x);
	y = abs(y);
	z = abs(z);

	DATA |= static_cast<unsigned int>(x);
	DATA |= static_cast<unsigned int>(y) << 9;
	DATA |= static_cast<unsigned int>(z) << 18;

	return DATA;
}

//texture X, decimalX, texture Y, decimal Y, material
unsigned int PVD2(unsigned int texX, unsigned int texXdecimal, unsigned int texY, unsigned int texYdecimal, short material) {
	DATA = 0x00000000000000000000000000000000;
	texX = clampUint(0, texX, 32);
	texY = clampUint(0, texY, 32);
	texXdecimal = clampUint(0, texXdecimal, 31) * (texX != 32);
	texYdecimal = clampUint(0, texYdecimal, 31) * (texY != 32);
	material = clampUint(0, material, 255);
	DATA |= texX;
	DATA |= texXdecimal << 6;
	DATA |= texY << 11;
	DATA |= texYdecimal << 17;
	DATA |= material << 22;
	//std::cout << " texX: " << texX << " texXdecimal: " << texXdecimal << " texY: " << texY << " texYdecimal: " << texYdecimal << " material: " << material << " data: " << std::bitset<32>(data) << '\n';
	return DATA;
}

//normals
unsigned int PVD3(float normX, float normY, float normZ) {
	DATA = 0x00000000000000000000000000000000;
	normX = clampFloat(-1.0f, normX, 1.0f);
	normY = clampFloat(-1.0f, normY, 1.0f);
	normZ = clampFloat(-1.0f, normZ, 1.0f);
	if (normX < 0) 	bitOne(27);
	if (normY < 0)	bitOne(28);
	if (normZ < 0) 	bitOne(29);
	normX = abs(round(normX * 511.0f));
	normY = abs(round(normY * 511.0f));
	normZ = abs(round(normZ * 511.0f));
	DATA |= int(normX);
	DATA |= int(normY) << 9;
	DATA |= int(normZ) << 18;
	//std::cout << '\n' << std::bitset<32>(data) << '\n';
	return DATA;
}

//make all unsigned int's to uint for proper shader syntax
glm::vec3 unpack1(unsigned int Data) {
	float x = ((Data & 511u) * 0.01f);
	if ((Data & 134217728) == 134217728) x *= -1.0f;
	float y = ((Data >> 9 & 511u) * 0.01f);
	if ((Data & 268435456) == 268435456) y *= -1.0f;
	float z = ((Data >> 18 & 511u) * 0.01f);
	if ((Data & 536870912) == 536870912) z *= -1.0f;
	glm::vec3 toReturn = glm::vec3(x, y, z);
	//std::cout << "unpacker 1, x: " << toReturn.x << " y: " << toReturn.y << " z: " << toReturn.z;
	return toReturn;
}

glm::vec3 unpack2(unsigned int Data) {
	float texX = (((Data & 63) * 32.0f) + (Data >> 6 & 31)) / 1024.0f;
	float texY = (((Data >> 11 & 63) * 32.0f) + (Data >> 17 & 31)) / 1024.0f;
	float material = float(Data >> 22 & 1023);
	glm::vec3 toReturn = glm::vec3(texX, texY, material);
	//std::cout << "x: " << toReturn.x << " y: " << toReturn.y << '\n';
	//std::cout << "x: " << toReturn.x * 1024.0f << " y: " << toReturn.y * 1024.0f;
	return toReturn;
}

glm::vec3 unpack3(unsigned int Data) {
	float x = (Data & 511) / 511.0f;
	if ((Data & 134217728) == 134217728) x *= -1.f;
	float y = (Data >> 9 & 511) / 511.0f;
	if ((Data & 268435456) == 268435456) y *= -1.f;
	float z = (Data >> 18 & 511) / 511.0f;
	if ((Data & 536870912) == 536870912) z *= -1.f;
	glm::vec3 toReturn = glm::vec3(x, y, z);
	//std::cout << " unpacker 3, x: " << toReturn.x << " y: " << toReturn.y << " z: " << toReturn.z << '\n';
	return toReturn;
}

void arrayRange(unsigned int array[], unsigned int index0, unsigned int index1, unsigned int toFill[]) {
	unsigned int size = difference(index0, index1);
	for (unsigned int i = 0; i < size; i++) {
		toFill[i] = array[index0 + i];
	}
}

float fullScreenQuadVertices[] = {
	-1.0f,  1.0f,  0.0f, 1.0f,
	-1.0f, -1.0f,  0.0f, 0.0f,
	 1.0f, -1.0f,  1.0f, 0.0f,

	-1.0f,  1.0f,  0.0f, 1.0f,
	 1.0f, -1.0f,  1.0f, 0.0f,
	 1.0f,  1.0f,  1.0f, 1.0f
};

float skyboxVertices[] = {
	0.f,
	1.f,
	2.f,
	3.f,
	4.f,
	5.f,
	6.f,
	7.f
};

unsigned int skyboxIndices[] = {
	5, 6, 2,
	2, 1, 5,

	7, 4, 0,
	0, 3, 7,

	7, 6, 5,
	5, 4, 7,

	0, 1, 2,
	2, 3, 0,

	4, 5, 1,
	1, 0, 4,

	6, 7, 3,
	3, 2, 6
};

float debugVertices[] = {
	0.f, 0.1875f,   1.0f,
	1.f, 0.21875f,	1.0f,
	2.f, 0.21875f,	0.96875f,
	2.f, 0.21875f,	0.96875f,
	3.f, 0.1875f,	0.96875f,
	0.f, 0.1875f,   1.0f,

	7.f, 0.1875f,   1.0f,
	6.f, 0.21875f,	1.0f,
	5.f, 0.21875f,	0.96875f,
	5.f, 0.21875f,	0.96875f,
	4.f, 0.1875f,	0.96875f,
	7.f, 0.1875f,   1.0f,

	7.f, 0.1875f,   1.0f,
	4.f, 0.21875f,	1.0f,
	0.f, 0.21875f,	0.96875f,
	0.f, 0.21875f,	0.96875f,
	3.f, 0.1875f,	0.96875f,
	7.f, 0.1875f,   1.0f,

	5.f, 0.1875f,   1.0f,
	6.f, 0.21875f,	1.0f,
	2.f, 0.21875f,	0.96875f,
	2.f, 0.21875f,	0.96875f,
	1.f, 0.1875f,	0.96875f,
	5.f, 0.1875f,   1.0f,

	4.f, 0.1875f,   1.0f,
	5.f, 0.21875f,	1.0f,
	1.f, 0.21875f,	0.96875f,
	1.f, 0.21875f,	0.96875f,
	0.f, 0.1875f,	0.96875f,
	4.f, 0.1875f,   1.0f,

	6.f, 0.1875f,   1.0f,
	7.f, 0.21875f,	1.0f,
	3.f, 0.21875f,	0.96875f,
	3.f, 0.21875f,	0.96875f,
	2.f, 0.1875f,	0.96875f,
	6.f, 0.1875f,   1.0f,

};

unsigned int foliageVertices[] = {
//bush1
 PVD2(5, 16, 31, 16, 0), 
	PVD2(5, 16, 31, 16, 0),
	PVD2(6, 0, 31, 16, 0),
	PVD2(5, 16, 31, 20, 0),
	PVD2(6, 0, 31, 20, 0),
	PVD2(5, 16, 31, 24, 0),
	PVD2(6, 0, 31, 24, 0),
	PVD2(5, 16, 31, 28, 0),
	PVD2(6, 0, 31, 28, 0),
	PVD2(5, 16, 32, 0, 0),
	PVD2(6, 0, 32, 0, 0),
 PVD2(6, 0, 32, 0, 0),

 PVD2(5, 16, 31, 16, 0),
	PVD2(5, 16, 31, 16, 0),
	PVD2(6, 0, 31, 16, 0),
	PVD2(5, 16, 31, 20, 0),
	PVD2(6, 0, 31, 20, 0),
	PVD2(5, 16, 31, 24, 0),
	PVD2(6, 0, 31, 24, 0),
	PVD2(5, 16, 31, 28, 0),
	PVD2(6, 0, 31, 28, 0),
	PVD2(5, 16, 32, 0, 0),
	PVD2(6, 0, 32, 0, 0),
 PVD2(6, 0, 32, 0, 0),

 PVD2(5, 16, 31, 16, 0),
	PVD2(5, 16, 31, 16, 0),
	PVD2(6, 0, 31, 16, 0),
	PVD2(5, 16, 31, 20, 0),
	PVD2(6, 0, 31, 20, 0),
	PVD2(5, 16, 31, 24, 0),
	PVD2(6, 0, 31, 24, 0),
	PVD2(5, 16, 31, 28, 0),
	PVD2(6, 0, 31, 28, 0),
	PVD2(5, 16, 32, 0, 0),
	PVD2(6, 0, 32, 0, 0),
 PVD2(6, 0, 32, 0, 0),

 PVD2(5, 16, 31, 16, 0),
	PVD2(5, 16, 31, 16, 0),
	PVD2(6, 0, 31, 16, 0),
	PVD2(5, 16, 31, 20, 0),
	PVD2(6, 0, 31, 20, 0),
	PVD2(5, 16, 31, 24, 0),
	PVD2(6, 0, 31, 24, 0),
	PVD2(5, 16, 31, 28, 0),
	PVD2(6, 0, 31, 28, 0),
	PVD2(5, 16, 32, 0, 0),
	PVD2(6, 0, 32, 0, 0),
 PVD2(6, 0, 32, 0, 0),

 PVD2(5, 16, 31, 16, 0),
	PVD2(5, 16, 31, 16, 0),
	PVD2(6, 0, 31, 16, 0),
	PVD2(5, 16, 31, 20, 0),
	PVD2(6, 0, 31, 20, 0),
	PVD2(5, 16, 31, 24, 0),
	PVD2(6, 0, 31, 24, 0),
	PVD2(5, 16, 31, 28, 0),
	PVD2(6, 0, 31, 28, 0),
	PVD2(5, 16, 32, 0, 0),
	PVD2(6, 0, 32, 0, 0),
 PVD2(6, 0, 32, 0, 0),

 PVD2(5, 16, 31, 16, 0),
	PVD2(5, 16, 31, 16, 0),
	PVD2(6, 0, 31, 16, 0),
	PVD2(5, 16, 31, 20, 0),
	PVD2(6, 0, 31, 20, 0),
	PVD2(5, 16, 31, 24, 0),
	PVD2(6, 0, 31, 24, 0),
	PVD2(5, 16, 31, 28, 0),
	PVD2(6, 0, 31, 28, 0),
	PVD2(5, 16, 32, 0, 0),
	PVD2(6, 0, 32, 0, 0),
 PVD2(6, 0, 32, 0, 0),

 //bush2
 PVD2(5, 0, 31, 0, 1),
	PVD2(5, 0, 31, 0, 1),
	PVD2(5, 16, 31, 0, 1),
	PVD2(5, 0, 31, 5, 1),
	PVD2(5, 16, 31, 5, 1),
	PVD2(5, 0, 31, 10, 1),
	PVD2(5, 16, 31, 10, 1),
	PVD2(5, 0, 31, 16, 1),
	PVD2(5, 16, 31, 16, 1),
 PVD2(5, 16, 31, 16, 1),

 PVD2(5, 0, 31, 0, 1),
	PVD2(5, 0, 31, 0, 1),
	PVD2(5, 16, 31, 0, 1),
	PVD2(5, 0, 31, 5, 1),
	PVD2(5, 16, 31, 5, 1),
	PVD2(5, 0, 31, 10, 1),
	PVD2(5, 16, 31, 10, 1),
	PVD2(5, 0, 31, 16, 1),
	PVD2(5, 16, 31, 16, 1),
 PVD2(5, 16, 31, 16, 1),

 PVD2(5, 0, 31, 0, 1),
	PVD2(5, 0, 31, 0, 1),
	PVD2(5, 16, 31, 0, 1),
	PVD2(5, 0, 31, 5, 1),
	PVD2(5, 16, 31, 5, 1),
	PVD2(5, 0, 31, 10, 1),
	PVD2(5, 16, 31, 10, 1),
	PVD2(5, 0, 31, 16, 1),
	PVD2(5, 16, 31, 16, 1),
 PVD2(5, 16, 31, 16, 1),

 PVD2(5, 0, 31, 0, 1),
	PVD2(5, 0, 31, 0, 1),
	PVD2(5, 16, 31, 0, 1),
	PVD2(5, 0, 31, 5, 1),
	PVD2(5, 16, 31, 5, 1),
	PVD2(5, 0, 31, 10, 1),
	PVD2(5, 16, 31, 10, 1),
	PVD2(5, 0, 31, 16, 1),
	PVD2(5, 16, 31, 16, 1),
PVD2(5, 16, 31, 16, 1),

 PVD2(5, 0, 31, 0, 1),
	PVD2(5, 0, 31, 0, 1),
	PVD2(5, 16, 31, 0, 1),
	PVD2(5, 0, 31, 5, 1),
	PVD2(5, 16, 31, 5, 1),
	PVD2(5, 0, 31, 10, 1),
	PVD2(5, 16, 31, 10, 1),
	PVD2(5, 0, 31, 16, 1),
	PVD2(5, 16, 31, 16, 1),
PVD2(5, 16, 31, 16, 1),

 PVD2(5, 0, 31, 0, 1),
	PVD2(5, 0, 31, 0, 1),
	PVD2(5, 16, 31, 0, 1),
	PVD2(5, 0, 31, 5, 1),
	PVD2(5, 16, 31, 5, 1),
	PVD2(5, 0, 31, 10, 1),
	PVD2(5, 16, 31, 10, 1),
	PVD2(5, 0, 31, 16, 1),
	PVD2(5, 16, 31, 16, 1),
PVD2(5, 16, 31, 16, 1),

//grass blade 1
PVD2(5, 2, 31, 16, 2),
	PVD2(5, 2, 31, 16, 2),
	PVD2(5, 16, 31, 16, 2),
	PVD2(5, 2, 31, 20, 2),
	PVD2(5, 16, 31, 20, 2),
	PVD2(5, 2, 31, 25, 2),
	PVD2(5, 16, 31, 25, 2),
PVD2(5, 16, 31, 25, 2),

PVD2(5, 2, 31, 16, 2),
	PVD2(5, 2, 31, 16, 2),
	PVD2(5, 16, 31, 16, 2),
	PVD2(5, 2, 31, 20, 2),
	PVD2(5, 16, 31, 20, 2),
	PVD2(5, 2, 31, 25, 2),
	PVD2(5, 16, 31, 25, 2),
PVD2(5, 16, 31, 25, 2),

PVD2(5, 2, 31, 16, 2),
	PVD2(5, 2, 31, 16, 2),
	PVD2(5, 16, 31, 16, 2),
	PVD2(5, 2, 31, 20, 2),
	PVD2(5, 16, 31, 20, 2),
	PVD2(5, 2, 31, 25, 2),
	PVD2(5, 16, 31, 25, 2),
PVD2(5, 16, 31, 25, 2),

PVD2(5, 2, 31, 16, 2),
	PVD2(5, 2, 31, 16, 2),
	PVD2(5, 16, 31, 16, 2),
	PVD2(5, 2, 31, 20, 2),
	PVD2(5, 16, 31, 20, 2),
	PVD2(5, 2, 31, 25, 2),
	PVD2(5, 16, 31, 25, 2),
PVD2(5, 16, 31, 25, 2),

PVD2(5, 2, 31, 16, 2),
	PVD2(5, 2, 31, 16, 2),
	PVD2(5, 16, 31, 16, 2),
	PVD2(5, 2, 31, 20, 2),
	PVD2(5, 16, 31, 20, 2),
	PVD2(5, 2, 31, 25, 2),
	PVD2(5, 16, 31, 25, 2),
PVD2(5, 16, 31, 25, 2),

PVD2(5, 2, 31, 16, 2),
	PVD2(5, 2, 31, 16, 2),
	PVD2(5, 16, 31, 16, 2),
	PVD2(5, 2, 31, 20, 2),
	PVD2(5, 16, 31, 20, 2),
	PVD2(5, 2, 31, 25, 2),
	PVD2(5, 16, 31, 25, 2),
PVD2(5, 16, 31, 25, 2),

PVD2(5, 2, 31, 16, 2),
	PVD2(5, 2, 31, 16, 2),
	PVD2(5, 16, 31, 16, 2),
	PVD2(5, 2, 31, 20, 2),
	PVD2(5, 16, 31, 20, 2),
	PVD2(5, 2, 31, 25, 2),
	PVD2(5, 16, 31, 25, 2),
PVD2(5, 16, 31, 25, 2),

PVD2(5, 2, 31, 16, 2),
	PVD2(5, 2, 31, 16, 2),
	PVD2(5, 16, 31, 16, 2),
	PVD2(5, 2, 31, 20, 2),
	PVD2(5, 16, 31, 20, 2),
	PVD2(5, 2, 31, 25, 2),
	PVD2(5, 16, 31, 25, 2),
PVD2(5, 16, 31, 25, 2),

PVD2(5, 2, 31, 16, 2),
	PVD2(5, 2, 31, 16, 2),
	PVD2(5, 16, 31, 16, 2),
	PVD2(5, 2, 31, 20, 2),
	PVD2(5, 16, 31, 20, 2),
	PVD2(5, 2, 31, 25, 2),
	PVD2(5, 16, 31, 25, 2),
PVD2(5, 16, 31, 25, 2),

PVD2(5, 2, 31, 16, 2),
	PVD2(5, 2, 31, 16, 2),
	PVD2(5, 16, 31, 16, 2),
	PVD2(5, 2, 31, 20, 2),
	PVD2(5, 16, 31, 20, 2),
	PVD2(5, 2, 31, 25, 2),
	PVD2(5, 16, 31, 25, 2),
PVD2(5, 16, 31, 25, 2),

PVD2(5, 2, 31, 16, 2),
	PVD2(5, 2, 31, 16, 2),
	PVD2(5, 16, 31, 16, 2),
	PVD2(5, 2, 31, 20, 2),
	PVD2(5, 16, 31, 20, 2),
	PVD2(5, 2, 31, 25, 2),
	PVD2(5, 16, 31, 25, 2),
PVD2(5, 16, 31, 25, 2),

PVD2(5, 2, 31, 16, 2),
	PVD2(5, 2, 31, 16, 2),
	PVD2(5, 16, 31, 16, 2),
	PVD2(5, 2, 31, 20, 2),
	PVD2(5, 16, 31, 20, 2),
	PVD2(5, 2, 31, 25, 2),
	PVD2(5, 16, 31, 25, 2),
PVD2(5, 16, 31, 25, 2),
};

unsigned int tileVertices[] = {
	//stone
	PVDV(0, 992, 0, 69420),
	PVDV(1, 992, 0, 69420),
	PVDV(2, 992, 0, 69420),
	PVDV(2, 992, 0, 69420),
	PVDV(3, 992, 0, 69420),
	PVDV(0, 992, 0, 69420),

	PVDV(7, 992, 1, 69420),
	PVDV(6, 992, 1, 69420),
	PVDV(5, 992, 1, 69420),
	PVDV(5, 992, 1, 69420),
	PVDV(4, 992, 1, 69420),
	PVDV(7, 992, 1, 69420),

	PVDV(7, 992, 2, 69420),
	PVDV(4, 992, 2, 69420),
	PVDV(0, 992, 2, 69420),
	PVDV(0, 992, 2, 69420),
	PVDV(3, 992, 2, 69420),
	PVDV(7, 992, 2, 69420),

	PVDV(5, 992, 3, 69420),
	PVDV(6, 992, 3, 69420),
	PVDV(2, 992, 3, 69420),
	PVDV(2, 992, 3, 69420),
	PVDV(1, 992, 3, 69420),
	PVDV(5, 992, 3, 69420),

	PVDV(4, 992, 4, 69420),
	PVDV(5, 992, 4, 69420),
	PVDV(1, 992, 4, 69420),
	PVDV(1, 992, 4, 69420),
	PVDV(0, 992, 4, 69420),
	PVDV(4, 992, 4, 69420),

	PVDV(6, 992, 5, 69420),
	PVDV(7, 992, 5, 69420),
	PVDV(3, 992, 5, 69420),
	PVDV(3, 992, 5, 69420),
	PVDV(2, 992, 5, 69420),
	PVDV(6, 992, 5, 69420),

	//grass
	PVDV(0, 994, 0, 69420),
	PVDV(1, 994, 0, 69420),
	PVDV(2, 994, 0, 69420),
	PVDV(2, 994, 0, 69420),
	PVDV(3, 994, 0, 69420),
	PVDV(0, 994, 0, 69420),

	PVDV(7, 995, 1, 69420),
	PVDV(6, 995, 1, 69420),
	PVDV(5, 995, 1, 69420),
	PVDV(5, 995, 1, 69420),
	PVDV(4, 995, 1, 69420),
	PVDV(7, 995, 1, 69420),

	PVDV(7, 993, 2, 69420),
	PVDV(4, 993, 2, 69420),
	PVDV(0, 993, 2, 69420),
	PVDV(0, 993, 2, 69420),
	PVDV(3, 993, 2, 69420),
	PVDV(7, 993, 2, 69420),

	PVDV(5, 993, 3, 69420),
	PVDV(6, 993, 3, 69420),
	PVDV(2, 993, 3, 69420),
	PVDV(2, 993, 3, 69420),
	PVDV(1, 993, 3, 69420),
	PVDV(5, 993, 3, 69420),

	PVDV(4, 993, 4, 69420),
	PVDV(5, 993, 4, 69420),
	PVDV(1, 993, 4, 69420),
	PVDV(1, 993, 4, 69420),
	PVDV(0, 993, 4, 69420),
	PVDV(4, 993, 4, 69420),

	PVDV(6, 993, 5, 69420),
	PVDV(7, 993, 5, 69420),
	PVDV(3, 993, 5, 69420),
	PVDV(3, 993, 5, 69420),
	PVDV(2, 993, 5, 69420),
	PVDV(6, 993, 5, 69420),

	//dirt
	PVDV(0, 994, 0, 69420),
	PVDV(1, 994, 0, 69420),
	PVDV(2, 994, 0, 69420),
	PVDV(2, 994, 0, 69420),
	PVDV(3, 994, 0, 69420),
	PVDV(0, 994, 0, 69420),

	PVDV(7, 994, 1, 69420),
	PVDV(6, 994, 1, 69420),
	PVDV(5, 994, 1, 69420),
	PVDV(5, 994, 1, 69420),
	PVDV(4, 994, 1, 69420),
	PVDV(7, 994, 1, 69420),

	PVDV(7, 994, 2, 69420),
	PVDV(4, 994, 2, 69420),
	PVDV(0, 994, 2, 69420),
	PVDV(0, 994, 2, 69420),
	PVDV(3, 994, 2, 69420),
	PVDV(7, 994, 2, 69420),

	PVDV(5, 994, 3, 69420),
	PVDV(6, 994, 3, 69420),
	PVDV(2, 994, 3, 69420),
	PVDV(2, 994, 3, 69420),
	PVDV(1, 994, 3, 69420),
	PVDV(5, 994, 3, 69420),

	PVDV(4, 994, 4, 69420),
	PVDV(5, 994, 4, 69420),
	PVDV(1, 994, 4, 69420),
	PVDV(1, 994, 4, 69420),
	PVDV(0, 994, 4, 69420),
	PVDV(4, 994, 4, 69420),

	PVDV(6, 994, 5, 69420),
	PVDV(7, 994, 5, 69420),
	PVDV(3, 994, 5, 69420),
	PVDV(3, 994, 5, 69420),
	PVDV(2, 994, 5, 69420),
	PVDV(6, 994, 5, 69420),
};

float halfBlock[] = {
	-0.25f, -0.25f, -0.25f,
	 0.25f, -0.25f, -0.25f,
	 0.25f,  0.25f, -0.25f,
	 0.25f,  0.25f, -0.25f,
	-0.25f,  0.25f, -0.25f,
	-0.25f, -0.25f, -0.25f,
	
	-0.25f, -0.25f,  0.25f,
	 0.25f, -0.25f,  0.25f,
	 0.25f,  0.25f,  0.25f,
	 0.25f,  0.25f,  0.25f,
	-0.25f,  0.25f,  0.25f,
	-0.25f, -0.25f,  0.25f,
	
	-0.25f,  0.25f,  0.25f,
	-0.25f,  0.25f, -0.25f,
	-0.25f, -0.25f, -0.25f,
	-0.25f, -0.25f, -0.25f,
	-0.25f, -0.25f,  0.25f,
	-0.25f,  0.25f,  0.25f,
	
	 0.25f,  0.25f,  0.25f,
	 0.25f,  0.25f, -0.25f,
	 0.25f, -0.25f, -0.25f,
	 0.25f, -0.25f, -0.25f,
	 0.25f, -0.25f,  0.25f,
	 0.25f,  0.25f,  0.25f,
	
	-0.25f, -0.25f, -0.25f,
	 0.25f, -0.25f, -0.25f,
	 0.25f, -0.25f,  0.25f,
	 0.25f, -0.25f,  0.25f,
	-0.25f, -0.25f,  0.25f,
	-0.25f, -0.25f, -0.25f,
	
	-0.25f,  0.25f, -0.25f,
	 0.25f,  0.25f, -0.25f,
	 0.25f,  0.25f,  0.25f,
	 0.25f,  0.25f,  0.25f,
	-0.25f,  0.25f,  0.25f,
	-0.25f,  0.25f, -0.25f,
};
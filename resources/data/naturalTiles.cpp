struct verts {
	unsigned int data[36];
};

std::map<short, verts> naturalTiles;

static void loadTiles() {
	naturalTiles.clear();
	for (unsigned int i = 0; i < (sizeof(tileVertices) / sizeof(tileVertices[0])) / (36.f); i++) {
		verts vert;
		int count = 0;
		for (unsigned int j = (36 * i); j < (36 * i) + 36; j++) {
			if(j - (36 * i) < 36) vert.data[count] = tileVertices[j];
			count++;
		}
		naturalTiles.insert(std::pair<short, verts>(i + 1, vert));
	}
}

static void DisplayData(short key, bool binary) {
	if (binary) {
		for (unsigned int i = 0; i < 36; i++) {
			std::cout << std::bitset<32>(naturalTiles[key].data[i]) << '\n';
		}
	}
	else {
		for (unsigned int i = 0; i < 36; i++) {
			glm::vec3 data = unpackSingle(naturalTiles[key].data[i], 0);
			std::cout << "vertex " << i << " x: " << data.x << " y: " << data.y << " z: " << data.z << " , ";
			data = unpackSingle(naturalTiles[key].data[i], 1);
			std::cout << " texture x: " << data.x << " texture y: " << data.y << " material: " << data.z << " , ";
			data = unpackSingle(naturalTiles[key].data[i], 2);
			std::cout << " normal x: " << data.x << " normal y: " << data.y << " normal z: " << data.z << '\n';
		}
	}
	
}
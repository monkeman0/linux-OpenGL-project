struct Button_State {
	bool is_down;
	bool is_changed;
};

enum buttons {
	BUTTON_SPACE,
	BUTTON_F3,
	BUTTON_0,
	BUTTON_1,
	BUTTON_2,
	BUTTON_3,
	BUTTON_4,
	BUTTON_5,
	BUTTON_6,
	BUTTON_7,
	BUTTON_8,
	BUTTON_9,
	BUTTON_F11,
	BUTTON_W,
	BUTTON_A,
	BUTTON_S,
	BUTTON_D,
	BUTTON_LEFTSHIFT,
	BUTTON_ESCAPE,
	BUTTON_F4,
	BUTTON_B,
	BUTTON_C,
	BUTTON_E,
	BUTTON_F,
	BUTTON_G,
	BUTTON_H,
	BUTTON_I,
	BUTTON_J,
	BUTTON_K,
	BUTTON_L,
	BUTTON_M,
	BUTTON_N,
	BUTTON_O,
	BUTTON_P,
	BUTTON_Q,
	BUTTON_R,
	BUTTON_T,
	BUTTON_U,
	BUTTON_V,
	BUTTON_X,
	BUTTON_Y,
	BUTTON_Z,
	BUTTON_LEFT_CTRL,
	BUTTON_COUNT
};

struct Input {
	Button_State buttons[BUTTON_COUNT];
};

struct Debug {
	bool showWireFrame = false;
	bool showInfo = false;
	unsigned short nextFPScounter = 0;
	short int FPS = 0;
	bool fullscreen = true;
	bool mouseLocked = true;
	char moveMode = '0';
	float speed = 4.0f;
	bool showChunkBorders = false;
	bool useLOD = true;
};
/*
 * USART.c
 *
 * Created: 5/30/2018 11:50:34 PM
 * Author : David
 */

#include <avr/io.h>
#include <ucr/usart.h>
#include <ucr/scheduler.h>

//task declaration
task Mytasks[6];
unsigned char MyTasks = 6;

#define INPUT_BUTTONS PORTA
#define INPUT_RESET 0x01
#define INPUT_PAUSE 0x02
#define INPUT_LEFT 0x04
#define INPUT_RIGHT 0x08


//states
enum G_States {G_Init, G_Pause, G_End, G_Reset, G_InProgress} G_State;
//task declaration
task GState;
//tick function prototype
int Gstate_Tick();
//shared variables written to
unsigned char GAME_STATUS;
//values for variables written to
#define IN_PROGRESS 0
#define END 1
#define PAUSED 2
#define RESET 3
//functions called by various states in this SM


enum LED_States {LED_Init, LED_Update} LED_State;
task LEDUpdate;
int LEDUpdate_Tick();
unsigned char UPDATE_SUCCESS;
#define FAILED 0
#define SUCCESS 1
//USART Prefix Messages so Arduino knows what type of information is inbound
#define PREFIX_UPDATE 0
#define PREFIX_WIN 1
#define PREFIX_PAUSE 2
#define PREFIX_LOSE 3
#define PREFIX_ERR 4
#define PREFIX_RESET 5
int updateLED();
int clearScreen();

struct blockCoord {
    unsigned char x; //top left x-coordinate
    unsigned char y; //top left y-coordinate
    unsigned char *blockType;
}
enum GLogic_States {GLogic_Init, GLogic_Input, GLogic_CheckRows, GLogic_SpawnBlock, GLogic_Advance} GLogic_State;
task GLogic;
int GLogic_Tick();
unsigned char LED_MATRIX[32][16]; //27x16 is where game takes place; extra shows score
const unsigned char LED_MATRIX_ROW = 32;
const unsigned char LED_MATRIX_COL = 16;
unsigned char LOGIC_STATUS;
unsigned char SCORE;
unsigned char BLOCK_STATUS;
unsigned char BLOCK_POS;
blockCoord CURRENT_BLOCK;
#define GAME_OVER 0
#define WIN 1
#define NEITHER 2
#define SPAWN 1
#define NO_SPAWN 0
int parseInput();
int checkRows();
int spawnBlock();
int advance();
int updateBlockPos();

enum ERR_States {ERR_Init, ERR_Msg} Err_State;
task Err;
int Err_Tick();
unsigned char ERR_TYPE;
#define NONE 0
#define LED 1
#define ERR_LOGIC_PARSE 2
#define ERR_LOGIC_CHECK 3
#define ERR_LOGIC_ADVANCE 4
#define ERR_LOGIC_SPAWN 5
#define GAME 6

enum RAND_States {RAND_Init, RAND_Loop} RAND_State;
task RAND;
int RAND_Tick();
unsigned char SEED;





//BEGIN BLOCK_MATRIX_ALL ELEMENT DECLARATIONS
unsigned char BLOCK_MATRIX[2][2] = {
	{1,1},
	{1,1}
};

//BEGIN HOOK_L_MATRIX_ALL ELEMENT DECLARATIONS
//RIGHT = CCW
//LEFT = CW
unsigned char HOOK_L_MATRIX_ONE = {
	{1,0,0},
	{1,1,1}
};
unsigned char HOOK_L_MATRIX_TWO = {
    {0,1},
    {0,1},
    {1,1}
};
unsigned char HOOK_L_MATRIX_THREE = {
    {1,1,1},
	{0,0,1}
};
unsigned char HOOK_L_MATRIX_FOUR = {
    {1,1},
    {1,0},
    {1,0}
};
//END HOOK_L_MATRIX_ALL ELEMENT DECLARATIONS

//BEGIN HOOK_R_MATRIX_ALL ELEMENT DECLARATIONS
//RIGHT = CCW
//LEFT = CW
unsigned char HOOK_R_MATRIX_ONE = {
	{0,0,1},
	{1,1,1}
};
unsigned char HOOK_R_MATRIX_TWO = {
    {1,1},
    {0,1},
    {0,1}
};
unsigned char HOOK_R_MATRIX_THREE = {
    {1,1,1},
	{1,0,0}
};
unsigned char HOOK_R_MATRIX_FOUR = {
    {1,0},
    {1,0},
    {1,1}
};
//END HOOK_R_MATRIX_ALL ELEMENT DECLARATIONS

//BEGIN Z_MATRIX_ALL ELEMENT DECLARATIONS
//RIGHT = CCW
//LEFT = CW
unsigned char Z_MATRIX_ONE = {
	{1,1,0},
	{0,1,1}
};
unsigned char Z_MATRIX_TWO = {
	{0,1},
	{1,1},
    {1,0}
};
//END Z_MATRIX_ALL ELEMENT DECLARATIONS

//BEGIN Z_BACK_MATRIX_ALL ELEMENT DECLARATIONS
unsigned char Z_BACK_MATRIX_ONE = {
	{0,1,1},
	{1,1,0}
};
unsigned char Z_BACK_MATRIX_TWO = {
    {1,0},
	{1,1},
    {0,1}
};
//END Z_BACK_MATRIX_ALL ELEMENT DECLARATIONS

unsigned char LINE_MATRIX[1][4] = {
	{1,1,1,1}
};
unsigned char WASD_MATRIX[2][3] = {
	{0,1,0},
	{1,1,1}
};

unsigned char BLOCK_MATRIX_ALL[1];
unsigned char HOOK_L_MATRIX_ALL[4];
unsigned char HOOK_R_MATRIX_ALL[4];
unsigned char Z_MATRIX_ALL[4];
unsigned char Z_BACK_MATRIX_ALL[4];
unsigned char LINE_MATRIX_ALL[4];
unsigned char WASD_MATRIX_ALL[4];

unsigned char ONE[5][1] = {
	{1},
	{1},
	{1},
	{1},
	{1},
};
unsigned char TWO[5][3] = {
	{1, 1, 1},
	{0, 0, 1},
	{1, 1, 1},
	{1, 0, 0},
	{1, 1, 1},
};
unsigned char THREE[5][3] = {
	{1, 1, 1},
	{0, 0, 1},
	{1, 1, 1},
	{0, 0, 1},
	{1, 1, 1},
};
unsigned char FOUR[5][3] = {
	{1, 0, 1},
	{1, 0, 1},
	{1, 1, 1},
	{0, 0, 1},
	{0, 0, 1},
};
unsigned char FIVE[5][3] = {
	{1, 1, 1},
	{1, 0, 0},
	{1, 1, 1},
	{0, 0, 1},
	{1, 1, 1},
};
unsigned char SIX[5][3] = {
	{1, 0, 0},
	{1, 0, 0},
	{1, 1, 1},
	{1, 0, 1},
	{1, 1, 1},
};
unsigned char SEVEN[5][3] = {
	{1, 1, 1},
	{0, 0, 1},
	{0, 0, 1},
	{0, 0, 1},
	{0, 0, 1}
};
unsigned char EIGHT[5][3] = {
	{1, 1, 1},
	{1, 0, 1},
	{1, 1, 1},
	{1, 0, 1},
	{1, 1, 1},
};
unsigned char NINE[5][3] = {
	{1, 1, 1},
	{1, 0, 1},
	{1, 1, 1},
	{0, 0, 1},
	{0, 0, 1},
};


int main(void)
{
    //PORT INITS
	DDRA = 0x00; PORTA = 0xFF;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;

    //USART INIT
	initUSART();

    //FILL BLOCK MATRICES
    BLOCK_MATRIX_ALL = {&BLOCK_MATRIX};
    HOOK_L_MATRIX_ALL = {&HOOK_L_MATRIX};

    //GLOBAL INITS
    SEED = 0;
    ERR_TYPE = NONE;
    CURRENT_BLOCK.x = 0;
    CURRENT_BLOCK.y = LED_MATRIX_COL - 1 - 5; //0 indexed & first 5 rows are for score
    CURRENT_BLOCK.blockType = &BLOCK_MATRIX; //TODO: need to check if it will compile


    UPDATE_SUCCESS = SUCCESS; //for initialization
    LOGIC_STATUS = NEITHER;


    /* Replace with your application code */
    while (1)
    {
		if (USART_IsSendReady()) {
			USART_Send(0x10);
			PORTC = 0x01;
		} else {
			//USART_Send(0x01);
			PORTC = 0x00;
		}
		//while (!USART_HasTransmitted());
    }
}

int Gstate_Tick() {
	switch (G_State) {
		case G_Init:
			G_State = G_InProgress;
			break;
		case G_InProgress:
			if (INPUT_BUTTONS == INPUT_RESET) {
				G_State = G_Reset;
			} else if (INPUT_BUTTONS = INPUT_PAUSE) {
				G_State = G_Pause;
			} else {
				G_State = G_InProgress;
			}
			break;
		case G_End:
			if (INPUT_BUTTONS == INPUT_RESET) {
				G_State = G_Reset;
			} else {
				G_State = G_End;
			}
			break;
		case G_Pause:
			if (INPUT_BUTTONS == INPUT_PAUSE) {
				G_State = G_InProgress;
			} else {
				G_State = G_Pause;
			}
			break;
		case G_Reset:
			G_State = G_InProgress;
			break;
		default:
			G_State = G_Init;
	}
	switch (G_State) {
		case G_InProgress:
			GAME_STATUS = IN_PROGRESS;
			break;
		case G_End:
			GAME_STATUS = END;
			break;
		case G_Pause:
			GAME_STATUS = PAUSED;
			break;
		case G_Reset:
			GAME_STATUS = RESET;
			break;
	}
	return G_State;
}

int LEDUpdate_Tick() {
	switch (LED_State) {
		case LED_Init:
			//clear screen
			if (USART_IsSendReady()) {
				USART_Send(PREFIX_RESET);
				if (clearScreen()) {UPDATE_SUCCESS = SUCCESS; LED_State = LED_Update;}
				else {UPDATE_SUCCESS = FAILED; LED_State = LED_Init;}
			} else {
				UPDATE_SUCCESS = FAILED;
				LED_State = LED_Init;
			}
			break;
		case LED_Update:
			if (GAME_STATUS == RESET) {
				LED_State = LED_Init;
			} else {
				UPDATE_SUCCESS = updateLed();
			}
	}
	return LED_State;
}

int clearScreen() {
	for (unsigned char i = 0; i < LED_MATRIX_ROW; i++) {
		for(unsigned char j = 0; j < LED_MATRIX_COL; j++) {
			LED_MATRIX[i][j] = 0;
		}
	}
	//funciton to clear screen and init to 0 already exists on arduino
	//so it is not necessary to send this over USART
	return SUCCESS;
}

int updateLED() {
	//could send 8 LED'S at a time but just wanna get something working
	//so just gonna send 1 by 1.
	//convert to binary and send over USART
	for (unsigned char i = 0; i < LED_MATRIX_ROW; i++) {
		for(unsigned char j = 0; j < LED_MATRIX_COL; j++) {
			if (USART_IsSendReady()) {
				USART_Send(LED_MATRIX[i][j]);
			} else {
				return FAILED;
			}
		}
	}
	return SUCCESS;
}

int GLogic_Tick() {
	switch (GLogic_State) {
		case GLogic_Init:
			GLogic_State = GLogic_Input;
			LOGIC_STATUS = NEITHER;//neither win nor loss
			SCORE = 0;
			break;
		case GLogic_Input:
			GLogic_State = GLogic_CheckRows;
			break;
		case GLogic_CheckRows:
			if (BLOCK_STATUS == SPAWN) {
				GLogic_State = GLogic_SpawnBlock;
			} else if (BLOCK_STATUS == NO_SPAWN) {
				GLogic_State = GLogic_Advance;
			}
			break;
		case GLogic_SpawnBlock:
			GLogic_State = GLogic_Input;
			break;
		case GLogic_Advance:
			GLogic_State = GLogic_Input;
			break;
	}
	switch(GLogic_State) {
		case GLogic_Input:
			if (!parseInput()) {
				ERR_TYPE = ERR_LOGIC_PARSE;
			} else {
                if (!updateBlockPos()){ERR_TYPE = ERR_BLOCK_POS;}
            }
			break;
		case GLogic_CheckRows:
			if (!checkRows()) {
				ERR_TYPE = ERR_LOGIC_CHECK;
			}
			break;
		case GLogic_SpawnBlock:
			if (!spawnBlock()) {
				ERR_TYPE = ERR_LOGIC_SPAWN;
			}
			break;
		case GLogic_Advance:
			if (!advance()) {
				ERR_TYPE = ERR_LOGIC_ADVANCE;
			}

	}
	return GLogic_State;
}

int parseInput() {
    if (INPUT_BUTTONS == INPUT_LEFT) {
        BLOCK_POS--;
    } else if (INPUT_BUTTONS == INPUT_RIGHT) {
        BLOCK_POS++;
    }
    //can't really fail this
    return SUCCESS;
}
int updateBlockPos() {

}

int spawnBlock() {
    CURRENT_BLOCK.x = 0;
    CURRENT_BLOCK.y = LED_MATRIX_COL - 1 - 5; //0 indexed & first 5 rows are for score
    CURRENT_BLOCK.blockType = &BLOCK_MATRIX; //TODO: need to check if it will compile
}


/*
draws the
*/
int draw () {

}

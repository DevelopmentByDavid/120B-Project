/*
 * USART.c
 *
 * Created: 5/30/2018 11:50:34 PM
 * Author : David
 */

#include <avr/io.h>
#include <ucr/usart.h>
#include <ucr/scheduler.h>
#include <stdlib.h>
#include <ucr/timer.h>

//task declaration
task myTasks[6];
unsigned char myTasksSize = 6;
void resetGlobals();
unsigned long tasksPeriodGCD = 500;

#define INPUT_BUTTONS PORTA
#define INPUT_RESET 0x01
#define INPUT_PAUSE 0x02
#define INPUT_MOVE_LEFT 0x04
#define INPUT_MOVE_RIGHT 0x08
#define INPUT_ROTATE_LEFT 0x10
#define INPUT_ROTATE_RIGHT 0x20

//defining different types of pixels
#define EMPTY 0
#define PLACED_BLOCK_PIXEL 1
#define PIXEL 2
#define SCORE_PIXEL 3
#define CORNER_PIXEL_ABSENT 4
#define CORNER_PIXEL_PRESENT 5
#define CURRENT_BLOCK_PIXEL 6
#define CORNER_PIXEL_ABSENT_PLACED 7
#define END_MSG 8



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
//period
unsigned long GState_Period = 500;


enum LED_States {LED_Init, LED_Update} LED_State;
task LEDUpdate;
int LEDUpdate_Tick();
unsigned char UPDATE_SUCCESS;
#define FAILED 0
#define SUCCESS 1
//USART Prefix Messages so Arduino knows what type of information is inbound
//0 is the arduino's internal default number
#define PREFIX_UPDATE 1
#define PREFIX_WIN 2
#define PREFIX_PAUSE 3
#define PREFIX_LOSE 4
#define PREFIX_ERR 5
#define PREFIX_RESET 6
int updateLED();
int clearScreen();
unsigned long LED_Period = 500;



enum GLogic_States {GLogic_Init, GLogic_Input, GLogic_CheckRows, GLogic_SpawnBlock, GLogic_Advance} GLogic_State;
task GLogic;
int GLogic_Tick();
unsigned char LED_MATRIX[32][16] = {{0}}; //27x16 is where game takes place; extra shows score
#define LED_MATRIX_ROW 32 //screen size
#define LED_MATRIX_COL 16 //screen width
#define TETRIS_BOUND_ROW 27 //row boundary for the game
#define TETRIS_BOUND_COL 16 //col boundary for the game
#define BLOCK_ROW 5
#define BLOCK_COL 5
#define SCORE_ROW 5
#define SCORE_COL 16
unsigned char LOGIC_STATUS; //changes to either GAME OVER/WIN/NEITHER
unsigned char SCORE;//incremented by +1 per row cleared
unsigned char BLOCK_STATUS; //changes to either SPAWN or NO_SPAWN indicated whether to spawn a block or not
unsigned char CURRENT_BLOCK_INDEX;//is 0 when new block is spawned
unsigned char CURRENT_BLOCK[BLOCK_ROW][BLOCK_COL] = {{0}}; //re assigned when a new block is spawned; is the current block's _ALL array
unsigned char CURRENT_BLOCK_SIZE; //calculated when a new block is spawned (size of the block's _ALL array);
unsigned char RANDOM_BLOCK;
#define GAME_OVER 0
#define WIN 1
#define NEITHER 2
#define SPAWN 1
#define NO_SPAWN 0
#define INVALID_DRAW 1
#define VALID_DRAW 2
int parseInput(); //parses any input and decides what to do from there
unsigned char draw(signed char, signed char);
void checkRows(); //checks to see if any rows should be cleared and increments the score
void spawnBlock(); //spawns a block
void advance(); //advances the current block's position
void updateBlock(); //updates internal matrix
void translateLeft();
void translateRight();
void moveDown(unsigned char);
void connect();
void drawScore();
void clearScore();
unsigned long GLogic_Period = 500;


enum ERR_States {ERR_Init, ERR_Msg} Err_State;
task Err;
int ERR_Tick();
unsigned char ERR_TYPE; //other states may change the err type and this SM just yells if ERR_TYPE != NONE
#define NONE 0
#define LED 1
#define ERR_LOGIC_PARSE 2
#define ERR_LOGIC_CHECK 3
#define ERR_LOGIC_ADVANCE 4
#define ERR_LOGIC_SPAWN 5
#define GAME 6
#define ERR_BLOCK_POS 7
unsigned long ERR_Period = 500;

enum RAND_States {RAND_Init, RAND_Loop} RAND_State;
task RAND;
int RAND_Tick();
unsigned char SEED;
unsigned long RAND_Period = 1000;

enum MV_States {MV_Init, MV_Loop} MV_State;
task MV;
int MV_Tick();
unsigned long MV_Period = 500;




#define BLOCK_MATRIX_SIZE 1
#define HOOK_L_MATRIX_SIZE 4
#define HOOK_R_MATRIX_SIZE 4
#define Z_MATRIX_SIZE 2
#define Z_BACK_MATRIX_SIZE 2
#define LINE_MATRIX_SIZE 2
#define WASD_MATRIX_SIZE 4
unsigned char BLOCK_MATRIX_ALL[1][BLOCK_ROW][BLOCK_COL] = {
	{
		{CORNER_PIXEL_PRESENT,PIXEL},
		{PIXEL,PIXEL}
	}
};
unsigned char HOOK_L_MATRIX_ALL[4][BLOCK_ROW][BLOCK_COL] = {
	{
		{CORNER_PIXEL_PRESENT,EMPTY,EMPTY},
		{PIXEL,PIXEL,PIXEL}
	},
	{
		{CORNER_PIXEL_ABSENT,PIXEL},
		{EMPTY,PIXEL},
		{PIXEL,PIXEL}
	},
	{
		{CORNER_PIXEL_PRESENT,PIXEL,PIXEL},
		{EMPTY,EMPTY,PIXEL}
	},
	{
		{CORNER_PIXEL_PRESENT,PIXEL},
		{PIXEL,EMPTY},
		{PIXEL,EMPTY}
	},
};
unsigned char HOOK_R_MATRIX_ALL[4][BLOCK_ROW][BLOCK_COL] = {
	{
		{CORNER_PIXEL_ABSENT,EMPTY,PIXEL},
		{PIXEL,PIXEL,PIXEL}
	},
	{
		{CORNER_PIXEL_PRESENT,PIXEL},
		{EMPTY,PIXEL},
		{EMPTY,PIXEL}
	},
	{
		{CORNER_PIXEL_PRESENT,PIXEL,PIXEL},
		{PIXEL,EMPTY,EMPTY}
	},
	{
		{CORNER_PIXEL_PRESENT,EMPTY},
		{PIXEL,EMPTY},
		{PIXEL,PIXEL}
	}
};
unsigned char Z_MATRIX_ALL[2][BLOCK_ROW][BLOCK_COL] = {
	{
		{CORNER_PIXEL_PRESENT,PIXEL,EMPTY},
		{EMPTY,PIXEL,PIXEL}
	},
	{
		{CORNER_PIXEL_ABSENT,PIXEL},
		{PIXEL,PIXEL},
		{PIXEL,EMPTY}
	},
	
};
unsigned char Z_BACK_MATRIX_ALL[2][BLOCK_ROW][BLOCK_COL] = {
	{
		{CORNER_PIXEL_ABSENT,PIXEL,PIXEL},
		{PIXEL,PIXEL,EMPTY}
	},
	{
		{CORNER_PIXEL_PRESENT,EMPTY},
		{PIXEL,PIXEL},
		{EMPTY,PIXEL}
	}
};
unsigned char LINE_MATRIX_ALL[2][BLOCK_ROW][BLOCK_COL] = {
	{
		{CORNER_PIXEL_PRESENT,PIXEL,PIXEL,PIXEL}
	},
	{
		{CORNER_PIXEL_PRESENT},
		{PIXEL},
		{PIXEL},
		{PIXEL}
	}
};
unsigned char WASD_MATRIX_ALL[4][BLOCK_ROW][BLOCK_COL] = {
	{
		{CORNER_PIXEL_ABSENT,PIXEL,EMPTY},
		{PIXEL,PIXEL,PIXEL}
	},
	{
		{CORNER_PIXEL_ABSENT,PIXEL},
		{PIXEL,PIXEL},
		{EMPTY,PIXEL},
	},
	{
		{CORNER_PIXEL_PRESENT,PIXEL,PIXEL},
		{EMPTY,PIXEL,EMPTY}
	},
	{
		{CORNER_PIXEL_PRESENT,EMPTY},
		{PIXEL,PIXEL},
		{PIXEL,EMPTY},
	}
};
////BEGIN BLOCK_MATRIX_ALL ELEMENT DECLARATIONS
//unsigned char BLOCK_MATRIX[5][5] = {;
//
////BEGIN HOOK_L_MATRIX_ALL ELEMENT DECLARATIONS
////RIGHT = CCW
////LEFT = CW
//unsigned char HOOK_L_MATRIX_ONE[5][5] = ;
//unsigned char HOOK_L_MATRIX_TWO[5][5] = ;
//unsigned char HOOK_L_MATRIX_THREE[5][5] =;
//unsigned char HOOK_L_MATRIX_FOUR[5][5] = ;
////END HOOK_L_MATRIX_ALL ELEMENT DECLARATIONS
//
////BEGIN HOOK_R_MATRIX_ALL ELEMENT DECLARATIONS
////RIGHT = CCW
////LEFT = CW
//unsigned char HOOK_R_MATRIX_ONE[5][5] =;
//unsigned char HOOK_R_MATRIX_TWO[5][5] = ;
//unsigned char HOOK_R_MATRIX_THREE[5][5] = ;
//unsigned char HOOK_R_MATRIX_FOUR[5][5] =;
////END HOOK_R_MATRIX_ALL ELEMENT DECLARATIONS
//
////BEGIN Z_MATRIX_ALL ELEMENT DECLARATIONS
////RIGHT = CCW
////LEFT = CW
//unsigned char Z_MATRIX_ONE[5][5] = ;
//unsigned char Z_MATRIX_TWO[5][5] = ;
////END Z_MATRIX_ALL ELEMENT DECLARATIONS
//
////BEGIN Z_BACK_MATRIX_ALL ELEMENT DECLARATIONS
//unsigned char Z_BACK_MATRIX_ONE[5][5] = ;
//unsigned char Z_BACK_MATRIX_TWO[5][5] = ;
////END Z_BACK_MATRIX_ALL ELEMENT DECLARATIONS
//
//unsigned char LINE_MATRIX_ONE[5][5] = ;
//unsigned char LINE_MATRIX_TWO[5][5] = ;
//unsigned char WASD_MATRIX_ONE[5][5] = ;
//unsigned char WASD_MATRIX_TWO[5][5] = ;
//unsigned char WASD_MATRIX_THREE[5][5] = ;
//unsigned char WASD_MATRIX_FOUR[5][5] = ;



unsigned char ONE[5][1] = {
	{4},
	{4},
	{4},
	{4},
	{4},
};
unsigned char TWO[5][3] = {
	{4, 4, 4},
	{0, 0, 4},
	{4, 4, 4},
	{4, 0, 0},
	{4, 4, 4},
};
unsigned char THREE[5][3] = {
	{4, 4, 4},
	{0, 0, 4},
	{4, 4, 4},
	{0, 0, 4},
	{4, 4, 4},
};
unsigned char FOUR[5][3] = {
	{4, 0, 4},
	{4, 0, 4},
	{4, 4, 4},
	{0, 0, 4},
	{0, 0, 4},
};
unsigned char FIVE[5][3] = {
	{4, 4, 4},
	{4, 0, 0},
	{4, 4, 4},
	{0, 0, 4},
	{4, 4, 4},
};
unsigned char SIX[5][3] = {
	{4, 0, 0},
	{4, 0, 0},
	{4, 4, 4},
	{4, 0, 4},
	{4, 4, 4},
};
unsigned char SEVEN[5][3] = {
	{4, 4, 4},
	{0, 0, 4},
	{0, 0, 4},
	{0, 0, 4},
	{0, 0, 4}
};
unsigned char EIGHT[5][3] = {
	{4, 4, 4},
	{4, 0, 4},
	{4, 4, 4},
	{4, 0, 4},
	{4, 4, 4},
};
unsigned char NINE[5][3] = {
	{4, 4, 4},
	{4, 0, 4},
	{4, 4, 4},
	{0, 0, 4},
	{0, 0, 4},
};


int main(void)
{
    //PORT INITS
	DDRA = 0x00; PORTA = 0xFF; //INPUT
	DDRC = 0xFF; PORTC = 0x00;
	//DDRD = 0xFF; PORTD = 0x00;

    //USART INIT
	initUSART();
	
	//TASK INIT
	myTasks[0].state = G_Init;
	myTasks[0].period = GState_Period;
	myTasks[0].elapsedTime = 0;
	myTasks[0].TickFct = &Gstate_Tick;
	
	myTasks[1].state = LED_Init;
	myTasks[1].period = LED_Period;
	myTasks[1].elapsedTime = 0;
	myTasks[1].TickFct = &LEDUpdate_Tick;
	
	myTasks[2].state = GLogic_Init;
	myTasks[2].period = GLogic_Period;
	myTasks[2].elapsedTime = 0;
	myTasks[2].TickFct = &GLogic_Tick;
	
	myTasks[3].state = ERR_Init;
	myTasks[3].period = ERR_Period;
	myTasks[3].elapsedTime = 0;
	myTasks[3].TickFct = &ERR_Tick;
	
	myTasks[4].state = RAND_Init;
	myTasks[4].period = RAND_Period;
	myTasks[4].elapsedTime = 0;
	myTasks[4].TickFct = &RAND_Tick;
	
	myTasks[5].state = MV_Init;
	myTasks[5].period = MV_Period;
	myTasks[5].elapsedTime = 0;
	myTasks[5].TickFct = &MV_Tick;
	

    //FILL BLOCK MATRICES
    //BLOCK_MATRIX_ALL[0] = BLOCK_MATRIX;
    //HOOK_L_MATRIX_ALL[0] = HOOK_L_MATRIX_ONE;
	//HOOK_L_MATRIX_ALL[1] = HOOK_L_MATRIX_TWO;
	//HOOK_L_MATRIX_ALL[2] = HOOK_L_MATRIX_THREE;
	//HOOK_L_MATRIX_ALL[3] = HOOK_L_MATRIX_FOUR;
	//HOOK_R_MATRIX_ALL[0] = HOOK_R_MATRIX_ONE;
	//HOOK_R_MATRIX_ALL[1] = HOOK_R_MATRIX_TWO;
	//HOOK_R_MATRIX_ALL[2] = HOOK_R_MATRIX_THREE;
	//HOOK_R_MATRIX_ALL[3] = HOOK_R_MATRIX_FOUR;
	//Z_MATRIX_ALL[0] = Z_MATRIX_ONE;
	//Z_MATRIX_ALL[1] = Z_MATRIX_TWO;
	//Z_BACK_MATRIX_ALL[0] = Z_BACK_MATRIX_ONE;
	//Z_BACK_MATRIX_ALL[1] = Z_BACK_MATRIX_TWO;
	//LINE_MATRIX_ALL[0] = LINE_MATRIX_ONE;
	//LINE_MATRIX_ALL[1] = LINE_MATRIX_TWO;
	//WASD_MATRIX_ALL[0] = WASD_MATRIX_ONE;
	//WASD_MATRIX_ALL[1] = WASD_MATRIX_TWO;
	//WASD_MATRIX_ALL[2] = WASD_MATRIX_THREE;
	//WASD_MATRIX_ALL[3] = WASD_MATRIX_FOUR;
	
	//resets globals
	resetGlobals();
	clearScreen();
	TimerSet(tasksPeriodGCD);
	TimerOn();
	unsigned char u;

    /* Replace with your application code */
    while (1)
    {
		//SEED
		srand(SEED);
		//PORTC = 0x01;
		for (u = 0; u < myTasksSize; u++) { // Heart of the scheduler code
			if ( myTasks[u].elapsedTime >= myTasks[u].period ) { // Ready
				myTasks[u].state = myTasks[u].TickFct(myTasks[u].state);
				myTasks[u].elapsedTime = 0;
			}
			myTasks[u].elapsedTime += tasksPeriodGCD;
		}
		while(!TimerFlag) {}
		//PORTC = GLogic_State;
		TimerFlag = 0;
		
    }
}

void resetGlobals() {
	//GLOBAL INITS
    SEED = 0;
	ERR_TYPE = NONE;
	CURRENT_BLOCK_SIZE = 0;
	CURRENT_BLOCK_INDEX = 0;
	BLOCK_STATUS = SPAWN;
	SCORE = 0;
	LOGIC_STATUS = NEITHER;
	UPDATE_SUCCESS = SUCCESS;
	GAME_STATUS = IN_PROGRESS;
	LED_State = 0;
	G_State = 0;
	GLogic_State = 0;
	Err_State = 0;
	RAND_State = 0;
}

int Gstate_Tick() {
	switch (G_State) {
		case G_Init:
			G_State = G_InProgress;
			break;
		case G_InProgress:
			if (INPUT_BUTTONS == INPUT_RESET) {
				G_State = G_Reset;
			} else if (INPUT_BUTTONS == INPUT_PAUSE) {
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
			resetGlobals();
			clearScreen();
			break;
		default:
			//do nothing
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
			LED_State = LED_Update;
			break;
		default:
			LED_State = LED_Init;
	}
	switch (LED_State) {
		case LED_Update:
			UPDATE_SUCCESS = updateLED();
			break;
		default:
		//do nothing
		break;
	}
	return LED_State;
}

int clearScreen() {
	for (unsigned char i = 0; i < LED_MATRIX_ROW; i++) {
		for(unsigned char j = 0; j < LED_MATRIX_COL; j++) {
			LED_MATRIX[i][j] = EMPTY;
		}
	}
	//function to clear screen and init to 0 already exists on arduino
	//so it is not necessary to send this over USART
	return SUCCESS;
}

int updateLED() {
	//could send 8 LED'S at a time but just wanna get something working
	//so just gonna send 1 by 1.
	//convert to binary and send over USART
	if (USART_IsSendReady()) {
		USART_Send(PREFIX_UPDATE);
	}
	unsigned char temp = 0;
	for (unsigned char i = 0; i < LED_MATRIX_ROW; i++) {
		for(unsigned char j = 0; j < LED_MATRIX_COL; j++) {
			//if (USART_IsSendReady()) {
				USART_Send(LED_MATRIX[i][j]);
				//if (LED_MATRIX[i][j] == PLACED_BLOCK_PIXEL) {
					//PORTC = 0xFF;
				//}
			//}
			
			//PORTC = LED_MATRIX[0][0];
			
			//if (LED_MATRIX[i][j] != EMPTY) {
				//temp = (temp << 1) | 0x01;
			//} else {
				//temp = temp << 1 & 0xFE;
			//}
			////
			//if ( ((j + 1) % 8) == 0 && USART_IsSendReady()) {
				//USART_Send(0xFF);
				//PORTC = temp;
				//temp = 0x00;
			//}
			
			//if (USART_IsSendReady()) {
				//PORTC &= 0x0F;
				//if (LED_MATRIX[i][j] != EMPTY) {
					//USART_Send(1);
				//} else {
					//USART_Send(0);
				//}
			//} else {
				//PORTC |= 0xF0;
				//j--;
			//}
		}
	}
	return SUCCESS;
}

int INPUT_Tick() {
	
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
				BLOCK_STATUS = NO_SPAWN;
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
			parseInput();
			break;
		case GLogic_CheckRows:
			checkRows();
			break;
		case GLogic_SpawnBlock:
			spawnBlock();
			BLOCK_STATUS = NO_SPAWN;
			break;
		case GLogic_Advance:
			//advance();
			break;
		default:
			//do nothing;
			break;
	}
	return GLogic_State;
}

void connect() {
	for (unsigned char i = 0; i < TETRIS_BOUND_ROW; i++) {
		for(unsigned char j = 0; j < TETRIS_BOUND_COL; j++) {
			if (LED_MATRIX[i][j] != EMPTY &&
			 LED_MATRIX[i][j] != PLACED_BLOCK_PIXEL &&
			 LED_MATRIX[i][j] != SCORE_PIXEL) {
				LED_MATRIX[i][j] = PLACED_BLOCK_PIXEL;
			}
		}
	}
}

void advance() {
	for (unsigned char i = 0; i < TETRIS_BOUND_ROW; i++) {
		for(unsigned char j = 0; j < TETRIS_BOUND_COL; j++) {
			if (LED_MATRIX[i][j] == CORNER_PIXEL_PRESENT ||
			LED_MATRIX[i][j] == CORNER_PIXEL_ABSENT ||
			LED_MATRIX[i][j] == CORNER_PIXEL_ABSENT_PLACED) {
				if (draw(i - 1,j) != VALID_DRAW) {
					connect();
				}
				i = TETRIS_BOUND_ROW;
				j = TETRIS_BOUND_COL;
			}
		}
	}
}

void checkRows() {
	unsigned char myBool = 0; //0 if does not exist, 1 if exists
	for (unsigned char i = 0; i < TETRIS_BOUND_ROW; i++) {
		for(unsigned char j = 0; j < TETRIS_BOUND_COL; j++) {
			if (LED_MATRIX[i][j] != EMPTY) {
				myBool = 1;
				if (j == TETRIS_BOUND_COL - 1) {
					SCORE++;
					moveDown(i);
				}
			} else {
				myBool = 0;
				j = TETRIS_BOUND_COL;
			}
		}
		myBool = 0;
	}	
}


//moves blocks above it down when there is a score increment
void moveDown(unsigned char row) {
	for (unsigned char i = row - 1; i > 0; i--) {
		for(unsigned char j = 0; j < TETRIS_BOUND_COL; j++) {
			if (i > 0) {
				LED_MATRIX[i][j] = LED_MATRIX[i + 1][j];
			} else  if (i == 1) {
				LED_MATRIX[i][j] = EMPTY;
			}
		}
	}
}

int RAND_Tick() {
	switch (RAND_State) {
		case RAND_Init:
			SEED = 0;
			RAND_State = RAND_Loop;
			break;
		case RAND_Loop:
			RAND_State = RAND_Loop;
			break;
		default:
			RAND_State = RAND_Init;
	}
	
	switch (RAND_State) {
		case RAND_Loop:
			if (SEED == 128) {
				SEED = 0;
			}
			SEED++;
			break;
		default:
		//do nothing
		break;
	}
	
	return RAND_State;
}

int parseInput() {
    if (INPUT_BUTTONS == INPUT_ROTATE_LEFT) {
        CURRENT_BLOCK_INDEX--;
		updateBlock();
    } else if (INPUT_BUTTONS == INPUT_ROTATE_RIGHT) {
        CURRENT_BLOCK_INDEX++;
		updateBlock();
    } else if (INPUT_BUTTONS == INPUT_MOVE_LEFT) {
		translateLeft();
	} else if (INPUT_BUTTONS == INPUT_MOVE_RIGHT) {
		translateRight();
	}
    //can't really fail this
    return SUCCESS;
}

void translateRight() {
	signed char rowArg = -1;
	signed char colArg = -1;
	for (unsigned char i = 0; i < LED_MATRIX_ROW; i++) {
		for (unsigned char j = 0; j < LED_MATRIX_COL; j++) {
			if (LED_MATRIX[i][j] == CORNER_PIXEL_PRESENT || LED_MATRIX[i][j] == CORNER_PIXEL_ABSENT || LED_MATRIX[i][j] == CORNER_PIXEL_ABSENT_PLACED) {
				rowArg = i;
				colArg = j;
				i = LED_MATRIX_ROW;
				j = LED_MATRIX_COL;
				
			}
		}
	}
	draw(rowArg, colArg + 1);
}

void translateLeft() {
	signed char rowArg = -1;
	signed char colArg = -1;
	for (unsigned char i = 0; i < LED_MATRIX_ROW; i++) {
		for (unsigned char j = 0; j < LED_MATRIX_COL; j++) {
			if (LED_MATRIX[i][j] == CORNER_PIXEL_PRESENT || LED_MATRIX[i][j] == CORNER_PIXEL_ABSENT || LED_MATRIX[i][j] == CORNER_PIXEL_ABSENT_PLACED) {
				rowArg = i;
				colArg = j;
				i = LED_MATRIX_ROW;
				j = LED_MATRIX_COL;
				
			}
		}
	}
	draw(rowArg, colArg - 1);
}

void spawnBlock() {
	CURRENT_BLOCK_INDEX = 0;
	RANDOM_BLOCK = rand() % 7 + 1;
	//PORTC = RANDOM_BLOCK;
	updateBlock();
	draw(0,0);
}

void updateBlock() {
	switch(RANDOM_BLOCK) {
		case 1:
			if (CURRENT_BLOCK_INDEX == BLOCK_MATRIX_SIZE - 1) {
				CURRENT_BLOCK_INDEX = 0;
			} else if (CURRENT_BLOCK_INDEX < 0) {
				CURRENT_BLOCK_INDEX = BLOCK_MATRIX_SIZE -1;
			}
			for (unsigned char i = 0; i < BLOCK_ROW; i++) {
				for(unsigned char j = 0; j < BLOCK_COL; j++) {
					CURRENT_BLOCK[i][j] = BLOCK_MATRIX_ALL[CURRENT_BLOCK_INDEX][i][j];
				}
			}
			break;
		case 2:
			if (CURRENT_BLOCK_INDEX == HOOK_L_MATRIX_SIZE - 1) {
				CURRENT_BLOCK_INDEX = 0;
			}  else if (CURRENT_BLOCK_INDEX < 0) {
				CURRENT_BLOCK_INDEX = HOOK_L_MATRIX_SIZE -1;
			}
			for (unsigned char i = 0; i < BLOCK_ROW; i++) {
				for(unsigned char j = 0; j < BLOCK_COL; j++) {
					CURRENT_BLOCK[i][j] = HOOK_L_MATRIX_ALL[CURRENT_BLOCK_INDEX][i][j];
				}
			}
		break;
		case 3:
			if (CURRENT_BLOCK_INDEX == HOOK_R_MATRIX_SIZE - 1) {
				CURRENT_BLOCK_INDEX = 0;
			} else if (CURRENT_BLOCK_INDEX < 0) {
				CURRENT_BLOCK_INDEX = HOOK_R_MATRIX_SIZE -1;
			}
			for (unsigned char i = 0; i < BLOCK_ROW; i++) {
				for(unsigned char j = 0; j < BLOCK_COL; j++) {
					CURRENT_BLOCK[i][j] = HOOK_R_MATRIX_ALL[CURRENT_BLOCK_INDEX][i][j];
				}
			}
		break;
		case 4:
			if (CURRENT_BLOCK_INDEX ==	Z_MATRIX_SIZE - 1) {
				CURRENT_BLOCK_INDEX = 0;
			} else if (CURRENT_BLOCK_INDEX < 0) {
				CURRENT_BLOCK_INDEX = Z_MATRIX_SIZE -1;
			}
			for (unsigned char i = 0; i < BLOCK_ROW; i++) {
				for(unsigned char j = 0; j < BLOCK_COL; j++) {
					CURRENT_BLOCK[i][j] = Z_MATRIX_ALL[CURRENT_BLOCK_INDEX][i][j];
				}
			}
		break;
		case 5:
			if (CURRENT_BLOCK_INDEX == Z_BACK_MATRIX_SIZE - 1) {
				CURRENT_BLOCK_INDEX = 0;
			} else if (CURRENT_BLOCK_INDEX < 0) {
				CURRENT_BLOCK_INDEX = Z_BACK_MATRIX_SIZE -1;
			}
			for (unsigned char i = 0; i < BLOCK_ROW; i++) {
				for(unsigned char j = 0; j < BLOCK_COL; j++) {
					CURRENT_BLOCK[i][j] = Z_BACK_MATRIX_ALL[CURRENT_BLOCK_INDEX][i][j];
				}
			}
		break;
		case 6:
			if (CURRENT_BLOCK_INDEX == LINE_MATRIX_SIZE - 1) {
				CURRENT_BLOCK_INDEX = 0;
			} else if (CURRENT_BLOCK_INDEX < 0) {
				CURRENT_BLOCK_INDEX = LINE_MATRIX_SIZE -1;
			}
			for (unsigned char i = 0; i < BLOCK_ROW; i++) {
				for(unsigned char j = 0; j < BLOCK_COL; j++) {
					CURRENT_BLOCK[i][j] = LINE_MATRIX_ALL[CURRENT_BLOCK_INDEX][i][j];
					PORTC = i;
				}
			}
		break;
		case 7:
			if (CURRENT_BLOCK_INDEX == WASD_MATRIX_SIZE - 1) {
				CURRENT_BLOCK_INDEX = 0;
			} else if (CURRENT_BLOCK_INDEX < 0) {
				CURRENT_BLOCK_INDEX = WASD_MATRIX_SIZE -1;
			}
			for (unsigned char i = 0; i < BLOCK_ROW; i++) {
				for(unsigned char j = 0; j < BLOCK_COL; j++) {
					CURRENT_BLOCK[i][j] = WASD_MATRIX_ALL[CURRENT_BLOCK_INDEX][i][j];
				}
			}
		break;
	}
}

/*
draws the rotation or block if it is valid
rowArg is highest 2 in the block
colArg is left most 2 in the block
*/
unsigned char draw (signed char rowArg, signed char colArg) {
	unsigned char temp = VALID_DRAW; 
	//checks for invalid draw first
	for (unsigned char i = 0; i < BLOCK_ROW; i++) {
		for(unsigned char j = 0; j < BLOCK_COL; j++) {
			if (LED_MATRIX[rowArg + i][colArg + j] == PLACED_BLOCK_PIXEL && (CURRENT_BLOCK[i][j] == PIXEL ||
			CURRENT_BLOCK[i][j] == CORNER_PIXEL_PRESENT)) {
				temp = INVALID_DRAW;
				i = BLOCK_ROW;
				j = BLOCK_COL;
			} else if (rowArg + i >= TETRIS_BOUND_ROW || colArg + j >= TETRIS_BOUND_COL) {
				temp = INVALID_DRAW;
				i = BLOCK_ROW;
				j = BLOCK_COL;
			}
		}
	}
	//if not an invalid draw, then actually draw it
	if (temp != INVALID_DRAW) {
		//clear all CURRENT_BLOCK_PIXELS
		for (unsigned char i = 0; i < LED_MATRIX_ROW; i++) {
			for(unsigned char j = 0; j < LED_MATRIX_COL; j++) {
				if (LED_MATRIX[i][j] == CURRENT_BLOCK_PIXEL ||
				LED_MATRIX[i][j] == CORNER_PIXEL_ABSENT ||
				LED_MATRIX[i][j] == CORNER_PIXEL_PRESENT) {
					LED_MATRIX[i][j] = EMPTY;
				} else if (LED_MATRIX[i][j] == CORNER_PIXEL_ABSENT_PLACED) {
					LED_MATRIX[i][j] = PLACED_BLOCK_PIXEL;
				}
			}
		}
		//draw corner block only for now
		if (LED_MATRIX[rowArg][colArg] == PLACED_BLOCK_PIXEL) {
			LED_MATRIX[rowArg][colArg] = CORNER_PIXEL_ABSENT_PLACED;
		} else {
			LED_MATRIX[rowArg][colArg] = CURRENT_BLOCK[0][0];
		}
		//DRAW NEW BLOCK
		for (unsigned char i = 0; i < BLOCK_ROW; i++) {
			for(unsigned char j = 0; j < BLOCK_COL; j++) {
				if (/*LED_MATRIX[rowArg + i][colArg + j] == EMPTY &&*/ CURRENT_BLOCK[i][j] == PIXEL) {
					LED_MATRIX[rowArg + i][colArg + j] = CURRENT_BLOCK_PIXEL;
					//PORTC = 0xFF;
				}
				 //else if (LED_MATRIX[rowArg + i][colArg + j] == CORNER_PIXEL_ABSENT && CURRENT_BLOCK[i][j] == CORNER_PIXEL_PRESENT) {
					//LED_MATRIX[rowArg + i][colArg + j] = CORNER_PIXEL_PRESENT;
				//} else if (LED_MATRIX[rowArg + i][colArg + j] == PLACED_BLOCK_PIXEL && CURRENT_BLOCK[i][j] == CORNER_PIXEL_ABSENT) {
					//LED_MATRIX[rowArg + i][colArg + j] = CORNER_PIXEL_ABSENT_PLACED;
				//}
			}
		}
	}
	return temp;
}

void clearScore() {
	for (unsigned char i = TETRIS_BOUND_ROW; i < LED_MATRIX_ROW; i++) {
		for(unsigned char j = 0; j < SCORE_COL; j++) {
			if (LED_MATRIX[i][j] != EMPTY) {
				LED_MATRIX[i][j] = EMPTY;
			}
		}
	}
}

void drawScore() {
	clearScore();
	int whole =  (int) SCORE/10;
	int remainder = SCORE % 10;
	for (unsigned char i = TETRIS_BOUND_ROW; i < LED_MATRIX_ROW; i++) {
		for(unsigned char j = 0; j < SCORE_COL; j++) {
			if (LED_MATRIX[i][j] != EMPTY) {
				LED_MATRIX[i][j] = EMPTY;
			}
		}
	}
}

int MV_Tick() {
	switch (MV_State) {
		case MV_Init:
			MV_State = MV_Loop;
			break;
		case MV_Loop:
			MV_State = MV_Loop;
			break;
		default:
			MV_State = MV_Init;
			break;
	}
	switch (MV_State) {
		case MV_Init:
			//MV_State = MV_Loop;
			break;
		case MV_Loop:
			advance();
			break;
		default:
			//MV_State = MV_Init;
			break;
	}
	return MV_State;
}

int ERR_Tick() {
	return Err_State;
}


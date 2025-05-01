#ifndef MYAI_INCLUDED
#define MYAI_INCLUDED 

#include "../4T_header.h"
#include <math.h>

using std::stoi;
using std::string;

#define BOARD_SIZE 6
#define COMMAND_NUM 7

class MyAI  
{
	const char* commands_name[COMMAND_NUM] = {
		"name",
		"version",
		"time_setting",
		"board_setting",
		"ini",
		"get",
		"exit"
	};
public:
	MyAI(void);
	~MyAI(void);

	void Ini(const char* data[], char* response);
	void Set(char* response);
	void Get(const char* data[], char* response);
	void Exit(const char* data[], char* response);

private:
	bool p1_exist[PIECES], p2_exist[PIECES];
	int player;
	int p1_time, p2_time;
	int board_size;
	int dice;
	int board[BOARD_SIZE][BOARD_SIZE];
	int p1_piece_num, p2_piece_num;
	char piece_colors[PIECES * 2]; //0~7: p1, 8~15: p2
	int piece_pos[PIECES * 2];

	// Board
	void Init_board_state(char* position);
	void Set_board(char* position);
	void Print_chessboard();
	void Generate_move(char* move);
};

#endif
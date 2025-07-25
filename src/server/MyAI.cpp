#include "../4T_header.h"
#include "MyAI.h"
#include "../ismcts.hpp"
// 連線server主程式
DATA data;
GST game;
ISMCTS ismcts(10000);

// =============================
// MyAI 建構子/解構子
// =============================
MyAI::MyAI(void){
    data.init_data();           // 初始化4-tuple資料
    data.read_data_file(500000);// 讀取資料
}

MyAI::~MyAI(void){}

// =============================
// Ini：初始化玩家、設定初始棋盤
// =============================
void MyAI::Ini(const char* data[], char* response)
{
    // 設置玩家
    if(!strcmp(data[2], "1"))
    {
        player = USER;
    }
    else if(!strcmp(data[2], "2"))
    {
        player = ENEMY;
    }

    char position[16];
    Init_board_state(position); // 產生初始棋盤
    
    // P2 的棋子位置
    snprintf(response, 50, "%c%c %c%c %c%c %c%c %c%c %c%c %c%c %c%c", 
             position[0], position[1], position[2], position[3],
             position[4], position[5], position[6], position[7],
             position[8], position[9], position[10], position[11], 
             position[12], position[13], position[14], position[15]);
}

// =============================
// Set：隨機選擇紅棋
// =============================
auto now = std::chrono::high_resolution_clock::now();
auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();

void MyAI::Set(char* response){        //設定紅棋
    std::mt19937 generator(nanos);
    std::string pieces = "ABCDEFGH";
    std::shuffle(pieces.begin(), pieces.end(), generator);

    std::string redString = pieces.substr(0, 4);

    snprintf(response, 50, "SET:%s\r\n", redString.c_str());

    // snprintf(response, 50, "SET:ABDH");
}

// =============================
// Get：取得移動
// =============================
void MyAI::Get(const char* data[], char* response)      //get move
{    
    // set dice & board
    char position[49];  //3*16+1
    position[0] = '\0';
    snprintf(position, sizeof(position), "%s", data[0] + 4);
    Set_board(position); // 設定棋盤

    // generate move
    char move[50];
    Generate_move(move); // 產生移動
    snprintf(response, 50, "MOV:%s", move); 
}

// =============================
// Exit：離開
// =============================
void MyAI::Exit(const char* data[], char* response)
{
    fprintf(stderr, "Bye~\n");
}

// *********************** AI 函數 *********************** //

// =============================
// Init_board_state：產生初始棋盤狀態
// =============================
void MyAI::Init_board_state(char* position)
{
    int order[PIECES] = {0, 1, 2, 3, 4, 5, 6, 7}; // A~H
    std::string p2_init_position = "4131211140302010";
    std::string p1_init_position = "1424344415253545";

    for(int i = 0; i < PIECES; i++)
    {
        if(player == USER)
        {
            position[order[i] * 2] = p1_init_position[i * 2];
            position[order[i] * 2 + 1] = p1_init_position[i * 2 + 1];
        }
        else if(player == ENEMY)
        {
            position[order[i] * 2] = p2_init_position[i * 2];
            position[order[i] * 2 + 1] = p2_init_position[i * 2 + 1];
        }
    }
}

// =============================
// Set_board：根據 position 設定棋盤
// =============================
void MyAI::Set_board(char* position)
{
    memset(board, -1, sizeof(board));
	memset(piece_colors,'0',sizeof(piece_colors));
    memset(p2_exist, 1, sizeof(p2_exist));
    memset(p1_exist, 1, sizeof(p1_exist));
    p2_piece_num = PIECES;
    p1_piece_num = PIECES;

    for(int i = 0; i < PIECES * 2; i++)
    {
        int index = i * 3;      //座標與顏色

        if(position[index] == '9' && position[index + 1] == '9')
        {
            // P1
            if(i < PIECES) 
            {
                p1_piece_num--;
                p1_exist[i] = 0;
            }
            // P2
            else 
            {
                p2_piece_num--;
                p2_exist[i - PIECES] = 0;
            }
            piece_pos[i] = -1;
        }
        // 0~7: P1 pieces; 8~15: P2 pieces
        else
        {
            board[position[index + 1] - '0'][position[index] - '0'] = i;
            piece_pos[i] = (position[index + 1] - '0') * BOARD_SIZE + (position[index] - '0');
        }
        piece_colors[i] = position[index + 2];  // 設定棋子顏色
    }
    
    game.set_board(position); // 設定 GST 棋盤
    // game.record_board();

    Print_chessboard(); // 印出棋盤
}

// =============================
// Print_chessboard：印出棋盤狀態
// =============================
void MyAI::Print_chessboard()
{
    fprintf(stderr, "\n");
    // 0~7 represent P1 pieces; 8~15 represent P2 pieces
    for(int i = 0; i < BOARD_SIZE; i++)
    {
        fprintf(stderr, "<%d>", i);
        for(int j = 0; j < BOARD_SIZE; j++)
        {
            if(board[i][j] == -1) fprintf(stderr, "   -");
            else if(board[i][j] < PIECES) {
                fprintf(stderr, "%4c", board[i][j] + 'A');
            }
            else fprintf(stderr, "%4c", board[i][j] - PIECES + 'a'); // 小寫代表 P2 棋子
        }
        fprintf(stderr, "\n");
    }
    fprintf(stderr, "\n     ");
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        fprintf(stderr, "<%d> ", i);
    }
    fprintf(stderr, "\n\n");
    fprintf(stderr, "The number of P1 pieces: %d\nThe number of P2 pieces: %d\n", p1_piece_num, p2_piece_num);
}

// =============================
// Generate_move：產生最佳移動
// =============================
void MyAI::Generate_move(char* move)
{
    // int best_move = game.highest_weight(data); //純粹4tuple決定下一步
    int best_move = ismcts.findBestMove(game, data); // 使用 ISMCTS 找到最佳移動 (目前是4tuple + ISMCTS)
    
    int piece = best_move >> 4;
    int direction = best_move & 0xf;
    
    char piece_char = piece + 'A';
    
    const char* dir_str;
    switch(direction) {
        case 0: dir_str = "NORTH"; break;
        case 1: dir_str = "WEST"; break;
        case 2: dir_str = "EAST"; break;
        case 3: dir_str = "SOUTH"; break;
        default: dir_str = "UNKNOWN"; break;
    }

    snprintf(move, 50, "%c,%s", piece_char, dir_str);
    
    game.do_move(best_move); // 執行移動

}
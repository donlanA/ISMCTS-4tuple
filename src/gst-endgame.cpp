#define _CRT_RAND_S
#include <stdlib.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <map>
#include <iostream>
#include "gst-endgame.hpp"
#include "ismcts.hpp"
#include "mcts.hpp" 

#include <time.h>
#include <random>
#include <chrono>
// #include <unistd.h>
#include <iomanip>

static std::map<char, int> piece_index = {
    {'A', 0}, {'B', 1}, {'C', 2}, {'D', 3}, {'E', 4}, {'F', 5}, {'G', 6}, {'H', 7},
    {'a', 8}, {'b', 9}, {'c', 10}, {'d', 11}, {'e', 12}, {'f', 13}, {'g', 14}, {'h', 15}
};
static std::map<char, int> dir_index = {{'N', 0}, {'W', 1}, {'E', 2}, {'S', 3}};
static std::map<int, char> print_piece = {
    {0, 'A'}, {1, 'B'}, {2, 'C'}, {3, 'D'}, {4, 'E'}, {5, 'F'}, {6, 'G'}, {7, 'H'},
    {8, 'a'}, {9, 'b'}, {10, 'c'}, {11, 'd'}, {12, 'e'}, {13, 'f'}, {14, 'g'}, {15, 'h'}
};
static const int init_pos[2][PIECES] = {{25,26,27,28,31,32,33,34}, {10,9,8,7,4,3,2,1}};
static const int dir_val[4] = {-COL, -1, 1, COL};

void GST::init_board(){
    /*
        A  B  C  D  E  F  G  H  a  b  c  d  e  f  g  h
        0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15  <-piece index, used in pos and color
       25 26 27 28 31 32 33 34 10  9  8  7  4  3  2  1  <-position on board
    */

    memset(board, 0, sizeof(board));
    memset(pos, -1, sizeof(pos));
    memset(revealed, false, sizeof(revealed));
    for(int i=0; i<ROW*COL; i++) piece_board[i] = -1;
    for(int i=0; i<4; i++) piece_nums[i] = 0;
    for(int i=0; i<PIECES; i++) {   //set color
        color[i] = BLUE;
        color[i+8] = -BLUE;
    }
    nowTurn = USER;
    winner = -1;
    n_plies = 0;

    printf("請輸入自訂盤面（6x6，使用 A-H, a-h 表示棋子，0 表示空位）：\n");
    for (int row = 0; row < ROW; row++) {
        for (int col = 0; col < COL; col++) {
            char ch;
            scanf(" %c", &ch);
            int index = row * COL + col;
            if (ch == '0') continue;
            if (piece_index.find(ch) != piece_index.end()) {
                int p = piece_index[ch];
                if (pos[p] != -1) {
                    printf("錯誤：重複輸入 %c，請重新輸入\n", ch);
                    exit(1);
                }
                pos[p] = index;
                piece_board[index] = p;
                board[index] = (p < PIECES) ? 1 : -1;
            } else {
                printf("錯誤：%c 是無效的代號，請重新輸入\n", ch);
                exit(1);
            }
        }
    }
    for (int i = 0; i < PIECES * 2; i++) {
        if (pos[i] == -1) {
            revealed[i] = true;
        }
    }
    // 設置紅色棋子 
    char red[4] = {'A','B','D','F'}; // 用戶
    char red2[4] = {'a','b','d','f'}; // 敵人
    printf("預設紅色幽靈: A B D F (玩家1) 和 a b d f (玩家2)\n");
    
    for(int i=0; i<4; i++){
        color[piece_index[red[i]]] = RED;
        color[piece_index[red2[i]]] = -RED;
        
    }
    
    for(int i=0; i<PIECES; i++) {
        revealed[i] = true;
        
    }

    // // USER 紅色幽靈設定
    // char red[4];
    // bool used_red[PIECES] = {};
    // printf("請設定 USER 的紅色幽靈（A-H 中選四個）：\n");
    // for (int i = 0; i < 4; i++) {
    //     scanf(" %c", &red[i]);
    //     if (red[i] < 'A' || red[i] > 'H') {
    //         printf("錯誤：%c 不是有效的紅棋\n", red[i]);
    //         exit(1);
    //     }
    //     int idx = piece_index[red[i]];
    //     if (used_red[idx]) {
    //         printf("錯誤：重複設定紅棋 %c\n", red[i]);
    //         exit(1);
    //     }
    //     used_red[idx] = true;
    //     color[idx] = RED;

    //     // 標記玩家的棋子從開始就被揭露
    //     revealed[idx] = true; 
    // }

    // // ENEMY 紅色幽靈設定
    // char red2[4];
    // bool used_red2[PIECES] = {};
    // printf("請設定 ENEMY 的紅色幽靈（a-h 中選四個）：\n");
    // for (int i = 0; i < 4; i++) {
    //     scanf(" %c", &red2[i]);
    //     if (red2[i] < 'a' || red2[i] > 'h') {
    //         printf("錯誤：%c 不是有效的紅棋\n", red2[i]);
    //         exit(1);
    //     }
    //     int idx = piece_index[red2[i]];
    //     if (used_red2[idx - 8]) {
    //         printf("錯誤：重複設定紅棋 %c\n", red2[i]);
    //         exit(1);
    //     }
    //     used_red2[idx - 8] = true;
    //     color[idx] = -RED;
    // }

    // 重算各方棋子數
    for (int i = 0; i < PIECES * 2; i++) {
        if (pos[i] == -1) continue;
        if (color[i] == RED) piece_nums[0]++;
        else if (color[i] == BLUE) piece_nums[1]++;
        else if (color[i] == -RED) piece_nums[2]++;
        else if (color[i] == -BLUE) piece_nums[3]++;
    }

    return;
}


void GST::print_board(){    //print the board now & print User's remain chess & print eaten Enemy's chess
    for (int i = 0; i < ROW; i++) {
        for (int j = 0; j < COL; j++) {
            if(piece_board[i * ROW + j] != -1) 
                printf("%4c", print_piece[piece_board[i * ROW + j]]);
            else if(i == 0 && j == 0) 
                printf("%4c", '<');
            else if(i == 0 && j == COL-1) 
                printf("%4c", '>');
            else 
                printf("%4c", '-');
        }
        printf("\n");
    }
    printf("\n");
    printf("User remaining ghosts: ");
    for(int i=0; i<PIECES; i++){
        if(pos[i] != -1) printf("%c: %s ", print_piece[i], color[i]==RED ? "red":"blue");
    }
    printf("\n");
    printf("Eaten enemy ghosts: ");
    for(int i=8; i<PIECES*2; i++){
        if(pos[i] == -1) printf("%c: %s ", print_piece[i], color[i]==-RED ? "red":"blue");
    }
    printf("\n");
}

int GST::gen_move(int* move_arr, int piece, int location, int& count){  //generate the possible step
    int row = location / ROW;
    int col = location % COL;
    
    if(nowTurn == USER){
        if(row != 0 && board[location-6] <= 0) move_arr[count++] = piece << 4;            //up
        if(row != ROW-1 && board[location+6] <= 0) move_arr[count++] = piece << 4 | 3;    //down
        if(col != 0 && board[location-1] <= 0) move_arr[count++] = piece << 4 | 1;        //left
        if(col != COL-1 && board[location+1] <= 0) move_arr[count++] = piece << 4 | 2;    //right
        if(color[piece] == BLUE){                                                         //exit move  
            if(location == 0) move_arr[count++] = piece << 4 | 1;
            if(location == 5) move_arr[count++] = piece << 4 | 2;
        }
    }else{
        if(row != 0 && board[location-6] >= 0) move_arr[count++] = piece << 4;            
        if(row != ROW-1 && board[location+6] >= 0) move_arr[count++] = piece << 4 | 3;    
        if(col != 0 && board[location-1] >= 0) move_arr[count++] = piece << 4 | 1;        
        if(col != COL-1 && board[location+1] >= 0) move_arr[count++] = piece << 4 | 2;
        if(color[piece] == -BLUE){
            if(location == 30) move_arr[count++] = piece << 4 | 1;
            if(location == 35) move_arr[count++] = piece << 4 | 2;
        }
    }
    return count;   //the number of possible step
}

int GST::gen_all_move(int* move_arr){   //gernerate all posibility of chess step
    int count = 0;
    int offset = nowTurn == ENEMY ? PIECES : 0;
    int* nowTurn_pos = pos + offset;

    for(int i=0; i<PIECES; i++){
        if(pos[i+offset] != -1){
            gen_move(move_arr, i+offset, nowTurn_pos[i], count);
        }
    }

    return count;
}

bool check_win_move(int location, int dir){     //if chess is in corner, check next move will win or not
    if(location == 0 || location == 30) return dir == 1 ? true : false;
    else if(location == 5 || location == 35) return dir == 2 ? true : false;
    return false;
}

void GST::do_move(int move){    //move chess
    int piece = move >> 4;
    int direction = move & 0xf;

    if(abs(color[piece]) == BLUE){
        if(check_win_move(pos[piece], direction)){
            winner = nowTurn;
            n_plies++;
            nowTurn ^= 1;
            is_escape = true;
            return;
        }
    }
    if (n_plies == 1000) {
        fprintf(stderr, "cannot do anymore moves\n");
        exit(1);
    }

    int dst = pos[piece] + dir_val[direction];
    //dst: the chess's location after move / pos: the location of chess / dir_val: up down left right
    
    if(board[dst] < 0){     //Enemy's color
        pos[piece_board[dst]] = -1;     //chess is eaten
        move |= piece_board[dst] << 8;
        revealed[piece_board[dst]] = true;
        if(color[piece_board[dst]] == -RED) piece_nums[2] -= 1; //check the remain color of red & blue chess
        else if(color[piece_board[dst]] == -BLUE) piece_nums[3] -= 1;
        else if(color[piece_board[dst]] == -UNKNOWN){}  //先什麼都不做
        else{
            fprintf(stderr, "do_move error, eaten color wrong!\n");
            exit(1);
        }
    }
    else if(board[dst] > 0){    //User's color
        pos[piece_board[dst]] = -1;
        move |= piece_board[dst] << 8;
        revealed[piece_board[dst]] = true;
        if(color[piece_board[dst]] == RED) piece_nums[0] -= 1;
        else if(color[piece_board[dst]] == BLUE) piece_nums[1] -= 1;
        else if(color[piece_board[dst]] == UNKNOWN){}  //先什麼都不做
        else{
            fprintf(stderr, "do_move error, eaten color wrong!\n");
            exit(1);
        }
    }
    else{
        move |= 0x1000;
    }

    board[pos[piece]] = 0;          //set 0 at the location which stay before => space: color = 0
    piece_board[pos[piece]] = -1;   //set 0 at the location which stay before => space: no chess
    board[dst] = color[piece];      //color the chess color at the location after move
    piece_board[dst] = piece;       //set chess number at the location after move
    pos[piece] = dst;               //the location of chess now
    history[n_plies++] = move;
    nowTurn ^= 1; //change player
}

bool GST::is_over(){    //game end or not => the number of remain chess color
    // if(n_plies >= 200) {
    //     winner = -2; // -2表示平局
    //     return true;
    // }
    if(winner != -1) return true;
    else{
        if(piece_nums[0] == 0 || piece_nums[3] == 0){
            winner = USER;
            return true;
        }
        else if(piece_nums[1] == 0 || piece_nums[2] == 0){
            winner = ENEMY;
            return true;
        }
    }
    return false;
}

int move_index = 0;

int main(){
    // 為Mac初始化隨機數生成
    std::random_device rd;
    std::mt19937 gen(std::chrono::system_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<> dist(0, 7);
    
    GST game;
    MCTS mcts(10000);
    ISMCTS ismcts(10000);
    
    bool my_turn = true;
    
    // 初始化棋盤
    game.init_board();
    game.print_board();
    
    std::cout << "\n===== 遊戲開始 =====\n\n";
    
    // 主遊戲循環
    while(!game.is_over()){
        if(my_turn){
            std::cout << "Player 1 (ISMCTS) 思考中...\n";
            int move = ismcts.findBestMove(game);
            if (move == -1) {
                std::cout << "MCTS 無法找到有效移動，玩家1可能已經輸了！\n";
                break;
            }
            
            // 直接執行移動
            game.do_move(move);
        } else {
            std::cout << "Player 2 (MCTS) 思考中...\n";
            int move = mcts.findBestMove(game);
            if (move == -1) {
                std::cout << "ISMCTS 無法找到有效移動，玩家2可能已經輸了！\n";
                break;
            }
            
            // 執行移動
            game.do_move(move);
        }

        // 顯示當前棋盤狀態
        game.print_board();
        
        // 顯示當前回合數
        std::cout << "當前回合數: " << game.n_plies << std::endl;
        
        // 切換玩家
        my_turn = !my_turn;
    }
    
    // 宣布勝者
    int winner = game.get_winner();
    if(winner == -2) {
        printf("遊戲結束！達到20回合，判定為平局！\n");
    } else {
        printf("遊戲結束！%s 獲勝！\n", winner ? "Player 2 (MCTS)" : "Player 1 (ISMCTS)");
        
        // 顯示勝利原因
        if(game.is_escape) {
            printf("勝利方式：藍色棋子成功逃脫！\n");
        } else if(game.piece_nums[0] == 0) {
            printf("勝利方式：Player 1 的紅色棋子全部被吃光！\n");
        } else if(game.piece_nums[2] == 0) {
            printf("勝利方式：Player 2 的紅色棋子全部被吃光！\n");
        } else if(game.piece_nums[1] == 0) {
            printf("勝利方式：Player 1 的藍色棋子全部被吃光！\n");
        } else if(game.piece_nums[3] == 0) {
            printf("勝利方式：Player 2 的藍色棋子全部被吃光！\n");
        }
    }
    
    return 0;
}
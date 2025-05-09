#define _CRT_RAND_S
#include <stdlib.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <map>
#include <iostream>
#include "gst.hpp"
#include "ismcts.hpp"
#include "mcts.hpp" 

#include <time.h>
#include <random>
#include <chrono>
#include <unistd.h>
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

//initialize data
void GST::init_board(){
    /*
        A  B  C  D  E  F  G  H  a  b  c  d  e  f  g  h
        0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15  <-piece index, used in pos and color
       25 26 27 28 31 32 33 34 10  9  8  7  4  3  2  1  <-position on board
    */

    memset(board, 0, sizeof(board));
    memset(pos, 0, sizeof(pos));
    memset(revealed, false, sizeof(revealed));
    for(int i=0; i<ROW*COL; i++) piece_board[i] = -1;
    for(int i=0; i<4; i++) piece_nums[i] = 4;
    for(int i=0; i<PIECES; i++) {   //set color
        color[i] = BLUE;
        color[i+8] = -BLUE;
    }
    nowTurn = USER;
    winner = -1;
    n_plies = 0;

    // random set red pieces
    int red_num = 0;        // 現在有幾個紅棋
    bool red_or_not[8];     // 哪個位置已經是紅棋了
    std::fill(std::begin(red_or_not), std::end(red_or_not), false);
    char red[4];
    char red2[4];

    while(red_num != 4){
        int x = rng(8);
        if(!red_or_not[x]){
            red[red_num] = print_piece[x];
            red_or_not[x] = true;
            red_num += 1;
        }
    }

    red_num = 0;
    std::fill(std::begin(red_or_not), std::end(red_or_not), false);
    while(red_num != 4){
        int x = rng(8);
        if(!red_or_not[x]){
            red2[red_num] = print_piece[x+8];
            red_or_not[x] = true;
            red_num += 1;
        }
    }
    
    for(int i=0; i<4; i++){
        color[piece_index[red[i]]] = RED;
        color[piece_index[red2[i]]] = -RED;
        
        if (piece_index[red2[i]] >= PIECES) {
            revealed[piece_index[red2[i]]] = true;
        }
    }
    for(int i=PIECES; i<PIECES*2; i++) {
        if(color[i] == -BLUE) {
            revealed[i] = true;
        }
    }

    //set all pieces position and board
    int offset = 0;
    for(int player=0; player<2; player++){
        for(int i=0; i<PIECES; i++){
            board[init_pos[player][i]] = color[i+offset];   //board: record the color in this location(0~3)
            piece_board[init_pos[player][i]] = i + offset;  //piece_booard: write chess number in board, chess number(0~15)/ space(-1)
            pos[i+offset] = init_pos[player][i];    //pos: record the chess index(0~15)
        }
        offset+=8;
    }

    return;
}

void GST::print_board(){    //print the board now & print User's remain chess & print eaten Enemy's chess
    printf("step = %d\n", step - 1);
    for (int i = 0; i < ROW * COL; i++) {
        if(piece_board[i] != -1){
            if(abs(color[piece_board[i]]) == RED) SetColor(4);
            else if(abs(color[piece_board[i]]) == BLUE) SetColor(9);
            printf("%4c", print_piece[piece_board[i]]);
            SetColor();
        }
        else if(i == 0 || i == 30) printf("%4c", '<');
        else if(i == 5 || i == 35) printf("%4c", '>');
        else printf("%4c", '-');           
        if(i % 6 == 5) printf("\n");
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

    // for(int i = 0; i < 36; i++){
    //     printf("%d ", board[i]);
    //     if(i % 6 == 5) printf("\n");
    // }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
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

void GST::undo(){   //return to last move(use to return status of random move)
    if(winner != -1) winner = -1;
    
    if (n_plies == 0) {
        fprintf(stderr, "no history\n");
        exit(1);
    }
    nowTurn ^= 1; //change player

    int move = history[--n_plies];
    int check_eaten = move >> 12;
    int eaten_piece = (move & 0xfff) >> 8;
    int piece = (move & 0xff) >> 4;
    int direction = move & 0xf;
    int src = pos[piece] - dir_val[direction];  //location of last move

    if(is_escape){
        is_escape = false;
        return;
    }

    if (check_eaten != 0x1) {
        board[pos[piece]] = color[eaten_piece];
        piece_board[pos[piece]] = eaten_piece;
        pos[eaten_piece] = pos[piece];
        if(nowTurn == USER){
            if(color[eaten_piece] == -RED) piece_nums[2] += 1;
            else if(color[eaten_piece] == -BLUE) piece_nums[3] += 1;
            else if(color[eaten_piece] == -UNKNOWN){}  //先什麼都不做
            else{
                fprintf(stderr, "undo error, eaten color wrong!");
                exit(1);
            }
        }else{
            if(color[eaten_piece] == RED) piece_nums[0] += 1;
            else if(color[eaten_piece] == BLUE) piece_nums[1] += 1;
            else if(color[eaten_piece] == UNKNOWN){}  //先什麼都不做
            else{
                fprintf(stderr, "undo error, eaten color wrong!");
                exit(1);
            }
        }
    }
    else{
        board[pos[piece]] = 0;
        piece_board[pos[piece]] = -1;
    } 
    board[src] = color[piece];
    piece_board[src] = piece;
    pos[piece] = src;
}

bool GST::is_over(){    //game end or not => the number of remain chess color
    if(n_plies >= 200) {
        winner = -2; // -2表示平局
        return true;
    }
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

// int flat_mc(GST& game, int simu_times){
//     int root_nmove, n_move;
//     int root_moves[MAX_MOVES];
//     int moves[MAX_MOVES];
//     int wins, count;
//     int maxWins = -1;
//     int move_index = 0;
    
//     root_nmove = game.gen_all_move(root_moves);
//     for(int m=0; m<root_nmove; m++){
//         game.do_move(root_moves[m]);
//         //random simu
//         wins = 0;
//         for(int t=0; t<simu_times; t++){
//             count = 0;
//             while(!game.is_over()){
//                 n_move = game.gen_all_move(moves);
//                 int x = rand();
//                 game.do_move(moves[x%n_move]);
//                 count++;
//             }
            
//             if(game.get_winner() == ENEMY){
//                 wins++;
//             }

//             for(int c=0; c<count; c++){
//                 game.undo();
//             }
//         }
//         if(wins > maxWins){
//             maxWins = wins;
//             move_index = m;
//         }
//         printf("move %d, win rate = %f\n", m, (float)wins/(float)simu_times);
//         game.undo();
//     }
//     return root_moves[move_index];
// }
// 使用ismcts替代flat_mc函數
// int ismcts_move(GST& game, int simu_times) {
//     ISMCTS ismcts(simu_times);
//     return ismcts.findBestMove(game);
// }


// 新增統計結構
// struct GameStats {
//     int total_games;
//     // MCTS (Player 1) 統計
//     int mcts_wins;
//     int mcts_escape;
//     int mcts_enemy_red;
//     int mcts_enemy_blue;
//     // ISMCTS (Player 2) 統計
//     int ismcts_wins;
//     int ismcts_escape;
//     int ismcts_enemy_red;
//     int ismcts_enemy_blue;
//     // 平局
//     int draws;

//     GameStats() : total_games(0), mcts_wins(0), mcts_escape(0), mcts_enemy_red(0), mcts_enemy_blue(0),
//                   ismcts_wins(0), ismcts_escape(0), ismcts_enemy_red(0), ismcts_enemy_blue(0), draws(0) {}
// };

// void print_game_stats(const GameStats& stats) {
//     std::cout << "\n===== 統計結果 =====\n";
//     std::cout << "總場次: " << stats.total_games << "\n\n";
    
//     std::cout << "MCTS 獲勝: " << stats.mcts_wins << " 場\n";
//     std::cout << "  - 藍子逃脫: " << stats.mcts_escape << " 場\n";
//     std::cout << "  - 紅子被吃光: " << stats.mcts_enemy_red << " 場\n";
//     std::cout << "  - 吃光對手藍子: " << stats.mcts_enemy_blue << " 場\n\n";
    
//     std::cout << "ISMCTS 獲勝: " << stats.ismcts_wins << " 場\n";
//     std::cout << "  - 藍子逃脫: " << stats.ismcts_escape << " 場\n";
//     std::cout << "  - 紅子被吃光: " << stats.ismcts_enemy_red << " 場\n";
//     std::cout << "  - 吃光對手藍子: " << stats.ismcts_enemy_blue << " 場\n\n";
    
//     std::cout << "平局: " << stats.draws << " 場\n";
// }

// void print_progress_bar(int current, int total) {
//     const int bar_width = 50;
//     float progress = (float)current / total;
//     int pos = bar_width * progress;

//     std::cout << "\r[";
//     for (int i = 0; i < bar_width; ++i) {
//         if (i < pos) std::cout << "=";
//         else if (i == pos) std::cout << ">";
//         else std::cout << " ";
//     }
//     std::cout << "] " << int(progress * 100.0) << "% (" << current << "/" << total << ")" << std::flush;
// }

// int main() {
//     std::random_device rd;
//     std::mt19937 gen(std::chrono::system_clock::now().time_since_epoch().count());
//     std::uniform_int_distribution<> dist(0, 7);
    
//     int num_games;
//     std::cout << "請輸入要進行的遊戲場數: ";
//     std::cin >> num_games;
    
//     GameStats stats;
//     stats.total_games = num_games;
    
//     if (num_games > 1) {
//         std::cout << "\n開始進行多場遊戲模擬...\n";
//     }
    
//     for(int game_num = 1; game_num <= num_games; game_num++) {
//         GST game;
//         MCTS mcts(5000);
//         ISMCTS ismcts(5000);
//         bool my_turn = true;
        
//         // 重置所有遊戲狀態
//         game.init_board();
//         mcts.reset();
//         ismcts.reset();
        
//         if(num_games == 1) {
//             std::cout << "\n===== 遊戲開始 =====\n\n";
//             game.print_board();
//         } else {
//             print_progress_bar(game_num - 1, num_games);
//         }
        
//         while(!game.is_over()) {
//             if(my_turn) {
//                 if(num_games == 1) std::cout << "Player 1 (MCTS) 思考中...\n";
//                 int move = mcts.findBestMove(game);
//                 if (move == -1) break;
//                 game.do_move(move);
//             } else {
//                 if(num_games == 1) std::cout << "Player 2 (ISMCTS) 思考中...\n";
//                 int move = ismcts.findBestMove(game);
//                 if (move == -1) break;
//                 game.do_move(move);
//             }
            
//             if(num_games == 1) {
//                 game.print_board();
//                 std::cout << "當前回合數: " << game.n_plies << std::endl;
//             }
            
//             my_turn = !my_turn;
//         }
        
//         // Update statistics
//         int winner = game.get_winner();
//         if(winner == -2) {
//             stats.draws++;
//         } else if(winner == USER) {
//             stats.mcts_wins++; // 總勝場+1
//             if(game.is_escape) {
//                 stats.mcts_escape++; // 藍子逃脫+1
//             } else if(game.piece_nums[0] == 0) {
//                 stats.mcts_enemy_red++; // 我方紅子被吃光+1
//             } else if(game.piece_nums[3] == 0) {
//                 stats.mcts_enemy_blue++; // 敵方藍子被吃光+1
//             }
//         } else if(winner == ENEMY) {
//             stats.ismcts_wins++; // 總勝場+1
//             if(game.is_escape) {
//                 stats.ismcts_escape++; // 藍子逃脫+1
//             } else if(game.piece_nums[2] == 0) {
//                 stats.ismcts_enemy_red++; // 我方紅子被吃光+1
//             } else if(game.piece_nums[1] == 0) {
//                 stats.ismcts_enemy_blue++; // 敵方藍子被吃光+1
//             }
//         }

//         // Only show detailed output for single game
//         if(num_games == 1) {
//             if(winner == -2) {
//                 printf("遊戲結束！達到200回合，判定為平局！\n");
//             } else {
//                 printf("遊戲結束！%s 獲勝！\n", winner ? "Player 2 (ISMCTS)" : "Player 1 (MCTS)");
//                 if(game.is_escape) {
//                     printf("勝利方式：藍色棋子成功逃脫！\n");
//                 } else if(game.piece_nums[0] == 0) {
//                     printf("勝利方式：Player 1 的紅色棋子全部被吃光！\n");
//                 } else if(game.piece_nums[2] == 0) {
//                     printf("勝利方式：Player 2 的紅色棋子全部被吃光！\n");
//                 } else if(game.piece_nums[1] == 0) {
//                     printf("勝利方式：Player 1 的藍色棋子全部被吃光！\n");
//                 } else if(game.piece_nums[3] == 0) {
//                     printf("勝利方式：Player 2 的藍色棋子全部被吃光！\n");
//                 }
//             }
//         }
//     }
    
//     // Complete the progress bar and print final stats
//     if(num_games > 1) {
//         print_progress_bar(num_games, num_games);
//         std::cout << "\n\n";
//         print_game_stats(stats);
//     }
    
//     return 0;
// }
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
#include "4T_DATA.hpp"

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

DATA data;

void GST::set_board(char* position){        //for server
    // memset(board, 0, sizeof(board));
    // memset(pos, 0, sizeof(pos));
    // for(int i = 0; i < ROW * COL; i++) piece_board[i] = -1;
    // for(int i = 0; i < 4; i++) piece_nums[i] = 4;
    // nowTurn = USER;
    // winner = -1;

    // // Server傳過來的訊息格式：MOV?10B24B34B99b15R25R35R99r45u31u21u99r40u30u20u99b
    // for(int i = 0; i < PIECES * 2; i++){
    //     int index = i * 3;
    //     char x = position[index];
    //     char y = position[index + 1];
    //     char c = position[index + 2];

    //     if(x == '9' && y == '9'){
    //         pos[i] = -1;    //被吃掉

    //         if(i < PIECES){
    //             if(c == 'r'){
    //                 color[i] = RED;
    //                 piece_nums[0]--;
    //             }
    //             else if(c == 'b'){
    //                 color[i] = BLUE;
    //                 piece_nums[1]--;
    //             }
    //         }
    //         else{
    //             if(c == 'r'){
    //                 color[i] = -RED;
    //                 piece_nums[2]--;
    //             }
    //             else if(c == 'b'){
    //                 color[i] = -BLUE;
    //                 piece_nums[3]--;
    //             }
    //         }
    //     }
    //     else{
    //         int pos_val = (x - '0') + (y - '0') * COL;
    //         pos[i] = pos_val;
            
    //         if(i < PIECES) {     //User
    //             if(c == 'R') color[i] = RED;
    //             else if(c == 'B') color[i] = BLUE;
    //         }
    //         else{       //Enemy
    //             if(c == 'u') color[i] = -UNKNOWN;      //未知的棋
    //         }
            
    //         // 更新棋盤
    //         board[pos_val] = color[i];     // 記錄顏色
    //         piece_board[pos_val] = i;      // 記錄棋子編號
    //     }
    // }

    // print_board();

    // return;
}


void GST::init_board(){
    /*
        A  B  C  D  E  F  G  H  a  b  c  d  e  f  g  h
        0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15  <-棋子索引，用於pos和color
       25 26 27 28 31 32 33 34 10  9  8  7  4  3  2  1  <-棋盤上的位置
    */

    memset(board, 0, sizeof(board));
    memset(pos, 0, sizeof(pos));
    memset(revealed, false, sizeof(revealed));
    for(int i=0; i<ROW*COL; i++) piece_board[i] = -1;
    for(int i=0; i<4; i++) piece_nums[i] = 4;
    for(int i=0; i<PIECES; i++) {
        color[i] = BLUE;
        color[i+8] = -BLUE;
    }
    nowTurn = USER;
    winner = -1;
    n_plies = 0;

    // 設置紅色棋子 
    char red[4] = {'A','B','D','F'}; // 用戶
    char red2[4] = {'a','b','d','f'}; // 敵人
    printf("預設紅色幽靈: A B D F (玩家1) 和 a b d f (玩家2)\n");
    
    for(int i=0; i<4; i++){
        color[piece_index[red[i]]] = RED;
        color[piece_index[red2[i]]] = -RED;
        
        // 修改這部分，設置玩家二的所有棋子為已揭露
        if (piece_index[red2[i]] >= PIECES) {  // 玩家二的紅色棋子
            revealed[piece_index[red2[i]]] = true;
        }
    }
    for(int i=PIECES; i<PIECES*2; i++) {
        if(color[i] == -BLUE) {  // 玩家二的藍色棋子
            revealed[i] = true;
        }
    }
    // 設置所有棋子位置和棋盤
    int offset = 0;
    for(int player=0; player<2; player++){
        for(int i=0; i<PIECES; i++){
            board[init_pos[player][i]] = player == 0 ? 1 : -1; // 僅存儲存在，非顏色
            piece_board[init_pos[player][i]] = i + offset;
            pos[i+offset] = init_pos[player][i];
        }
        offset+=8;
    }

    return;
}

void GST::print_board(){
    // 直接在控制台顯示棋盤
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

    // 顯示棋子信息
    printf("\n玩家剩餘棋子: ");
    for(int i=0; i<PIECES; i++){
        if(pos[i] != -1) printf("%c: %s ", print_piece[i], color[i]==1 ? "red":"blue");
    }
    printf("\n被吃掉的敵方棋子: ");
    for(int i=8; i<PIECES*2; i++){
        if(pos[i] == -1) printf("%c: %s ", print_piece[i], color[i]==-1 ? "red":"blue");
    }
    printf("\n");
}

int GST::gen_move(int* move_arr, int piece, int location, int& count){
    int row = location / ROW;
    int col = location % COL;
    
    if(nowTurn == USER){
        if(row != 0 && board[location-6] <= 0) move_arr[count++] = piece << 4;            // 上
        if(row != ROW-1 && board[location+6] <= 0) move_arr[count++] = piece << 4 | 3;    // 下
        if(col != 0 && board[location-1] <= 0) move_arr[count++] = piece << 4 | 1;        // 左
        if(col != COL-1 && board[location+1] <= 0) move_arr[count++] = piece << 4 | 2;    // 右
        
        // 只檢查玩家棋子的藍色即可逃脫
        bool canExit = (piece < PIECES && color[piece] == BLUE);
        if(canExit){                                                         // 出口移動    
            if(location == 0) move_arr[count++] = piece << 4 | 1;
            if(location == 5) move_arr[count++] = piece << 4 | 2;
        }
    }else{
        if(row != 0 && board[location-6] >= 0) move_arr[count++] = piece << 4;            
        if(row != ROW-1 && board[location+6] >= 0) move_arr[count++] = piece << 4 | 3;    
        if(col != 0 && board[location-1] >= 0) move_arr[count++] = piece << 4 | 1;        
        if(col != COL-1 && board[location+1] >= 0) move_arr[count++] = piece << 4 | 2;
        
        // AI的藍色棋子也應該能夠逃脫
        bool canExit = (piece >= PIECES && color[piece] == -BLUE);
        if(canExit){
            if(location == 30) move_arr[count++] = piece << 4 | 1;
            if(location == 35) move_arr[count++] = piece << 4 | 2;
        }
    }
    
    return count;
}

int GST::gen_all_move(int* move_arr){
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

bool check_win_move(int location, int dir){
    if(location == 0 || location == 30) return dir == 1 ? true : false;
    else if(location == 5 || location == 35) return dir == 2 ? true : false;
    return false;
}

void GST::do_move(int move){
    int piece = move >> 4;
    int direction = move & 0xf;

    // 只檢查是否是藍色棋子的勝利移動（逃脫），不考慮revealed
    if((piece < PIECES && color[piece] == BLUE) || 
       (piece >= PIECES && color[piece] == -BLUE)){
        if(check_win_move(pos[piece], direction)){
            winner = nowTurn;
            n_plies++;
            nowTurn ^= 1;
            is_escape = true;
            return;
        }
    }
    if (n_plies == MAX_PLIES) {
        fprintf(stderr, "無法進行更多移動\n");
        exit(1);
    }

    int dst = pos[piece] + dir_val[direction];
    
    if(board[dst] < 0){ // 捕獲敵方棋子
        int captured_piece = piece_board[dst];
        pos[captured_piece] = -1;
        move |= captured_piece << 8;
        // 標記被捕獲的棋子為已揭露
        revealed[captured_piece] = true;
        
        if(color[captured_piece] == -RED) piece_nums[2] -= 1;
        else if(color[captured_piece] == -BLUE) piece_nums[3] -= 1;
        else{
            fprintf(stderr, "do_move錯誤，被吃顏色錯誤！\n");
            exit(1);
        }
    }
    else if(board[dst] > 0){ // 捕獲玩家棋子
        int captured_piece = piece_board[dst];
        pos[captured_piece] = -1;
        move |= captured_piece << 8;
        
        if(color[captured_piece] == RED) piece_nums[0] -= 1;
        else if(color[captured_piece] == BLUE) piece_nums[1] -= 1;
        else{
            fprintf(stderr, "do_move錯誤，被吃顏色錯誤！\n");
            exit(1);
        }
    }
    else{
        move |= 0x1000;
    }

    board[pos[piece]] = 0;
    piece_board[pos[piece]] = -1;
    board[dst] = (piece < PIECES) ? 1 : -1; // 僅存儲存在，非顏色
    piece_board[dst] = piece;
    pos[piece] = dst;
    history[n_plies++] = move;
    nowTurn ^= 1; // 切換玩家
}

void GST::undo(){
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
    int src = pos[piece] - dir_val[direction];

    if(is_escape){
        is_escape = false;
        return;
    }

    if (check_eaten != 0x1) {
        board[pos[piece]] = (eaten_piece < PIECES) ? 1 : -1; // Just store presence, not color
        piece_board[pos[piece]] = eaten_piece;
        pos[eaten_piece] = pos[piece];
        
        if(nowTurn == USER){
            if(color[eaten_piece] == -RED) piece_nums[2] += 1;
            else if(color[eaten_piece] == -BLUE) piece_nums[3] += 1;
            else{
                fprintf(stderr, "undo error, eaten color wrong!");
                exit(1);
            }
        }else{
            if(color[eaten_piece] == RED) piece_nums[0] += 1;
            else if(color[eaten_piece] == BLUE) piece_nums[1] += 1;
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

    board[src] = (piece < PIECES) ? 1 : -1; // Just store presence, not color
    piece_board[src] = piece;
    pos[piece] = src;
}

bool GST::is_over(){

    if(n_plies >= 200) {
        winner = -2; // -2表示平局
        return true;
    }
    
    if(winner != -1) return true;
    else{
        // 1. 玩家1的紅色棋子全部被吃光，玩家1獲勝
        if(piece_nums[0] == 0) {
            winner = USER;
            return true;
        }
        // 2. 玩家2的紅色棋子全部被吃光，玩家2獲勝
        else if(piece_nums[2] == 0) {
            winner = ENEMY;
            return true;
        }
        // 3. 玩家1的藍色棋子全部被吃光，玩家2獲勝
        else if(piece_nums[1] == 0) {
            winner = ENEMY;
            return true;
        }
        // 4. 玩家2的藍色棋子全部被吃光，玩家1獲勝
        else if(piece_nums[3] == 0) {
            winner = USER;
            return true;
        }
        // 5. 藍色棋子逃到對角已經在do_move中處理並設置了winner和is_escape
    }
    return false;
}

int move_index = 0;

int flat_mc(GST& game, int simu_times){
    int root_nmove, n_move;
    int root_moves[MAX_MOVES];
    int moves[MAX_MOVES];
    int wins, count;
    int maxWins = -1;
    int move_index = 0;
    
    root_nmove = game.gen_all_move(root_moves);
    for(int m=0; m<root_nmove; m++){
        game.do_move(root_moves[m]);
        //random simu
        wins = 0;
        for(int t=0; t<simu_times; t++){
            count = 0;
            while(!game.is_over()){
                n_move = game.gen_all_move(moves);
                int x = rand();
                game.do_move(moves[x%n_move]);
                count++;
            }
            
            if(game.get_winner() == ENEMY){
                wins++;
            }

            for(int c=0; c<count; c++){
                game.undo();
            }
        }
        if(wins > maxWins){
            maxWins = wins;
            move_index = m;
        }
        printf("move %d, win rate = %f\n", m, (float)wins/(float)simu_times);
        game.undo();
    }
    return root_moves[move_index];
}

// 使用ismcts替代flat_mc函數
int ismcts_move(GST& game, int simu_times) {
    ISMCTS ismcts(simu_times);
    return ismcts.findBestMove(game, data);
}

// 新增統計結構
struct GameStats {
    int total_games;
    // MCTS (Player 1) 統計
    int mcts_wins;
    int mcts_escape;
    int mcts_enemy_red;
    int mcts_enemy_blue;
    // ISMCTS (Player 2) 統計
    int ismcts_wins;
    int ismcts_escape;
    int ismcts_enemy_red;
    int ismcts_enemy_blue;
    // 平局
    int draws;

    GameStats() : total_games(0), mcts_wins(0), mcts_escape(0), mcts_enemy_red(0), mcts_enemy_blue(0),
                  ismcts_wins(0), ismcts_escape(0), ismcts_enemy_red(0), ismcts_enemy_blue(0), draws(0) {}
};

void print_game_stats(const GameStats& stats) {
    std::cout << "\n===== 統計結果 =====\n";
    std::cout << "總場次: " << stats.total_games << "\n\n";
    
    std::cout << "MCTS 獲勝: " << stats.mcts_wins << " 場\n";
    std::cout << "  - 藍子逃脫: " << stats.mcts_escape << " 場\n";
    std::cout << "  - 紅子被吃光: " << stats.mcts_enemy_red << " 場\n";
    std::cout << "  - 吃光對手藍子: " << stats.mcts_enemy_blue << " 場\n\n";
    
    std::cout << "ISMCTS 獲勝: " << stats.ismcts_wins << " 場\n";
    std::cout << "  - 藍子逃脫: " << stats.ismcts_escape << " 場\n";
    std::cout << "  - 紅子被吃光: " << stats.ismcts_enemy_red << " 場\n";
    std::cout << "  - 吃光對手藍子: " << stats.ismcts_enemy_blue << " 場\n\n";
    
    std::cout << "平局: " << stats.draws << " 場\n";
}

void print_progress_bar(int current, int total) {
    const int bar_width = 50;
    float progress = (float)current / total;
    int pos = bar_width * progress;

    std::cout << "\r[";
    for (int i = 0; i < bar_width; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << int(progress * 100.0) << "% (" << current << "/" << total << ")" << std::flush;
}

int main() {
    std::random_device rd;
    std::mt19937 gen(std::chrono::system_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<> dist(0, 7);
    
    int num_games;
    std::cout << "請輸入要進行的遊戲場數: ";
    std::cin >> num_games;
    
    GameStats stats;
    stats.total_games = num_games;
    
    if (num_games > 1) {
        std::cout << "\n開始進行多場遊戲模擬...\n";
    }
    
    for(int game_num = 1; game_num <= num_games; game_num++) {
        GST game;
        MCTS mcts(5000);
        ISMCTS ismcts(5000);
        bool my_turn = true;
        
        // 重置所有遊戲狀態
        game.init_board();
        mcts.reset();
        ismcts.reset();
        
        if(num_games == 1) {
            std::cout << "\n===== 遊戲開始 =====\n\n";
            game.print_board();
        } else {
            print_progress_bar(game_num - 1, num_games);
        }
        
        while(!game.is_over()) {
            if(my_turn) {
                if(num_games == 1) std::cout << "Player 1 (MCTS) 思考中...\n";
                int move = mcts.findBestMove(game);
                if (move == -1) break;
                game.do_move(move);
            } else {
                if(num_games == 1) std::cout << "Player 2 (ISMCTS) 思考中...\n";
                int move = ismcts.findBestMove(game, data);
                if (move == -1) break;
                game.do_move(move);
            }
            
            if(num_games == 1) {
                game.print_board();
                std::cout << "當前回合數: " << game.n_plies << std::endl;
            }
            
            my_turn = !my_turn;
        }
        
        // Update statistics
        int winner = game.get_winner();
        if(winner == -2) {
            stats.draws++;
        } else if(winner == USER) {
            stats.mcts_wins++; // 總勝場+1
            if(game.is_escape) {
                stats.mcts_escape++; // 藍子逃脫+1
            } else if(game.piece_nums[0] == 0) {
                stats.mcts_enemy_red++; // 我方紅子被吃光+1
            } else if(game.piece_nums[3] == 0) {
                stats.mcts_enemy_blue++; // 敵方藍子被吃光+1
            }
        } else if(winner == ENEMY) {
            stats.ismcts_wins++; // 總勝場+1
            if(game.is_escape) {
                stats.ismcts_escape++; // 藍子逃脫+1
            } else if(game.piece_nums[2] == 0) {
                stats.ismcts_enemy_red++; // 我方紅子被吃光+1
            } else if(game.piece_nums[1] == 0) {
                stats.ismcts_enemy_blue++; // 敵方藍子被吃光+1
            }
        }

        // Only show detailed output for single game
        if(num_games == 1) {
            if(winner == -2) {
                printf("遊戲結束！達到200回合，判定為平局！\n");
            } else {
                printf("遊戲結束！%s 獲勝！\n", winner ? "Player 2 (ISMCTS)" : "Player 1 (MCTS)");
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
        }
    }
    
    // Complete the progress bar and print final stats
    if(num_games > 1) {
        print_progress_bar(num_games, num_games);
        std::cout << "\n\n";
        print_game_stats(stats);
    }
    
    return 0;
}
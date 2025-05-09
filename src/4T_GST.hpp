#ifndef GST_HPP
#define GST_HPP

#include "4T_header.h"

class DATA;

// 共用參數
constexpr double EXPLORATION_PARAM = 1.414;

// 前向聲明
class ISMCTS;
class MCTS;

class GST {
    friend class ISMCTS; 
    friend class MCTS;   
    friend int main();

private:
    int board[ROW * COL];        //record piece color
    int piece_board[ROW * COL];  //record piece index
    int pos[PIECES * 2];    //index: my pieces A~H:0~7 ; enemy pieces a~h:8~15; value:postion of board
    int color[PIECES * 2];  //index: my pieces A~H:0~7 ; enemy pieces a~h:8~15; value: 1->red, 2->blue
    int piece_nums[4];      //index: 0:my red nums; 1:my blue nums; 2:enemy red nums; 3:enemy blue nums 
    int nowTurn;
    int winner;
    bool is_escape = false;
    bool revealed[PIECES * 2] = {false};

    int history[MAX_PLIES];
    int n_plies;

    // orig ver
    // int color_his_E[201][36];
    // int color_his_U[201][36];

    // R1 ver
    int color_his_E[201][37];
    int color_his_U[201][37];

    // long long game_his[201][2];       //棋譜
    int step;

public:
    void init_board();
    void print_board();
    int gen_all_move(int* move_arr);
    int gen_move(int* move_arr, int piece, int location, int& count);
    void do_move(int move);
    void undo();
    bool is_over();
    bool is_revealed(int piece) const { return revealed[piece]; }
    // int get_piece(int index){ return piece_nums[index]; }

    bool is_valid_pattern(int base_pos, const int* offset);
    int get_loc(int base_pos, const int* offset);
    int get_feature(int base_pos, const int* offset, int step_idx, int who);
    int get_feature_unknown(int base_pos, const int* offset);
    // int get_base_pos(int pos, int offset_idx, const int* offset){ return pos - offset[offset_idx]; }
    float get_weight(int base_pos, const int* offset, DATA&);
    float compute_board_weight(DATA&);
    int highest_weight(DATA&);

    int get_color(int piece) const { return color[piece]; }
    int get_pos(int piece) const { return pos[piece]; }
    void set_color(int piece, int new_color) { color[piece] = new_color; }
    // int get_nowTurn(){ return this->nowTurn; }

    int get_winner(){ return this->winner; }
    int get_nplies(){ return this->n_plies; }
   
    // MCTS 作弊用
    const int* get_full_colors() const { return color; }
    // ISMCTS 查詢revealed狀態
    const bool* get_revealed() const { return revealed; }

    // void record_board();
    void update_tuple_trans(int step, int base_pos, const int* offset, int now_win, int U, int E, DATA&, int piece_step[4]);    // piece_step 0:U red、1:U blue、2:E red、3:E blue
    // void update_tuple(int now_win, DATA&);
    // void show_color_his(int WHO);
    // void show_game_his();
    // int get_step(){ return step; }
    // void write_history_file(int run);

    // int get_dst(int idx){ return pos[idx]; }
    // int get_piece_board(int idx) { return piece_board[idx]; }
    
    // flat_mc
    // void guess_board(int player);

    // server
    void set_board(char* position);
};

#endif
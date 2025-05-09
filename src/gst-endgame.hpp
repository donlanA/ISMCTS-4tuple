#ifndef GST_HPP
#define GST_HPP

#define ROW 6
#define COL 6
#define PIECES 8
#define MAX_PLIES 1000
#define MAX_MOVES 32

#define RED 1
#define BLUE 2
#define UNKNOWN 3

#define USER 0
#define ENEMY 1

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

public:
    void init_board();
    void print_board();
    int gen_all_move(int* move_arr);
    int gen_move(int* move_arr, int piece, int location, int& count);
    void do_move(int move);
    void undo();
    bool is_over();
    bool is_revealed(int piece) const { return revealed[piece]; }
    
    int get_color(int piece) const { return color[piece]; }
    int get_pos(int piece) const { return pos[piece]; }
    void set_color(int piece, int new_color) { color[piece] = new_color; }
    
    int get_winner(){ return this->winner; }
    int get_nplies(){ return this->n_plies; }
    
    // MCTS 作弊用
    const int* get_full_colors() const { return color; }
    // ISMCTS 查詢revealed狀態
    const bool* get_revealed() const { return revealed; }
};

#endif
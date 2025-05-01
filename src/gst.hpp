#ifndef GST_HPP
#define GST_HPP

#define ROW 6
#define COL 6
#define PIECES 8
#define MAX_PLIES 1000
#define MAX_MOVES 32

#define RED 1
#define BLUE 2

#define USER 0
#define ENEMY 1

// 共用參數
constexpr double EXPLORATION_PARAM = 1.414;

// 前向聲明
class ISMCTS;
class MCTS;  // 添加 MCTS 前向聲明

class GST {
    friend class ISMCTS; 
    friend class MCTS;   
    friend int main();   

private:
    int board[ROW * COL];        // 記錄棋子顏色
    int piece_board[ROW * COL];  // 記錄棋子索引
    int pos[PIECES * 2];    // 索引: 我方棋子 A~H:0~7 ; 敵方棋子 a~h:8~15; 值: 棋盤上的位置
    int color[PIECES * 2];  // 索引: 我方棋子 A~H:0~7 ; 敵方棋子 a~h:8~15; 值: 1->紅色, 2->藍色
    int piece_nums[4];      // 索引: 0:我方紅色數量; 1:我方藍色數量; 2:敵方紅色數量; 3:敵方藍色數量 
    int nowTurn;
    int winner;
    bool is_escape = false;
    bool revealed[PIECES * 2] = {false}; // 跟蹤棋子顏色是否已被揭露

    int history[MAX_PLIES];
    int n_plies;

public:

    // 接收 server 資訊
    void set_board(char* position);

    void init_board();
    void setup_custom_board(const char* layout[ROW]); // New function for custom board setup
    void print_board();
    int gen_all_move(int* move_arr);
    int gen_move(int* move_arr, int piece, int location, int& count);
    void do_move(int move);
    void undo();
    bool is_over();
    bool is_revealed(int piece) const { return revealed[piece]; }
    
    // 額外的訪問器方法
    int get_color(int piece) const { return color[piece]; }
    int get_pos(int piece) const { return pos[piece]; }
    void set_color(int piece, int new_color) { color[piece] = new_color; }
    
    int get_winner(){
        return this->winner;
    }
    int get_nplies(){
        return this->n_plies;
    }
    
    // MCTS 作弊用
    const int* get_full_colors() const { return color; }
    // ISMCTS 查詢revealed狀態
    const bool* get_revealed() const { return revealed; }


};

#endif // GST_HPP
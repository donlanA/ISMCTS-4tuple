#include "4T_header.h"
#include "ismcts.hpp"
#include "mcts.hpp"

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

static const int offset_1x4[4] = {0, 1, 2, 3};
static const int offset_2x2[4] = {0, 1, 6, 7};
static const int offset_4x1[4] = {0, 6, 12, 18};

void SetColor(int color = 7){
    #ifdef _WIN32
        HANDLE hConsole;
        hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(hConsole,color);
    #else
        // For non-Windows platforms, use ANSI escape codes
        switch(color) {
            case 4: printf("\033[31m"); break;  // RED
            case 9: printf("\033[34m"); break;  // BLUE
            default: printf("\033[0m"); break;  // Reset to default
        }
    #endif
}

void GST::set_board(char* position){        //for server
    memset(board, 0, sizeof(board));
    memset(pos, 0, sizeof(pos));
    memset(revealed, false, sizeof(revealed));
    for(int i = 0; i < ROW * COL; i++) piece_board[i] = -1;
    for(int i = 0; i < 4; i++) piece_nums[i] = 4;


    nowTurn = USER;
    winner = -1;
    for(int i=0; i<PIECES; i++) {//預設自己的都可看到
        revealed[i] = true;
    }
    // Server傳過來的訊息格式：MOV?10B24B34B99b15R25R35R99r45u31u21u99r40u30u20u99b
    for(int i = 0; i < PIECES * 2; i++){
        int index = i * 3;
        char x = position[index];
        char y = position[index + 1];
        char c = position[index + 2];

        if(x == '9' && y == '9'){
            pos[i] = -1;    //被吃掉
            revealed[i] = true;
            if(i < PIECES){
                if(c == 'r'){
                    color[i] = RED;
                    piece_nums[0]--;
                }
                else if(c == 'b'){
                    color[i] = BLUE;
                    piece_nums[1]--;
                }
            }
            else{
                if(c == 'r'){
                    color[i] = -RED;
                    piece_nums[2]--;
                }
                else if(c == 'b'){
                    color[i] = -BLUE;
                    piece_nums[3]--;
                }
            }
        }
        else{
            int pos_val = (x - '0') + (y - '0') * COL;
            pos[i] = pos_val;
            
            if(i < PIECES) {     //User
                if(c == 'R') color[i] = RED;
                else if(c == 'B') color[i] = BLUE;
                revealed[i] = true;
            }
            else{       //Enemy
                if(c == 'u') color[i] = -UNKNOWN;      //未知的棋
                revealed[i] = false;
            }
            
            // 更新棋盤
            board[pos_val] = color[i];     // 記錄顏色
            piece_board[pos_val] = i;      // 記錄棋子編號
        }
    }

    print_board();

    return;
}

//initialize data
void GST::init_board(){
    auto now = std::chrono::system_clock::now();
    auto now_as_duration = now.time_since_epoch();
    auto now_as_microseconds = std::chrono::duration_cast<std::chrono::microseconds>(now_as_duration).count();
    pcg32 rng(now_as_microseconds);
    
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

    // for (int i = 0; i < 201; i++) {
    //     for (int j = 0; j < ROW * COL + 1; j++) {
    //         color_his_U[i][j] = 0;
    //         color_his_E[i][j] = 0;
    //     }
    // }
    step = 0;

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
    if (n_plies == MAX_PLIES) {
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
            fprintf(stderr, "piece: %d, direction: %d\n", piece, direction);
            fprintf(stderr, "pos[piece]: %d, dir_val[direction]: %d\n", pos[piece], dir_val[direction]);
            fprintf(stderr, "color[piece_board[dst]]: %d, piece_board[dst]: %d\n", color[piece_board[dst]], piece_board[dst]);
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
            fprintf(stderr, "piece: %d, direction: %d\n", piece, direction);
            fprintf(stderr, "pos[piece]: %d, dir_val[direction]: %d\n", pos[piece], dir_val[direction]);
            fprintf(stderr, "color[piece_board[dst]]: %d, piece_board[dst]: %d\n", color[piece_board[dst]], piece_board[dst]);
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

// Helper function to check if a pattern is valid given base position
bool GST::is_valid_pattern(int base_pos, const int* offset){
    int base_row = base_pos / COL;
    int base_col = base_pos % COL;
    
    if(offset == offset_1x4){
        if(base_col > 2) return false;
    }
    else if(offset == offset_2x2){
        if(base_col > 4 || base_row > 4) return false;
    }
    else if(offset == offset_4x1){
        if(base_row > 2) return false;
    }

    return true;
}

int GST::get_loc(int base_pos, const int* offset){ 
    int position[4];
    for(int i = 0; i < 4; i++){
        position[i] = base_pos + offset[i];
    }
    return (position[0] * 36 * 36 * 36 + position[1] * 36 * 36 + position[2] * 36 + position[3]);
}

int GST::get_feature_unknown(int base_pos, const int* offset){           // the piece color
    int features[4];
    for(int i = 0; i < 4; i++){
        int pos = base_pos + offset[i];
        if(nowTurn == USER){
            features[i] = (board[pos] < 0) ? 3 : board[pos];
        }
        else{
            features[i] = (board[pos] > 0) ? 3 : -board[pos];
        }
    }
    return (features[0] * 64 + features[1] * 16 + features[2] * 4 + features[3]);
}

float GST::get_weight(int base_pos, const int* offset, DATA& d){
    int feature = get_feature_unknown(base_pos, offset);
    int LUTidx = d.LUT_idx(d.trans[get_loc(base_pos, offset)], feature);
    float weight = 0;
    if(nowTurn == USER){
        if(piece_nums[2] == 1){   // E R = 1
            weight = (float)(d.LUTw_U_R1[LUTidx]) / (float)(d.LUTv_U_R1[LUTidx]);
            // printf("U R1 using\n");
        }else if(piece_nums[1] == 1){   // U B = 1
            weight = (float)(d.LUTw_U_B1[LUTidx]) / (float)(d.LUTv_U_B1[LUTidx]);
        }else{
            weight = (float)(d.LUTw_U[LUTidx]) / (float)(d.LUTv_U[LUTidx]);
        }
    }else{
        if(piece_nums[0] == 1){   // U R = 1
            weight = (float)(d.LUTw_E_R1[LUTidx]) / (float)(d.LUTv_E_R1[LUTidx]);
            // printf("E R1 using\n");
        }else if(piece_nums[3] == 1){   // E B = 1
            weight = (float)(d.LUTw_E_B1[LUTidx]) / (float)(d.LUTv_E_B1[LUTidx]);
        }else{
            weight = (float)(d.LUTw_E[LUTidx]) / (float)(d.LUTv_E[LUTidx]);
        }
    }

    //printf("location = %d, feature = %d, LUTidx = %d, weight = %f\n", d.trans[get_loc(base_pos, offset)], feature, LUTidx, weight);

    return weight;
}

float GST::compute_board_weight(DATA& d){
    float total_weight = 0;
    
    for(int pos = 0; pos < ROW * COL; pos++) {
        int row = pos / COL;
        int col = pos % COL;
        
        if(is_valid_pattern(pos, offset_1x4)){
            total_weight += get_weight(pos, offset_1x4, d);
        }
        if(is_valid_pattern(pos, offset_4x1)){
            total_weight += get_weight(pos, offset_4x1, d);
        }
        if(is_valid_pattern(pos, offset_2x2)){
            total_weight += get_weight(pos, offset_2x2, d);
        }
    }

    return total_weight / (float)TUPLE_NUM;
}

int GST::highest_weight(DATA& d){
    float WEIGHT[MAX_MOVES] = {0};
    int root_nmove;
    int root_moves[MAX_MOVES];
    root_nmove = gen_all_move(root_moves);

    for(int m = 0; m < root_nmove; m++){
        int move_index = m;
        int piece = root_moves[m] >> 4;
        int direction = root_moves[m] & 0xf;
        int src = pos[piece];
        int dst = src + dir_val[direction];  //the position after move

        if(pos[piece] == 0 && direction == 1 && nowTurn == USER && board[0] == BLUE){  // if check_win_move() = true, won't move and return directly
            WEIGHT[move_index] = 1;
        }
        else if(pos[piece] == 5 && direction == 2 && nowTurn == USER && board[5] == BLUE){
            WEIGHT[move_index] = 1;
        }
        else if(pos[piece] == 30 && direction == 1 && nowTurn == ENEMY && board[30] == -BLUE){
            WEIGHT[move_index] = 1;
        }
        else if(pos[piece] == 35 && direction == 2 && nowTurn == ENEMY && board[35] == -BLUE){
            WEIGHT[move_index] = 1;        
        }
        else if(pos[piece] == 4 && direction == 2 && nowTurn == USER && color[piece] == BLUE){      //客製化
            if(board[5] >= 0 && board[11] >= 0){
                WEIGHT[move_index] = 1;
            }
        }
        else if(pos[piece] == 1 && direction == 1 && nowTurn == USER && color[piece] == BLUE){      //客製化
            if(board[0] >= 0 && board[6] >= 0){
                WEIGHT[move_index] = 1;
            }
        }
        else{
            int tmp_color[PIECES * 2];      //去除上帝視角
            for(int i = 0; i < PIECES * 2; i++) tmp_color[i] = color[i];
            if(nowTurn == USER) for(int i = PIECES; i < PIECES * 2; i++) color[i] = -UNKNOWN;
            else for(int i = 0; i < PIECES; i++) color[i] = UNKNOWN;

            do_move(root_moves[m]);
            nowTurn ^= 1;

            WEIGHT[move_index] = compute_board_weight(d);

            nowTurn ^= 1;
            undo();

            for(int i = 0; i < PIECES * 2; i++) color[i] = tmp_color[i];
        }

        //調參數時間
        int row = dst / 6;
        int col = dst % 6;
        
        // 計算目標位置到各角落的距離
        int d0 = row + col;                     // 到 (0,0) 的距離
        int d5 = row + (5 - col);               // 到 (0,5) 的距離
        int d30 = (5 - row) + col;              // 到 (5,0) 的距離
        int d35 = (5 - row) + (5 - col);        // 到 (5,5) 的距離
        
        // 存儲每個棋子到各角落的距離
        std::vector<std::tuple<int, int, int>> pieces_distances; // (棋子索引, 角落編號, 距離)
            
        if(nowTurn == USER){
            // 計算所有棋子到各角落的距離
            for(int i = 0; i < PIECES; i++){
                if(pos[i] != -1){
                    int p_row = pos[i] / 6;
                    int p_col = pos[i] % 6;
                    
                    // 計算這個棋子到四個角落的距離
                    int dist_to_0 = p_row + p_col;
                    int dist_to_5 = p_row + (5 - p_col);
                    int dist_to_30 = (5 - p_row) + p_col;
                    int dist_to_35 = (5 - p_row) + (5 - p_col);
                    
                    // 添加所有棋子-角落距離組合
                    pieces_distances.push_back(std::make_tuple(i, 0, dist_to_0));
                    pieces_distances.push_back(std::make_tuple(i, 1, dist_to_5));
                    pieces_distances.push_back(std::make_tuple(i, 2, dist_to_30));
                    pieces_distances.push_back(std::make_tuple(i, 3, dist_to_35));
                }
            }
        }
        else if(nowTurn == ENEMY){
            for(int i = PIECES; i < PIECES * 2; i++){
                if(pos[i] != -1){
                    int p_row = pos[i] / 6;
                    int p_col = pos[i] % 6;
                    
                    // 計算這個棋子到四個角落的距離
                    int dist_to_0 = p_row + p_col;
                    int dist_to_5 = p_row + (5 - p_col);
                    int dist_to_30 = (5 - p_row) + p_col;
                    int dist_to_35 = (5 - p_row) + (5 - p_col);
                    
                    // 添加所有棋子-角落距離組合
                    pieces_distances.push_back(std::make_tuple(i, 0, dist_to_0));
                    pieces_distances.push_back(std::make_tuple(i, 1, dist_to_5));
                    pieces_distances.push_back(std::make_tuple(i, 2, dist_to_30));
                    pieces_distances.push_back(std::make_tuple(i, 3, dist_to_35));
                }
            }
        }
            
        // 按距離排序所有棋子-角落組合
        std::sort(pieces_distances.begin(), pieces_distances.end(), 
        [](const std::tuple<int, int, int>& a, const std::tuple<int, int, int>& b){
            return std::get<2>(a) < std::get<2>(b);
        });
        
        // 已分配的棋子和角落
        bool piece_assigned[PIECES * 2];
        bool corner_assigned[4];
        int assigned_corner_for_piece[PIECES * 2];

        memset(piece_assigned, false, sizeof(piece_assigned));
        memset(corner_assigned, false, sizeof(corner_assigned));
        memset(assigned_corner_for_piece, -1, sizeof(assigned_corner_for_piece));
        
        // 按距離分配棋子到角落
        for(const auto& tuple : pieces_distances) {
            int p_idx = std::get<0>(tuple);
            int corner = std::get<1>(tuple);
            
            // 如果這個棋子和角落都還沒被分配，則進行分配
            if(!piece_assigned[p_idx] && !corner_assigned[corner]) {
                piece_assigned[p_idx] = true;
                corner_assigned[corner] = true;
                assigned_corner_for_piece[p_idx] = corner;
            }
            
            // 如果所有角落都已分配，停止循環
            if(corner_assigned[0] && corner_assigned[1] && corner_assigned[2] && corner_assigned[3]) {
                break;
            }
        }
        
        // 角落獎勵係數
        float corner_bonus = 1.0;
        
        // 檢查當前棋子是否被分配到某個角落
        if(assigned_corner_for_piece[piece] != -1) {
            int assigned_corner = assigned_corner_for_piece[piece];
            int current_dist;

            //printf("piece = %c, assigned_corner = %d\n", print_piece[piece], assigned_corner);
            
            // 計算當前位置到分配角落的距離
            int p_row = src / 6;
            int p_col = src % 6;
            
            if(assigned_corner == 0) {
                current_dist = p_row + p_col;
                // 檢查移動是否更接近分配的角落
                if(d0 < current_dist) {
                    corner_bonus = 1.01;
                }
            } else if(assigned_corner == 1) {
                current_dist = p_row + (5 - p_col);
                if(d5 < current_dist) {
                    corner_bonus = 1.01;
                }
            } else if(assigned_corner == 2) {
                current_dist = (5 - p_row) + p_col;
                if(d30 < current_dist) {
                    corner_bonus = 1.01;
                }
            } else if(assigned_corner == 3) {
                current_dist = (5 - p_row) + (5 - p_col);
                if(d35 < current_dist) {
                    corner_bonus = 1.01;
                }
            }
        }

        WEIGHT[move_index] *= corner_bonus;

        if(piece_nums[2] <= 1 && board[dst] == 0){
            WEIGHT[move_index] *= 1.01;
        }

        //printf("Move = %d | piece: %c, direction: %d, WEIGHT[]: %f\n\n", move_index, print_piece[piece], direction, WEIGHT[move_index]);
    }

    float max_weight = -1;
    int do_idx = -1, same_idx = 0, SAME[MAX_MOVES] = {0};

    for(int i = 0; i < root_nmove; i++){
        if(WEIGHT[i] == max_weight){
            SAME[same_idx++] = i;
        }
        else if(WEIGHT[i] > max_weight){
            max_weight = WEIGHT[i];
            do_idx = i;           //which step we decided to move finally
            SAME[0] = i;
            same_idx = 1;
        }
    }

    auto now = std::chrono::system_clock::now();
    auto now_as_duration = now.time_since_epoch();
    auto now_as_microseconds = std::chrono::duration_cast<std::chrono::microseconds>(now_as_duration).count();
    pcg32 rng(now_as_microseconds);
    if(same_idx > 1){
        int x = rng(same_idx - 1);
        do_idx = SAME[x];
    }
    // printf("return: %d\n", root_moves[do_idx]);

    return root_moves[do_idx];
}

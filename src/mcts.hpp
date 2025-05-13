#ifndef MCTS_HPP
#define MCTS_HPP

#include "4T_GST.hpp"
#include "node.hpp"
#include <vector>
#include <memory>
#include <random>
#include <cmath>

class MCTS {
private:
    int simulations;
    std::mt19937 rng;
    std::unique_ptr<Node> root; // 添加根節點成員

    void selection(Node*& node);
    void expansion(Node* node);
    int simulation(GST& state);
    void backpropagation(Node* node, int result);
    double calculateUCB(const Node* node) const;
    bool would_eat_enemy_red(const GST& game, int piece, int dst) const;

    // 模擬移動相關的常量
    static constexpr int dir_val[4] = {-6, -1, 1, 6}; // 上、左、右、下

public:
    explicit MCTS(int simulations);
    void reset();  // 添加 reset 方法
    int findBestMove(GST& game);
};

#endif // MCTS_HPP

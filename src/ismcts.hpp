#ifndef ISMCTS_HPP
#define ISMCTS_HPP

#include "4T_GST.hpp"
#include "node.hpp"

class ISMCTS {
private:
    int simulations;
    std::mt19937 rng;
    std::unique_ptr<Node> root;

    void selection(Node*& node, GST& determinizedState);
    void expansion(Node* node, const GST& determinizedState);
    double simulation(GST& state, DATA& d);
    void backpropagation(Node* node, double result);
    double calculateUCB(const Node* node) const;
    GST getDeterminizedState(const GST& originalState, int current_iteration);
    void randomizeUnrevealedPieces(GST& state, int current_iteration);
    std::unordered_map<std::string, std::pair<int, int>> arrangement_stats;

    // 模擬移動相關的常量
    static constexpr int dir_val[4] = {-6, -1, 1, 6}; // 上、左、右、下

public:
    ISMCTS(int simulations);
    void reset();
    int findBestMove(GST& game, DATA& d);
    void printNodeStats(const Node* node, int indent = 0) const;
};

#endif // ISMCTS_HPP

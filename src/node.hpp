#ifndef NODE_HPP
#define NODE_HPP
#include "4T_GST.hpp"

#include "4T_GST.hpp"
#include <vector>
#include <memory>

class Node {
public:
    GST state;
    int move;
    double wins;
    int visits;
    Node* parent;
    std::vector<std::unique_ptr<Node>> children;

    Node(GST state, int move = -1);

    static void cleanup(std::unique_ptr<Node>& node) {
        if (node) {
            // 遞迴清理子節點
            for (auto& child : node->children) {
                cleanup(child);
            }
            // 清理當前節點
            node->children.clear();
            // 重置根節點
            node.reset();
        }
    }
};

#endif // NODE_HPP

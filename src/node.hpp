#ifndef NODE_HPP
#define NODE_HPP

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
            // 先遞歸清理所有子節點
            for (auto& child : node->children) {
                cleanup(child);
            }
            // 清空子節點列表
            node->children.clear();
            // 最後重置根節點
            node.reset();
        }
    }
};

#endif // NODE_HPP

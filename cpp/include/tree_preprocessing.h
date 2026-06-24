
#include <set>

struct NodeWithParent {
    Node *v;
    Node *p;
};

int get_order_of_op_node(Node *v) {
    assert(v != nullptr);
    assert(v->type == NodeType::Op_Bin);
    assert(v->children.size() == 3);
    assert(v->children[1]->type == NodeType::Leaf);

    //AND, OR, GR, LE, GR_E, LE_E, EQ, PLUS, MINUS, MUL, DIV
    using enum TokenType;
    switch(v->children[1]->tok().type) {
        case OR: return 0;
        case AND: return 1;
        case GR: case LE: case GR_E: case LE_E: case EQ: return 2;
        case PLUS: case MINUS: return 3;
        case MUL: case DIV: return 4;
        default: assert(false && "UNEXPECTED TOKEN TYPE"); return -1;
    } 
}

std::vector<Node*> build_path_rightmost(Node* cur) {
    std::vector<Node*> path;
    while (cur->type == NodeType::Op_Bin) {
        path.push_back(cur);
        assert(cur->children.size() == 3);
        cur = cur->children[2]; 
    }
    return path;
}

Node* _sort_node_Op_Bin(Node *cur) {
    assert(cur->type == NodeType::Op_Bin);

    auto path = build_path_rightmost(cur);
    Node* lead = path.back();
    path.pop_back();
    while (!path.empty()) {
        auto cur = path.back();
        path.pop_back();
        auto cur_order = get_order_of_op_node(cur); 

        auto in = lead;
        auto p_in = lead;
        while (in->type == NodeType::Op_Bin && cur_order >= get_order_of_op_node(in)) {
            assert(in->children.size() == 3);
            p_in = in;
            in = in->children[0];
        }

        if (in->type != NodeType::Op_Bin) {
            p_in->children[0] = cur;
            cur->children[2] = in;
        }
        else if (in == lead) {
            cur->children[2] = lead;
            lead = cur;
        }
        else {
            p_in->children[0] = cur;
            cur->children[2] = in;
        }
    }

    return lead;
}


std::set<Node*> assemble_nodes(Node *root) {
    std::set<Node*> set;
    std::vector<Node*> st = {root};
    while (!st.empty()) {
        Node* cur = st.back();
        st.pop_back();
        set.insert(cur);
        for (auto child: cur->children) {
            st.push_back(child);
        }
    }
    return set;
}


void sort_Op_Bin_nodes(Node** root) {

    std::vector<NodeWithParent> st = {NodeWithParent{*root, nullptr}};

    while (st.size() > 0) {
        auto cur = st.back();
        st.pop_back();


        if (cur.v->type == NodeType::Op_Bin && (!cur.p || cur.p->type != NodeType::Op_Bin)) {

            #define DEBUG

            if (cur.p) {
                #ifdef DEBUG
                auto prevSet = assemble_nodes(cur.p);
                #endif

                auto curPos = std::find(cur.p->children.begin(), cur.p->children.end(), cur.v);
                assert(curPos != cur.p->children.end());
                *curPos = _sort_node_Op_Bin(cur.v);

                #ifdef DEBUG
                auto afterSet = assemble_nodes(cur.p);
                assert(prevSet == afterSet);
                #endif
            }
            else {
                #ifdef DEBUG
                auto prevSet = assemble_nodes(*root);
                #endif

                *root = _sort_node_Op_Bin(cur.v);

                #ifdef DEBUG
                auto afterSet = assemble_nodes(*root);
                assert(prevSet == afterSet);
                #endif
            }
            #undef DEBUG
            
        }
        else {
            for (auto child: cur.v->children) {
                st.push_back(NodeWithParent{.v = child, .p = cur.v});
            }
        }
    }
}

void remove_Plug_nodes(Node **root) {
    assert(root && *root);

    std::vector<NodeWithParent> stack = { NodeWithParent{.v = *root, .p = nullptr} };

    while (!stack.empty()) {
        auto cur = stack.back();
        stack.pop_back();
        if (cur.v->type == NodeType::Plug) {
            assert(cur.v->children.size() == 0 && "remove_Plug_nodes: Plug node should have ZERO children");
            assert(cur.p && "remove_Plug_nodes: Plug node should have parent node");
            auto pos = std::find(cur.p->children.begin(), cur.p->children.end(), cur.v);
            assert(pos != cur.p->children.end() && "remove_Plug_nodes: expected child inside parent");
            cur.p->children.erase(pos);
            continue;
        }
        for (auto child: cur.v->children) {
            stack.push_back({.v = child, .p = cur.v});
        }
    }
}

void remove_empty_Any_nodes(Node **root) {
    assert(root && *root);
    std::vector<NodeWithParent> stack = { NodeWithParent{.v = *root, .p = nullptr} };
    while (!stack.empty()) {
        auto cur = stack.back();
        stack.pop_back();
        
        if (cur.v->type == NodeType::Any && cur.v->children.empty()) {
            Destruct d{[&cur](){ delete cur.v; cur.v = nullptr; }};

            if (!cur.p) {
                assert (cur.v == *root);
                *root = nullptr;
                continue;
            }

            auto pos = std::find(cur.p->children.begin(), cur.p->children.end(), cur.v);
            assert(pos != cur.p->children.end() && "remove_Plug_nodes: expected child inside parent");
            cur.p->children.erase(pos);
            
            continue;
        }
        
        for (auto child: cur.v->children) {
            stack.push_back({.v = child, .p = cur.v});
        }
    }
    
}

void preprocess_tree(Node **root) {
    remove_Plug_nodes(root);
    remove_empty_Any_nodes(root);
    sort_Op_Bin_nodes(root);
}
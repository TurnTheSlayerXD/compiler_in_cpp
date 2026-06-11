

int get_order_of_op_node(Node *v) {
    assert(v != nullptr);
    assert(v->type == NodeType::Op_Bin);
    assert(v->children.size() == 3);
    assert(v->children[1].type == NodeType::Leaf);

    //AND, OR, GR, LE, GR_E, LE_E, EQ, PLUS, MINUS, MUL, DIV
    using enum class TokenType;
    switch(v->children[1].tok.type) {
        case OR: return 0;
        case AND: return 1;
        case GR: case LE: case GR_E: case LE_E: case EQ: return 2;
        case PLUS: case MINUS: return 3;
        case MUL: case DIV: return 4;
        default: assert(false && "UNEXPECTED TOKEN TYPE"); return -1;
    } 
}


void _traverse_Op_Bin(NodeWithParent cur) {

    Node* lead = cur->v;
    Node *p = lead;
    Node *v;
    int leadOrd; 
    
    while (true) {
        assert(lead->children.size() == 3);
        p = lead;
        v = lead->children[2];
        leadOrd = get_order_of_op_node(lead); 
        while (v->type == NodeType::Op_Bin && get_order_of_op_node(v) > leadOrd) {
            assert(v->children.size() == 3);
            p = v;
            v = v->children[2];            
        }
        if (v->type != NodeType::Op_Bin) {
            break;
        }
        p->children[2] = v->children[0];
        v->children[0] = lead;
        lead = v;
    }

}

struct NodeWithParent {
    Node *v;
    Node *p;
};

void sort_Op_Bin_nodes(Node* v) {

    std::vector<NodeWithParent> stack = {v};

    while (stack.size() > 0) {
        auto cur = stack.pop_back();
        if (cur.v->type == NodeType::Op_Bin && (!cur.p || cur.p->type != NodeType::Op_Bin)) {
            _traverse_Op_Bin(cur);
        }
        else {
            for (auto child: cur.v->children) {
                stack.push_back(NodeWithParent{.v = child, .p = cur.v});
            }
        }
    }

}

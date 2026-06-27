
#ifndef NODE_H
#define NODE_H

#include <cassert>
#include <tokenizer.h>

enum class NodeType {
    Plug,


    Op_Bin,
    Op_Un,
    Brace,

    Op_Call,
    Op_Call_Brace,

    Op_Comma,

    Subscr,
    Op_Subscr,

    Leaf,

    Assignment,

    Lvalue,

    StructDecl,
    TypespecStar,
    TypespecSubscr,
    TypeUse,
    VarDecl,
    VarDeclWithAssign,
    Statement,

    FunDecl,
    
    ForStatement,
    WhileStatement,
    Break,
    Continue,
    
    IfStatement,

    IfBranch,
    ElseIfBranch,
    ElseBranch,
    
    Any,
    OneOrMore,

    FunDeclParams,
    Call_Args,
};
std::string_view to_string(NodeType tp) {
    using enum NodeType;
    switch (tp) {
        case Leaf: return "Leaf";
        case Any: return "Any";
        case OneOrMore: return "OneOrMore";

        case Brace: return "Brace";

        case Op_Bin: return "Bin Op";
        case Op_Un: return "Un Op";

        case Op_Call: return "Call op";
        case Op_Comma: return "Comma op";
        case Op_Call_Brace: return "Call braces";
        
        case Subscr: return "Subscript";
        case Op_Subscr: return "Subscript op";

        case Assignment: return "Assignment";

        case Lvalue: return "Lvalue";

        case StructDecl : return "Struct decl";
        case TypespecStar : return "Typespec *";
        case TypespecSubscr : return "Typespec []";
        case TypeUse : return "Typeuse";
        case VarDecl : return "Var decl";
        case VarDeclWithAssign: return "Var decl with assignment";
        case Statement : return "Statement";

        case Plug: return "Plug";

        case FunDecl: return "FunDecl";

        case ForStatement: return "For";
        case WhileStatement: return "While";

        case IfStatement : return "IfStatement";
        case IfBranch : return "If";
        case ElseIfBranch : return "Else If";
        case ElseBranch : return "Else";

        case Break: return "Break";
        case Continue: return "Continue";

        case FunDeclParams : return "Fun decl params";
        case Call_Args: return "Call fun args";

        default: assert(false && "Unexpected"); return "";
    }
}

// template <>
// struct std::formatter<NodeType> : std::formatter<std::string> {
//   auto format(NodeType type, format_context& ctx) const {
//     return formatter<string>::format(
//       std::format("{}", to_string(type)), ctx);
//   }
// };

std::ostream& operator<<(std::ostream& o, NodeType type) {
    o << to_string(type);
    return o;
}

struct NodePrintOpts {
    size_t indentStep = 4;
    std::string_view newLine = "\n\r";
};
NodePrintOpts _PO;

struct Node;
static Node* node_plug();

struct Node {

    std::vector<Node*> children;
    const NodeType type;
    
private:
    const Token _tok;

public:
    Node(NodeType type): type{type}, _tok{} {}
    Node(NodeType type, Token relatedToken): type{type}, _tok{relatedToken} { assert(type == NodeType::Leaf); }

    Node(const Node& rhs) = delete;
    Node(Node&& rhs) noexcept = default;
    Node& operator=(const Node& rhs) = delete;
    Node& operator=(Node&& rhs) = delete;

    ~Node() {
        for (auto &child: children) {
            if (child != node_plug()) {
                delete child;
            }
            child = nullptr;    
        }
    }

    std::string_view text() {
        return tok().text;
    }

    Token tok() {
        assert(type == NodeType::Leaf && "Should not be accessed");
        return _tok;
    }

    Node* child(size_t i) {
        if (i >= children.size()) {
            return nullptr;
        }
        return children[i];
    }

    std::string to_string() {
        if (type == NodeType::Leaf) {
            return std::string(::to_string(type)) + " `"+ std::string(_tok.text) + "`";
        }
        return std::string(::to_string(type));
    }

    std::string get_str_repr(NodePrintOpts opts = { .indentStep = 4, .newLine = "\n\r" }) {
        _PO = opts;
        std::string buf;
        _print(0, this, buf);
        return buf;
    }

    Cursor* get_first_cursor() {
        static_assert(false && "NOT IMPLEMENTED");
    }

    static void set_indent(size_t count, std::string&buf) {
        for (size_t i = 0; i < count; ++i) {
            buf.push_back(' ');
        }
    }

    static void _print(size_t indent, Node *v, std::string &buf) {
        set_indent(indent, buf);
        buf += v->to_string();
        if (v->children.size() > 0) {
            buf += std::string_view(": {");
            buf += _PO.newLine;
            for (auto child: v->children) {
                _print(indent + _PO.indentStep, child, buf);
            }
            set_indent(indent, buf);
            buf += std::string_view("},");
            buf += _PO.newLine;
        }
        else {
            buf += std::string_view(",");
            buf += _PO.newLine; 
        }
    }

};

static Node _plugSt(NodeType::Plug);
static Node* node_plug() {
    return &_plugSt;
}

#endif

#ifndef NODE_H
#define NODE_H

#include <cassert>
#include <tokenizer.h>

enum class NodeType {
    Op,
    Op_Un,
    Op_Brace,
    Op_Call,

    Op_Call_Brace,
    Op_Call_Brace_Seq,

    Op_Comma,
    Op_Comma_Seq,

    Leaf,
};
std::string_view to_string(NodeType tp) {
    using enum NodeType;
    switch (tp) {
        case Op: return "Op";
        case Op_Un: return "Unar Op";
        case Op_Brace: return "Brace Op";
        case Op_Call: return "Call op";
        case Leaf: return "Leaf";
        case Op_Comma: return "Comma op";
        case Op_Comma_Seq: return "Comma seq";
        case Op_Call_Brace: return "Call braces";
        case Op_Call_Brace_Seq: return "Seq Call braces";
        default: assert(false && "Unexpected"); return "";
    }
}

template <>
struct std::formatter<NodeType> : std::formatter<std::string> {
  auto format(NodeType type, format_context& ctx) const {
    return formatter<string>::format(
      std::format("{}", to_string(type)), ctx);
  }
};


struct NodePrintOpts {
    size_t indentStep = 4;
} po;

struct Node {

    std::vector<Node*> children;
    NodeType _type;
    Token relatedToken;

    Node(NodeType type): _type{type} {}
    Node(NodeType type, Token relatedToken): _type{type}, relatedToken{relatedToken} {}
    
    ~Node() {
        for (auto child: children) {
            delete child;
        }
    }

    std::string_view to_string() {
        return ::to_string(_type);
    }

    std::string get_str_repr() {
        std::string buf;
        _print(0, this, buf);
        return buf;
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
            buf += ": {";
            buf += "\n\r";
            for (auto child: v->children) {
                _print(indent + po.indentStep, child, buf);
            }
            set_indent(indent, buf);
            buf += "},\n\r";
        }
        else {
            buf += ",\n\r"; 
        }
    }

};

#endif
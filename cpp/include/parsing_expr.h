#ifndef PARSING_EXPR_H
#define PARSING_EXPR_H

#include <parser.h>


const Expr* init_parsing_expr(Parser& p) {
    using enum TokenType;

    auto operand = p.or_("bracket_op", "un_op", "brace", WORD, NUM_INT, NUM_FLOAT, CHAR, STRING);

    auto unOp = p.or_(
        p.seq(NodeType::Op_Un, p.or_(PLUS, MINUS, MUL, ADDR, INCR, DECR), operand)
    )
    ->set_name("un_op");

    (void) unOp;

    auto binSigns = p.or_(AND, OR, GR, LE, GR_E, LE_E, EQ, PLUS, MINUS, MUL, DIV);

    auto binOp = p.seq(
        NodeType::Op_Bin,
         operand, binSigns, "rvalue"
    )->set_name("bin_op");

    auto brace = p.seq(NodeType::Brace,
    /**/L_BR, "rvalue", R_BR/**/
    )->set_name("brace");
    (void)(brace);

    auto callArgs = p.seq(NodeType::Call_Args,
        "rvalue", p.any(p.seq(NodeType::Op_Comma, COMMA, "rvalue"))
    );

    auto bracketSeq = p.one_or_more(
        p.or_(
            p.seq(NodeType::Op_Call_Brace, L_BR, p.opt(callArgs), R_BR),
            p.seq(NodeType::Subscr, L_SUBSCR, "rvalue", R_SUBSCR)
        
        ))->set_name("bracket_seq");

    auto bracketOp = 
        p.seq (
            NodeType::Op_Call,
            p.or_("brace", WORD, NUM_INT, NUM_FLOAT), bracketSeq
    )->set_name("bracket_op"); 
    (void)(bracketOp);


    auto rvalue = p.or_(binOp, operand)
    ->set_name("rvalue");

    auto lvalue = p.or_(p.seq(NodeType::Lvalue, MUL, operand), WORD);

    auto assign = p.seq(NodeType::Assignment, lvalue, ASSIGN, rvalue);

    auto expr = p.or_(assign, rvalue, lvalue
        )->set_name("expr");

    auto typespecStar = p.any(p.seq(NodeType::TypespecStar, MUL, p.opt(KWD_CONST)));

    auto typespecSubscr = p.any(p.seq(NodeType::TypespecSubscr, L_SUBSCR, p.opt(NUM_INT), R_SUBSCR));
    
    auto typeinfer = p.seq(NodeType::TypeUse, p.opt(KWD_CONST)->set_name("opt kwd const"), p.typeuse(), p.opt(typespecStar))->set_name("typeinfer");

    auto varDecl = p.seq(NodeType::VarDecl, typeinfer, WORD, p.opt(typespecSubscr))->set_name("var_decl");
    
    auto varDeclWithAssign = p.seq(NodeType::VarDeclWithAssign, varDecl, ASSIGN, rvalue)->set_name("var_decl_with_assign");

    auto semiStatement = p.seq(NodeType::Statement, p.or_(varDeclWithAssign, varDecl, expr, KWD_BREAK, KWD_CONTINUE)->set_name("without_semi_statement"), SEMICOLON)->set_name("semi_statement");

    auto forStatement = p.seq(NodeType::ForStatement, 
        KWD_FOR, L_BR, semiStatement, expr, SEMICOLON, expr, R_BR, L_CURL, "statement_any", R_CURL);

    auto whileStatement = p.seq(NodeType::WhileStatement, 
        KWD_WHILE, L_BR, expr, R_BR, L_CURL, "statement_any", R_CURL);

    auto ifStatement = p.seq(NodeType::IfStatement, 
        p.seq(NodeType::IfBranch, KWD_IF, L_BR, expr, R_BR, L_CURL, "statement_any", R_CURL),
        p.any(
            p.seq(NodeType::ElseIfBranch, KWD_ELSE, KWD_IF, L_BR, expr, R_BR, L_CURL, "statement_any", R_CURL)
        ),
        p.opt(
            p.seq(NodeType::ElseBranch, KWD_ELSE, L_CURL, "statement_any", R_CURL)
        )
    );

    auto singleStatement = p.or_(semiStatement, forStatement, whileStatement, ifStatement)->set_name("single_statement");

    auto statementAny = p.any(singleStatement
        )->set_name("statement_any");

    auto funDeclParams = p.opt(
        p.seq(NodeType::FunDeclParams, p.or_(varDecl, typeinfer), p.any(p.seq(NodeType::Op_Comma, COMMA, p.or_(varDecl, typeinfer))))
    );

    auto funDeclBraces = p.seq(NodeType::Op_Call_Brace, 
        L_BR, p.opt(funDeclParams), R_BR
    );

    auto funDecl = p.seq(NodeType::FunDecl, typeinfer, WORD, funDeclBraces, SEMICOLON)->set_name("fun_decl");

    auto funDeclWithBody = p.seq(NodeType::FunDecl, typeinfer, WORD, funDeclBraces, L_CURL, statementAny, R_CURL)->set_name("fun_decl_with_body");

    auto _final = p.one_or_more(p.or_(funDecl, funDeclWithBody, singleStatement)->set_name("or_fun_or_funbody_or_singlestatement"))->set_name("_final");
//______cycle check______
    bool isCycled = p.detect_cycles(_final);
    
    if (isCycled) {
        std::cout << "________________________________________" << std::endl;
        std::cout << "WARNING: DETECTED CYCLES IN NODE PROVIDED" << std::endl;
        std::cout << "________________________________________" << std::endl;
        return nullptr;
    }

    return _final;
}




#endif

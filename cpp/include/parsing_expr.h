#ifndef PARSING_EXPR_H
#define PARSING_EXPR_H

CExpr* get_parsing_expr(Parser& p) {
    using enum TokenType;

    auto opSign = p.or_(PLUS, MINUS, MUL, DIV, GR, LE, GR_E, LE_E, EQ, AND, OR);

    auto operand = p.or_("brace", WORD, NUM_INT, NUM_FLOAT);

    auto unOp = p.or_(
        "call_op", 
        p.seq(NodeType::Op_Un, p.or_(PLUS, MINUS, MUL, ADDR, INCR, DECR), p.or_("un_op", operand))
    )
    ->set_name("un_op");

    auto binOp = p.seq(
        NodeType::Op_Bin,
    /**/p.or_(unOp, operand), opSign, "rvalue"/**/
    )->set_name("bin_op");

    auto brace = p.seq( NodeType::Brace,
    /**/L_BR, "expr", R_BR/**/
    )->set_name("brace");
    (void)(brace);

    auto commaOp = p.seq(NodeType::Op_Comma,
        "rvalue", p.any(p.seq(NodeType::Op_Comma, COMMA, "rvalue"))
    );

    auto callBraces = p.one_or_more(
        p.or_(
            p.seq(NodeType::Op_Call_Brace, L_BR, p.opt(commaOp), R_BR),
            p.seq(NodeType::Subscr, L_SUBSCR, "rvalue", R_SUBSCR)
        )
        )->set_name("call_braces");

    auto callOp = 
        p.seq (
            NodeType::Op_Call,
            operand, callBraces
    )->set_name("call_op"); 
    (void)(callOp);


    auto rvalue = p.or_(binOp, unOp, operand)
    ->set_name("rvalue");

    auto lvalue = p.or_(p.seq(NodeType::Lvalue, MUL, p.or_(unOp, operand)), operand);

    auto assign = p.seq(NodeType::Assignment, lvalue, ASSIGN, rvalue);

    auto expr = p.or_(assign, rvalue, lvalue
        )->set_name("expr");

    auto typespecStar = p.any(p.seq(NodeType::TypespecStar, MUL));

    auto typespecSubscr = p.any(p.seq(NodeType::TypespecSubscr, L_SUBSCR, p.opt(NUM_INT), R_SUBSCR));
    
    auto typeinfer = p.seq(NodeType::TypeUse, p.opt(KWD_CONST), p.typeuse(), p.opt(typespecStar));

    auto varDecl = p.seq(NodeType::VarDecl, typeinfer, WORD, p.opt(typespecSubscr));
    
    auto varDeclWithAssign = p.seq(NodeType::VarDeclWithAssign, varDecl, ASSIGN, rvalue);

    auto semiStatement = p.seq(NodeType::Statement, p.or_(varDeclWithAssign, varDecl, expr, KWD_BREAK, KWD_CONTINUE), SEMICOLON);


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

    auto singleStatement = p.or_(semiStatement, forStatement, whileStatement, ifStatement);

    auto statementAny = p.any(singleStatement
        )->set_name("statement_any");

    auto funDeclArgs = p.opt(
        p.seq(NodeType::Op_Comma, varDecl, p.any(p.seq(NodeType::Op_Comma, COMMA, varDecl)))
    );

    auto funDeclBraces = p.seq(NodeType::Op_Call_Brace, 
        L_BR, p.opt(funDeclArgs), R_BR
    );

    auto funDecl = p.seq(NodeType::FunDecl, typeinfer, WORD, funDeclBraces, SEMICOLON);

    auto funDeclWithBody = p.seq(NodeType::FunDecl, typeinfer, WORD, funDeclBraces, L_CURL, statementAny, R_CURL);

    auto _final = p.or_(funDecl, funDeclWithBody, statementAny);
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

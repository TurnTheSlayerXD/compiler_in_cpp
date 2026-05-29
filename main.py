

# "[w]"
# "[+]"
# "[-]"
# "[(]"
# "[)]"
# "[*]"
# "[/]"

# "[op]=[+]|[-]|[*]|[/]"

# "[un]=[op][w]"

# "[bin]=[w][op][w]"

# //"[expr]=[bin]|[expr]"

# "[expr]= [bin] | [un] | ([expr] [op] [expr])"

from enum import auto, Flag

from tokenizer import Tokenizer, Token, TokenType, TextPos

class NodeType(Flag):
    WORD = auto()
    PLUS = auto()

class Node:
    type: NodeType | None
    subnodes: list

    def __init__(self, nodetype: NodeType | None):
        self.nodetype = nodetype
        self.subnodes = []

class NodeWord(Node):
    text: str
    text_pos: TextPos

    def __init__(self, nodetype: NodeType | None, token: Token):
        super().__init__(nodetype)
        self.text = token.text
        self.text_pos = token.text_pos

class RuleSet:
    rules: list

    def __init__(self):
        self.rules = []

    def add_rule(self, rule):
        if isinstance(rule.name, str): 
            if not rule.name:
                raise Exception("Empty rule name")
            for r in self.rules:
                if isinstance(r.name, str) and r.name == rule.name:
                    raise Exception("RuleSet: Trying add rule with same name")
            self.rules.append(rule)

    def get_rule(self, name: str):
        if not isinstance(name, str):
            raise Exception(f"RuleSet: Expected name arg to be String instance, actual = [{name}]")
        for r in self.rules:
            if r.name == name:
                return r
        raise Exception(f"RuleSet: No rule with name = [{name}]")
    
class Rule:
    name: str | None
    nodetype: NodeType | None
    ruleset: RuleSet

    def __init__(self, nodetype: NodeType | None):
        self.nodetype = nodetype
        self.name = None
        pass

    def set_name(name: str):
        if not isinstance(name, str):
            raise Exception(f"set_name: Expected name to be of type [str], actual = {name}")

    def try_match(tokenizer: Tokenizer, ruleset: RuleSet) -> Node | None:
        raise Exception("Abstract method try_match")


class WordRule(Rule):
    tok_type: TokenType

    def __init__(self, nodetype: NodeType | None, tok_type: TokenType):
        super().__init__(nodetype)
        self.tok_type = tok_type

    def try_match(self, tokenizer: Tokenizer, ruleset: RuleSet):
        if tokenizer.eof():
            return None
        init_pos = tokenizer.get_cur_pos()
        tok = tokenizer.next_tok()
        if tok.type != self.tok_type:
            tokenizer.reset_pos(init_pos)
            return None
        return NodeWord(self.nodetype, tok)


class SeqRule(Rule):
    seq_rules: tuple[Rule]

    def __init__(self, nodetype: NodeType | None, *seq_rules: Rule):
        super().__init__(nodetype)
        self.seq_rules = seq_rules

    def try_match(self, tokenizer: Tokenizer, ruleset: RuleSet):
        init_pos = tokenizer.get_cur_pos()
        node = Node(self.nodetype)

        for rule in self.seq_rules:
            if tokenizer.eof():
                tokenizer.reset_pos(init_pos)
                return None
            child = rule.try_match(tokenizer, ruleset)
            if not child:
                tokenizer.reset_pos(init_pos)
                return None
            node.subnodes.append(child)

        return node

class OrRule(Rule):
    or_rules: tuple[Rule]

    def __init__(self, nodetype: NodeType | None, *or_rules: Rule | str):
        super().__init__(nodetype)
        self.or_rules = or_rules

    def try_match(self, tokenizer: Tokenizer, ruleset: RuleSet):
        init_pos = tokenizer.get_cur_pos()
        for rule in self.or_rules:
            if isinstance(rule, str):
                rule = self.ruleset.get_rule(rule)
            subnode = rule.try_match(tokenizer, ruleset)
            if subnode:
                return subnode
            tokenizer.reset_pos(init_pos)
        return None


class AnyRule(Rule):
    rule_repeated: Rule

    def __init__(self, nodetype: NodeType | None, rule_repeated: Rule | str):
        super().__init__(nodetype)
        self.rule_repeated = rule_repeated

    def try_match(self, tokenizer: Tokenizer, ruleset: RuleSet):
        node = Node(self.nodetype)
        while True:
            if tokenizer.eof():
                break
            pos = tokenizer.get_cur_pos()
            child = self.rule_repeated.try_match(tokenizer, ruleset)
            if not child:
                tokenizer.reset_pos(pos)
                break
            node.subnodes.append(child)

        return node


class OneOrMoreRule:
    rule_repeated: Rule

    def __init__(self, nodeType: NodeType | None, *rule_repeated: Rule | str):
        super().__init__(nodeType)
        self.rule_repeated = rule_repeated

    def try_match(self, tokenizer: Tokenizer, ruleset: RuleSet):
        node = Node(self.nodetype)
        pos = tokenizer.get_cur_pos()
        child = self.rule_repeated.try_match(tokenizer, ruleset)
        if not child:
            tokenizer.reset_pos(pos)
            return None
        while True:
            if tokenizer.eof():
                break
            pos = tokenizer.get_cur_pos()
            child = self.rule_repeated.try_match(tokenizer, ruleset)
            if not child:
                tokenizer.reset_pos(pos)
                break
            node.subnodes.append(child)
        return node

def rule_with_name(constructor, name, nodetype, *args):
    rule = constructor(nodetype, *args)
    rule.name = name
    return rule


def main():

    tokenizer = Tokenizer("")

    ruleset = RuleSet()

    ruleset.add_rule(
        rule_with_name(WordRule, "+", NodeType.PLUS, TokenType.T_PLUS)
    )
    
    print(ruleset.rules)
    
    pass


if __name__ == "__main__":
    main()

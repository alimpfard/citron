from lexer import Lexer
import ast

class CtrParserError(Exception):
    def __init__(self, t, msg):
        super().__init__("%s: %s" % (ast.describe_token(t), msg))

class Parser:
    def __init__(self, prg, path="stdin"):
        self.lexer = Lexer(prg)
        self.current_program = path
        self.raw_prgstr = prg

    def parse(self):
        program = self.parse_program()
        program.value = path
        program.type = ast.ast("PROGRAM")
        return program

    def parse_program(self):
        program = ast.node("PROGRAM")
        pli = None
        first = True
        while True:
            li = self.parse_statement()
            if first:
                first = False
                program.nodes = li
            else:
                pli.next = li
            if li.node is None or li.node.type == ast.nodetype("ENDOFPROGRAM"):
                li.node = self.parse_fin()
                break
            pli = li
        return program

    def parse_statement(self):
        li = ast. ListItem.empty()
        t = self.lexer.token()
        self.lexer.putback()

        if t.type == ast.tokentype("TOKEN_FIN"):
            li.node = self.parse_fin()
            return li
        elif t.type == ast.tokentype("TOKEN_RET"):
            li.node = self.parse_ret()
        else:
            li.node = self.parse_expr(0)
        t = lexer.token()
        if t.type != ast.tokentype('TOKEN_DOT'):
            raise CtrParserError(t, "Expected a dot (.).\n")
        return li

    def parse_fin(self):
        f = ast.Node.empty()
        self.lexer.token()
        f.type = ast.nodetype("ENDOFPROGRAM")
        return f

    def parse_ret(self):
        li = ast.ListItem.empty()
        r = ast.Node.empty()
        self.lexer.token()
        r =

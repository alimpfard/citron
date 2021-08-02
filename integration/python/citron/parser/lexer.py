import ast
import token

class CtrLexerError(Exception):
    def __init__(self, msg):
        super().__init__(msg)

class Lexer(object):
    """docstring for Lexer."""
    def __init__(self, program):
        self.code = program
        self.length = len(program)
        self.charpos = 0
        self.old_charpos = 0
        self.older_charpos = 0
        self.string_interpolation = 0
        self.ivarname = None
        self.lineno = 0
        self.old_lineno = 0
        self.verbatim_mode = False
        self.verbatim_mode_insert_quote = 0

    @property
    def current_char(self):
        return self.code[self.charpos]

    @property
    def next_char(self):
        return self.code[self.charpos+1]

    @property
    def snext_char(self):
        return self.code[self.charpos+2]

    def future_slice(self, count):
        return self.code[self.charpos:self.charpos+count]

    def next_nth(self, n):
        return self.code[self.charpos+n]

    def in_bounds(self, count_a):
        return self.charpos + count_a < self.length

    def is_delimiter(self):
        return self.current_char in '(),.: '

    def emit_error(self, message):
        raise CtrLexerError("%s on line: %d. \n" % (message, self.lineno))

    def describe_token(self, tokentype):
        return ast.ASTToken.static_describe(tokentype)

    def putback(self):
        if self.string_interpolation > 0:
            self.string_interpolation -= 1
            return
        self.charpos = self.old_charpos
        self.old_charpos = self.older_charpos
        self.lineno = self.old_lineno

    def set_buffer(self, i, c):
        self.buffer = self.buffer[:i] + c + self.buffer[i+1:]

    def token(self):
        self.older_charpos = self.old_charpos
        self.old_charpos = self.charpos
        self.old_lineno = self.lineno
        self.buffer = ""
        i, comment_mode, preset_token = 0, 0, -1

        if self.string_interpolation == 1:
            preset_token = ast.tokentype("TOKEN_QUOTE")
        elif self.string_interpolation in [2, 4]:
            self.buffer += '+'
            preset_token = ast.tokentype("TOKEN_REF")
        elif self.string_interpolation == 3:
            self.buffer += ivarname
        elif self.string_interpolation == 5:
            self.charpos = self.chareoi
            preset_token = ast.tokentype("TOKEN_QUOTE")

        if self.string_interpolation:
            self.string_interpolation += 1
            return preset_token

        if self.verbatim_mode and self.verbatim_mode_insert_quote == self.charpos:
            return ast.tokentype("TOKEN_QUOTE")

        while self.charpos != self.length and (self.current_char.isspace() or self.current_char == '#' or comment_mode):
            if self.current_char == '\n':
                comment_mode = False
                self.lineno += 1
            if self.current_char == '#':
                comment_mode = True
            self.charpos += 1

        if self.charpos == self.length: return ast.tokentype("TOKEN_FIN")
        if self.current_char == '(':
            self.charpos += 1
            return ast.tokentype("TOKEN_PAROPEN")
        if self.current_char == ')':
            self.charpos += 1
            return ast.tokentype("TOKEN_PARCLOSE")
        if self.current_char == '[':
            self.charpos += 1
            return ast.tokentype("TOKEN_TUPOPEN")
        if self.current_char == ']':
            self.charpos += 1
            return ast.tokentype("TOKEN_TUPCLOSE")
        if self.current_char == '{':
            self.charpos += 1
            return ast.tokentype("TOKEN_BLOCKOPEN")
        if self.current_char == '}':
            self.charpos += 1
            return ast.tokentype("TOKEN_BLOCKCLOSE")
        if self.current_char == '(':
            self.charpos += 1
            return ast.tokentype("TOKEN_PAROPEN")
        if self.current_char == '.':
            self.charpos += 1
            return ast.tokentype("TOKEN_DOT")
        if self.current_char == ',':
            self.charpos += 1
            return ast.tokentype("TOKEN_CHAIN")
        if (self.in_bounds(3) and self.current_char == 'i' and\
            self.next_char == 's' and self.snext_char.isspace()
            ) or (self.current_char == ':' and self.next_char == '='):
                self.charpos += 2
                return ast.tokentype("TOKEN_ASSIGNMENT")
        if self.current_char == ':':
            self.charpos += 1
            return ast.tokentype("TOKEN_COLON")
        if self.current_char in '^↑':
            self.charpos += 1
            return ast.tokentype("TOKEN_RET")
        if self.current_char == "'":
            self.charpos += 1
            return ast.tokentype("TOKEN_QUOTE")
        if self.in_bounds(2) and self.current_char == '-' and self.next_char.isdigit() or self.current_char.isdigit():
            self.set_buffer(i, self.current_char)
            i += 1
            self.charpos += 1
            while self.current_char.isdigit():
                self.set_buffer(i, self.current_char)
                i += 1
                self.charpos += 1
            if self.in_bounds(1) and self.current_char == '.' and not self.next_char.isdigit():
                return ast.tokentype("TOKEN_NUMBER")
            if self.current_char == '.':
                self.set_buffer(i, self.current_char)
                i += 1
                self.charpos += 1
            while self.current_char.isdigit():
                self.set_buffer(i, self.current_char)
                i += 1
                self.charpos += 1
            return ast.tokentype("TOKEN_NUMBER")
        if self.future_slice(4) == 'True' and self.is_delimiter(self.next_nth(5)):
            self.charpos += 4
            return ast.tokentype("TOKEN_BOOLEANYES")

        if self.future_slice(5) == 'False' and self.is_delimiter(self.next_nth(6)):
            self.charpos += 5
            return ast.tokentype("TOKEN_BOOLEANNO")

        if self.future_slice(3) == 'Nil' and self.is_delimiter(self.next_nth(4)):
            self.charpos += 3
            return ast.tokentype("TOKEN_NIL")

        if self.future_slice(2) == '?>':
            self.verbatim_mode = True
            self.charpos += 1
            self.set_buffer(0, '?')
            return ast.tokentype("TOKEN_REF")
        if self.future_slice(1) == '>' and self.verbatim_mode:
            self.verbatim_mode_insert_quote = self.charpos + 1
            self.charpos += 1
            self.set_buffer(0, '>')
            return ast.tokentype("TOKEN_REF")
        while self.in_bounds(0) and not self.current_char.isspace() and self.current_char not in '#(){}[],.^:\'↑':
            self.set_buffer(i, self.current_char)
            i += 1
            self.charpos += 1
        return ast.tokentype("TOKEN_REF")

    def Token(self):
        t = token.CtrToken
        return t(self.token(), self.buffer)

    def readstr(self):
        strbuff = ''
        escape = False
        beginbuf = ''
        if self.string_interpolation == 6:
            self.ivarname = ""
            self.string_interpolation = 0
        while not self.verbatim_mode and (self.current_char != "'" or escape) or self.verbatim_mode and not (self.in_bounds(1) and self.current_char == '<' and self.next_char == '?'):
            if self.in_bounds(1) and not self.verbatim_mode and not escape and self.future_slice(2) == '$$':
                q = 2
                while self.in_bounds(q) and not self.next_nth(q).isspace() and self.next_nth not in '$\'' and q < 255: q += 1
                if self.next_nth(q).isspace() or self.next_nth(q) in '$\'':
                    self.ivarname = self.future_slice(q)[2:]
                    self.string_interpolation = 1
                    self.chareoi = self.charpos + q
                    break
            if self.current_char == '\n': self.lineno += 1
            c = self.current_char
            if escape and c in 'nrtvbaf':
                c = eval("\\"+c)
            if c == '\\' and not escape and not self.verbatim_mode:
                escape = True
                self.charpos += 1
                c = self.current_char
                continue
            escape = False
            strbuff = strbuff + c
            self.charpos += 1
        if self.verbatim_mode:
            if not self.in_bounds(0):
                self.code.append("<?.")
                self.length += 3
            self.charpos += 1
        self.verbatim_mode = False
        self.verbatim_mode_insert_quote = ''
        return strbuff

if __name__ == '__main__':
    lexer = Lexer(input(">>> "))
    t = lexer.Token()
    print (t.describe(), t.value)
    while t.type != ast.tokentype("TOKEN_FIN"):
        t = lexer.Token()
        print (t.describe(), t.value)

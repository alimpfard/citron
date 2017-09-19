import ply.lex as lex

tokens = (
    'NUMBER', 'STRING', 'ID',
    #'BMSSAG',
    'UMSSAG', 'KMSSAG', 'VAR', 'MY', 'CHAIN', 'DOT', 'LPAREN', 'RPAREN', 'IS'
)

t_NUMBER = r'\d+(\.\d+)?'
t_STRING = r"\'[^'|\\\']*\'"
t_VAR    = '\bvar\b'
t_MY     = '\bmy\b'
t_IS     = '\bis\b'
t_CHAIN  = ','
t_DOT    = r'\.'
t_LPAREN = r'\('
t_RPAREN = r'\)'
t_UMSSAG = r'\w+'

def t_KMSSAG(t):
    r'\w+\:'
    return t


def t_ID(t):
    r'[\w_]+'
    if t.value in ['var', 'is', 'my']:
        t.type = t.value.upper()
    return t

def t_newline(t):
    r'\n+'
    t.lexer.lineno += 1

t_ignore = '\t '

def t_error(t):
    print("Illegal character '%s'" % t.value[0])
    t.lexer.skip(1)

lexer = lex.lex()

data = """my test is 'test'.\ntest is Object new: 'test'."""

lexer.input(data)

while True:
    tok = lexer.token()
    if not tok:
        break      # No more input
    print(tok)

from pygments.lexer import RegexLexer, bygroups
from pygments.token import *

import re

__all__=['CitronLexer']

class CitronLexer(RegexLexer):
    name = 'Citron'
    aliases = ['Citron']
    filenames = ['*.ctr']
    flags = re.MULTILINE | re.UNICODE

    tokens = {
        'root' : [
            (u'(?:([^\\s\\d:.,\\[\\]\\(\\)\\{\\}\\#\\n\\r][^\\s:.,\\[\\]\\(\\)\\{\\}\\#\\n\\r]*)(\\s+)(\u2039)([^\u203a\\n\\r]*?)(\u203a))', bygroups(Keyword.Pseudo, Name, String, String.Regex, String)),
            (u'(?:(@comptime)\\b)', bygroups(Generic.Deleted), 'main__1'),
            (u'\\b(var|my|const|frozen)\\b', bygroups(Keyword.Pseudo), 'main__2'),
            (u'(?:(pure)(\\s*\\{))', bygroups(Keyword.Pseudo, Name), 'main__3'),
            (u'(\\$[!,\'`](?!=\\s))', bygroups(String.Escape)),
            (u'(\\$(?=\\())', bygroups(String.Escape)),
            (u'([\\[\\]\\(\\)\\^\\}])', bygroups(Name)),
            (u'(?:(\\s*)(\\bis\\b|\\:\\=|\\=\\>)(\\s*))', bygroups(Generic, Keyword.Reserved, Generic)),
            (u'(?:(\\?>))', bygroups(String.Regex), 'main__4'),
            (u'(0[xX][0-9a-fA-F]+|0[cC][0-7]+|0[bB][01]+|\\d+\\.\\d+|\\d+)', bygroups(Number)),
            (u'(\u2039)', bygroups(String), 'fstring'),
            (u'(\')', bygroups(String), 'string'),
            (u'(?:(\\\\)((?:(?:\\:[^\\s\\d:.,\\[\\]\\(\\)\\{\\}\\#\\n\\r][^\\s:.,\\[\\]\\(\\)\\{\\}\\#\\n\\r]*\\s*)|(?:\\:\\(.*\\)))+))', bygroups(Keyword.Reserved, Keyword.Pseudo), 'main__5'),
            (u'(?:(\\{)(asm)(\\s*(?:\\:)\\s*(?:\\w+))*(\\s*(?:intel|att|at\\&t))?(\\s*\\(.*\\)\\s*))', bygroups(Name, Generic.Deleted, Keyword.Pseudo, Generic.Deleted, Generic.Deleted), 'main__6'),
            (u'(?:(\\{)((?:\\s*\\:(?:(?:[^\\s\\d:.,\\[\\]\\(\\)\\{\\}\\#\\n\\r][^\\s:.,\\[\\]\\(\\)\\{\\}\\#\\n\\r]*)|(?:\\(.*\\))))*))', bygroups(Name, Keyword.Pseudo), 'main__7'),
            (u'(?:(\\#:))', bygroups(Comment), 'main__8'),
            (u'(\\#.*$)', bygroups(Comment)),
            (u'((?:[^\\s\\d:.,\\[\\]\\(\\)\\{\\}\\#\\n\\r][^\\s:.,\\[\\]\\(\\)\\{\\}\\#\\n\\r]*)\\:)', bygroups(Name.Decorator), 'main__9'),
            (u'(\\\\)', bygroups(String.Escape), 'main__10'),
            (u'(?:(\\s+)([^\\s\\d:.,\\[\\]\\(\\)\\{\\}\\#\\n\\r]|[-+*&%$@!=\";/\\\\<>?~]+)(\\s+))', bygroups(Name, Name.Decorator, Name), 'main__11'),
            (u'(?:(\\`)([^\\s\\d:.,\\[\\]\\(\\)\\{\\}\\#\\n\\r][^\\s:.,\\[\\]\\(\\)\\{\\}\\#\\n\\r]*)(\\1))', bygroups(String.Escape, Name.Function, String.Escape), 'main__12'),
            (u'(?:(\\`?)([^\\s\\d:.,\\[\\]\\(\\)\\{\\}\\#\\n\\r][^\\s:.,\\[\\]\\(\\)\\{\\}\\#\\n\\r]*)(\\1))', bygroups(String.Escape, Generic, String.Escape), 'main__13'),
            ('(\n|\r|\r\n)', String),
            ('.', String),
        ],
        'asm' : [
            (u'(.*)', bygroups(Comment)),
            ('(\n|\r|\r\n)', String),
            ('.', String),
        ],
        'expr_interpolation' : [
            (u'\\b(var|my|const|frozen)\\b', bygroups(Keyword.Pseudo), 'expr_interpolation__1'),
            (u'(?:(pure)(\\s*\\{))', bygroups(Keyword.Pseudo, Name), 'expr_interpolation__2'),
            (u'(\\$[!,\'`](?!=\\s))', bygroups(String.Escape)),
            (u'(\\$(?=\\())', bygroups(String.Escape)),
            (u'([\\[\\]\\(\\)\\^\\}])', bygroups(Name)),
            (u'(?:(\\s*)(\\bis\\b|\\:\\=|\\=\\>)(\\s*))', bygroups(Generic, Keyword.Reserved, Generic)),
            (u'(?:(\\?>))', bygroups(String.Regex), 'expr_interpolation__3'),
            (u'(0[xX][0-9a-fA-F]+|0[cC][0-7]+|0[bB][01]+|\\d+\\.\\d+|\\d+)', bygroups(Number)),
            (u'(\')', bygroups(String), 'string'),
            (u'(?:(\\\\)((?:\\:[^\\s\\d:.,\\[\\]\\(\\)\\{\\}\\#\\n\\r][^\\s:.,\\[\\]\\(\\)\\{\\}\\#\\n\\r]*\\s*)+))', bygroups(Keyword.Reserved, Keyword.Pseudo), 'expr_interpolation__4'),
            (u'(?:(\\{)(asm)(\\s*(?:\\:)\\s*(?:\\w+))*(\\s*(?:intel|att|at\\&t))?(\\s*\\(.*\\)\\s*))', bygroups(Name, Generic.Deleted, Keyword.Pseudo, Generic.Deleted, Generic.Deleted), 'expr_interpolation__5'),
            (u'(?:(\\{)((?:\\s*\\:(?:[^\\s\\d:.,\\[\\]\\(\\)\\{\\}\\#\\n\\r][^\\s:.,\\[\\]\\(\\)\\{\\}\\#\\n\\r]*))*))', bygroups(Name, Keyword.Pseudo), 'expr_interpolation__6'),
            (u'(?:(\\#:))', bygroups(Comment), 'expr_interpolation__7'),
            (u'(\\#.*$)', bygroups(Comment)),
            (u'((?:[^\\s\\d:.,\\[\\]\\(\\)\\{\\}\\#\\n\\r][^\\s:.,\\[\\]\\(\\)\\{\\}\\#\\n\\r]*)\\:)', bygroups(Name.Decorator), 'expr_interpolation__8'),
            (u'(\\\\)', bygroups(String.Escape), 'expr_interpolation__9'),
            (u'(?:(\\s+)([^\\s\\d:.,\\[\\]\\(\\)\\{\\}\\#\\n\\r]|[-+*&%$@!=\";/\\\\<>?~]+)(\\s+))', bygroups(Name, Name.Decorator, Name), 'expr_interpolation__10'),
            (u'(?:(\\`)([^\\s\\d:.,\\[\\]\\(\\)\\{\\}\\#\\n\\r][^\\s:.,\\[\\]\\(\\)\\{\\}\\#\\n\\r]*)(\\1))', bygroups(String.Escape, Name.Function, String.Escape), 'expr_interpolation__11'),
            (u'(?:(\\`?)([^\\s\\d:.,\\[\\]\\(\\)\\{\\}\\#\\n\\r][^\\s:.,\\[\\]\\(\\)\\{\\}\\#\\n\\r]*)(\\1))', bygroups(String.Escape, Generic, String.Escape), 'expr_interpolation__12'),
            ('(\n|\r|\r\n)', String),
            ('.', String),
        ],
        'expr_interpolation__1' : [
            ('(\n|\r|\r\n)', String),
            ('.', Name.Variable),
        ],
        'expr_interpolation__10' : [
            ('(\n|\r|\r\n)', String),
            ('.', Name),
        ],
        'expr_interpolation__11' : [
            ('(\n|\r|\r\n)', String),
            ('.', Name),
        ],
        'expr_interpolation__12' : [
            ('(\n|\r|\r\n)', String),
            ('.', Name),
        ],
        'expr_interpolation__2' : [
            (u'(.)', bygroups(Generic)),
            ('(\n|\r|\r\n)', String),
            ('.', String),
        ],
        'expr_interpolation__3' : [
            (u'(.)', bygroups(String.Regex)),
            ('(\n|\r|\r\n)', String),
            ('.', String),
        ],
        'expr_interpolation__4' : [
            ('(\n|\r|\r\n)', String),
            ('.', Generic),
        ],
        'expr_interpolation__5' : [
            (u'(?:\\s*)', bygroups(Name), 'asm'),
            ('(\n|\r|\r\n)', String),
            ('.', String),
        ],
        'expr_interpolation__6' : [
            ('(\n|\r|\r\n)', String),
            ('.', Name),
        ],
        'expr_interpolation__7' : [
            ('(\n|\r|\r\n)', String),
            ('.', Generic.Deleted),
        ],
        'expr_interpolation__8' : [
            ('(\n|\r|\r\n)', String),
            ('.', Name),
        ],
        'expr_interpolation__9' : [
            ('(\n|\r|\r\n)', String),
            ('.', Keyword.Reserved),
        ],
        'fstring' : [
            (u'(.)', bygroups(String)),
            ('(\n|\r|\r\n)', String),
            ('.', String),
        ],
        'main__1' : [
            ('(\n|\r|\r\n)', String),
            ('.', Name),
        ],
        'main__10' : [
            ('(\n|\r|\r\n)', String),
            ('.', Keyword.Reserved),
        ],
        'main__11' : [
            ('(\n|\r|\r\n)', String),
            ('.', Name),
        ],
        'main__12' : [
            ('(\n|\r|\r\n)', String),
            ('.', Name),
        ],
        'main__13' : [
            ('(\n|\r|\r\n)', String),
            ('.', Name),
        ],
        'main__2' : [
            ('(\n|\r|\r\n)', String),
            ('.', Name.Variable),
        ],
        'main__3' : [
            (u'(.)', bygroups(Generic)),
            ('(\n|\r|\r\n)', String),
            ('.', String),
        ],
        'main__4' : [
            (u'(.)', bygroups(String.Regex)),
            ('(\n|\r|\r\n)', String),
            ('.', String),
        ],
        'main__5' : [
            ('(\n|\r|\r\n)', String),
            ('.', Generic),
        ],
        'main__6' : [
            (u'(?:\\s*)', bygroups(Name), 'asm'),
            ('(\n|\r|\r\n)', String),
            ('.', String),
        ],
        'main__7' : [
            ('(\n|\r|\r\n)', String),
            ('.', Name),
        ],
        'main__8' : [
            ('(\n|\r|\r\n)', String),
            ('.', Generic.Deleted),
        ],
        'main__9' : [
            ('(\n|\r|\r\n)', String),
            ('.', Name),
        ],
        'string' : [
            (u'(?:(\\$\\$)(\\w+)\\b)', bygroups(String.Escape, Name.Decorator)),
            (u'(\\\\[^xu\\n\\r])', bygroups(String.Escape)),
            (u'(?:(\\\\[xuXU])([{]?)([0-9a-fA-F]+)([}]?))', bygroups(String.Escape, String.Escape, Number, String.Escape)),
            (u'(\\$\\{\\{)', bygroups(String.Escape), 'expr_interpolation'),
            (u'(.)', bygroups(String)),
            ('(\n|\r|\r\n)', String),
            ('.', String),
        ]
    }

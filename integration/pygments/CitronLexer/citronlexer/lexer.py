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
            (u'\\b(var|my|const|frozen)\\b', bygroups(Keyword.Pseudo), 'main__1'),
            (u'(\\$[!,\'`](?!=\\s))', bygroups(String.Escape)),
            (u'(\\$(?=\\())', bygroups(String.Escape)),
            (u'([\\[\\]\\(\\)\\^\\}])', bygroups(Name)),
            (u'(?:(\\s*)(\\bis\\b|\\:\\=|\\=\\>)(\\s*))', bygroups(Generic, Keyword.Reserved, Generic)),
            (u'(?:(\\?>))', bygroups(String.Regex), 'main__2'),
            (u'(0[xX][0-9a-fA-F]+|0[cC][0-7]|\\d+\\.\\d+|\\d+)', bygroups(Number)),
            (u'(\')', bygroups(String), 'main__3'),
            (u'(?:(\\\\)((?:\\:[^\\s\\d:.,\\[\\]\\(\\)\\{\\}\\#\\n\\r][^\\s:.,\\[\\]\\(\\)\\{\\}\\#\\n\\r]*\\s*)+))', bygroups(Keyword.Reserved, Keyword.Pseudo), 'main__4'),
            (u'(?:(\\{)((?:\\:(?:[^\\s\\d:.,\\[\\]\\(\\)\\{\\}\\#\\n\\r][^\\s:.,\\[\\]\\(\\)\\{\\}\\#\\n\\r]*))*))', bygroups(Name, Keyword.Pseudo), 'main__5'),
            (u'(?:(\\#:)(.*)$)', bygroups(Comment, Generic.Deleted)),
            (u'(\\#.*$)', bygroups(Comment)),
            (u'((?:[^\\s\\d:.,\\[\\]\\(\\)\\{\\}\\#\\n\\r][^\\s:.,\\[\\]\\(\\)\\{\\}\\#\\n\\r]*)\\:)', bygroups(Name.Decorator), 'main__6'),
            (u'(\\\\)', bygroups(String.Escape), 'main__7'),
            (u'(?:(\\s+)([^\\s\\d:.,\\[\\]\\(\\)\\{\\}\\#\\n\\r])(\\s+))', bygroups(Name, Name.Decorator, Name), 'main__8'),
            (u'(?:(\\`?)([^\\s\\d:.,\\[\\]\\(\\)\\{\\}\\#\\n\\r][^\\s:.,\\[\\]\\(\\)\\{\\}\\#\\n\\r]*)(\\1))', bygroups(String.Escape, Generic, String.Escape), 'main__9'),
            ('(\n|\r|\r\n)', String),
            ('.', String),
        ],
        'main__1' : [
            ('(\n|\r|\r\n)', String),
            ('.', Name.Variable),
        ],
        'main__2' : [
            (u'(.)', bygroups(String.Regex)),
            ('(\n|\r|\r\n)', String),
            ('.', String),
        ],
        'main__3' : [
            (u'(\\\\[^xu\\n\\r])', bygroups(String.Escape)),
            (u'(?:(\\\\[xuXU])([{]?)([0-9a-fA-F]+)([}]?))', bygroups(String.Escape, String.Escape, Number, String.Escape)),
            (u'(.)', bygroups(String)),
            ('(\n|\r|\r\n)', String),
            ('.', String),
        ],
        'main__4' : [
            ('(\n|\r|\r\n)', String),
            ('.', Generic),
        ],
        'main__5' : [
            ('(\n|\r|\r\n)', String),
            ('.', Name),
        ],
        'main__6' : [
            ('(\n|\r|\r\n)', String),
            ('.', Name),
        ],
        'main__7' : [
            ('(\n|\r|\r\n)', String),
            ('.', Keyword.Reserved),
        ],
        'main__8' : [
            ('(\n|\r|\r\n)', String),
            ('.', Name),
        ],
        'main__9' : [
            ('(\n|\r|\r\n)', String),
            ('.', Name),
        ]
    }

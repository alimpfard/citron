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
            (u'\\b(var|my|const)\\b', bygroups(Keyword.Pseudo), 'main__1'),
            (u'(\\.)', bygroups(Name), ('#pop', 'root')),
            (u'([\\[\\]\\(\\)\\^\\}])', bygroups(Name)),
            (u'(?:(\\s*)(\\bis\\b|\\:\\=|\\=\\>)(\\s*))', bygroups(Generic, Keyword.Reserved, Generic)),
            (u'(?:(\\?>))', bygroups(String.Regex), 'main__2'),
            (u'(0[xX][0-9a-fA-F]+|0[cC][0-7]|\\d+\\.\\d+|\\d+)', bygroups(Number)),
            (u'(\')', bygroups(String), 'main__3'),
            (u'(?:(\\{)((?:\\:(?:[^\\s\\d:.,\\[\\]\\(\\)\\{\\}\\#\\n\\r][^\\s:.,\\[\\]\\(\\)\\{\\}\\#\\n\\r]*))*))', bygroups(Name, Keyword.Pseudo), 'main__4'),
            (u'(\\#.*$)', bygroups(Comment)),
            (u'((?:[^\\s\\d:.,\\[\\]\\(\\)\\{\\}\\#\\n\\r][^\\s:.,\\[\\]\\(\\)\\{\\}\\#\\n\\r]*)\\:)', bygroups(Name.Decorator), 'main__5'),
            (u'(\\\\)', bygroups(String.Escape), 'main__6'),
            (u'(?:(\\s+)([^\\s\\d:.,\\[\\]\\(\\)\\{\\}\\#\\n\\r])(\\s+))', bygroups(Name, Name.Decorator, Name), 'main__7'),
            (u'([^\\s\\d:.,\\[\\]\\(\\)\\{\\}\\#\\n\\r][^\\s:.,\\[\\]\\(\\)\\{\\}\\#\\n\\r]*)', bygroups(Generic), 'main__8'),
            ('(\n|\r|\r\n)', String),
            ('.', String),
        ],
        'main__1' : [
            (u'([^\\s\\d:.,\\[\\]\\(\\)\\{\\}\\#\\n\\r][^\\s:.,\\[\\]\\(\\)\\{\\}\\#\\n\\r]*)', bygroups(Name.Variable), '#pop'),
            ('(\n|\r|\r\n)', String),
            ('.', Name.Variable),
        ],
        'main__2' : [
            (u'(<\\?)', bygroups(String.Regex), '#pop'),
            (u'(.)', bygroups(String.Regex)),
            ('(\n|\r|\r\n)', String),
            ('.', String),
        ],
        'main__3' : [
            (u'((?!<\\\\)\')', bygroups(String), '#pop'),
            (u'(\\\\[^xu\\n\\r])', bygroups(String.Escape)),
            (u'(?:(\\\\[xuXU])([{]?)([0-9a-fA-F]+)([}]?))', bygroups(String.Escape, String.Escape, Number, String.Escape)),
            (u'(.)', bygroups(String)),
            ('(\n|\r|\r\n)', String),
            ('.', String),
        ],
        'main__4' : [
            (u'(\\s*)', bygroups(Name), '#pop'),
            ('(\n|\r|\r\n)', String),
            ('.', Name),
        ],
        'main__5' : [
            (u'(\\s*)', bygroups(Name), '#pop'),
            ('(\n|\r|\r\n)', String),
            ('.', Name),
        ],
        'main__6' : [
            (u'(?:([^:\\n\\r]\\w*)|(\\:(?:[^\\s\\d:.,\\[\\]\\(\\)\\{\\}\\#\\n\\r][^\\s:.,\\[\\]\\(\\)\\{\\}\\#\\n\\r]*)))', bygroups(String.Escape, Keyword.Reserved), '#pop'),
            ('(\n|\r|\r\n)', String),
            ('.', Keyword.Reserved),
        ],
        'main__7' : [
            (u'(\\s*)', bygroups(Name), '#pop'),
            ('(\n|\r|\r\n)', String),
            ('.', Name),
        ],
        'main__8' : [
            (u'(\\s*)', bygroups(Name), '#pop'),
            ('(\n|\r|\r\n)', String),
            ('.', Name),
        ]
    }

    def analyse_text(text):
        shbang = re.match(r'\#\!.*ctr', text.strip().split('\n', 1)[0]) is not None
        if shbang: return 1
        maybe = len(filter(lambda x: x in text,
            ['Reflect', 'Map', 'String', '?>', '<?',
             'Pen', 'Library/', 'is']))
        return maybe / 8

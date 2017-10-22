CTR_VALID_TOKEN_TYPES = {
    1: "TOKEN_REF", 2: "TOKEN_QUOTE", 3: "TOKEN_NUMBER",
    4: "TOKEN_PAROPEN", 5: "TOKEN_PARCLOSE", 6: "TOKEN_BLOCKOPEN",
    7: "TOKEN_BLOCKCLOSE", 8: "TOKEN_COLON", 9: "TOKEN_DOT",
    10: "TOKEN_CHAIN", 12: "TOKEN_BOOLEANYES", 13: "TOKEN_BOOLEANNO",
    14: "TOKEN_NIL", 15: "TOKEN_ASSIGNMENT", 16: "TOKEN_RET",
    17: "TOKEN_TUPOPEN", 18: "TOKEN_TUPCLOSE", 99: "TOKEN_FIN",
    "TOKEN_UNKNOWN": "TOKEN_UNKNOWN"
}
for k,v in CTR_VALID_TOKEN_TYPES.copy().items(): CTR_VALID_TOKEN_TYPES[v] = k

CTR_TOKEN_DESCRIPTIONS = {
    "TOKEN_REF": "reference",
    "TOKEN_QUOTE": "'",
    "TOKEN_NUMBER": "number",
    "TOKEN_PAROPEN": "(",
    "TOKEN_PARCLOSE": ")",
    "TOKEN_BLOCKOPEN": "{",
    "TOKEN_BLOCKCLOSE": "}",
    "TOKEN_TUPOPEN": "[",
    "TOKEN_TUPCLOSE": "]",
    "TOKEN_COLON": ":",
    "TOKEN_DOT": ".",
    "TOKEN_CHAIN": ",",
    "TOKEN_BOOLEANYES": "True",
    "TOKEN_BOOLEANNO": "False",
    "TOKEN_NIL": "Nil",
    "TOKEN_ASSIGNMENT": ":=",
    "TOKEN_RET": "^",
    "TOKEN_RET_UNICODE": "↑",
    "TOKEN_FIN": "end of program",
    "TOKEN_UNKNOWN": "(unknown token)"
}

CTR_AST_NODES = {
    "EXPRASSIGNMENT": 51,
    "EXPRMESSAGE": 52,
    "UNAMESSAGE": 53,
    "BINMESSAGE": 54,
    "KWMESSAGE": 55,
    "LTRSTRING": 56,
    "REFERENCE": 57,
    "LTRNUM": 58,
    "CODEBLOCK": 59,
    "RETURNFROMBLOCK": 60,
    "IMMUTABLE": 61,
    "PARAMLIST": 76,
    "INSTRLIST": 77,
    "ENDOFPROGRAM": 79,
    "NESTED": 80,
    "LTRBOOLTRUE": 81,
    "LTRBOOLFALSE": 82,
    "LTRNIL": 83,
    "PROGRAM": 84,
}
for k,v in CTR_AST_NODES.copy().items(): CTR_AST_NODES[v] = k


def tokentype(type):
    return CTR_VALID_TOKEN_TYPES.get(type, "TOKEN_UNKNOWN")

def ast(type):
    return 1 if type == 'NODE' else 3

def node(type):
    return Node(CTR_AST_NODES[type])

def nodetype(type):
    return CTR_AST_NODES[type]

class CtrInvalidTokenException(Exception):
    def __init__(self, msg):
        super().__init__(msg)


class ASTToken(object):
    """docstring for ASTToken."""
    def __init__(self, type):
        if type in CTR_VALID_TOKEN_TYPES:
            self.type = type
        else: raise CtrInvalidTokenException(str(type))

    def type_string(self):
        return CTR_VALID_TOKEN_TYPES[self.type]

    def describe(self):
        return CTR_TOKEN_DESCRIPTIONS[CTR_VALID_TOKEN_TYPES[self.type]]

    @staticmethod
    def static_describe(type):
        return CTR_TOKEN_DESCRIPTIONS[CTR_VALID_TOKEN_TYPES.get(type, "TOKEN_UNKNOWN")]

class Node(object):
    def __init__(self, type):
        self.type = type if type in CTR_AST_NODES else None
        self.value = ''
        self.modifier = 0
        self.nodes = [] #listitem

    @staticmethod
    def empty():
        return Node(None)

class ListItem(object):
    def __init__(self, node, next=None):
        self.node = node
        self.next = next

    @staticmethod
    def empty():
        return ListItem(None)

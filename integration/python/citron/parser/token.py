from ast import ASTToken
class CtrToken(ASTToken):
    """docstring for CtrToken."""
    def __init__(self, type, value = ""):
        super().__init__(type)
        self.value = value

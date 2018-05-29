from setuptools import setup, find_packages

setup (
        name='citronlexer',
        packages=find_packages(),
        entry_points =
        """
        [pygments.lexers]
        citronlexer = citronlexer.lexer:CitronLexer
        """,
)

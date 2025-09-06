# lamina_lexer.py：Lamina语言的Pygments Lexer
# 该脚本用于在Github上高亮Lamina文件的颜色
from pygments.lexer import RegexLexer
from pygments.token import Keyword, Text, Comment  # 导入Pygments的“标记类型”（用于高亮分类）

class LaminaLexer(RegexLexer):
    """
    Pygments Lexer for Lamina programming language.
    """
    # 1. 定义Lexer的基本信息（关键！GitHub会通过这些信息关联语言）
    name = 'Lamina'
    aliases = ['lamina']  # 别名
    filenames = ['*.lm']  # 匹配的文件扩展名
    mimetypes = ['text/x-lamina']  # MIME类型

    # 这里先实现最基础的if,while关键词高亮，后续可扩展其他关键词、字符串等
    tokens = {
        # root是语法分析的入口（所有代码从这里开始匹配）
        'root': [
            (r'\b(if|while)\b', Keyword.Control),

            (r'//.*$', Comment.Single),

            (r'.+?', Text),
        ]
    }
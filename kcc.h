#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h> //va_list
#include <stdbool.h> //bool
#include <ctype.h> //isspace
#include <string.h>

/* parce.c */
//enumという列挙型がTokenKindという名前で使えるようになる
typedef enum {
	TK_RESERVED, //記号
    TK_IDENT, //識別子
	TK_NUM, //整数トークン
	TK_EOF, //入力終了のトークン
} TokenKind;

typedef struct Token Token;

//トークン型
struct Token {
	TokenKind kind; //トークンの型
	Token *next; //次の入力トークン
	int val; //TokenKindがTK_NUMのときに数値で受け取っておく用
	char *str; //トークン文字列
    int len; //トークンの長さ 文字列に対応するため
};

typedef struct LVar LVar;
//変数は連結リストで表す
struct LVar {
  LVar *next;
  char *name;
  int len;
  int offset;
};

extern Token *token;
extern char *user_input;
extern LVar *locals;


void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
bool consume(char *op);

//?
Token *consume_ident();

void expect(char *op);
int expect_number();
bool at_eof() ;
Token *new_token(TokenKind kind, Token *cur, char *str, int len);
bool startswith(char *p, char *q);
Token *tokenize();
LVar *find_lvar(Token *tok);







/* codegen.c */
//抽象構文木のノードの種類
typedef enum {
    ND_ADD, // +
    ND_SUB, // -
    ND_MUL, // *
    ND_DIV, // /
    ND_ASSIGN, // =
    ND_EQ,  // ==
    ND_NE,  // !=
    ND_LT,  // <
    ND_LE,  // <=
    ND_LVAR, // ローカル変数
    ND_NUM, //整数
} NodeKind;

typedef struct Node Node;

// 抽象構文木のノードの形
struct Node {
    NodeKind kind; //ノードの型 NUMなのか符号なのか
    Node *lhs; //左辺 子ノード
    Node *rhs; // 右辺 子ノード
    int val; // 数字のノードのときにつかう
    int offset; //変数のノードのときにつかう
};

extern Node *code[];

Node *new_node(NodeKind kind);
Node *new_binary(NodeKind kind, Node *lhs, Node *rhs);
Node *new_num(int val);
Node *assign();
Node *expr();
Node *stmt();
void program();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();
void gen(Node *node);










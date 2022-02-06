#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h> //va_list
#include <stdbool.h> //bool
#include <ctype.h> //isspace
#include <string.h>


//enumという列挙型がTokenKindという名前で使えるようになる
typedef enum {
	TK_RESERVED, //記号
	TK_NUM, //整数トークン
	TK_EOF, //入力終了のトークン
} TokenKind;

//抽象構文木のノードの種類
typedef enum {
    ND_ADD, // +
    ND_SUB, // -
    ND_MUL, // *
    ND_DIV, // /
    ND_NUM, //整数
} NodeKind;

typedef struct Node Node;

// 抽象構文木のノードの形
struct Node {
    NodeKind kind; //ノードの型 NUMなのか符号なのか
    Node *lhs; //左辺 子ノード
    Node *rhs; // 右辺 子ノード
    int val; // 数字のノードのときにつかう
};


typedef struct Token Token;

//トークン型
struct Token {
	TokenKind kind; //トークンの型
	Token *next; //次の入力トークン
	int val; //TokenKindがTK_NUMのときに数値で受け取っておく用
	char *str; //トークン文字列
};

//入力プログラム
char *user_input;


//現在注目しているトークン
Token *token;

//エラーを報告するための関数
//printfと同じ引数をとる
void error(char *fmt, ...) {
    //va_list=可変長引数の入ったchar*型
	va_list ap;
    //(va_list型の変数, 可変引数でない最後の引数)
	va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

//エラーの場所を報告する
void error_at(char *loc, char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);

    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, " "); // pos個の空白を出力
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1); 
}


//次のトークンが期待する記号かを判定する関数
bool consume(char op) {
    //構造体のポインタに->で*.まとめて使ってる感じ
    if (token->kind != TK_RESERVED || token->str[0] != op)
        return false;
    
    token = token->next;
    return true;
}

//次のトークンが期待する記号の場合 トークンを１つ進めて
//それ以外だったらエラー
void expect(char op) {
    if (token->kind != TK_RESERVED || token->str[0] != op)
        //error("'%c'ではありません", op);
        //メッセージ付きのエラーに変更
        error_at(token->str, "expected '%c", op);
    token = token->next;
}

//次のトークンが数値の場合 トークンを１つ読み進めて その数値を返す
//数値がきていなかったらエラー
int expect_number() {
    if (token->kind != TK_NUM)
        //error("数ではありません");
        //メッセージ付きのエラーに変更
        error_at(token->str, "数ではありません");
    int val = token->val;
    token = token->next;
    return val;
}

bool at_eof() {
    return token->kind == TK_EOF;
}

//新しいトークンを作成してcurにつなげる
Token *new_token(TokenKind kind, Token *cur, char *str) {
    //Token型1個分のメモリを確保
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    cur->next = tok;
    return tok;
}

//入力文字列pをトークナイズしてそれを返す
//user_inputの*pに変更
Token *tokenize() {
    char *p = user_input;
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while (*p) {
        //空白文字をスキップする
        if (isspace(*p)) {
            p++;
            continue;
        }
        if (strchr("+-*/()", *p)) {
            cur = new_token(TK_RESERVED, cur, p++);
            //cur->val = strtol(p, &p, 10);
            continue;
        }

        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p);
            cur->val = strtol(p, &p, 10);
            continue;
        }

        error_at(p, "トークナイズできません");
    }

    new_token(TK_EOF, cur, p);
    return head.next;
}

Node *new_node(NodeKind kind) {
    Node *node = calloc(1, sizeof(Node));
    //メモリ確保
    node->kind = kind;
    return node;
}

Node *new_binary(NodeKind kind, Node *lhs, Node *rhs){
    Node *node = new_node(kind);
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

//数字が入ってる末端のノード
Node *new_num(int val) {
    Node *node = new_node(ND_NUM);
    node-> val = val;
    return node;
}

//四則演算の文法
//プロトタイプ宣言する
Node *expr();
Node *mul();
Node *unary();
Node *primary();


// primary = num | "(" expr ")"
Node *primary() {
    //トークンが（）
    if (consume('(')) {
        Node *node = expr();
        expect(')');
        return node;    
    }
    
    //トークンが数字
    return new_num(expect_number());

}

// unary   = ("+" | "-")? unary  |  primary
Node *unary() {
    // +x は x とする
    if (consume('+'))
        return unary();
    // -x は 0-x とする
    if (consume('-'))
        return new_binary(ND_SUB, new_num(0), unary());

    //演算子の処理が終わったら
    return primary();
}


// mul     = unary ("*" unary | "/" unary)*
Node *mul() {
    Node *node = unary();

    for (;;) {
        if (consume('*'))
            node = new_binary(ND_MUL, node, unary());
        else if (consume('/'))
            node = new_binary(ND_DIV, node, unary());
        else 
            return node;
    }
}


// expr    = mul ("+" mul | "-" mul)*
Node *expr() {
    Node *node = mul();

    for (;;) {
        if (consume('+'))
            node = new_binary(ND_ADD, node, mul());
        else if (consume('-'))
            node = new_binary(ND_SUB, node, mul());
        else 
            return node;
    }
}





void gen(Node *node) {
    if (node->kind == ND_NUM) {
        printf("  push %d\n", node->val);
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");

    switch (node->kind) {
    case ND_ADD:
        printf("  add rax, rdi\n");
        break;
    case ND_SUB:
        printf("  sub rax, rdi\n");
        break;
    case ND_MUL:
        printf("  imul rax, rdi\n");
        break;
    case ND_DIV:
        printf("  cqo\n");
        printf("  idiv rdi\n");
        break;
    }

    printf("  push rax\n");
}



//command line arguments ,count/vecter
int main(int argc, char **argv) {
    //filename と arguments で二個になってるか
    if (argc != 2) {
        // fprintf(stderr, "引数の個数が正しくありません\n");
        error("引数の個数が正しくありません");
        return 1;
    }

    //トークナイズする
    //改良版
    user_input = argv[1];
    //token = tokenize(); どっちだ？
    token = tokenize(user_input);
    Node *node = expr();

    //アセンブリの前半
    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");

    //抽象構文木を下りながらコードを生成
    gen(node);
    
    // スタックのトップが式全体の値になる
    // それをRAXにロードして関数からの返り値とする
    printf("  pop rax\n");
    printf("  ret\n");
    return 0;
}
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

typedef struct Token Token;

//トークン型
struct Token {
	TokenKind kind; //トークンの型
	Token *next; //次の入力トークン
	int val; //TokenKindがTK_NUMのときに数値で受け取っておく用
	char *str; //トークン文字列
};

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
        error("'%c'ではありません", op);
    token = token->next;
}

//次のトークンが数値の場合 トークンを１つ読み進めて その数値を返す
//数値がきていなかったらエラー
int expect_number() {
    if (token->kind != TK_NUM)
        error("数ではありません");
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
Token *tokenize(char *p) {
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while (*p) {
        //空白文字をスキップする
        if (isspace(*p)) {
            p++;
            continue;
        }
        if (*p == '+' || *p == '-') {
            cur = new_token(TK_RESERVED, cur, p++);
            //cur->val = strtol(p, &p, 10);
            continue;
        }

        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p);
            cur->val = strtol(p, &p, 10);
            continue;
        }

        error("トークナイズできません");
    }

    new_token(TK_EOF, cur, p);
    return head.next;
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
    token = tokenize(argv[1]);

    //アセンブリの前半
    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");
    
    //式の最初が数字だと確認して mov命令を出力
    printf("  mov rax, %d\n", expect_number());
    
    while (!at_eof()) {
        if (consume('+')) {
            printf("  add rax, %d\n", expect_number());
            continue;
        }
        // if (consume('-')) {
        //     printf("  add sub, %d\n", expect_number());
        //     continue;
        // }
        expect('-');
        printf("  sub rax, %d\n", expect_number());
        

    }

    printf("  ret\n");
    return 0;
}
#include "kcc.h"

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
bool consume(char *op) {
    //
    if (token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(token->str, op, token->len))
        return false;
    
    token = token->next;
    return true;
}

//次のトークンが期待する記号の場合 トークンを１つ分進めて
//それ以外だったらエラー
void expect(char *op) {
    if (token->kind != TK_RESERVED || strlen(op) != token->len || 
        memcmp(token->str, op, token->len))
        
        error_at(token->str, "\"%s\" ではありません", op);
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
Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
    //Token型1個分のメモリを確保
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
    cur->next = tok;
    return tok;
}

bool startswith(char *p, char *q){
    return memcmp(p, q, strlen(q)) == 0;
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
        //２つの演算子
        if (startswith(p, "==") || startswith(p, "!=") ||
            startswith(p, "<=") || startswith(p, ">=")) {
            cur = new_token(TK_RESERVED, cur, p, 2);
            //演算子分ずらす
            p += 2;
            continue;
        } 
        //１つの演算子
        if (strchr("+-*/()<>", *p)) {
            cur = new_token(TK_RESERVED, cur, p++, 1);
            continue;
        }

        if (isdigit(*p)) {
            //数字のながさをとる
            cur = new_token(TK_NUM, cur, p, 0);
            char *q = p;
            cur->val = strtol(p, &p, 10);
            cur->len = p - q;
            continue;
        }

        error_at(p, "トークナイズできません");
    }

    new_token(TK_EOF, cur, p, 0);
    return head.next;
}







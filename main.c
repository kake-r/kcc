#include "kcc.h" 


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
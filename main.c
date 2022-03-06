#include "kcc.h" 


int main(int argc, char **argv) {
    //filename と arguments で二個になってるか
    if (argc != 2) {
        // fprintf(stderr, "引数の個数が正しくありません\n");
        error("引数の個数が正しくありません");
        return 1;
    }

    //localが最初NULLじゃないから動いてなかった？
    //初期化ってされないのか？
    locals = NULL;

    //トークナイズする
    //結果はcodeに保存
    user_input = argv[1];
    token = tokenize();
    program();


    //アセンブリの前半
    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");

    // プロローグ？
    // 変数26個分の領域を確保する
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    // 8*26で208
    // 変数分確保して置いて 変数が来たらその場所に置く
    printf("  sub rsp, 208\n");

    //1行1ノードを評価していく
    // 先頭の式から順にコード生成
    for (int i = 0; code[i]; i++) {
        gen(code[i]);

    // 式の評価結果としてスタックに一つの値が残っている
    // はずなので、スタックが溢れないようにポップしておく
        printf("  pop rax\n");
    }

    //エピローグ？(プロローグの逆)
    // 最後の式の結果がRAXに残っているのでそれが返り値になる
    //変数領域を使わないことにする？
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    return 0;

    
    // スタックのトップが式全体の値になる
    // それをRAXにロードして関数からの返り値とする
    printf("  pop rax\n");
    printf("  ret\n");
    return 0;
}
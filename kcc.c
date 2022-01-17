#include <stdio.h>
#include <stdlib.h>


//command line arguments ,count/vecter
int main(int argc, char **argv) {
    //filename と arguments で二個になってるか
    if (argc != 2) {
        fprintf(stderr, "引数の個数が正しくありません\n");
        return 1;
    }
    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");
    //文字列から整数型にする
    printf("  mov rax, %d\n", atoi(argv[1]));
    printf("  ret\n");
    return 0;
}
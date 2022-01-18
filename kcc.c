#include <stdio.h>
#include <stdlib.h>


//command line arguments ,count/vecter
int main(int argc, char **argv) {
    //filename と arguments で二個になってるか
    if (argc != 2) {
        fprintf(stderr, "引数の個数が正しくありません\n");
        return 1;
    }

    char *p = argv[1];

    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");
    //文字列から整数型にする
    printf("  mov rax, %ld\n", strtol(p, &p, 10));
    
    while (*p) {
        if (*p == '+') {
            //演算子の次の数字にpをすすめる
            p++;
            //変換後は&pが読み込んだ文字列の最終ポインタに更新されるので
            //項を１つ読み込んで次にすすんだ状態にできる
            printf("  add rax, %ld\n", strtol(p, &p, 10));
            continue;
        }

        if (*p == '-') {
            p++;
            printf("  sub rax, %ld\n", strtol(p ,&p, 10));
			continue;
        }

        fprintf(stderr, "予期しない文字列です: '%c'\n", *p);
		return 1;
    }

    printf("  ret\n");
    return 0;
}
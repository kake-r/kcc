#include "kcc.h"

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
/*
program    = stmt*
stmt       = expr ";"
expr       = assign
assign     = equality ("=" assign)?    //new
equality   = relational ("==" relational | "!=" relational)*
relational = add ("<" add | "<=" add | ">" add | ">=" add)*
add        = mul ("+" mul | "-" mul)*
mul        = unary ("*" unary | "/" unary)*
unary      = ("+" | "-")? primary
primary    = num | ident | "(" expr ")"
*/

//最大行数
Node *code[100];

Node *assign() {
  Node *node = equality();
  if (consume("="))
    node = new_binary(ND_ASSIGN, node, assign());
  return node;
}


Node *expr() {
    return assign();
}

Node *stmt() {
  Node *node = expr();
  expect(";");
  return node;
}

void program() {
  int i = 0;
  while (!at_eof())
    code[i++] = stmt();
  code[i] = NULL;
}



Node *equality() {
  Node *node = relational();

  for (;;) {
    if (consume("=="))
      node = new_binary(ND_EQ, node, relational());
    else if (consume("!="))
      node = new_binary(ND_NE, node, relational());
    else
      return node;
  }
}


Node *relational() {
  Node *node = add();

  for (;;) {
    if (consume("<"))
      node = new_binary(ND_LT, node, add());
    else if (consume("<="))
      node = new_binary(ND_LE, node, add());
    else if (consume(">"))
      node = new_binary(ND_LT, add(), node);
    else if (consume(">="))
      node = new_binary(ND_LE, add(), node);
    else
      return node;
  }
}

Node *add() {
  Node *node = mul();

  for (;;) {
    if (consume("+"))
      node = new_binary(ND_ADD, node, mul());
    else if (consume("-"))
      node = new_binary(ND_SUB, node, mul());
    else
      return node;
  }
}



Node *mul() {
    Node *node = unary();

    for (;;) {
        if (consume("*"))
            node = new_binary(ND_MUL, node, unary());
        else if (consume("/"))
            node = new_binary(ND_DIV, node, unary());
        else 
            return node;
    }
}


Node *unary() {
    // +x は x とする
    if (consume("+"))
        return unary();
    // -x は 0-x とする
    if (consume("-"))
        return new_binary(ND_SUB, new_num(0), unary());

    //演算子の処理が終わったら
    return primary();
}


Node *primary() {
    //トークンが（）
    if (consume("(")) {
        Node *node = expr();
        expect(
            ")");
        return node;    
    }
    //トークンが変数
    Token *tok = consume_ident();
    if (tok) {
      Node *node = calloc(1, sizeof(Node));
      node->kind = ND_LVAR;
      node->offset = (tok->str[0] - 'a' + 1) * 8;
      return node;
    }

    //トークンが数字
    return new_num(expect_number());

}

//変数のときの処理
void gen_lval(Node *node) {
  if (node->kind != ND_LVAR)
    error("代入の左辺値が変数ではありません");

  //raxに変数のアドレス(rbp)を入れる 
  printf("  mov rax, rbp\n");
  //aからzの目的のとこのアドレスになる
  printf("  sub rax, %d\n", node->offset);
  //それをpushして次の処理で使う
  printf("  push rax\n");
}




void gen(Node *node) {
  switch (node->kind) {
  case ND_NUM:
    printf("  push %d\n", node->val);
    return;
  //raxに入ってる変数のアドレスに置き換えてpush
  //pushされるのは変数値！！
  case ND_LVAR:
    gen_lval(node);
    printf("  pop rax\n");
    //中身を入れる
    printf("  mov rax, [rax]\n");
    printf("  push rax\n");
    return;
  
  //raxに変数のアドレス rdiに右辺の計算結果(genの結果)
  case ND_ASSIGN:
    gen_lval(node->lhs);
    gen(node->rhs);

    //rdiに右辺の計算結果が入る
    printf("  pop rdi\n");
    //raxに変数のアドレスが入る
    printf("  pop rax\n");
    //変数のアドレスのさす先に rdiを入れる
    printf("  mov [rax], rdi\n");
    //計算結果自体を入れる (連続代入に対応？)
    printf("  push rdi\n");
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
    case ND_EQ:
        printf("  cmp rax, rdi\n");
        printf("  sete al\n");
        printf("  movzb rax, al\n");
        break;
    case ND_NE:
        printf("  cmp rax, rdi\n");
        printf("  setne al\n");
        printf("  movzb rax, al\n");
        break;
    case ND_LT:
        printf("  cmp rax, rdi\n");
        printf("  setl al\n");
        printf("  movzb rax, al\n");
        break;
    case ND_LE:
        printf("  cmp rax, rdi\n");
        printf("  setle al\n");
        printf("  movzb rax, al\n");
        break;
    }

    printf("  push rax\n");
}
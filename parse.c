#include "9cc.h"

// 現在の「ローカル変数連結リストの先頭」を保持．
Var *locals;

// 与えられたトークンが，「ローカル変数連結リスト」に含まれているか，全探索で確認．
Var *find_var(Token *tok) {
  for (Var *var = locals; var; var = var->next)
    if (strlen(var->name) == tok->len && !memcmp(tok->str, var->name, tok->len))
      return var;
  return NULL;
}

// 与えられた「ノード型」を持つノードを作成．
Node *new_node(NodeKind kind) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  return node;
}

// 与えられた「ノード型・左辺ノード・右辺ノード」を持つノードを作成．
Node *new_binary(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = new_node(kind);
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

// 与えられた「ノード型・左辺ノード」を持つノードを作成．
Node *new_unary(NodeKind kind, Node *expr) {
  Node *node = new_node(kind);
  node->lhs = expr;
  return node;
}

// 与えられた「数値」を持つノードを作成
Node *node_num(int val) {
  Node *node = new_node(ND_NUM);
  node->val = val;
  return node;
}

// 与えられた「ローカル変数構造」を持つノードを作成．
Node *new_var(Var *var) {
  Node *node = new_node(ND_VAR);
  node->var = var;
  return node;
}

// 与えられた「変数名」を持つ「ローカル変数構造」を作成し，「ローカル変数連結リストの先頭」に繋げる．
Var *push_var(char *name) {
  Var *var = calloc(1, sizeof(Var));
  var->next = locals;
  var->name = name;
  locals = var;
  return var;
}

// 前方宣言
Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

// primary = "(" expr ")" | ident | num
Node *primary() {
  if (consume("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }

  Token *tok = consume_ident();
  if (tok) {
    Var *var = find_var(tok);
    if (!var)
      var = push_var(duplicate(tok->str, tok->len));
    return new_var(var);
  }

  return node_num(expect_number());
}

// unary = ("+" unary | "-" unary)? | primary
Node *unary() {
  if (consume("+"))
    return unary();
  if (consume("-"))
    return new_binary(ND_SUB, node_num(0), unary());
  return primary();
}

// mul = unary ("*" unary | "/" unary)*
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

// add = mul ("+" mul | "-" mul)*
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

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
Node *relational() {
  Node *node = add();

  for (;;) {
    if (consume("<"))
      node = new_binary(ND_LT, node, add());
    if (consume("<="))
      node = new_binary(ND_LE, node, add());
    else if (consume(">")) // コードジェネレータでサポートしなくていい．パーサで両辺を入れ替えて対応．
      node = new_binary(ND_LT, add(), node);
    else if (consume(">="))
      node = new_binary(ND_LE, add(), node);
    else
      return node;
  }
}

// equality = relational ("==" relational | "!=" relational)*
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

// assign = equality ("=" assign)?
Node *assign() {
  Node *node = equality();
  if (consume("="))
    node = new_binary(ND_ASSIGN, node, assign());
  return node;
}

// expr = assign
Node *expr() {
  return assign();
}

Node *read_expr_stmt() {
  return new_unary(ND_EXPR_STMT, expr());
}

// stmt = "return" expr ";" | "if" "(" expr ")" stmt ("else" stmt)? | expr ";"
Node *stmt() {
  if (consume("return")) {
    Node *node = new_unary(ND_RETURN, expr());
    expect(";");
    return node;
  }

  if (consume("if")) {
    Node *node = new_node(ND_IF);
    expect("(");
    node->cond = expr();
    expect(")");
    node->then = stmt();
    if (consume("else"))
      node->els = stmt();
    return node;
  }

  // 今までは，Node *node = expr();としていたが，ここで式としてのノードを用意してる？
  Node *node = read_expr_stmt();
  expect(";");
  return node;
}

// program = stmt*
Program *program() {

  // 「ローカル変数連結リスト」の先頭を作成．
  locals = NULL;

  // 「ノード連結リスト」の先頭を作成
  Node head;
  head.next = NULL;
  Node *cur = &head;

  // 現在のトークンを「ノード」に置き換え，「ノード連結リスト」を作成（後ろに連結）．
  // トークンの終端まで繰り返す．
  while (!at_eof()) {
    cur->next = stmt();
    cur = cur->next;
  }

  // プログラムの作成（「ノード連結リスト」の先頭，「ローカル変数連結リスト」の先頭を持つ．）
  Program *prog = calloc(1, sizeof(Program));
  prog->node = head.next;
  prog->locals = locals;
  return prog;
}


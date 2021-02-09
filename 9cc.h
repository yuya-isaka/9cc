#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>


//
// tokenize.c
//
typedef enum {
  TK_RESERVED, // 記号
  TK_NUM,      // 整数トークン
  TK_EOF,      // 入力の終わりを表すトークン
} TokenKind;

// トークン型
typedef struct Token Token;
struct Token {
  TokenKind kind; // トークンの型
  Token *next; // 次の入力トークン
  int val; // kindがTK_NUMの場合，その数値
  char *str; // トークン文字列
  int len; // トークンの長さ
};

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
bool consume(char *op);
void expect(char *op);
int expect_number();
bool at_eof();
Token *new_token(TokenKind kind, Token *cur, char *str, int len);
Token *tokenize();

extern char *user_input;
extern Token *token;


//
// parse.c
//

// 抽象構文木のノードの種類
typedef enum {
  ND_ADD,
  ND_SUB,
  ND_MUL,
  ND_DIV,
  ND_EQ, // ==
  ND_NE, // !=
  ND_LT, // <
  ND_LE, // <=
  ND_NUM,
} NodeKind;

typedef struct Node Node;

struct Node {
  NodeKind kind; // ノードの型
  Node *lhs; // 左辺
  Node *rhs; // 右辺
  int val; // kindがND_NUMの場合のみ使う
};

Node *new_node(NodeKind kind);
Node *new_binary(NodeKind kind, Node *lhs, Node *rhs);
Node *node_num(int val);

// したで再帰的に呼び出すコードは書かなくてもいい
Node *expr();
// 呼び出した関数の先で何が行われても，呼び出し基にとってはそのうち単にリターンしてくるだけだから．（コンパイルするときには必要ない）

//
// codegen.c
//

void codegen(Node *node);

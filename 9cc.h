#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

//
// tokenize.c
//

// Token
typedef enum {
  TK_RESERVED,  // 記号トークン
  TK_IDENT,     // 識別子(ローカル変数)トークン
  TK_NUM,       // 整数トークン
  TK_EOF,       // 入力の終わりを表すトークン
} TokenKind;

// トークン構造体
typedef struct Token Token; // 再帰的に使うために宣言
struct Token {
  TokenKind kind;  // トークンの型
  Token *next;     // 次の入力トークン
  int val;         // kindがTK_NUMの場合，その数値
  char *str;       // トークン文字列
  int len;         // トークンの長さ
};

// 複数のCファイルで使用する，関数たち，だからここで宣言する
void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
bool consume(char *op);
char *duplicate(char *p, int len);
Token *consume_ident();
void expect(char *op);
int expect_number();
bool at_eof();
Token *new_token(TokenKind kind, Token *cur, char *str, int len);
Token *tokenize();

extern char *user_input;
extern Token *token;


// なんなら，consume, expect, expect_number あたりはtokenize.cにあるけど，その中では使っていない．


//
// parse.c
//

// ローカル変数構造体
// 連想リストみたいに，変数を保持している
// 全探索で，ここに引っ掛かるかfind_varで調べる
typedef struct Var Var;
struct Var {
  Var *next;
  char *name; // 変数の名前
  int offset; // Offset 
};

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
  ND_RETURN, // return 
  ND_EXPR_STMT, // expression statement
  ND_ASSIGN, // =
  ND_VAR, // variable
  ND_NUM,
} NodeKind;

// 抽象構文木のノードの構造体
typedef struct Node Node;
struct Node {
  NodeKind kind; // ノードの型
  Node *next; // 次のノード
  Node *lhs; // 左辺
  Node *rhs; // 右辺
  int val; // kindがND_NUMの場合のみ使う
  Var *var; // kindがND_VARのときに使う
};

// program構造体
typedef struct {
  Node *node;
  Var *locals;
  int stack_size;
} Program;

Program *program();
// 呼び出した関数の先で何が行われても，呼び出し基にとってはそのうち単にリターンしてくるだけだから．（コンパイルするときには必要ない）

//
// codegen.c
//

void codegen(Program *node);

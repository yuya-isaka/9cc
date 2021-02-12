#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

//
// tokenize.c ==========================================================================================
//

// トークンの型
typedef enum {
  TK_RESERVED,  // 記号
  TK_IDENT,     // 識別子（ローカル変数）
  TK_NUM,       // 数
  TK_EOF,       // 終端（入力の終わり）
} TokenKind;

// トークンの構造
typedef struct Token Token; // 再帰的に使うために宣言
struct Token {
  TokenKind kind;  // トークンの型
  Token *next;     // 次のトークン
  int val;         // kindがTK_NUMの場合，数値を表す
  char *str;       // トークン文字列
  int len;         // トークンの長さ
};

// 複数のCファイルで使用する関数を宣言．（tokenize.cで定義）
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

// 複数のCファイルで使用する変数を宣言．（tokenize.cで定義）
extern char *user_input;
extern Token *token;


//
// parse.c ========================================================================================
//

// ローカル変数の構造
typedef struct Var Var;
struct Var {
  Var *next;   // 次のローカル変数
  char *name;  // 変数の名前
  int offset;  // 変数のoffset 
};

// ノードの型
typedef enum {
  ND_ADD,        // +
  ND_SUB,        // -
  ND_MUL,      	 // *
  ND_DIV,      	 // /
  ND_EQ,       	 // ==
  ND_NE,       	 // !=
  ND_LT,       	 // <
  ND_LE,       	 // <=
  ND_RETURN,   	 // return
  ND_IF,         // if
  ND_EXPR_STMT,  // expression statement
  ND_ASSIGN,     // 代入
  ND_VAR,        // 変数
  ND_NUM,        // 数
} NodeKind;

// ノードの構造
typedef struct Node Node;
struct Node {
  NodeKind kind;   // ノードの型
  Node *next;      // 次のノード
  
  Node *lhs;       // 左辺のノード
  Node *rhs;       // 右辺のノード
  // if構文
  Node *cond;
  Node *then;
  Node *els;
  
  int val;         // kindがND_NUMの場合，数値を表す
  Var *var;        // kindがND_VARの場合，変数を表す
};

// プログラムの構造（「ノード連結リスト（抽象構文木）」の先頭，「ローカル変数連結リスト」の先頭，スタックサイズを持つ）
typedef struct {
  Node *node;
  Var *locals;
  int stack_size;
} Program;

// 複数のCファイルで使用する関数を宣言（関数はextern要らない．つけてもいい．）
Program *program();


//
// codegen.c =========================================================================================
//

void codegen(Program *node);

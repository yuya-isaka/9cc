#include "9cc.h"

// 定義
char *user_input;
Token *token;

// エラー報告
void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// 場所ありエラー報告
void error_at(char *loc, char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, "");
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// 与えられた文字列（ここでは現在のトークン）を用いて，与えられた文字数分の「文字列」を返す．
char *duplicate(char *p, int len) {
  char *buf = malloc(len + 1);
  strncpy(buf, p, len);
  buf[len] = '\0';
  return buf;
}

// 現在のトークンが「指定した記号」か確認．（トークン進めて，trueを返す）
bool consume(char *op) {
  if (token->kind != TK_RESERVED ||
      strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
    return false;
  token = token->next;
  return true;
}

// 現在のトークンが「ローカル変数」か確認．（トークン進めて，現在のトークン返す）
Token *consume_ident() {
  if (token->kind != TK_IDENT)
    return NULL;
  Token *t = token;
  token = token->next;
  return t;
}

// 現在のトークンが「指定した記号」か確認．
void expect(char *op) {
  if (token->kind != TK_RESERVED ||
      strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
    error_at(token->str, "expected \"%s\"", op);
  token = token->next;
}

// 現在のトークンが「数」か確認．（トークン進めて，数返す）
int expect_number() {
  if (token->kind != TK_NUM)
    error_at(token->str, "expected a number");
  int val = token->val;
  token = token->next;
  return val;
}

// 現在のトークンが「終端」か確認．
bool at_eof() {
  return token->kind == TK_EOF;
}

// ===================================================================================================================================

// 新しいトークンを作成し，現在のトークンの「後方」に繋げる．（連結リスト）
Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  tok->len = len;
  cur->next = tok;
  return tok;
}

// 与えられた文字列と，与えられた文字列が，等しいか確認．
bool startswith(char *p, char *q) {
  return memcmp(p, q, strlen(q)) == 0; // memcmpは等しいと0を返す．
}

// 与えられた文字が「アルファベット」か確認
bool is_alpha(char c) {
  return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
}

// 与えられた文字が「アルファベット」・「数字」か確認
bool is_alnum(char c) {
  return is_alpha(c) || ('0' <= c && c <= '9');
}

// 与えられた文字列を，先頭から「トークンの連結リスト」に，変換．その後，先頭を返す．
Token *tokenize() {
  char *p = user_input;
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while (*p) {

    // スペースはスキップ
    if (isspace(*p)) {
      p++;
      continue;
    }

    // 「return」のトークンを作成．
    if (startswith(p, "return") && !is_alnum(p[6])) {
      cur = new_token(TK_RESERVED, cur, p, 6);
      p += 6;
      continue;
    }

    // 「比較演算子」のトークンを作成．
    if (startswith(p, "==") ||
	startswith(p, "!=") ||
	startswith(p, "<=") ||
	startswith(p, ">=")) {
      cur = new_token(TK_RESERVED, cur, p, 2);

      p += 2;
      continue;
    }

    // 「単項演算子」のトークンを作成．
    if (strchr("+-*/()<>;=", *p)) {
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }

    // 「ローカル変数」のトークンを作成．
    if (is_alpha(*p)) {
      char *q = p++;

      while (is_alnum(*p))
	p++;
      cur = new_token(TK_IDENT, cur, q, p - q);
      continue;
    }

    // 「数」のトークンを作成
    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p, 0);
      char *q = p;
      cur->val = strtol(p, &p, 10);
      cur->len = p - q;
      continue;
    }

    error_at(p, "invalid token");
  }

  // 「終端」のトークンを作成．
  new_token(TK_EOF, cur, p, 0);
  return head.next;
}


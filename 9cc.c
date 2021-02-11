#include "9cc.h"


int main(int argc, char **argv) {
  if (argc != 2)
    error("%s: invalid number of arguments", argv[0]);

  // エラー箇所報告用の，先頭を指すポインタを保持する
  // トークナイザー，このグローバルtoken変数を，他の関数で参照するだけにする．

  // 9cc.hで宣言したグローバル変数を初期化する
  // 定義はtokenize.cで実施されている．
  user_input = argv[1];
  token = tokenize();

  // スタックマシン，AST（抽象構文木を作成）
  //  Node *node = program();

  Program *prog = program();

  int offset = 0;
  for (Var *var = prog->locals; var; var = var->next) {
    offset += 8;
    var->offset = offset;
  }
  prog->stack_size = offset;

  // 抽象構文木を下りながらコード生成
  // 木のルートのノードを受け取る
  // コードジェネレーター
  codegen(prog);


  return 0;
}


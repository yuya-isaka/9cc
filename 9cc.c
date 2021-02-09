#include "9cc.h"


int main(int argc, char **argv) {
  if (argc != 2)
    error("%s: invalid number of arguments", argv[0]);

  // エラー箇所報告用の，先頭を指すポインタを保持する
  // トークナイザー，このグローバルtoken変数を，他の関数で参照するだけにする．
  user_input = argv[1];
  token = tokenize();

  // スタックマシン，AST（抽象構文木を作成）
  Node *node = expr();


  // 抽象構文木を下りながらコード生成
  // 木のルートのノードを受け取る
  // コードジェネレーター
  codegen(node);


  return 0;
}


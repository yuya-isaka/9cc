#include "9cc.h"

int main(int argc, char **argv) {

  // エラー確認
  if (argc != 2)
    error("%s: invalid number of arguments", argv[0]);

  // 「入力文字列の先頭」を保持．
  user_input = argv[1];

  // トークナイズ（文字列を「トークン連結リスト」に変換）
  token = tokenize();

  // 構文解析（トークン連結リストを「ノード連結リスト」に変換）
  // 再帰下降構文解析法を使用（LLパーサ）
  // 最終的に「プログラム」を出力（「ノード連結リスト・ローカル変数連結リスト」を持つ）
  Program *prog = program();

  // 「プログラムのローカル変数連結リスト」を用いて，各ローカル変数のオフセットを求めて値を保持．
  // オフセットは，スタック内での「関数フレーム(ベースポインタ)」からの距離
  int offset = 0;
  for (Var *var = prog->locals; var; var = var->next) {
    offset += 8;
    var->offset = offset;
  }

  // 合計のオフセット数を，スタックサイズとして保持．
  prog->stack_size = offset;

  // プログラムから，「アセンブリ」を出力
  codegen(prog);

  return 0;
}


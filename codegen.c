#include "9cc.h"

// ロード・ストアの前処理
// 与えられたノードが「ローカル変数」の場合，指している「アドレス（重要）」をスタックから取り出し，raxレジスタにセットし．スタックにプッシュ．
void gen_addr(Node *node) {
   if (node->kind == ND_VAR) {
     printf("  lea rax, [rbp-%d]\n", node->var->offset); // raxに「指定したアドレス」をセット．
     printf("  push rax\n");
     return;
   }

  error("代入の左辺値が変数ではありません");
}

// ロード処理
// スタックトップ値をraxに割り当てル．（ここで，raxにはアドレスが入っている）
// raxに，「raxで保持されているアドレス」の値をセット．
// その後，raxの値をスタックにプッシュ．
void load() {
  printf("  pop rax\n");
  printf("  mov rax, [rax]\n");
  printf("  push rax\n");
}

// ストア処理
// rdiにスタックトップ値をセット，raxに次のスタックトップ値をセット．
// 「raxの指すアドレス」に，rdiの値をセット．
// その後，rdiの値をスタックにプッシュ
void store() {
  printf("  pop rdi\n");
  printf("  pop rax\n");
  printf("  mov [rax], rdi\n");
  //  printf("  push rdi\n");  // わからん．なくてもいい．//わかった．return a=3;とかに対応してる．// 違うかも
}

// 与えられたノードに応じて，「アセンブリ」を出力しreturn．
void gen(Node *node) {
  switch (node->kind) {
    // 数値なら，スタックに値（数値）をプッシュ
  case ND_NUM:
    printf("  push %d\n", node->val);
    return;
    // コロンなら，左辺ノードを生成し，スタックポインタを一つ分戻す．
  case ND_EXPR_STMT:
    gen(node->lhs);
    //printf("  add rsp, 8\n"); // わからん．なくてもいい．//わかった，return a=3;とかに対応してる//違うかも
    return;
    // ローカル変数なら，ロードする．
  case ND_VAR:
    gen_addr(node);
    load();
    return;
    // 代入式なら，ストアする．
    // まず左辺値として評価した後に，スタックトップにある計算結果をアドレスとみなして，そのアドレスから
  case ND_ASSIGN:
    gen_addr(node->lhs);
    gen(node->rhs);
    store();
    return;
    // 「return」なら，左辺ノードを生成し，スタックトップを返す．
    //  その後，エピローグにジャンプ
  case ND_RETURN:
    gen(node->lhs);    
    printf("  pop rax\n"); // スタックトップに式全体の値があるはずなので，それをraxにセットして関数からの返り値にする
    printf("  jmp .Lreturn\n"); // jmp .Lreturnで，エピローグに飛ぶ
    return;
  }

  // 「数値・コロン・ローカル変数・代入式・return」に当てはまらなかったら，
  // 「左辺ノード」と「右辺ノード」を生成．
  gen(node->lhs);
  gen(node->rhs);

  // rdiにスタックトップ値をセット，raxに次のスタックトップ値をセット．
  printf("  pop rdi\n");
  printf("  pop rax\n");

  // raxに，「レジスタを用いた演算結果」をセット
  switch (node->kind) {
    // 「+」なら，rax += rdi．
  case ND_ADD:
    printf("  add rax, rdi\n");
    break;
    // 「-」なら，rax -= rdi．
  case ND_SUB:
    printf("  sub rax, rdi\n");
    break;
    // 「*」なら，rax *= rdi．
  case ND_MUL:
    printf("  imul rax, rdi\n");
    break;
    //「/」なら，rax /= rdi．
  case ND_DIV:
    printf("  cqo\n"); // 128ビットにraxを拡張．その後，rdx:raxにストア．
    printf("  idiv rdi\n"); // 符号あり除算．rdx:rax(128ビット)を引数(64ビット)で除算し，raxに商を，rdxに余りをセット．
    break;
    // 「==」なら，rax = (rax == rdi) ? 1 : 0
  case ND_EQ:
    printf("  cmp rax, rdi\n");
    printf("  sete al\n");
    printf("  movzb rax, al\n");
    break;
    // 「!=」なら，rax = (rax != rdi) ? 1 : 0
  case ND_NE:
    printf("  cmp rax, rdi\n");
    printf("  setne al\n");
    printf("  movzb rax, al\n");
    break;
    // 「>」なら，rax = (rax > rdi) ? 1 : 0　（右辺ノード(rdi)の方が小さいはず）
    // 「<」なら，rax = (rax < rdi) ? 1 : 0
  case ND_LT:
    printf("  cmp rax, rdi\n");
    printf("  setl al\n");
    printf("  movzb rax, al\n");
    break;
    // 「>=」なら，rax = (rax >= rdi) ? 1 : 0  （右辺ノード(rdi)の方が小さいはず）
    // 「<=」なら，rax = (rax <= rdi) ? 1 : 0
  case ND_LE:
    printf("  cmp rax, rdi\n");
    printf("  setle al\n");
    printf("  movzb rax, al\n");
    break;
  default:
    error("エラー");
  }

  // 「演算結果の入っているrax」をスタックにプッシュ
  printf("  push rax\n");
 }

// 与えられたプログラムから，「アセンブリ」を出力
void codegen(Program *prog) {
  
  // アセンブリ前半部分
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");

  // プロローグ（ローカル変数をスタック領域に確保）
  
  // RBP(ベースレジスタ)の値(ベースポインタ)を，スタックにプッシュ．（ローカル変数にアクセスするときの基準となる）
  // RBPに，RSPの値(スタックポインタ)をセット．（この下にローカル変数を溜める）
  // rsp -= prog->stack_size（スタック領域確保）
  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  printf("  sub rsp, %d\n", prog->stack_size);

  // 与えられたプログラムの「ノード連結リスト」を用いて，順番に「各ノードのアセンブリ」を出力
  for (Node *node = prog->node; node; node = node->next)
    gen(node);
  
  // エピローグ（RBPに元の値を書き戻して，RSPがリターンアドレスを指している状態にして，ret命令を呼び出す）
  printf(".Lreturn:\n");
  printf("  mov rsp, rbp\n");  // RBPをRSPの位置に戻す
  printf("  pop rbp\n"); // RBPをポップ（元のRBPに移動）
  printf("  ret\n");  // スタックからアドレスをポップ（RSP(スタックポインタ)を利用）して，そこにジャンプ
}

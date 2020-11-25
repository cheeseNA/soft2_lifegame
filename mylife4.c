/*
コマンドライン引数としてルールを受け取れるようにした。
形式はB??/S?? デフォルトはB23/S3

https://www.conwaylife.com/wiki/List_of_Life-like_cellular_automata
にあるruleをいくつか試してみた。
通常のruleの反転版であるB0123478/S01234678を試してみたところ、
初期の生きているセルの割合が10％に近いためgen = 1でほぼ#となった。
フィールド外は空セル判定のため、gen = 2以降で対照的なパターンが外側
から内側へ広がっていった。

Stains(B3678/S235678)では、生きているセルが固まった集団で存在し、
パターン名の通り染みが生じていた。
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>  // sleep()関数を使う

// #include "gol.h"

void my_init_cells(const int height, const int width, int cell[height][width],
                   FILE *fp);
void my_print_cells(FILE *fp, int gen, const int height, const int width,
                    int cell[height][width], const int rule[2][9]);
int my_count_adjacent_cells(int h, int w, const int height, const int width,
                            int cell[height][width]);
void my_update_cells(const int height, const int width, int cell[height][width],
                     const int rule[2][9]);

int main(int argc, char **argv) {
  srand(time(NULL));

  FILE *fp = stdout;
  const int height = 40;
  const int width = 70;

  int cell[height][width];
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      cell[y][x] = 0;
    }
  }

  int rule[2][9] = {{0, 0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0}};

  /* ファイルを引数にとるか、ない場合はデフォルトの初期値を使う */
  if (argc > 2) {
    fprintf(stderr, "usage: %s [rule]\n", argv[0]);
    return EXIT_FAILURE;
  } else if (argc == 2) {
    my_init_cells(height, width, cell, NULL);
    size_t len = strlen(argv[1]);
    int target = 0;
    for (int i = 0; i < len; i++) {
      if (argv[1][i] == 'B') {
        target = 0;
      } else if (argv[1][i] == 'S') {
        target = 1;
      } else if ('0' <= argv[1][i] && argv[1][i] <= '9') {
        rule[target][argv[1][i] - '0'] = 1;
      }
    }
  } else {
    my_init_cells(height, width, cell, NULL);  // デフォルトの初期値を使う
    rule[0][3] = 1;
    rule[1][2] = 1;
    rule[1][3] = 1;
  }

  my_print_cells(fp, 0, height, width, cell, rule);  // 表示する
  sleep(1);                                          // 1秒休止

  /* 世代を進める*/
  for (int gen = 1;; gen++) {
    my_update_cells(height, width, cell, rule);          // セルを更新
    my_print_cells(fp, gen, height, width, cell, rule);  // 表示する
    sleep(1);                                            // 1秒休止する
    fprintf(fp, "\e[%dA",
            height + 3);  // height+3 の分、カーソルを上に戻す(壁2、表示部1)
  }

  return EXIT_SUCCESS;
}

void my_init_cells(const int height, const int width, int cell[height][width],
                   FILE *fp) {
  if (fp == NULL) {
    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        if (rand() % 10 == 0) {
          cell[y][x] = 1;
        }
      }
    }
    return;
  }

  const size_t bufsize = 500;
  char buf[bufsize];
  int endfile = 0;
  int x = 0, y = 0;
  int lenbuffer = 0;
  while (fgets(buf, bufsize, fp) != NULL) {
    size_t len = strlen(buf) - 1;
    // printf("%zd\n", len);
    if (buf[0] == '#' || buf[0] == 'x') {
      continue;
    } else {
      for (int i = 0; i < len; i++) {
        if ('0' <= buf[i] && buf[i] <= '9') {
          lenbuffer = lenbuffer * 10 + (buf[i] - '0');
        } else {
          if (buf[i] == '!') {
            endfile = 1;
            break;
          } else if (buf[i] == '$') {
            x = 0;
            y++;
          } else if (buf[i] == 'b' || buf[i] == 'o') {
            if (lenbuffer == 0) {
              lenbuffer = 1;
            }
            if (buf[i] == 'o') {
              for (int j = 0; j < lenbuffer; j++) {
                cell[y][x + j] = 1;
              }
            }
            x += lenbuffer;
            lenbuffer = 0;
          }
        }
      }
    }
    if (endfile) {
      break;
    }
  }
}

void my_print_cells(FILE *fp, int gen, const int height, const int width,
                    int cell[height][width], const int rule[2][9]) {
  int living = 0;
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      if (cell[y][x]) {
        living++;
      }
    }
  }
  fprintf(fp, "generation = %d   living cells = %d/%d (%f%%)   B", gen, living,
          height * width, 100.0 * living / (height * width));
  for (int i = 0; i <= 8; i++) {
    if (rule[0][i] == 1) fprintf(fp, "%d", i);
  }
  fprintf(fp, "/S");
  for (int i = 0; i <= 8; i++) {
    if (rule[1][i] == 1) fprintf(fp, "%d", i);
  }
  fprintf(fp, "\n");

  fprintf(fp, "+");
  for (int x = 0; x < width; x++) fprintf(fp, "-");
  fprintf(fp, "+\n");

  for (int y = 0; y < height; y++) {
    fprintf(fp, "|");
    for (int x = 0; x < width; x++) {
      if (cell[y][x]) {
        fprintf(fp, "\e[31m#\e[0m");
      } else {
        fprintf(fp, " ");
      }
    }
    fprintf(fp, "|\n");
  }

  fprintf(fp, "+");
  for (int x = 0; x < width; x++) fprintf(fp, "-");
  fprintf(fp, "+\n");

  fflush(fp);
}

int my_count_adjacent_cells(int h, int w, const int height, const int width,
                            int cell[height][width]) {
  int dx[8] = {-1, 0, 1, 1, 1, 0, -1, -1};
  int dy[8] = {-1, -1, -1, 0, 1, 1, 1, 0};

  int ret = 0;
  for (int i = 0; i < 8; i++) {
    if (0 <= w + dx[i] && w + dx[i] < width && 0 <= h + dy[i] &&
        h + dy[i] < height) {
      if (cell[h + dy[i]][w + dx[i]]) {
        ret++;
      }
    }
  }
  return ret;
}

void my_update_cells(const int height, const int width, int cell[height][width],
                     const int rule[2][9]) {
  int tmp[height][width];
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      tmp[y][x] = 0;
    }
  }
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      int adjacent = my_count_adjacent_cells(y, x, height, width, cell);
      if (cell[y][x]) {
        if (rule[1][adjacent] == 1) {
          tmp[y][x] = 1;
        }
      } else {
        if (rule[0][adjacent] == 1) {
          tmp[y][x] = 1;
        }
      }
    }
  }
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      cell[y][x] = tmp[y][x];
    }
  }
}
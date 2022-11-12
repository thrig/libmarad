// marad.h - a board game implementation

#ifndef _H_LIBMARAD_H_
#define _H_LIBMARAD_H_

#define MARAD_BOARD_SIZE 9
#define MARAD_BOARD_TOTAL_SIZE 81

#define MARAD_COORD(x, y) (MARAD_BOARD_SIZE * (y) + (x))

struct marad;

void marad_board(struct marad *game, uint8_t *board);
void marad_free(struct marad *game);
struct marad *marad_init(void);
int marad_move(struct marad *game, int moves, int srcx, int srcy, int dstx,
               int dsty);
int marad_owns(struct marad *game, int x, int y);
void marad_score(struct marad *game, int *score1, int *score2);

#endif

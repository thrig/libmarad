// libmarad - support a board game implemenation

#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "marad.h"

#define CENTER_POINT 40 // 9*4+4 or 4,4 is used for scoring

#define MOVE_NOPE 0
#define MOVE_SQUARE 1
#define MOVE_DIAGONAL 2

#define MOVED_MASK 128
#define PLAYER_BIT 6
#define TYPE_MASK 15

#define NOTHING 0

struct marad {
	unsigned player : 1; // whose turn is it, anyways?
	unsigned int score[2];
	uint8_t board[MARAD_BOARD_TOTAL_SIZE];
};

static uint8_t default_board[MARAD_BOARD_TOTAL_SIZE] = {
  0, 0, 0, 0, 0, 0, 0, 0,  0, // initial state
  0, 2, 0, 0, 0, 0, 0, 66, 0, //
  0, 1, 0, 0, 0, 0, 0, 65, 0, //
  0, 1, 0, 0, 0, 0, 0, 65, 0, //
  0, 3, 0, 0, 0, 0, 0, 67, 0, //
  0, 1, 0, 0, 0, 0, 0, 65, 0, //
  0, 1, 0, 0, 0, 0, 0, 65, 0, //
  0, 2, 0, 0, 0, 0, 0, 66, 0, //
  0, 0, 0, 0, 0, 0, 0, 0,  0};

static void domoves(struct marad *game, int moves, int srcx, int srcy,
                    int stepx, int stepy);
static uint8_t piece(struct marad *game, int x, int y);
static int movetype(int srcx, int srcy, int dstx, int dsty, int *stepx,
                    int *stepy);

void
marad_board(struct marad *game, uint8_t *board)
{
	memcpy(board, game->board, sizeof(default_board));
}

void
marad_free(struct marad *game)
{
	free(game);
}

struct marad *
marad_init(void)
{
	struct marad *game;
	game = malloc(sizeof(struct marad));
	if (!game) return NULL;
	game->player = game->score[0] = game->score[1] = 0;
	memcpy(game->board, default_board, sizeof(default_board));
	return game;
}

int
marad_move(struct marad *game, int moves, int srcx, int srcy, int dstx,
           int dsty)
{
	int i, move_type, stepx, stepy;
	uint8_t center, this;

	assert(moves >= 1 && moves <= 4);

	this = piece(game, srcx, srcy);
	if (this == NOTHING) return 0;
	if (((this >> PLAYER_BIT) & 1) != game->player) return 0;

	move_type = movetype(srcx, srcy, dstx, dsty, &stepx, &stepy);
	if (move_type == MOVE_NOPE) return 0;

	if ((move_type & (this & TYPE_MASK)) == 0) return 0;

	domoves(game, moves, srcx, srcy, stepx, stepy);

	center = game->board[CENTER_POINT];
	if (center && ((center & MOVED_MASK) == MOVED_MASK))
		game->score[(center >> PLAYER_BIT) & 1]++;
	for (i = 0; i < MARAD_BOARD_TOTAL_SIZE; i++)
		game->board[i] &= ~MOVED_MASK;

	game->player ^= 1;

	return 1;
}

int
marad_owns(struct marad *game, int x, int y)
{
	uint8_t this = piece(game, x, y);
	if (this == NOTHING) return 0;
	return ((this >> PLAYER_BIT) & 1) == game->player ? 1 : 0;
}

void
marad_score(struct marad *game, int *score1, int *score2)
{
	*score1 = game->score[0];
	*score2 = game->score[1];
}

inline static void
domoves(struct marad *game, int moves, int srcx, int srcy, int stepx, int stepy)
{
	int bi, newx, newy, targetx, targety, target_bi, where;
	uint8_t buf[MARAD_BOARD_SIZE];
	bi        = 0;
	newx      = srcx;
	newy      = srcy;
	target_bi = -1;

	buf[bi] = game->board[MARAD_COORD(srcx, srcy)];

	// find where the pieces are to be moved to and buffer any
	// to be moved
	while (moves > 0) {
		newx += stepx;
		newy += stepy;
		if (newx < 0 || newx >= MARAD_BOARD_SIZE || newy < 0 ||
		    newy >= MARAD_BOARD_SIZE)
			break;
		where = MARAD_COORD(newx, newy);
		if (game->board[where] == NOTHING) {
			// empty cell: one less move to make, and maybe
			// this is our target
			targetx   = newx;
			targety   = newy;
			target_bi = bi;
			moves--;
		} else {
			// occupied cell: record a piece to (maybe) be moved
			buf[++bi] = game->board[where];
		}
	}

	// if there are things to move, copy them, then fill any
	// remainder with empty cells
	if (target_bi >= 0) {
		while (target_bi >= 0) {
			where              = MARAD_COORD(targetx, targety);
			game->board[where] = buf[target_bi--];
			game->board[where] |= MOVED_MASK;
			targetx -= stepx;
			targety -= stepy;
		}
		while (targetx != srcx && targety != srcy) {
			where              = MARAD_COORD(targetx, targety);
			game->board[where] = NOTHING;
			targetx -= stepx;
			targety -= stepy;
		}
		where              = MARAD_COORD(srcx, srcy);
		game->board[where] = NOTHING;
	}
}

inline static int
movetype(int srcx, int srcy, int dstx, int dsty, int *stepx, int *stepy)
{
	int dx, dy, plus_x, plus_y;

	if (srcx == dstx && srcy == dsty) return MOVE_NOPE;

	dy     = dsty - srcy;
	plus_x = dstx > srcx;
	if (!dy) {
		*stepx = plus_x ? 1 : -1;
		*stepy = 0;
		return MOVE_SQUARE;
	}

	dx     = dstx - srcx;
	plus_y = dsty > srcy;
	if (!dx) {
		*stepx = 0;
		*stepy = plus_y ? 1 : -1;
	}

	if (abs(dx) == abs(dy)) {
		*stepx = plus_x ? 1 : -1;
		*stepy = plus_y ? 1 : -1;
		return MOVE_DIAGONAL;
	}

	return MOVE_NOPE;
}

inline static uint8_t
piece(struct marad *game, int x, int y)
{
	assert(x >= 0 && x < MARAD_BOARD_SIZE && y >= 0 &&
	       y < MARAD_BOARD_SIZE);
	return game->board[MARAD_COORD(x, y)];
}

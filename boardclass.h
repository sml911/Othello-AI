#include <iostream>
#include <string.h>
#include <array>
#include <stdlib.h>
#include <unistd.h>
#include <fstream>
#include <chrono>
#include <ctime>
#define DEFAULT_TIME 5000
#define INF 299792458
#define WIN_THRESH 100000
#define MAX_DEPTH 32
#define MAX_MOVES 32
#define H_DIM 6
#define H_RES 4

enum Color {EMPTY, POSSIBLE, LEGAL, OTHER, BLACK, WHITE};
enum PlayerType {HUMAN, COMPUTER1, MINIMAX_P, ALPHABETA_P, ALPHABETA_W, ALPHABETAX_W, MINIMAX_W, RANDOMMOVE};
enum Heuristic {MAXDISK, POSITIONAL, MINOPMOVES, WEIGHTED};
enum Search {MINIMAX, ALPHABETA, ALPHABETAX};
enum Stability {UNSTABLE, SEMISTABLE, STABLE};

struct Move
{
	int col;
	int row;
	Move(){};
	Move(int colinit, int rowinit)
	{
		col=colinit; 
		row=rowinit;
	};
	~Move(){};
	bool operator==(const Move& rhs)
	{
    	return (col==rhs.col) and (row==rhs.row);
	}
	bool operator!=(const Move& rhs)
	{
    	return (col!=rhs.col) or (row!=rhs.row);
	}
};

class Square
{
	friend class Board;
public:
	Square(){};
	Square(Color color){player=color;};
	Square(Color, int, int, const std::array<int,8>&);
	~Square(){};
	void Mov2Sq(Move);
	Move Sq2Mov();
	bool operator==(const Square& rhs)
	{
    	return (col==rhs.col) and (row==rhs.row);
	}
private:
	Color player;
	int col;
	int row;
	std::array<int,8> flipped;
	int flipcount;
};

class Board
{
public:
	Board();
	Board(Board*);
	~Board();
	void Show();
	void Reset();
	int Utility();
	void UtilitySort(int*, int[][MAX_DEPTH]);
	void MakeMove(Square);
	int GenLegalMoves();
	void ClearOther();
	bool TerminalTest();
	int MiniMax(int, bool);
	int AlphaBeta(int, int, int, bool, bool);
	int AlphaBetax(int , int , int, bool, bool);
	int MaxDisk(Color);
	int Positional(Color);
	int Weighted(int*);
	std::string MovePrompt();
	bool CheckLegal(Move, Square&);
	long TreeSearch(Square&, long, Search);
	long RandMove(Square&, long);
	Color Mov2State(Move);
	Square MoveGen(Square&, PlayerType, PlayerType, long);
	inline void SetParentPath(int* p){memcpy(parent->t_path,p, sizeof(int)*MAX_DEPTH);};
	inline void SetHeuristic(Heuristic h){heuristic=h;};
	inline int GetTurn() {return bscore+wscore-3;};
	void Option(char);
	long LoadBoard(char* filename);
	void Pass();
	int Score();
	void GameEnd();
private:
	Color ThePlay;
	Heuristic heuristic;
	int wscore;
	int bscore;
	Color state[8][8];
	Move LastMove;
	std::array<Square,MAX_MOVES> LegalMoves;
	int LegalMovesSize;
	bool passflag;
	bool terminalflag;
	bool verbose;
	Board* parent;
	int path[MAX_DEPTH];
	int t_path[MAX_DEPTH];
	Stability stability[8][8];
	bool showval;
};


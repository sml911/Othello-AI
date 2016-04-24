#include "boardclass.h"

using namespace std;

Move MoveDecode(std::string);
PlayerType find_player(char*, Color);


int main(int argc, char *argv[])
{
    Square t_square;
    int movecount;
    Board board0;
    Color color = EMPTY;
    std::srand(std::time(0));
    board0.Reset();
    PlayerType black_player=HUMAN;
    PlayerType white_player=ALPHABETA_W;
    long ThinkTime=DEFAULT_TIME;
    
    for(int i=1;i<argc;i++)
    {
        if(argv[i][1]=='v' and argv[i][3]==0)
        {
            black_player=find_player(argv[i], BLACK);
            white_player=find_player(argv[i], WHITE);
        }
        else if(argv[i][0]>='0' and argv[i][0]<='9')
        {   
            ThinkTime=(long) 1000 * atof(argv[i]);
        }
        else if(argv[i][0]=='-' and argv[i][2]==0)
        {
            board0.Option(argv[i][1]);
        }
        else
        {
            std::cout << argv[i] << " is not open, turn: " << board0.GetTurn() << std::endl;
            ThinkTime=board0.LoadBoard(argv[i]);
            if(!ThinkTime) ThinkTime = DEFAULT_TIME;
        }
    }
    cout << "AI search constraint:  " << ThinkTime << "ms" << endl;
    for(int t=1;t<=70;t++)
    {
        movecount=board0.GenLegalMoves();
        if(movecount==0)
        {
            board0.Pass();
            continue;
        }
        board0.Show();
        
        board0.MoveGen(t_square, black_player, white_player, ThinkTime);
        board0.MakeMove(t_square);
    }
    return 0;
}
Square Board::MoveGen(Square& t_square, PlayerType black_player, PlayerType white_player, long ThinkTime)
{
    string tmove;
    Move playermove;
    const Move badmove(-1,-1);

    if((ThePlay==BLACK and black_player==HUMAN) or (ThePlay==WHITE and white_player==HUMAN)) //player move
    {
        while(1)
        {
            tmove = this->MovePrompt();
            playermove = MoveDecode(tmove);
            if (playermove==badmove) 
            {
                cout << "invalid move: " << tmove << endl;
                continue;
            }
            if (this->CheckLegal(playermove, t_square))
            {
                cout << "illegal move: " << tmove << endl;
                continue;
            }
            break;
        }
    }
    else if((ThePlay==BLACK and black_player==RANDOMMOVE) or (ThePlay==WHITE and white_player==RANDOMMOVE)) //computer1 move
    {
        long move_time = this->RandMove(t_square, ThinkTime);
        cout<<"I took "<<move_time<< "ms to move."<<endl;
    }
    else if((ThePlay==BLACK and black_player==ALPHABETA_P) or (ThePlay==WHITE and white_player==ALPHABETA_P)) //computer2 move
    {
        SetHeuristic(POSITIONAL);
        long move_time = this->TreeSearch(t_square, ThinkTime,ALPHABETAX);
        cout<<"I took "<<move_time<< "ms to move."<<endl;
    }
    else if((ThePlay==BLACK and black_player==ALPHABETA_W) or (ThePlay==WHITE and white_player==ALPHABETA_W)) //computer2 move
    {
        SetHeuristic(WEIGHTED);
        long move_time = this->TreeSearch(t_square, ThinkTime,ALPHABETA);
        cout<<"I took "<<move_time<< "ms to move."<<endl;
    }
    else if((ThePlay==BLACK and black_player==ALPHABETAX_W) or (ThePlay==WHITE and white_player==ALPHABETAX_W)) //computer2 move
    {
        SetHeuristic(WEIGHTED);
        long move_time = this->TreeSearch(t_square, ThinkTime,ALPHABETAX);
        cout<<"I took "<<move_time<< "ms to move."<<endl;
    }
    else if((ThePlay==BLACK and black_player==MINIMAX_W) or (ThePlay==WHITE and white_player==MINIMAX_W)) //computer2 move
    {
        SetHeuristic(WEIGHTED);
        long move_time = this->TreeSearch(t_square, ThinkTime,MINIMAX);
        cout<<"I took "<<move_time<< "ms to move."<<endl;
    }
    else if((ThePlay==BLACK and black_player==MINIMAX_P) or (ThePlay==WHITE and white_player==MINIMAX_P)) //computer2 move
    {
        SetHeuristic(POSITIONAL);
        long move_time = this->TreeSearch(t_square, ThinkTime,MINIMAX);
        cout<<"I took "<<move_time<< "ms to move."<<endl;
    }
    else 
    {
        std::cout << "its nobody's turn";
        exit(1);
    }
    return t_square;
}

Move MoveDecode(std::string s_move)
{
	Move move;
	Move err {-1,-1};
	if (s_move.length()!=2) return err;
	if(s_move[0] >= 'A' and s_move[0] <= 'H') move.col=(int) (s_move[0]-'A'); //Capital Letters
	else if(s_move[0] >= 'a' and s_move[0] <= 'h') move.col=(int) (s_move[0]-'a'); //Lowercase Letters
	else return err;
	if(s_move[1] >= '1' and s_move[1] <= '8') move.row= (int) (s_move[1]-'1');
	else return err;
	return move;
};

PlayerType find_player(char* arg, Color color)
{
    int i;
    if(color==BLACK) i=0;
    else if(color==WHITE) i=2;
    else
    {
        std::cout << "invalid argument" << arg << std::endl;
        exit(1);
    }
    switch(arg[i])
    {
        case 'p': 
        case 'h':
        case 'H':
            return HUMAN;
        case 'P':
            return ALPHABETA_P;
        case 'a':
        case 'c':
            return ALPHABETAX_W;
        case 'A':
            return ALPHABETA_W;
        case 'C':
            return MINIMAX_W;
        case 'r': 
        case 'R':
            return RANDOMMOVE;
        case 'm':
            return MINIMAX_W;
        case 'M': 
            return MINIMAX_P;
        default:
            std::cout<<"Invalid player identifier " << arg[i] << " in " << arg << std::endl;
            exit(1);
            break;            
    }
}
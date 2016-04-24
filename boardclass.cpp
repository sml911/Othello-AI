#include "boardclass.h"
using std::chrono::system_clock;

inline int int2coloffset(int dir)
{
	int offset=(dir+1)/3;
	if (offset==2) offset=-1;
	return offset;
}

inline int int2rowoffset(int dir)
{
	int offset=(dir+1)%3;
	if (offset==2) offset=-1;
	return offset;
}
inline Color OtherColor(Color color)
{
	if (color==BLACK) return WHITE;
	if (color==WHITE) return BLACK;
}

inline void Square::Mov2Sq(Move move)
{
	col=move.col;
	row=move.row;
}
inline Move Square::Sq2Mov()
{
	Move move(col,row);
	return move;
}
inline Color Board::Mov2State(Move move)
{
	return state[move.col][move.row];
}
inline void msleep (int millitime)
{
	sleep(millitime/1000);
	usleep((millitime%1000)*1000);
}
inline long mscount(system_clock::time_point tic, system_clock::time_point toc)
{
	std::chrono::milliseconds mtime = std::chrono::duration_cast<std::chrono::milliseconds> (toc-tic);
	return (long) mtime.count();
}
inline bool isxsquare(int i, int j)
{
	return ((i==1 || i==6) && (j==1 || j==6));
}
inline bool iscsquare(int i, int j)
{
	return ((i==1 || j==1) && (i>1 && i<6) &&
			(i==6 || j==6));	
}
inline bool isedge(int i, int j)
{
	return (((i>1 && i<6) && (j<1 && j>6)) ||
			(j>1 && j<6) && (i<1 && i>6));	
}
inline bool iscorner(int i, int j)
{
	return((i==0 || i==7) && (j==0 || j==7));
}


void ShowLeaf(int v,bool maxplayer,int depth,bool terminalflag, int a=0,int b=0,bool reset = false)
{
	const int maxdepth=13;
	static int previousdepth = 0;
	static bool branch[maxdepth] = {0};
	if(reset) 
	{
		for(int i=0;i<maxdepth;i++)
		{
			branch[i]=0;
		}
		previousdepth = 0;
		return;
	}
	int exponent=0;
	std::cout << "\033[0;35m";

	if(depth==0)
	{
		std::cout << "\\";
		for(int g=previousdepth+1; g<15;g++)
		{
			if (branch[g]==true) std::cout << "       |";
			else std::cout << "        ";
		}
		for(int g=1; g<previousdepth;g++)	branch[g]=false;
		std::cout << std::endl;
	}
	else if(terminalflag)
	{
		std::cout << "\\";
		for(int g=previousdepth+1; g<15;g++)
		{
			if (branch[g]==true) std::cout << "       |";
			else std::cout << "        ";
		}
		std::cout<<std::endl;
		for(int g=0; g<depth;g++)
			std::cout << "        ";
	}
	while(v>=10 or v<=-10)
	{
		exponent++;
		v=v/10;
	}
	if (maxplayer)	std::cout << "\033[32m"; 
	else  std::cout << "\033[31m";
	if (v>=0) std::cout << '+';
	std::cout << v << 'e' << exponent << "\033[0;0m";
	std::cout << "~~~~";
	previousdepth=depth;
	branch[depth]=true;
}
Square::Square(Color color, int mycol, int myrow, const std::array<int,8>& rflipped)
{
	player=color;
	col=mycol;
	row=myrow;
	flipped=rflipped;
	flipcount=0;
	for(int i=0;i<8;i++) flipcount=flipcount+flipped[i];
};
Board::Board()
{
};
Board::Board(Board* rhs)
{
	heuristic=rhs->heuristic;
	ThePlay=rhs->ThePlay;
	LastMove=rhs->LastMove;
	wscore=rhs->wscore;
	bscore=rhs->bscore;
	memcpy(state, rhs->state, sizeof(Color)*64);
	LegalMoves=rhs->LegalMoves;
	passflag=rhs->passflag;
	terminalflag=rhs->terminalflag;
	parent = rhs;
	verbose = rhs->verbose;
	showval= rhs->showval;
	memcpy(path, rhs->path, sizeof(int)*MAX_DEPTH);
	memcpy(t_path, rhs->t_path, sizeof(int)*MAX_DEPTH);
	for (int i = 0; i < 8; i++)
    {
    	for (int j = 0; j < 8; j++)
    	{
    		stability[i][j]=SEMISTABLE;
    	}
    }
};
Board::~Board(){};

bool Board::CheckLegal(Move move, Square& square)
{
	for(int i=0;i<LegalMovesSize;i++)
	{
		if(LegalMoves[i].Sq2Mov()==move) 
		{
			square=LegalMoves[i];
			return 0;
		}
	}
	return 1;
}
void Board::Option(char opcode)
{
	switch(opcode)
    {
    	case 'v':
    		verbose=true;
    		break;
    	case 'h':
    		showval=true;
    	default:
    		std::cout<<"Invalid option " << opcode << std::endl;
    		break;
    }
    return;
}
void Board::MakeMove(Square square)
{
	int coladd, rowadd, scoreadd=0, colbase, rowbase;
	Move move;
	colbase=square.col;
	rowbase=square.row;
	LastMove.col=colbase;
	LastMove.row=rowbase;
	state[colbase][rowbase]=ThePlay;
	for(int dir=0;dir<8;dir++)
	{
		coladd=int2coloffset(dir);
		rowadd=int2rowoffset(dir);
		for(int flipline=1;flipline<=square.flipped[dir];flipline++)
		{
			state[colbase+(flipline*coladd)][rowbase+(flipline*rowadd)]=ThePlay;
			scoreadd++;
		}	
		move.col=colbase+coladd;
		move.row=rowbase+rowadd;
		if((move.col>7 or move.col<0) or (move.row>7 or move.row<0)) continue;
		if(state[move.col][move.row]==EMPTY) state[move.col][move.row]=POSSIBLE;
	}
	if(ThePlay==BLACK)
	{
		bscore=bscore+scoreadd+1;
		wscore=wscore-scoreadd;
	} 
	else if(ThePlay==WHITE) 
	{
		wscore=wscore+scoreadd+1; 
		bscore=bscore-scoreadd;
	}
	LegalMovesSize=0;
	passflag=false;
	ThePlay=OtherColor(ThePlay);
}
void Board::Pass()
{
	ThePlay=OtherColor(ThePlay);
	if (terminalflag) this->GameEnd();
	else if (passflag) terminalflag=true;
	else passflag=true;
}

void Board::Reset()
{
	for(int i=0;i<8;i++)
	{
		for(int j=0;j<8;j++)
		{
			state[i][j]=EMPTY;
		}
	}
	for(int j=0;j<MAX_DEPTH;j++)
	{
		path[j]=0;
	}
	parent=NULL;
	verbose=false;
	passflag=false;
	terminalflag=false;
	showval=false;
	ThePlay=BLACK;
	bscore=2;
	wscore=2;
	LegalMovesSize=0;
	state[3][3]=WHITE;
	state[3][4]=BLACK;
	state[4][3]=BLACK;
	state[4][4]=WHITE; 
	for(int i=2;i<=5;i++)
	{
		for(int j=2;j<=5;j++)
		{
			if(state[i][j]==EMPTY) state[i][j]=POSSIBLE;  
		}
	}
    for (int i = 0; i < 8; i++)
    {
    	for (int j = 0; j < 8; j++)
    	{
    		stability[i][j]=SEMISTABLE;
    	}
    }
}
long Board::LoadBoard(char* filename)
{
	int col=0,row=0;
    std::ifstream myReadFile;
    std::cout << filename << " is not open, turn: " << bscore+wscore-3;
    myReadFile.open(filename,std::ifstream::in);
    char output[100];
    if (myReadFile.is_open()) 
    {
    	wscore=0;
		bscore=0;
    	std::cout << filename << "is open";
        char c = myReadFile.get();
        int temptime=0;
        while (myReadFile.good()) 
        {
            switch (c)
            {
                case '0':
                    state[col++][row]=POSSIBLE;
                    break;
                case '1':
                    if (row<8)
                    {                   
                    	state[col++][row]=BLACK;
                    	bscore++;
                    }
                    else if (row==8) ThePlay=BLACK;
                    break;           
                case '2':
                    if (row<8)
                    {
	                    state[col++][row]=WHITE;
	                    wscore++;                    	
                    }
                    else if (row==8) ThePlay=WHITE;
                    break;
                case '\n':
                    col=0;
                    row++;
                    break;
                default:
                    break;    
            }
            c = myReadFile.get();
            if (row==9)
            {
            	int decfactor=1;
            	while (myReadFile.good())
            	{            		
            		temptime=temptime*10 + (int)(c-'0');
   		            c = myReadFile.get();
   		            if(c=='.')
   		            {
   		            	c = myReadFile.get();   		            	
   		            	while (myReadFile.good())
            			{
            				decfactor=decfactor*10;            		
            				temptime=temptime*10 + (int)(c-'0');
   		            		c = myReadFile.get();            			
            			}

   		            }
            	}
            	return (long) (temptime*1000)/decfactor;
            	break;
            }
        }
    }
    else std::cout << "Invalid Argument: " << filename << std::endl; 
    myReadFile.close();    
}
void Board::Show()
{
	int N=8;
	std::cout<<"\n\n\n  ";
	std::cout<< "\033[1;37;42m";
	for (int row=0;row<=N+1;row++)
	{
		for (int col=0; col<=N+1; col++)
		{
			if(row==0 or row==N+1)
			{
				if (col==0 or col==N+1)
					std::cout<<"  ";
				else std::cout<<" "<<(char)(col-1+'A');
			}
			else 
			{
				Move ThisMove(col-1,row-1);
/*	Stability			
				if (col==0 or col==N+1);
				else if (state[col-1][row-1]<OTHER);
				else if (stability[col-1][row-1]==UNSTABLE) std::cout<< "-";
				else if (stability[col-1][row-1]==SEMISTABLE) std::cout<< "=";
				else if (stability[col-1][row-1]==STABLE) std::cout<< "+";
*/		
				if (col==0 or col==N+1)	std::cout<<" "<<row;
				else if (state[col-1][row-1]==EMPTY) std::cout<< " .";
				else if (state[col-1][row-1]==POSSIBLE) std::cout<< " .";
				else if (state[col-1][row-1]==BLACK and (LastMove==ThisMove)) std::cout<< "\033[30m 0\033[1;37;42m";
				else if (state[col-1][row-1]==WHITE and (LastMove==ThisMove)) std::cout<< " 0";
				else if (state[col-1][row-1]==LEGAL and ThePlay==BLACK) std::cout<< "\033[30m *\033[1;37;42m";
				else if (state[col-1][row-1]==LEGAL and ThePlay==WHITE) std::cout<< " *";	
				else if (state[col-1][row-1]==BLACK) std::cout<< "\033[30m O\033[1;37;42m";
				else if (state[col-1][row-1]==WHITE) std::cout<< " O";
				else std::cout << " X";


			}
		}
		std::cout<< " "<<"\033[0;0m\n  "<<"\033[1;37;42m";
	}
	std::cout<< "\033[0;0m\n";
	std::cout << "Black: " << bscore << "\tWhite: " << wscore << std::endl;
}

void StableDiskLabel(Color color, Color (&curstate)[8][8], Stability (&stab_state)[8][8])
{
	int hstart[4]={0,7,7,0};
	int hend[4]={7,0,0,7};
	int hopdir[4]={1,-1,-1,1};
	int rstart[4]={0,0,7,7};
	int rend[4]={7,7,0,0};
	int rolldir[4]={1,1,-1,-1};
	int rcount, rmax, col, row;
	bool exitflag;
	for(int i=0;i<4;i++)
	{
		rmax=8;
		for (int hop = hstart[i]; hop!=hend[i]+hopdir[i]; hop+=hopdir[i])
		{
			rcount=0;
			exitflag=false;
			for (int roll = rstart[i]; roll!=rend[i]+rolldir[i]; roll+=rolldir[i])
			{
				if(i%2) //hop<=>col roll<=>row
				{
					col=hop;
					row=roll;
				}
				else //roll<=>col hop<=>row
				{
					col=roll;
					row=hop;
				}
				if(exitflag)
				{
					if (curstate[col][row]<OTHER) break;
					else if (roll==rend[i])
					{
						for(roll = rstart[i]+rcount*rolldir[i]; roll!=rend[i]+rolldir[i]; roll+=rolldir[i])
						{
							if(i%2) stab_state[hop][roll]=STABLE;
							else stab_state[roll][hop]=STABLE;
						}
					}
				}
				else if (curstate[col][row]==color)
				{
					rcount++;
					stab_state[col][row]=STABLE;
					if(rcount==rmax)
					{
						rmax=rcount-1;
						break;
					}
				} 
				else if (hop==hstart[i]) 
				{
					rmax=rcount-1;
					exitflag=true;
				}
				else
				{
					rmax=rcount-1;
					break;
				}
			}
			if(rmax==0) rmax=1;
			else if (rmax==-1) break;
		}
	}
}

int Board::GenLegalMoves()
{
	bool flip;
	std::array<int,8> flipdir;
	int coladd, rowadd, t_row, t_col, rowbase, colbase, capturecount;
	Color enemycolor;
	enemycolor=OtherColor(ThePlay);
	for (int i = 0; i < 8; i++)
    {
    	for (int j = 0; j < 8; j++)
    	{
    		stability[i][j]=SEMISTABLE;
    	}
    }
    StableDiskLabel(ThePlay,state,stability);
	//Gen Legal Moves
	for(int colbase=0;colbase<8;colbase++)
    {
    	for(int rowbase=0;rowbase<8;rowbase++)
    	{
    		if (state[colbase][rowbase]==LEGAL) state[colbase][rowbase]=POSSIBLE;
	    	if(state[colbase][rowbase]==POSSIBLE)
	    	{
	    		flip=false;
		    	for(int dir=0; dir<8;dir++)
		    	{
		    		coladd=int2coloffset(dir);
		    		rowadd=int2rowoffset(dir);
		    		capturecount=0;
		    		t_row=rowbase+rowadd;
		    		t_col=colbase+coladd;
	    			if((t_col>7 or t_col<0) or (t_row>7 or t_row<0))
	    			{
	    				flipdir[dir]=0;
	    				continue;
	    			}
		    		while (state[t_col][t_row]==enemycolor)
		    		{
		    			t_col=t_col+coladd;
		    			t_row=t_row+rowadd;
		    			if((t_col>7 or t_col<0) or (t_row>7 or t_row<0))
		    			{
		    				capturecount=0;
		    				break;
		    			}
		    			capturecount++;
		    		}
		    		if (state[t_col][t_row]==ThePlay and capturecount!=0) 
		    		{
		    			for(int n=0; n<capturecount;n++)
		    			{
		    				t_col=t_col-coladd;
		    				t_row=t_row-rowadd;    			
		    				stability[t_col][t_row]=UNSTABLE;
		    			}
		    			flipdir[dir]=capturecount;
		    			flip=true;
		    		}
		    		else flipdir[dir]=0;
		    	}
		    	if (flip)
		    	{
		    		Square tempsquare(ThePlay,colbase,rowbase,flipdir);
		    		LegalMoves[LegalMovesSize++]=tempsquare;
		    		state[colbase][rowbase]=LEGAL;
		    	}
		    }
		}
    }
    return LegalMovesSize;
}
int LegalMovesCount(Color color, Color (&curstate)[8][8], Stability (&stab_state)[8][8])
{
	bool flip;
	int coladd, rowadd, t_row, t_col, rowbase, colbase, capturecount;
	int movecount=0;
	Color enemycolor;
	enemycolor=OtherColor(color);
	//Clear Legal Moves
	for(int colbase=0;colbase<8;colbase++)
    {
    	for(int rowbase=0;rowbase<8;rowbase++)
    	{
    		if (curstate[colbase][rowbase]==LEGAL) curstate[colbase][rowbase]=POSSIBLE;
	    	if(curstate[colbase][rowbase]==POSSIBLE or curstate[colbase][rowbase]==LEGAL)
	    	{
	    		flip=false;
		    	for(int dir=0; dir<8;dir++)
		    	{
		    		coladd=int2coloffset(dir);
		    		rowadd=int2rowoffset(dir);
		    		capturecount=0;
		    		t_row=rowbase+rowadd;
		    		t_col=colbase+coladd;
	    			if((t_col>7 or t_col<0) or (t_row>7 or t_row<0))
	    			{
	    				continue;
	    			}
		    		while (curstate[t_col][t_row]==enemycolor)
		     		{
		    			t_col=t_col+coladd;
		    			t_row=t_row+rowadd;
		    			if((t_col>7 or t_col<0) or (t_row>7 or t_row<0))
		    			{
		    				capturecount=0;
		    				break;
		    			}
		    			capturecount++;
		    		}
		    		if (curstate[t_col][t_row]==color and capturecount!=0) 
		    		{
    					for(int n=0; n<capturecount;n++)
		    			{
		    				t_col=t_col-coladd;
		    				t_row=t_row-rowadd;    			
		    				stab_state[t_col][t_row]=UNSTABLE;
		    			}
						flip=true;
		    		}
		    	}
		    	if(flip==true)
		    	{
	    			movecount++;
	    			curstate[colbase][rowbase]=LEGAL;
	    		}
		    }
		}
    }
    return movecount;
}

std::string Board::MovePrompt()
{
	std::string t_move;
	int turn=bscore+wscore-3;
	if (ThePlay==BLACK)	std::cout << "black(" << turn << "): ";
	else if (ThePlay==WHITE) std::cout << "white(" << turn << "): ";
	std::cin >> t_move;
	return t_move;
}
int Board::MaxDisk(Color color)
{
	int myscore=0;
	for(int i=0;i<8;i++)
	{
		for(int j=0;j<8;j++)
		{
			if(state[i][j]==color)
			{
				myscore++;
			}
		}
	}
	return myscore;
}
int Board::Positional(Color color)
{
	int myscore=0;
	int PV[8][8] =
	{
		{20, -3, 11, 8, 8, 11, -3, 20},
    	{-3, -7, -4, 1, 1, -4, -7, -3},
    	{11, -4, 2, 2, 2, 2, -4, 11},
    	{8, 1, 2, -3, -3, 2, 1, 8},
    	{8, 1, 2, -3, -3, 2, 1, 8},
    	{11, -4, 2, 2, 2, 2, -4, 11},
    	{-3, -7, -4, 1, 1, -4, -7, -3},
    	{20, -3, 11, 8, 8, 11, -3, 20}
    };
	
	for(int i=0;i<8;i++)
	{
		for(int j=0;j<8;j++)
		{
			if(state[i][j]==color)
			{
				myscore += PV[i][j];
			}
		}
	}

	return myscore;
}
int Board::Weighted(int* weights)
{
/*
	index to heuristic

	0 - positional
	1 - mobility
	2 - corner occupancy
	3 - corner closeness
	4 - parity
	5 - stability
*/
	int myscore=0, denominator=0;
	int scores[H_DIM]={0};
	int &posscore=scores[0];
	int &mobscore=scores[1];
	int &corscore=scores[2];
	int &cclscore=scores[3];
	int &parscore=scores[4];
	int &stbscore=scores[5];

	Color maxcolor = ThePlay;
	Color mincolor =OtherColor(ThePlay);
	int minmoves = 0;
	int maxmoves = LegalMovesSize;
	Color oppstate[8][8];
	int PV[8][8] =
	{
		{20, -3, 11, 8, 8, 11, -3, 20},
    	{-3, -7, -4, 1, 1, -4, -7, -3},
    	{11, -4, 2, 2, 2, 2, -4, 11},
    	{8, 1, 2, -3, -3, 2, 1, 8},
    	{8, 1, 2, -3, -3, 2, 1, 8},
    	{11, -4, 2, 2, 2, 2, -4, 11},
    	{-3, -7, -4, 1, 1, -4, -7, -3},
    	{20, -3, 11, 8, 8, 11, -3, 20}
    };
    //Parity
    if ((wscore+bscore)%2) parscore += 50;
    else parscore -=50;

    //Mobility
	{
	    memcpy(oppstate, state, sizeof(Color)*64);
	    minmoves = LegalMovesCount(mincolor,oppstate,stability);
		mobscore = (200 * (maxmoves-minmoves))/(maxmoves + minmoves);
	}

	//Stable Disks
	{
		StableDiskLabel(mincolor,oppstate,stability);
	}

	//Board position of discs, Corners, Corner Closeness
	bool vacant_corner;
	int inv;
	for(int i=0;i<8;i++)
	{
		for(int j=0;j<8;j++)
		{
			if(state[i][j]==maxcolor) 
			{
				inv=1;
			}
			else if(state[i][j]==mincolor)
			{
				inv=-1;
			} 
			else continue;

			//Positional
			posscore += inv*PV[i][j];

			//Corner Closeness
			vacant_corner=((i<2 and j<2 and state[0][0]<OTHER)or
						   (i<2 and j>5 and state[0][7]<OTHER)or
		   				   (i>5 and j<2 and state[7][0]<OTHER)or
						   (i>5 and j>5 and state[7][7]<OTHER));
			
			if(vacant_corner && isxsquare(i,j))  cclscore -= inv*35; 
			else if(vacant_corner)  cclscore -= inv*15;
			
			//Corner
			else if(iscorner(i,j))  corscore += inv*50;
			
			//Stability
			if      (stability[i][j]==STABLE) stbscore += inv*8;
			else if (stability[i][j]==SEMISTABLE) stbscore += inv;
			else    stbscore -= inv*2;
		}
	}

	for(int n=0;n<H_DIM;n++)
	{
		myscore+=weights[n]*scores[n];
		denominator+=weights[n];
	}
	myscore =(10*myscore)/denominator;
	return myscore;
}
int Board::Utility() 
{
/*
	index to heuristic

	0 - positional
	1 - mobility
	2 - corner occupancy
	3 - corner closeness
	4 - parity
	5 - stability
*/
	if(terminalflag) 
	{
		if(ThePlay==BLACK) return (bscore-wscore)*WIN_THRESH;
		if(ThePlay==WHITE) return (wscore-bscore)*WIN_THRESH;
	}
	const static int WeightsByTurn[H_RES][H_DIM] = 
		{
			{3,5,6,7,1,6},  // turn 0-15
			{3,7,5,6,2,6},	// turn 16-31
			{3,7,4,5,3,5},	// turn 32-47
			{3,5,3,4,4,4}	// turn 48-63
		};
	switch (heuristic)
	{	case MAXDISK:
			return (this->MaxDisk(ThePlay)-this->MaxDisk(OtherColor(ThePlay)));
		case POSITIONAL:
			return (this->Positional(ThePlay)-this->Positional(OtherColor(ThePlay)));
		case WEIGHTED:
			int Weights[H_DIM];
			memcpy(Weights, WeightsByTurn[H_RES*(wscore+bscore-3)/64], sizeof(int)*H_DIM);
			return (this->Weighted(Weights));
	}
}
void Board::UtilitySort(int* value, int paths[][MAX_DEPTH])
{
	int passnum=0;
	bool exitflag=false;
	int temp,n;
	while(passnum<7 and exitflag==false)
	{
		exitflag=true;
		for(n=passnum;n<LegalMovesSize-1;n++)
		{
			if (value[n]<value[n+1])
			{
				temp = value[n];
				value[n] = value[n+1];
				value[n+1] = temp; 
				memcpy(t_path,paths[n],sizeof(int)*MAX_DEPTH);
				memcpy(paths[n],paths[n+1],sizeof(int)*MAX_DEPTH);
				memcpy(paths[n+1],t_path,sizeof(int)*MAX_DEPTH);
				Square tempsq(LegalMoves[n]);
				LegalMoves[n] = LegalMoves[n+1];
				LegalMoves[n+1] = tempsq; 				
				exitflag = false;
			}
			else if(value[n]==value[n+1] and std::rand()%2)
			{
				memcpy(t_path,paths[n],sizeof(int)*MAX_DEPTH);
				memcpy(paths[n],paths[n+1],sizeof(int)*MAX_DEPTH);
				memcpy(paths[n+1],t_path,sizeof(int)*MAX_DEPTH);
				Square tempsq(LegalMoves[n]);
				LegalMoves[n] = LegalMoves[n+1];
				LegalMoves[n+1] = tempsq; 
			}
		}
	}
	if (showval||verbose)
	{
		for(n=0;n<LegalMovesSize;n++)
		{
			std::cout << LegalMoves[n].col <<LegalMoves[n].row << ':'<< value[n] << ' '; 
		}
		std::cout << std::endl;
	}
}

int Board::AlphaBeta(int depth, int alpha, int beta, bool maxplayer, bool initialexplore=false)
{	
	int inverter;
	int v;

	if(maxplayer) inverter=1;
	else inverter=-1;

	if(LegalMovesSize==0)
	{
		if (passflag)
		{
			terminalflag=true;
			v=inverter*this->Utility();
			if(verbose) ShowLeaf(v,!maxplayer,depth,terminalflag,alpha,beta);
			return v;
		}
		
		else
		{
			passflag=true;
			ThePlay=OtherColor(ThePlay);
			this->GenLegalMoves();
			return this->AlphaBeta(depth, alpha,beta, !maxplayer);
		}
	}
	if (depth==0)
	{
		v=inverter*this->Utility();
		if(verbose) ShowLeaf(v,maxplayer,depth,terminalflag,alpha,beta);
		return v;
	}
	v=-inverter*INF;
	int firstnode=-1;

	for (int i=0; i<LegalMovesSize; i++)
	{
	//	if(i=firstnode) continue;
		Board state_x(this);
		state_x.MakeMove(LegalMoves[i]);
		state_x.GenLegalMoves();
		if(maxplayer)
		{
			v=std::max(v,state_x.AlphaBeta(depth-1,alpha,beta,!maxplayer));
			alpha=std::max(alpha,v);
			if(beta<alpha) 
			{
				if (verbose) std::cout << "\b\b\baX~";
				break;
			}
		}
		else
		{
			v=std::min(v,state_x.AlphaBeta(depth-1,alpha,beta,!maxplayer));	
			beta=std::min(beta,v);
			if(beta<alpha) 
			{
				if (verbose) std::cout << "\b\b\bbX~";
				break;
			}	
		}
//		if (verbose)	std::cout<<v<<','<<alpha<<','<<beta;	
	}
	if(verbose) ShowLeaf(v,maxplayer,depth,terminalflag,alpha,beta);
	return v;
}

int Board::AlphaBetax(int depth, int alpha, int beta, bool maxplayer, bool initialexplore=false)
{	
	int inverter;
	int v,t_v;

	if(maxplayer) inverter=1;
	else inverter=-1;

	if(LegalMovesSize==0)
	{
		if (passflag)
		{
			terminalflag=true;
			v=inverter*this->Utility();
			if(verbose) ShowLeaf(v,!maxplayer,depth,terminalflag,alpha,beta);
			SetParentPath(t_path); 
			return v;
		}
		
		else
		{
			passflag=true;
			ThePlay=OtherColor(ThePlay);
			this->GenLegalMoves();
			SetParentPath(t_path);		
			return this->AlphaBetax(depth, alpha,beta, !maxplayer);
		}
	}
	if (depth==0)
	{
		v=inverter*this->Utility();
		if(verbose) ShowLeaf(v,maxplayer,depth,terminalflag,alpha,beta);
		SetParentPath(t_path);
		return v;
	}
	v=-inverter*INF;
	int firstnode=-1;
	if(initialexplore==true)
	{
		firstnode=path[depth-2];
	}
	for (int i=0; i<LegalMovesSize; i++)
	{
		if(i==firstnode and initialexplore==false) continue;
		else if(initialexplore==true) i=firstnode;
		t_path[depth-1]=i;
		Board state_x(this);
		state_x.MakeMove(LegalMoves[i]);
		state_x.GenLegalMoves();
	
		if(initialexplore==true and depth > 2)
		{
			t_v = state_x.AlphaBetax(depth-1,alpha,beta,!maxplayer,true);
		}
		else t_v = state_x.AlphaBetax(depth-1,alpha,beta,!maxplayer);
	
		if(maxplayer)
		{
			if (t_v>v)
			{
				v=t_v;
				memcpy(path,t_path, sizeof(int)*MAX_DEPTH);
			}
			alpha=std::max(alpha,v);
			if(beta<alpha) 
			{
				if (verbose) std::cout << "\b\b\baX~";
				break;
			}
		}
		else
		{
			if (t_v<v)
			{
				v=t_v;
				memcpy(path,t_path, sizeof(int)*MAX_DEPTH);
			}
			beta=std::min(beta,v);
			if(beta<alpha) 
			{
				if (verbose) std::cout << "\b\b\bbX~";
				break;
			}	
		}
		if (initialexplore==true)
		{
			initialexplore=false;
			i=-1;
		}
	}
	if(verbose) ShowLeaf(v,maxplayer,depth,terminalflag,alpha,beta);
//	std::cout << firstnode;
	SetParentPath(path);
	return v;
}

int Board::MiniMax(int depth, bool maxplayer)
{	
	int inverter;
	if(maxplayer) inverter=1;
	else inverter=-1;

	if(LegalMovesSize==0)
	{
		if (passflag)
		{
			terminalflag=true;
			if(verbose) ShowLeaf(inverter*this->Utility(),!maxplayer,depth,terminalflag);
			return inverter*this->Utility();
		}
		
		else
		{
			passflag=true;
			ThePlay=OtherColor(ThePlay);
			this->GenLegalMoves();
			return this->MiniMax(depth, !maxplayer);
		}
	}
	if (depth==0)
	{
		if(verbose) ShowLeaf(inverter*this->Utility(),maxplayer,depth,terminalflag);
		return inverter*this->Utility();
	}
	int v=-inverter*INF;
	for (int i=0; i<LegalMovesSize; i++)
	{
		Board state_x(this);
		state_x.MakeMove(LegalMoves[i]);
		state_x.GenLegalMoves();
		if(maxplayer)
		{
			v=std::max(v,state_x.MiniMax(depth-1,!maxplayer));
		}
		else
		{
			v=std::min(v,state_x.MiniMax(depth-1,!maxplayer));	
		}
	}
	if(verbose) ShowLeaf(v,maxplayer,depth,terminalflag);
	return v;
}

long Board::TreeSearch(Square& action, long t_time, Search search)
{
	std::chrono::milliseconds maxtime (t_time);
	std::chrono::milliseconds timebuffer (10);
	std::chrono::milliseconds elapsedtime(0);
	system_clock::time_point tic = system_clock::now();
	system_clock::time_point toc = system_clock::now();
	
	char letter;
	int number;
	int utility[MAX_MOVES];
	int turn=bscore+wscore-3;
	std::string t_move;
	int mypaths[MAX_MOVES][MAX_DEPTH] = {0};
	int BFab[16] = {3,3,3,3,4,4,5,5,5,4,4,3,3,2,2,1};
	int BFmm[16] = {1,4,6,8,9,10,11,12,12,11,10,8,5,4,3,1};
	//////
	for (int depthlim=1;;depthlim++)
	{
		int v=-INF;
		signed int alpha=-INF;
		signed int beta=INF;
		bool exitflag = false;

		if (verbose) ShowLeaf(v,true,depthlim,terminalflag,alpha,beta,true); //reset static vars for tree
		for (int i=0; i<LegalMovesSize; i++)
		{
			memcpy(path, mypaths[i], sizeof(int)*MAX_DEPTH);
			Board state_x(this);
			state_x.MakeMove(LegalMoves[i]);
			state_x.GenLegalMoves();
			if(search==MINIMAX)
			{	
				utility[i] = state_x.MiniMax(depthlim-1,false);
			}
			else if(search==ALPHABETA)
			{
				utility[i] = state_x.AlphaBeta(depthlim-1,alpha,beta,false);
			}
			else if(search==ALPHABETAX)
			{
				utility[i] = state_x.AlphaBetax(depthlim-1,alpha,beta,false,depthlim>2);
				memcpy(mypaths[i], t_path, sizeof(int)*MAX_DEPTH);
			}
			v=std::max(v,utility[i]);
			alpha=v;

			toc=system_clock::now();
			elapsedtime=std::chrono::duration_cast<std::chrono::milliseconds>(toc-tic);
			if(i==0) timebuffer=elapsedtime;
			if(elapsedtime>(maxtime-timebuffer))
			{
				std::cout << "search to depth " << depthlim-1 << " from d1 state " << i << std::endl;
				exitflag=true;
				break;
			}

		}
		if(exitflag==false)
		{
			if(verbose) 
			{
				ShowLeaf(v,true,depthlim,false);
				std::cout << 'O' << std::endl;
			}
			this->UtilitySort(utility,mypaths);

			/////////////////////////////////////
			toc=system_clock::now();
			elapsedtime=std::chrono::duration_cast<std::chrono::milliseconds>(toc-tic);
			if(search==ALPHABETAX || search==ALPHABETA) timebuffer=elapsedtime*(BFab[(bscore+wscore+depthlim-3)/4]);
			else timebuffer=elapsedtime*(BFmm[(bscore+wscore+depthlim-3)/4]);

			if((elapsedtime>(maxtime-timebuffer)) or (alpha>=WIN_THRESH) or (alpha<=-WIN_THRESH) or (bscore+wscore+depthlim==64))
			{
				std::cout << "search to depth " << depthlim << std::endl;
				exitflag=true;
			}
			/////////////////////////////////////////
		}
		if(exitflag==true)
		{
			action=LegalMoves[0];
			if (ThePlay==BLACK)	std::cout << "black(" << turn << "): ";
			else if (ThePlay==WHITE) std::cout << "white(" << turn << "): ";
			letter = char (action.col + 65);
			number = action.row + 1;
			std::cout << letter << number << std::endl;

			return mscount(tic,system_clock::now());
		}
	}
}

long Board::RandMove(Square& action, long t_time)
{
	std::chrono::milliseconds maxtime (t_time);
	system_clock::time_point tic = system_clock::now();

	char letter;
	int number;
	int turn=bscore+wscore-3;
	std::string t_move;
	if (ThePlay==BLACK)	std::cout << "black(" << turn << "): ";
	else if (ThePlay==WHITE) std::cout << "white(" << turn << "): ";
	action=LegalMoves[std::rand()%LegalMovesSize];
	letter = char (action.col + 65);
	number = action.row + 1;
	//msleep(t_time);
	std::cout << letter << number << std::endl;

	system_clock::time_point toc = system_clock::now();
	return mscount(tic,toc);
}

int Board::Score()
{
	if(ThePlay==WHITE) return (wscore-bscore);
	else if(ThePlay==BLACK) return (bscore-wscore);
}

inline bool Board::TerminalTest()
{
	if(LegalMovesSize) return false;
	else
	{
		if(!passflag)
		{
			passflag=true;
			return false;
		}
		else
		{
			terminalflag=true;
			return true;
		}
	}

}

void Board::GameEnd()
{
	this->Show();
	std::cout << "Game Over" << std::endl;
	std::cout << "Black: " << bscore << std::endl;
	std::cout << "White: " << wscore << std::endl;
	if(bscore>wscore) {std::cout << "Black Wins!" << std::endl;} 
	else if (wscore>bscore) {std::cout << "White Wins" << std::endl;}
	else {std::cout << "Its a Draw!\n";}
	exit(0);
}
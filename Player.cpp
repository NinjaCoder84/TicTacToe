using namespace std;

class Board;

#include "Board.hpp"
#include "Player.hpp"
#include <cstring>
#include <iostream>
#include "cstdlib"
#include "Permutation3.hpp"
#include <limits>
#include "Bcurses.hpp"

Player::Player(PLAYER_TYPE player_t, char ticker) {

	this->player_t = player_t;
	this->ticker = ticker;
	board = NULL;
	move = -1;
	preffered_move_loc = 0;
	if ( player_t == COMPUTER )
	        strcpy(this->name,"COMPUTER");
	else
                strcpy(this->name,"HUMAN");
}

PLAYER_TYPE Player::get_player_type() {

	return player_t;

}

char Player::get_ticker() {

	return ticker;

}

void Player::set_name(char *n) {

	strncpy(name, n, MAX_NAME);
}

char * Player::get_name() {

	return name;

}

void Player::set_board(Board *board) {

	this->board = board;

}

void Player::make_player_move() {

	int valid_move = Board::OUT_OF_BOARD;
	string s;
	if (board) {
		if (player_t == HUMAN) {

			while (valid_move != Board::NO_ERROR) {
				s = "Hello ";
				s += get_name();
				s += " ( ";
				s += get_ticker();
				s += " ) , enter the position number for your move : ";
				cout << s ;
				while(!(cin >> move)){
					cin.clear();
					cin.ignore(numeric_limits<streamsize>::max(), '\n');
					cout << "\nInvalid input. Enter only number : ";
				}

				valid_move = board->validate_move(move);

				switch (valid_move) {
				
					case Board::NO_ERROR:
						board->set_move(move);
						break;
					
					case Board::ALREADY_USED_SPACE:
						s = "Invalid move. Location ";
						s += std::to_string(move);
						s += " is already played. Try again\n";
						cout << s;
						break;
					
					case Board::OUT_OF_BOARD:
                                                s = "Invalid move. ";
                                                s += move;
                                                s += "is out of board area. Enter any value from 0 to 8. Try again\n";
                                                cout << s;
						break;
					
					default:
                                                s = "Invalid move. ";
                                                s += move;
                                                s += "Try again\n";
						cout << s;
						break;
				}
			}
		} else {
                        s = "Its computer's turn, and the computer makes move now\n";
			cout << s;

			if (board->get_difficulty_level() == Board::EASY) {

				/* For easy level get the move randomly*/
				std::srand(time(0));
				int move;

				do {
					move = std::rand() % 9 ;
				} while (board->validate_move(move) != Board::NO_ERROR );

				board->set_move(move);

				return;
			}

			/* Lets think how we can apply artificial intelligence */

			Player *list[2] = {  board->turn ,/* first see if Comupter can win*/
					      board->get_last_player() /*this time check if we can defend*/
					   };
			int **p;
			int diagonal_back[3] = { 0, 4, 8 };
			int diagonal_forward[3] = { 2, 4, 6 };

			for(int player_turn = 0; player_turn < 2 ; player_turn ++ ) {
			
				/* vertical */
				for (int i = 0; i < 3; i++) {
					int arr[3] = { i, i + 3, i + 6 };
					Permutation3 perm3 = Permutation3(arr);
					p = perm3.get_permutation_tupple();
					char * ba = board->get_board_area();
					for (int j = 0; j < 6; j++) {
						if ( (ba[p[j][0]] == list[player_turn]->ticker) &&
							(ba[p[j][(1)]] == list[player_turn]->ticker) ) {
							valid_move = board->validate_move(p[j][(2)]);
							if (valid_move 	== Board::NO_ERROR) {
								board->set_move(p[j][(2)]);
								return;
							}
						}
					}
				}

				/* horizontal */
				for (int i = 0; i < 9; i+=3) {
					int arr[3] = { i, i + 1, i + 2 };
					Permutation3 perm3 = Permutation3(arr);
					p = perm3.get_permutation_tupple();
					char * ba = board->get_board_area();
					for (int j = 0; j < 6; j++) {
						if ( (ba[p[j][0]] == list[player_turn]->ticker) &&
								(ba[p[j][(1)]] == list[player_turn]->ticker) ) {
							valid_move = board->validate_move(p[j][(2)]);
							if (valid_move == Board::NO_ERROR) {
								board->set_move(p[j][(2)]);
								return;
							}
						}
					}
				}

				/* diagonal back 0-->4-->8 */
				Permutation3 perm3_diag_b = Permutation3(diagonal_back);
				p = perm3_diag_b.get_permutation_tupple();
				char * ba = board->get_board_area();
				for (int j = 0; j < 6; j++) {
					if ( (ba[p[j][0]] == list[player_turn]->ticker) &&
							(ba[p[j][(1)]] == list[player_turn]->ticker)) {
						valid_move = board->validate_move(p[j][(2)]);
						if (valid_move == Board::NO_ERROR) {
							board->set_move(p[j][(2)]);
							return;
						}
					}
				}

				/* diagonal forward 2-->4-->6*/
				Permutation3 perm3_diag_for = Permutation3(diagonal_forward);
				p = perm3_diag_for.get_permutation_tupple();
				for (int j = 0; j < 6; j++) {
					if ( ( ba[p[j][0]] == list[player_turn]->ticker) &&
							( ba[p[j][(1)]] == list[player_turn]->ticker) ) {
						valid_move = board->validate_move(p[j][(2)]);
						if (valid_move == Board::NO_ERROR) {
							board->set_move(p[j][(2)]);
							return;
						}
					}
				}
			}

			/* for intermediate level, we just checked if we can defend or we have best
			 * chance to win by filling the only one remaining position.
			 * if we cant do either of that then lets just get a ramdom position for fill
			 */
			if (board->get_difficulty_level() == Board::INTERMEDIATE) {

				std::srand(time(0));
				int move;

				do {
					move = std::rand() % 9 ;
				} while (board->validate_move(move) != Board::NO_ERROR );

				board->set_move(move);

				return;
			}

			/* make preffered moves, this sets up advanced moves */

			for( int  i = preffered_move_loc ; i < BOARD_SIZE ; i++) {
				valid_move = board->validate_move( preffered_move[i]);
				if (valid_move == Board::NO_ERROR) {
					board->set_move(preffered_move[i]);
					preffered_move_loc = i++;
					return;
				}
			}
		} 
	} else {
		
		cout << "Board not set for player";
		exit(-1);
	}
}

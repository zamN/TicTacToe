#include <iostream>
#include <string.h>
#include <cerrno> // errno!
#include <sys/types.h> // for socket()
#include <sys/socket.h> // for socket()
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstdio> // for fgets
#include <cstdlib> // for system
#include <cctype>
#include "GameBoard.h"
#include "User.h"
#include "ProtocolHandler.h"

using namespace std;

int play(int, int, bool);
void displayPlayers();
int displayMenu(int*);

User* player;
User* player2;
GameBoard* gb;
ProtocolHandler* ph;
string username;
char symbol;

int main(void) {
	struct addrinfo hints, *res;
	struct hostent *he;
	string ip;
	int sockfd;
	int con = 0;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	system("clear");
	cout << "Welcome to TicTacToe Online Multiplayer!" << endl;
	cout << "Please enter a server ip (IPv4):" << endl;
	cin >> ip;
	he = gethostbyname(ip.c_str());
	while ((he = gethostbyname(ip.c_str())) == 0) {
		cout << "Not a valid IPv4 Address!" << endl;
		cout << "Please enter a server ip (IPv4): " << endl;
		cin >> ip;
	}
	getaddrinfo(ip.c_str(), "6000", &hints, &res);
	sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

	while ((con = connect(sockfd, res->ai_addr, res->ai_addrlen)) != 0)  {
		cout << "Error with server IP " << ip << ". Please try again: " << endl;	
		cin >> ip;
		getaddrinfo(ip.c_str(), "6000", &hints, &res);
		sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	}

	ph = new ProtocolHandler(sockfd);
	cout << "Successfully connected to server!" << endl;
	cout << "Please enter your username (Max 12 char): " << endl;
	cin >> username;
	username = username.substr(0, 13);
	cout << "Please enter your symbol: " << endl;
	string temp;
	cin >> temp;
	symbol = temp[0];

	int status = -1;
	int gid = 0;
	
	if (ph->sendInfo(username, symbol) == 2) {
		int myDecision = 0;
		bool replay = false;
		int oldStatus;
		// SO MANY WHILE LOOPS!!!
		while (1) {
			while (status == -1 ||  status == 3 || status == 4) {
				system("clear");
				player = new User(username, symbol);
				player2 = new User("????", '?');
				status = displayMenu(&gid);
			}
			switch (status) {
				case 1: 
				case 2: {
					gb = new GameBoard(player, player2);
					// if both people want to play again
					// then recreate the game.
					// if neither want to play again then
					// bring them back to the menu and
					// destroy game.
					// if 1 wants to play again and the other
					// doesnt then recreate the game with
					// the player being 'creator'
					oldStatus = status;
					myDecision = play(gid, status, replay);
					if (myDecision == 0 || myDecision == 1) {
						cout << "OK GAIZ SENDING MY SHIAT NAO" << endl;
						status = ph->sendReplay(myDecision);
						if (status == -1) status = 0;
						cout << "my decision" << myDecision << endl;
						cout << "status" << status << endl;	
						if (myDecision == 1) {
							if (status == 0)
								status = 1; // if user wants to play again and opponent doesn then recreate game.
							else
								status = oldStatus; // if opponent wants to play again do shit
						}
						// if user doesnt want to play again then go back to menu.
						else {
							status = -1;
						}
					}
					else if (myDecision == -1) { // if opponent ranodmly quits out!
						status = 1;
					}
					break;
				}
				case 5: {
					return 0;
					break;
				}
			}
		}
	}
	else {
		cout << "Failed to join game!" << endl;
	}

	return 0;
}

int displayMenu(int* gid) {
	int option = 0;
	int status = 0;
	cout << "Your username is set to: " << username << endl;
	cout << "Your symbol is set to: " << symbol << endl;
	cout << "Please select an option from the following menu: " << endl;
	cout << "1. Create a new game" << endl;
	cout << "2. Join a game" << endl;
	cout << "3. Change username" << endl;
	cout << "4. Change symbol" << endl;
	cout << "5. exit " << endl;
	cout << "\n=> ";
	cin >> option;

	switch (option) {
		case 1: {
			if (ph->create(gid) == 2) {
				gb = new GameBoard(player, player2);
				status = 1;
			}
			break;
		}
		case 2: {
			int result = 0;
			string input;
			while (true) {
				cout << "Please enter the Game ID (X to go back): ";
				cin >> input;

				if ((*gid = atoi(input.c_str())) != 0) {
					if ((*gid > 256) || (*gid <= 0)) {
						cout << "[ERROR] Reason: Invalid Range (1-256)!" << endl;
					}
					else {
						result = ph->joinGame(*gid);
						if (result == 1) {
							cout << "[ERROR] Reason: Invalid Game ID!" << endl;
						}
						else if (result == 3) {
							cout << "[ERROR] Reason: Same symbol as player in game. Change your symbol! Type X to go back and then hit 4." << endl;
						}
						else if (result == 4) { 
							cout << "[ERROR] Reason: Same username as player in game. Change your username! Type X to go back and then hit 3." << endl;
						}
						else if (result == 5) {
							cout << "[ERROR] Reason: Game Full." << endl;
						}
						else {
							status = 2;
							break;
						}
					}
				}
				else {
					if (tolower(input.c_str()[0]) == 'x') {
						status = -1;
						break;
					}
					else {
						cout << "[ERROR] Reason: Invalid Range (1-256)!" << endl;
					}
				}
			}
			break;
		}
		case 3: {
			cout << "Please enter your new username (max 12 chars): " << endl;
			cin >> username;
			ph->sendNick(username);
			status = 3;
			break;
		}
		case 4: {
			string temp;
			cout << "Please enter your new symbol: " << endl;
			cin >> temp;
			symbol = temp[0];
			ph->sendSymbol(symbol);
			status = 4;
			break;
		}
		case 5: { 
			cout << "Now closing the program." << endl;
			status = 5;
			break;
		}
	}
	return status;
}


/* 
 * This function is repsonsible for displaying the current game and sending moves back and forth.
 * Once the 2 players are finished playing then they are asked if they would like to play again.
 * RETURN VALUES:
 * 	0 == Return back to menu
 * 	1 == Play again!
 * 	-1 == opponent disconnected
*/

int play(int gid, int status, bool replay) {
	string winner = "";
	system("clear");
	switch (status) {
		case 1: {
			cout << "Game created! Game ID: " << gid << endl;
			cout << "Waiting on an opponent to join...\n";
			ph->getOpponent(player2);
			break;
		}
		case 2: {
			cout << "Successfully joined Game ID: " << gid <<  endl;
			if (!ph->getOpponent(player2)) {
				exit(1);
			}
			break;
		}
		case 7: {
			status = 2;
			break;
		}
	}
	system("clear");
	int move;
	int opmove;
	if (status == 2) {
		displayPlayers();
		gb->draw();
		cout << "Waiting on other players move..." << endl;
		opmove = ph->getMove();
		if (opmove != -1)
			gb->update(opmove, *player2);
		else {
			cout << player2->getNick() << " has quit the game!" << endl;
			cout << "Resetting game..." << endl;
			sleep(5);
			return -1;
		}
		system("clear");
	}

	bool tie = false;
	char dec;

	while (1) {
		displayPlayers();
		gb->draw();
		cout << "Please enter the spot number where you want to move: ";
		cin >> move;
		int result;
		while ((result = gb->update(move, *player)) != 1) {
			if (result == -1) {
				cout << "[ERROR] Spot already taken." << endl;
			}
			else if (result == 2) {
				tie = true;
				break;
			}
			cout << "Please enter the spot number where you want to move: ";
			cin >> move;
		}
		system("clear");
		displayPlayers();
		ph->sendMove(move);
		gb->draw();
		if (tie) {
			system("clear");
			cout << "Tie game! Good job derps." << endl;
			break;
		}
		if (gb->getSpots() <= 4) {
			winner = gb->checkWinner();
			if (strcmp(winner.c_str(), "") != 0) {
				gb->gameOver(winner);
				break;
			}
		}
		cout << "Waiting on other players move..." << endl;
		opmove = ph->getMove();
		if (opmove == -1) {
			cout << player2->getNick() << " has quit the game!" << endl;
			cout << "Resetting game..." << endl;
			sleep(5);
			return -1;
		}
		if (gb->update(opmove, *player2) == 2) {
			cout << "Tie game! Good job dar.." << endl;
			break;
		}
		system("clear");
		if (gb->getSpots() <= 4) {
			winner = gb->checkWinner();
			if (strcmp(winner.c_str(), "") != 0) {
				displayPlayers();
				gb->draw();
				gb->gameOver(winner);
				break;
			}
		}
	}
	cout << "Play again? (y/n)" << endl;
	cin >> dec;
	dec = tolower(dec);
	while (dec != 'n' && dec != 'y') {
		cout << "Invalid choice!" << endl;
		cout << "Play again? (y/n)" << endl;
		cin >> dec;
		dec = tolower(dec);
	}
	if (dec == 'y') {
		return 1;
	}
	return 0;
}

void displayPlayers() {
		cout << "You: " << player->getNick() << " - Symbol: " << player->getSymbol() << endl;
		cout << "Opponent: " << player2->getNick() << " - Symbol: " << player2->getSymbol() << endl;
}

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

void play(int, int);
int displayMenu(int*);

User* player;
User* player2;
GameBoard* gb;
ProtocolHandler* ph;

int main(void) {
	struct addrinfo hints, *res;
	struct hostent *he;
	string ip;
	string username;
	char symbol;
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
	cin >> symbol;

	int status = -1;
	int gid = 0;
	
	if (ph->sendInfo(username, symbol) == 2) {
		while (status == -1) {
			system("clear");
			player = new User(username, symbol);
			player2 = new User("????", '?');
			cout << "Your username is set to: " << username << endl;
			cout << "Your symbol is set to: " << symbol << endl;
			status = displayMenu(&gid);
			cout << "STATUS: " << status << endl;
		}
		switch (status) {
			case 1: 
			case 2: {
				gb = new GameBoard(player, player2);
				play(gid, status);
				break;
			}
			case 3: {
				return 0;
				break;
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
	cout << "Please select an option from the following menu: " << endl;
	cout << "1. Create a new game" << endl;
	cout << "2. Join a game" << endl;
	cout << "3. exit " << endl;
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
			cout << "Now closing the program." << endl;
			status = 3;
			break;
		}
	}
	return status;
}

void play(int gid, int status) {
	system("clear");
	switch (status) {
		case 1: {
			cout << "Game created! Game ID: " << gid << endl;
			cout << "Waiting on an opponent to join...\n";
		}
		case 2: {
			cout << "Successfully joined Game ID: " << gid <<  endl;
		}
	}
	int move;
	while (!gb->gameOver(player2->getNick())) {
		gb->draw();
		cout << "Please enter the spot number where you want to move: ";
		cin >> move;
	}
}

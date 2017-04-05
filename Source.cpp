/*

Práctica 3: Sokoban

Autores:
Facundo Schiavoni Barreyro
Maksym Kharuk

Curso:
2016/2017

Grupo:
E

*/

#include <iostream>
#include <Windows.h>
#include <string>
#include <fstream>
#include <conio.h>
#include <cctype>


using namespace std;

const int MAX = 50;
const int MAXH = 10;
const int MAXE = 100;

const string FREE = " ", WALL = "", PLAYER = "", BOX = "";
const string LEVELS_FILE_NAME = "levels.txt";

typedef enum tSquare { Free, Wall, DestinationF, DestinationB, DestinationP, Player, Box };
// DestinationF: posicion de destino libre.
// DestinationB: posicion de destino con caja.
// DestinationP: posicion de destino con jugador.
typedef enum tKey { Up, Down, Right, Left, Exit, None, Undo };

typedef tSquare tBoard[MAX][MAX];


typedef struct {
	tBoard board;
	int rows, columns, playerPosRow, playerPosColumn, nBoxes, nBoxesDest, movements;
	// nBoxes: cajas del tablero.
	// nBoxesDest: cajas ya colocadas en destino.
	bool isBlocked;
} tSokoban;


typedef struct{
	tSokoban tList[MAXH];
	int posAct, count;
} tHistory;

typedef struct {
	tSokoban sokoban;
	int level;
	string fileName;
	tHistory history;
} tGame;

typedef struct {
	int actualLevel, minMoves;
	string inputFile;
} tPartida;

typedef tPartida tSuccesses[MAXE];

typedef struct {
	string playerName;
	tSuccesses winnings;
	int solvedGames;
} tInfo;


int menu(); // Muestra el menu y pide al usuario una opcion para ejecutar.
void bgColor(int color); // Rutina para ajustar los colores de fondo y primer plano.
void drawSquare(tSquare square); // Muestra una casilla del tablero.
void draw(const tGame &game); // Muestra el tablero del juego, el nombre del fichero desde que se ha cargado, su nivel, y el numero de movimientos realizados.
void initialize(tGame &game); // Inicializa las variables a 0.
bool loadGame(tGame &game); // Pide al usuario el archivo y el nivel.
bool loadLevel(ifstream &in, tSokoban &sokoban, int level); // Carga el nivel.
int getLineLevel(string line); // Obtiene la linea en la que se encuentra el nivel en el archivo.
void loadLine(tSokoban &sokoban, string line); // Carga una linea del nivel.
tKey readKey(); // Identifica la tecla que se pulsa.
void makeMovement(tGame &game, tKey key); // Mueve al jugador.
bool isMovementPossible(tSokoban sokoban, tKey key); // Determina si es posible el movimiento.
void assignNextPositionSquare(tGame &game, tKey key, int playerPosRow, int playerPosColumn);
void assignPreviousPositionSquare(tGame &game, int playerPosRow, int playerPosColumn);
void moveBox(tGame &game, tKey key, int boxPosRow, int boxPosColumn); // Mueve la caja.
bool isBoxBlocked(const tBoard &board, int boxPosRow, int boxPosColumn); // Determina si la caja se ha quedado bloqueada en una esquina.
bool undoMove(tGame &game); // Deshacer movimiento.
void saveState(tGame &game); // Guarda el estado actual del juego (array circular).
void welcome(ofstream &data, tInfo &info); // Pregunta al usuario el nombre.
bool win(tGame &game); // Determina si el nivel se ha ganado.
void loadInfo(ofstream &data, tInfo &info, tGame &game); // Carga la informacion del jugador.
void printInfo(tGame &game, tInfo &info); // Ejecuta la opcion 2.
void playGame(tGame &game, tInfo &info, ofstream &data); // Ejecuta la opcion 1.

int menu() {

	int selectedOption;

	do
	{
		cout << "#######################################" << endl
			<< "# Selecciona una opcion:              #" << endl
			<< "#  2 - Mostrar partidas ganadas       #" << endl
			<< "#  1 - Jugar                          #" << endl
			<< "#  0 - Salir                          #" << endl
			<< "#######################################" << endl;

		cin >> selectedOption;

		if (!cin) {
			cin.clear();
			cin.ignore();
			cout << "Introduce una opcion valida." << endl;
			system("pause");
			system("cls");
		}
		else if (selectedOption < 0 || selectedOption > 2){
			cout << "Introduce una opcion valida." << endl;
			system("pause");
			system("cls");
		}


	} while (selectedOption < 0 || selectedOption > 2);

	return selectedOption;
}

void bgColor(int color) {

	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(handle, 15 | (color << 4));
}

void drawSquare(tSquare square){

	switch (square) {
	case Free:
		bgColor(2); // Azul.
		cout << "  ";
		break;
	case Wall:
		bgColor(1); // Verde.
		cout << "  ";
		break;
	case DestinationF:
		bgColor(4); // Rojo.
		cout << "..";
		break;
	case DestinationB:
		bgColor(7); // Gris oscuro.
		cout << "()";
		break;
	case DestinationP:
		bgColor(14); // Amarillo.
		cout << "00";
		break;
	case Player:
		bgColor(6); // Marron.
		cout << "00";
		break;
	case Box:
		bgColor(8); // Gris claro.
		cout << "()";
		break;
	default:
		break;
	}

	bgColor(0);
}

void draw(const tGame &game){

	system("cls");

	cout << "Level: " << game.level << endl;

	for (int i = 0; i < game.sokoban.rows; i++)
	{

		for (int j = 0; j < game.sokoban.columns; j++){
			drawSquare(game.sokoban.board[i][j]);
		}

		cout << endl;
	}
	cout << "\n> Number of movements: " << game.sokoban.movements << endl;
	cout << "> Boxes: " << game.sokoban.nBoxesDest << "/" << game.sokoban.nBoxes << endl;

	if (game.sokoban.isBlocked) {
		cout << "> Box is blocked!!!!" << endl;
	}
}

void initialize(tGame &game) {

	game.sokoban.movements = 0;
	game.sokoban.columns = 0;
	game.sokoban.rows = 0;
	game.sokoban.nBoxes = 0;
	game.sokoban.nBoxesDest = 0;
	game.sokoban.isBlocked = false;

	game.history.count = 0;
	game.history.posAct = 0;
}

bool loadGame(tGame &game) {

	cout << "> Nombre del archivo que contiene los niveles: ";
	cin >> game.fileName;

	if (!cin) {
		cin.clear();
		cin.ignore();
		cout << "Introduce una opcion valida." << endl;
		system("pause");
		system("cls");
	}
	

		cout << "> Nivel: ";
		cin >> game.level;

		if (!cin) {
			cin.clear();
			cin.ignore();
			cout << "Introduce una opcion valida." << endl;
			system("pause");
			system("cls");
		}
	

	return true;
}

bool loadLevel(ifstream &in, tSokoban &sokoban, int level) {

	string line;
	bool levelFounded = false, levelLoaded = false;

	while (getline(in, line) && !levelLoaded) {

		if (levelFounded) {

			loadLine(sokoban, line);
			sokoban.rows++;

			if (line.size() == 1) {
				levelLoaded = true;
			}
		}

		if (line.find("Level ") != string::npos) {

			if (getLineLevel(line) == level) {

				levelFounded = true;
			}
		}
	}

	in.close();

	return true;
}

int getLineLevel(string line) {

	return atoi(line.substr(6, line.length()).c_str());
}

void loadLine(tSokoban &sokoban, string line) {

	if (sokoban.columns < line.length()) {
		sokoban.columns = line.length();
	}

	for (int i = 0; i < line.length(); i++) {

		switch (line[i])
		{
		case '#':
			sokoban.board[sokoban.rows][i] = Wall;
			break;
		case ' ':
			sokoban.board[sokoban.rows][i] = Free;
			break;
		case '$':
			sokoban.board[sokoban.rows][i] = Box;
			sokoban.nBoxes++;
			break;
		case '.':
			sokoban.board[sokoban.rows][i] = DestinationF;
			break;
		case '@':
			sokoban.board[sokoban.rows][i] = Player;
			sokoban.playerPosRow = sokoban.rows;
			sokoban.playerPosColumn = i;
			break;
		case '*':
			sokoban.board[sokoban.rows][i] = DestinationB;
			sokoban.nBoxes++;
			sokoban.nBoxesDest++;
			break;
		case '+':
			sokoban.board[sokoban.rows][i] = DestinationP;
			sokoban.playerPosRow = sokoban.rows;
			sokoban.playerPosColumn = i;
			break;
		default:
			break;
		}
	}
}

tKey readKey() {

	cin.sync();

	int dir = _getch(); // dir: tipo int

	tKey key = None;

	if (dir == 0xe0) {

		dir = _getch();

		switch (dir)
		{
		case 72:
			key = Up;
			break;
		case 80:
			key = Down;
			break;
		case 77:
			key = Right;
			break;
		case 75:
			key = Left;
			break;
		}
	}
	else if (dir == 27) {
		key = Exit;
	}
	else if (dir == 100 || dir == 68) {
		key = Undo;
	}

	return key;
}

void makeMovement(tGame &game, tKey key) {

	if (isMovementPossible(game.sokoban, key)) {

		int playerNextPosRow = game.sokoban.playerPosRow;
		int playerNextPosColumn = game.sokoban.playerPosColumn;

		switch (key)
		{
		case Up:
			playerNextPosRow -= 1;
			break;
		case Down:
			playerNextPosRow += 1;
			break;
		case Right:
			playerNextPosColumn += 1;
			break;
		case Left:
			playerNextPosColumn -= 1;
			break;
		}

		assignPreviousPositionSquare(game, game.sokoban.playerPosRow, game.sokoban.playerPosColumn);
		assignNextPositionSquare(game, key, playerNextPosRow, playerNextPosColumn);

		game.sokoban.playerPosRow = playerNextPosRow;
		game.sokoban.playerPosColumn = playerNextPosColumn;

		game.sokoban.movements++;
		saveState(game);
	}
}

bool isMovementPossible(tSokoban sokoban, tKey key) {

	bool isPossible = false;

	int playerNextPosRow = sokoban.playerPosRow;
	int playerNextPosColumn = sokoban.playerPosColumn;
	int pushedPosRow = sokoban.playerPosRow; // Utilizamos la variale pushedPos para comprobar si es posible el empuje de una caja en caso de que la haya.
	int pushedPosColumn = sokoban.playerPosColumn;

	switch (key)
	{
	case Up:
		playerNextPosRow -= 1;
		pushedPosRow -= 2;
		break;
	case Down:
		playerNextPosRow += 1;
		pushedPosRow += 2;
		break;
	case Right:
		playerNextPosColumn += 1;
		pushedPosColumn += 2;
		break;
	case Left:
		playerNextPosColumn -= 1;
		pushedPosColumn -= 2;
		break;
	}

	// Comprobamos si hay caja en la dirección destino y es posible su empuje
	if (sokoban.board[playerNextPosRow][playerNextPosColumn] == Box ||
		sokoban.board[playerNextPosRow][playerNextPosColumn] == DestinationB) {

		isPossible = (sokoban.board[pushedPosRow][pushedPosColumn] != Wall) &&
			(sokoban.board[pushedPosRow][pushedPosColumn] != Box) &&
			(sokoban.board[pushedPosRow][pushedPosColumn] != DestinationB);
	}
	// En cas de que no haya caja, comprobamos que no haya muro
	else {

		isPossible = (sokoban.board[playerNextPosRow][playerNextPosColumn] != Wall);
	}


	return isPossible;
}

void assignNextPositionSquare(tGame &game, tKey key, int playerPosRow, int playerPosColumn) {

	switch (game.sokoban.board[playerPosRow][playerPosColumn])
	{
	case Free:
		game.sokoban.board[playerPosRow][playerPosColumn] = Player;
		break;
	case DestinationF:
		game.sokoban.board[playerPosRow][playerPosColumn] = DestinationP;
		break;
	case Box:
		game.sokoban.board[playerPosRow][playerPosColumn] = Player;
		moveBox(game, key, playerPosRow, playerPosColumn);
		break;
	case DestinationB:
		game.sokoban.board[playerPosRow][playerPosColumn] = DestinationP;
		moveBox(game, key, playerPosRow, playerPosColumn);
		break;
	}
}

void assignPreviousPositionSquare(tGame &game, int playerPosRow, int playerPosColumn) {

	switch (game.sokoban.board[playerPosRow][playerPosColumn])
	{
	case Player:
		game.sokoban.board[playerPosRow][playerPosColumn] = Free;
		break;
	case DestinationP:
		game.sokoban.board[playerPosRow][playerPosColumn] = DestinationF;
		break;
	}
}

void moveBox(tGame &game, tKey key, int boxPosRow, int boxPosColumn) {

	bool isInDestination = (game.sokoban.board[boxPosRow][boxPosColumn] == DestinationP);

	switch (key)
	{
	case Up:
		boxPosRow -= 1;
		break;
	case Down:
		boxPosRow += 1;
		break;
	case Right:
		boxPosColumn += 1;
		break;
	case Left:
		boxPosColumn -= 1;
		break;
	}

	switch (game.sokoban.board[boxPosRow][boxPosColumn])
	{
	case Free:
		game.sokoban.board[boxPosRow][boxPosColumn] = Box;
		if (isInDestination) { game.sokoban.nBoxesDest--; }
		break;
	case DestinationF:
		game.sokoban.board[boxPosRow][boxPosColumn] = DestinationB;
		if (!isInDestination) { game.sokoban.nBoxesDest++; }
		break;
	}

	game.sokoban.isBlocked = isBoxBlocked(game.sokoban.board, boxPosRow, boxPosColumn);
}

bool isBoxBlocked(const tBoard &board, int boxPosRow, int boxPosColumn){

	return board[boxPosRow][boxPosColumn + 1] == Wall && board[boxPosRow + 1][boxPosColumn] == Wall
		|| board[boxPosRow][boxPosColumn - 1] == Wall && board[boxPosRow + 1][boxPosColumn] == Wall
		|| board[boxPosRow][boxPosColumn - 1] == Wall && board[boxPosRow - 1][boxPosColumn] == Wall
		|| board[boxPosRow][boxPosColumn + 1] == Wall && board[boxPosRow - 1][boxPosColumn] == Wall;
}

void saveState(tGame &game) {

	if (game.history.posAct == 9) {
		game.history.posAct = 0;
	}
	else {
		game.history.posAct++;
	}

	game.history.tList[game.history.posAct] = game.sokoban;

	if (game.history.count > 0) {
		game.history.count--;
	}
}

bool undoMove(tGame &game) {

	bool canUndoMove = false;

	if (game.history.count < 9) {

		if (game.history.posAct == 0) {
			game.history.posAct = 9;
		}
		else {
			game.history.posAct--;
		}

		game.history.count++;

		game.sokoban = game.history.tList[game.history.posAct];
		canUndoMove = true;
	}

	return canUndoMove;
}

bool fileExist(const string name){

	ifstream file(name + ".txt");
	bool exist = false;

	if (file){
		exist = true;
	}
	return exist;
}

void welcome(ofstream &data, tInfo &info){

	ifstream player;
	bool e = true;

	cout << "Introduce tu nombre: ";
	cin >> info.playerName;
	cout << "Bienvenido " << info.playerName << endl;

	system("pause");

	if (!fileExist(info.playerName))
	{
		cout << "No se encuentra la informacion del jugador. Se creara nueva." << endl;
		system("pause");
	}

	system("cls");
	
}

bool win(tGame &game){

	return game.sokoban.nBoxes == game.sokoban.nBoxesDest;
}


void loadInfo(ofstream &data, tInfo &info, tGame &game){

	info.solvedGames++;
	
	info.winnings[info.solvedGames].inputFile = game.fileName;
	info.winnings[info.solvedGames].actualLevel = game.level;
	info.winnings[info.solvedGames].minMoves = game.sokoban.movements;

	
	data.open(info.playerName + ".txt", fstream::app); // append mode, para escribir al final del archivo.
	
	
	data << info.winnings[info.solvedGames].inputFile << " "
		<< info.winnings[info.solvedGames].actualLevel << " "
		<< info.winnings[info.solvedGames].minMoves << endl;
	
}

void printInfo(tGame &game, tInfo &info){

	ifstream file(info.playerName + ".txt");
	int cont = 0;
	cout << "     Tableros derrotados por " << info.playerName << endl;
	cout << "FICHERO" << "          " << "NIVEL" << "          " << "MOVIMIENTOS" << endl;
	if (file){

		while (!file.eof())
		{
			file >> info.winnings[cont].inputFile >>
				info.winnings[cont].actualLevel >>
				info.winnings[cont].minMoves;
			cont++;
		}

		for (int i = 0; i < cont - 1; i++){
			cout << info.winnings[i].inputFile << "          "
				<< info.winnings[i].actualLevel << "          "
				<< info.winnings[i].minMoves << endl;
		}
	}
	else {
		cout << "No hay informacion que mostrar." << endl;
	}
	
}

void playGame(tGame &game, tInfo &info, ofstream &data){

	initialize(game);
	loadGame(game);

	ifstream in(game.fileName);

	loadLevel(in, game.sokoban, game.level);
	draw(game);

	tKey key = None;

	while (key != Exit && game.sokoban.nBoxes != game.sokoban.nBoxesDest) {

		key = readKey();

		if (key == Undo) {
			undoMove(game);
		}
		else {
			makeMovement(game, key);
		}

		draw(game);
	}

	if (win(game)) {

		loadInfo(data, info, game);
		cout << ">>>>> WINNER <<<<<" << endl;
	}
	else {
		cout << ">>>>> LOSER <<<<<" << endl;
	}
}

int main(){
	tGame game;
	ofstream data;
	tInfo info;
	info.solvedGames = 0;

	welcome(data, info);

	int menuOption;

	do {

		menuOption = menu();

		switch (menuOption) {
		case 1:
			playGame(game, info, data);
			break;
		case 2:
			printInfo(game, info);
			break;
		}

		} while (menuOption != 0);

		system("pause");

		return 0;
}

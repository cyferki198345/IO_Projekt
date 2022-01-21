#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <algorithm>
#include <fstream>
#include <cstdio>
using namespace std;

#define POJEMNOSC 128 //liczba miejsc
#define LOZA 28 //liczba miejsc w lozach
#define CENALOZY 30 //koszt miejsca w lozy
#define DEFPASS "Cwi2Dyd@For"
#define PRZERWA 10
#define OTWARCIE 8	//godzina otwarcia kina - liczba dodatnia
#define ZAMKNIECIE 23	//godzina zamkniecia kina - 23 lub mniej
#define MAXDLUGFILM 480	//nie wiecej niz OTWARCIE x 60
#define SYSPLATPLIK "sysPlat.dat"
#define UDBPLIK "uzytkownicy.dat"
#define REPERPLIK "repertuar.dat"
#define DODPLIK "dodatki.dat"
#define ZNIZPLIK "znizki.dat"
#define BILPLIK "bilety.dat"
#define BILARCHPLIK "bilety-archiwum.dat"
#define BILCANPLIK "bilety-anulowane.dat"

struct dataPL {
	int dzien;
	int miesiac;
	int rok;
	bool operator==(dataPL);
};

ostream& operator<<(ostream& stm, const dataPL& d) {
	stm << d.dzien << "/" << d.miesiac << "/" << d.rok;
	return stm;
}

bool dataPL::operator==(dataPL d2) {
	if(dzien == d2.dzien && miesiac == d2.miesiac && rok == d2.rok) return true;
	else return false;
}

bool dataComp(dataPL wzor, dataPL aktualna, int ogr) {	//wzor - data urodzenia uzytkownika lub data seansu
	bool check = (aktualna.rok > wzor.rok + ogr);
	check = check || (aktualna.rok == wzor.rok + ogr && aktualna.miesiac > wzor.miesiac);
	check = check || (aktualna.rok == wzor.rok + ogr && aktualna.miesiac == wzor.miesiac && aktualna.dzien >= wzor.dzien);
	if(check) return true;	//sprawdzenie ograniczenia - mozna wyswietlic; sprawdzenie odbycia seansu - mozna skasowac
	return false;
}

dataPL aktualnaData() {
	//time_t now1 = time(0);
	//tm now2;
	//localtime_s(&now2, &now1);
	//return{ now2.tm_mday,now2.tm_mon + 1,now2.tm_year + 1900 };
	
	//===
	//powyzsza alternatywa dziala w Visual Studio, ponizsza w Dev-C++
	//===
	
	time_t now1 = time(0);
	tm* now2 = localtime(&now1);
	return {now2->tm_mday,now2->tm_mon+1,now2->tm_year+1900};
}

bool weryfikacjaDaty(dataPL data) {
	int maxDzien[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
	if( data.rok%4 == 0 ) maxDzien[1] = 29;
	if( data.miesiac < 1 || data.miesiac > 12 ) return false;
	if( data.dzien < 1 || data.dzien > maxDzien[data.miesiac - 1] ) return false;
	return true;
}

struct dataPL2 {
	int dzien;
	int miesiac;
	int rok;
	int godz;
	int min;
};

dataPL2 aktualnaData2() {
	//time_t now1 = time(0);
	//tm now2;
	//localtime_s(&now2, &now1);
	//return{ now2.tm_mday,now2.tm_mon + 1,now2.tm_year + 1900,now2.tm_hour,now2.tm_min };
	
	//===
	//powyzsza alternatywa dziala w Visual Studio, ponizsza w Dev-C++
	//===
	
	time_t now1 = time(0);
	tm* now2 = localtime(&now1);
	return {now2->tm_mday,now2->tm_mon+1,now2->tm_year+1900,now2->tm_hour,now2->tm_min};
}

bool dataComp2(dataPL2 wzor, dataPL2 akt) { //funkcja na potrzeby kasowania nieaktualnych seansow
	bool check = (akt.rok > wzor.rok);
	check = check || (akt.rok == wzor.rok && akt.miesiac > wzor.miesiac);
	check = check || (akt.rok == wzor.rok && akt.miesiac == wzor.miesiac && akt.dzien > wzor.dzien);
	check = check || (akt.rok == wzor.rok && akt.miesiac == wzor.miesiac && akt.dzien == wzor.dzien && akt.godz > wzor.godz);
	check = check || (akt.rok == wzor.rok && akt.miesiac == wzor.miesiac && akt.dzien == wzor.dzien && akt.godz == wzor.godz && akt.min >= wzor.min);
	if(check) return true;
	return false;
}





// ========================
// === SYSTEM PLATNOSCI ===
// ========================
// ma symulowac transakcje z banku - demonstracja akceptacji/odrzucania oraz anulowania transakcji

class SystemPlatnosci {
	unsigned long start;
	vector<unsigned long> transakcje;
public:
	SystemPlatnosci();
	~SystemPlatnosci();
	unsigned long autoryzacja(double);
	bool anulowanie(unsigned long);
};

SystemPlatnosci::SystemPlatnosci() {
	ifstream plik(SYSPLATPLIK);
	if(plik.good()) {
		plik >> start;
		unsigned long i;
		while(plik >> i) transakcje.push_back(i);
		plik.close();
	}
	else {
		start = 1000000;
	}
}

SystemPlatnosci::~SystemPlatnosci() {
	ofstream plik(SYSPLATPLIK);
	if(plik.good()) {
		plik << start << endl;
		for(unsigned int i=0; i<transakcje.size(); i++) plik << transakcje[i] << endl;
		plik.close();
	}
}

unsigned long SystemPlatnosci::autoryzacja(double cena) {
	if(rand()%3 == 0) return 0;
	start++;
	transakcje.push_back(start);
	return start;
}

bool SystemPlatnosci::anulowanie(unsigned long ID) {
	if( find(transakcje.begin(),transakcje.end(),ID) == transakcje.end() ) return false;
	if(rand()%3 == 0) return false;
	transakcje.erase( find(transakcje.begin(),transakcje.end(),ID) );
	return true;
}

// ====================
// === OBSLUGA KONT ===
// ====================

struct dOs {
	string imie;
	string nazwisko;
	dataPL dataUrodzenia;
};

ostream& operator<<(ostream& stm, const dOs& d) {
	stm << "Imie: " << d.imie << endl << "Nazwisko: " << d.nazwisko << endl << "Data urodzenia: " << d.dataUrodzenia << endl;
	return stm;
}

class Uzytkownik {
	friend class UserDB;
	string login;
	string haslo;
	dOs daneOsobowe;
	Uzytkownik* next;
public:
	Uzytkownik(string, string, string, string, dataPL);
};

Uzytkownik::Uzytkownik(string l, string h, string i, string n, dataPL dataUr) {
	login = l;
	haslo = h;
	daneOsobowe = {i,n,dataUr};
	next = NULL;
}

class UserDB {
	Uzytkownik* list;
	Uzytkownik* last;
public:
	UserDB();
	~UserDB();
	bool istnieje(string);
	void dodaj(string, string, string, string, dataPL);
	long logowanie(string,string);
	void modyfikacja(long, string, string, string, dataPL);
	dOs getDaneOs(long);
	dataPL getWiek(long);
};

UserDB::UserDB() {
	ifstream plik;
	plik.open(UDBPLIK);
	list = NULL;
	if(plik.is_open() && plik.good()) {
		string buf1;
		string buf2;
		string buf3;
		string buf4;
		string buf5;
		int ibuf1 = 0;
		int ibuf2 = 0;
		int ibuf3 = 0;
		bool ctrl = false;
		while(true) {
			if(getline(plik,buf1)) {
				getline(plik,buf2);
				getline(plik,buf3);
				getline(plik,buf4);
				getline(plik,buf5);
				ibuf1 = stoi(buf5);
				getline(plik,buf5);
				ibuf2 = stoi(buf5);
				getline(plik,buf5);
				ibuf3 = stoi(buf5);
				if(ctrl) {
					last->next = new Uzytkownik(buf1,buf2,buf3,buf4,{ibuf1,ibuf2,ibuf3});
					last = last->next;
				} else {
					list = new Uzytkownik(buf1,buf2,buf3,buf4,{ibuf1,ibuf2,ibuf3});
					last = list;
					ctrl = true;
				}
			} else break;
		}
	}
	if(list == NULL) {
		list = new Uzytkownik("admin",DEFPASS,"defIm","defNaz",{1,1,0});
		last = list;
	}
	plik.close();
}

UserDB::~UserDB() {
	Uzytkownik* aux = list;
	ofstream plik(UDBPLIK);
	if(plik.good()) {
		while(aux != NULL) {
			plik << aux->login << endl;
			plik << aux->haslo << endl;
			plik << aux->daneOsobowe.imie << endl;
			plik << aux->daneOsobowe.nazwisko << endl;
			plik << aux->daneOsobowe.dataUrodzenia.dzien << endl;
			plik << aux->daneOsobowe.dataUrodzenia.miesiac << endl;
			plik << aux->daneOsobowe.dataUrodzenia.rok << endl;
			aux = aux->next;
		}
		aux = list;
	}
	plik.close();
	while(list != last) {
		aux = list->next;
		delete list;
		list = aux;
	}
	delete list;
}

bool UserDB::istnieje(string l) {
	Uzytkownik* aux = list;
	while(aux!=NULL) {
		if(aux->login == l) return true; //odwrocone
		aux = aux->next;
	}
	return false; //odwrocone - zwracamay NotFound
}

void UserDB::dodaj(string l, string h, string i, string n, dataPL dataUr) {
	last->next = new Uzytkownik(l,h,i,n,dataUr);
	last = last->next;
}

long UserDB::logowanie(string l, string h) {
	Uzytkownik* aux = list;
	long counter = 0;
	while(aux != NULL) {
		if(aux->login == l) {
			if(aux->haslo == h) {
				return counter;
			} else {
				return -1;
			}
		} else {
			aux = aux->next;
			counter++;
		}
	}
	return -1;
}

void UserDB::modyfikacja(long u, string h, string i, string n, dataPL dataUr) {
	Uzytkownik* aux = list;
	while(u--) aux = aux->next;
	aux->haslo = h;
	aux->daneOsobowe.imie = i;
	aux->daneOsobowe.nazwisko = n;
	aux->daneOsobowe.dataUrodzenia = dataUr;
}

dataPL UserDB::getWiek(long u) {
	Uzytkownik* aux = list;
	while(u--) aux = aux->next;
	return aux->daneOsobowe.dataUrodzenia;
}

dOs UserDB::getDaneOs(long u) {
	Uzytkownik* aux = list;
	while(u--) aux = aux->next;
	return aux->daneOsobowe;
}



// ======================
// === OBSLUGA OFERTY ===
// ======================

struct daneSeansu {
	int godzRozp;
	int minRozp;
	dataPL dataSeansu;
	int wolneMiejsca;
	bool operator==(daneSeansu);
};

ostream& operator<<(ostream& stm, const daneSeansu& d) {
	stm << d.dataSeansu << ", " << d.godzRozp << ":" << d.minRozp << ", " << "Wolne miejsca: " << d.wolneMiejsca << endl;
	return stm;
}

bool daneSeansu::operator==(daneSeansu dS2) {
	if(godzRozp == dS2.godzRozp && minRozp == dS2.minRozp && dataSeansu == dS2.dataSeansu) return true;
	else return false;
}

class Seans {
	friend class Film;
	daneSeansu daneS;
	long bilety[POJEMNOSC];	//tablica identyfikatorow uzytkownikow z biletem na poszczegolne miejsca, -2 oznacza wolne miejsce
	Seans* next;
public:
	Seans(int,int,dataPL);
};

Seans::Seans(int g, int mi, dataPL dataS) {
	daneS.godzRozp = g;
	daneS.minRozp = mi;
	daneS.dataSeansu = dataS;
	daneS.wolneMiejsca = POJEMNOSC;
	for(int i=0; i<POJEMNOSC; i++) bilety[i] = -2;
	next = NULL;
}

// ^^^ SEANS
// vvv FILM

struct daneFilmu {
	string tytul;
	string gatunek;
	string opis;
	int ogrWiek;
	int czasTrwania;
	int cenaBiletu;
};

ostream& operator<<(ostream& stm, const daneFilmu& d) {
	if(d.tytul == "") {
		stm << "Blad: nie ma takiego filmu." << endl;
		return stm;
	}
	stm << "TYTUL: " << d.tytul << endl << "GATUNEK: " << d.gatunek << endl << "Ograniczenie wiekowe: " << d.ogrWiek << endl;
	stm << "Czax trwania: " << d.czasTrwania << " minut" << endl << "Cena biletu: " << d.cenaBiletu << " PLN" << endl << "OPIS" << endl << d.opis << endl<<endl;
	return stm;
}

class Film {
	friend class Repertuar;
	daneFilmu daneF;
	Seans* terminy;
	Film* next;
	void update();
	int dodajSeans(int,int,dataPL);
	int usunSeans(int);
	vector<daneSeansu> getSeanse();
	vector<bool> getWolneMiejsca(int);
	bool dodajBilet(long,daneSeansu,int);
	bool usunBilet(long,daneSeansu,int);
	void printSeanseToFile();
	void setSeansFromFile(daneSeansu, int, vector<long>);
public:
	Film(string,string,string,int,int,int);
	~Film();
};

Film::Film(string t, string g, string o, int oW, int cT, int cB) {
	daneF.tytul = t;
	daneF.gatunek = g;
	daneF.opis = o;
	//if(oW<15) oW = 0;
	daneF.ogrWiek = oW;
	daneF.czasTrwania = cT;
	daneF.cenaBiletu = cB;
	terminy = NULL;
	next = NULL;
}

Film::~Film() {
	Seans* auxS = terminy;
	while(terminy != NULL) {
		auxS = terminy->next;
		delete terminy;
		terminy = auxS;
	}
}

void Film::update() {
	Seans* auxS = terminy;
	dataPL2 akt = aktualnaData2();
	dataPL2 wzor;
	while(terminy != NULL) {
		wzor.dzien = terminy->daneS.dataSeansu.dzien;
		wzor.miesiac = terminy->daneS.dataSeansu.miesiac;
		wzor.rok = terminy->daneS.dataSeansu.rok;
		wzor.godz = terminy->daneS.godzRozp;
		wzor.min = terminy->daneS.minRozp;
		if(dataComp2(wzor, akt)) {
			terminy = terminy->next;
			delete auxS;
			auxS = terminy;
		} else break;
	}
}

int Film::dodajSeans(int g, int mi, dataPL dataS) {
	int I0,I1,I2;
	bool b1g,b2g,b3g,b4g,b5g,b1e,b2e,b3e,b4e,ctrl;
	I0 = g*60+mi;
	if(daneF.czasTrwania + I0 + PRZERWA >= 1440) return 5;
	//zakladamy, ze zaden film nie moze trwac do polnocy
	if(terminy != NULL) {
		Seans* aux1 = terminy;
		Seans* aux2 = terminy;
		
		while(aux1 != NULL) {	//znajdz seans pozniejszy (aux1) i wczesniejszy (aux2)
			b1g = dataS.rok > aux1->daneS.dataSeansu.rok;
			b2g = dataS.miesiac > aux1->daneS.dataSeansu.miesiac;
			b3g = dataS.dzien > aux1->daneS.dataSeansu.dzien;
			b4g = g > aux1->daneS.godzRozp;
			b5g = mi > aux1->daneS.minRozp;
			b1e = dataS.rok == aux1->daneS.dataSeansu.rok;
			b2e = dataS.miesiac == aux1->daneS.dataSeansu.miesiac;
			b3e = dataS.dzien == aux1->daneS.dataSeansu.dzien;
			b4e = g == aux1->daneS.godzRozp;
			while(true) {
				if(b1g) {
					ctrl = true;
					break;
				}
				if(b1e && b2g) {
					ctrl = true;
					break;
				}
				if(b1e && b2e && b3g) {
					ctrl = true;
					break;
				}
				if(b1e && b2e && b3e && b4g) {
					ctrl = true;
					break;
				}
				if(b1e && b2e && b4e && b4e && b5g) {
					ctrl = true;
					break;
				}
				ctrl = false;
				break;
			}
			if(ctrl) {
				aux2 = aux1;
				aux1 = aux1->next;
				continue;
			}
			break;
		}
		
		ctrl = b1e && b2e && b3e;
		if(aux1 != NULL) I1 = aux1->daneS.godzRozp*60 + aux1->daneS.minRozp;
		if(aux2 != NULL) I2 = aux2->daneS.godzRozp*60 + aux2->daneS.minRozp;
		if(aux1 == terminy) { //poczatek listy - dodaj na poczatku
			if( ctrl && (I1-I0 < daneF.czasTrwania+PRZERWA) ) {
				return 6;
			} else {
				terminy = new Seans(g,mi,dataS);
				terminy->next = aux1;
				return 0;
			}
		}
		if(aux1 == NULL) {	//koniec listy - dodaj na koncu
			if( ctrl && (I0-I2 < daneF.czasTrwania+PRZERWA) ) {
				return 7;
			} else {
				aux1 = new Seans(g,mi,dataS);
				aux2->next = aux1;
				return 0;
			}
		}
		if( ctrl && (I0-I2 < daneF.czasTrwania+PRZERWA || I1-I0 < daneF.czasTrwania+PRZERWA) ) {
			return 8;
		} else {
			Seans* aux3 = new Seans(g,mi,dataS);
			aux3->next = aux1;
			aux2->next = aux3;
			return 0;
		}
	} else {
		terminy = new Seans(g,mi,dataS);
		return 0;
	}
	return 9; //cos poszlo nie tak
}

int Film::usunSeans(int num) {
	if(num<0) return -2; //-2 - ujemny indeks seansu
	Seans* aux1 = terminy;
	Seans* aux2;
	while(num>0 && aux1 != NULL) {
		aux1 = aux1->next;
		num--;
	}
	if(aux1 != NULL) {
		if(aux1->daneS.wolneMiejsca != POJEMNOSC) return 1;
		if(aux1==terminy) {
			terminy = aux1->next;
			delete aux1;
			return 0;
		}
		aux2 = terminy;
		while(aux2->next != aux1) aux2 = aux2->next;
		aux2->next = aux1->next;
		delete aux1;
		return 0;
	}
	return -3; //-3 - indeks seansu przewyzsza rozmiar listy
}

vector<bool> Film::getWolneMiejsca(int num) {
	vector<bool> boolTab;
	if(num<0) return boolTab;
	Seans* aux1 = terminy;
	while(num>0 && aux1 != NULL) {
		aux1 = aux1->next;
		num--;
	}
	if(aux1 != NULL) {
		unsigned int i;
		boolTab.resize(POJEMNOSC);
		for(i=0; i<POJEMNOSC; i++) {
			if(aux1->bilety[i] == -2) boolTab[i] = true;
			else boolTab[i] = false;
		}
	}
	return boolTab;
}

vector<daneSeansu> Film::getSeanse() {
	vector<daneSeansu> listForUser;
	Seans* auxS = terminy;
	while(auxS != NULL) {
		listForUser.push_back(auxS->daneS);
		auxS = auxS->next;
	}
	return listForUser;
}

bool Film::dodajBilet(long user, daneSeansu dS, int miejsce) {
	Seans* aux = terminy;
	if(miejsce < 0 || miejsce >= POJEMNOSC) return false;
	while(aux!=NULL) {
		if(aux->daneS == dS) {
			if(aux->bilety[miejsce] == -2) {
				aux->bilety[miejsce] = user;
				aux->daneS.wolneMiejsca -= 1;
				return true;
			} else {
				return false;
			}
		} else {
			aux = aux->next;
		}
	}
	return false;
}

bool Film::usunBilet(long user, daneSeansu dS, int miejsce) {
	Seans* aux = terminy;
	if(miejsce < 0 || miejsce >= POJEMNOSC) return false;
	while(aux!=NULL) {
		if(aux->daneS == dS) {
			if(aux->bilety[miejsce] == user) {
				aux->bilety[miejsce] = -2;
				aux->daneS.wolneMiejsca += 1;
				return true;
			} else {
				return false;
			}
		} else {
			aux = aux->next;
		}
	}
	return false;
}

void Film::printSeanseToFile() {
	ofstream plik(REPERPLIK, ios::out | ios::app);
	if(plik.good()) {
		unsigned int i;
		Seans* aux = terminy;
		while(aux != NULL) {
			plik << "SEANS" << endl;
			plik << aux->daneS.godzRozp << endl;
			plik << aux->daneS.minRozp << endl;
			plik << aux->daneS.dataSeansu.dzien << endl;
			plik << aux->daneS.dataSeansu.miesiac << endl;
			plik << aux->daneS.dataSeansu.rok << endl;
			plik << aux->daneS.wolneMiejsca << endl;
			for(i=0; i<POJEMNOSC; i++) plik << aux->bilety[i] << endl;
			aux = aux->next;
		}
	}
	plik.close();
}

void Film::setSeansFromFile(daneSeansu dS, int m, vector<long> vL) {
	Seans* aux = terminy;
	while(aux!=NULL) {
		if(aux->daneS == dS) {
			aux->daneS.wolneMiejsca = m;
			for(unsigned int i=0; i<POJEMNOSC; i++) aux->bilety[i] = vL[i];
			break;
		} else {
			aux = aux->next;
		}
	}
}

//	^^^ FILM
//	vvv REPERTUAR

class Repertuar {
	UserDB* udbR;
	Film* filmList;
	vector<string> gatList;
public:
	Repertuar(UserDB*);
	~Repertuar();
	void update();
	bool dodajFilm(string,string,string,int,int,int);
	int dodajSeansDoFilmu(string,int,int,dataPL);
	int modFilm(string, string, int, int, int);
	int usunSeansZFilmu(string, int);
	int usunFilm(string);
	vector<daneSeansu> getSeanseZFilmu(string);
	vector<bool> getWolneMiejscaZFilmu(string,int);
	daneFilmu getFilm(long,string);
	vector<string> listaFilmow(long);
	vector<string> listaFilmowG(long, string);
	vector<string> listaGatunkow();
	bool dodajBiletDoFilmu(long, string, daneSeansu, int);
	bool usunBiletZFilmu(long, string, daneSeansu, int);
};

Repertuar::Repertuar(UserDB* udbptr) {
	filmList = NULL;
	udbR = udbptr;
	
	ifstream plik;
	plik.open(REPERPLIK);
	if(plik.is_open() && plik.good()) {
		string buf1;
		string buf2;
		string buf3;
		string buf4;
		int ibuf1;
		int ibuf2;
		int ibuf3;
		int ibuf4;
		int ibuf5;
		int ibuf6;
		vector<long> lVbuf;
		bool ctrl = true;
		Film* filmAux = filmList;
		unsigned int i;
		while(true) {
			lVbuf.clear();
			if(ctrl) {
				getline(plik,buf1);
				ctrl = false;
			} else {
				buf1 = buf4;
			}
			if(buf1 == "") break;
			else {
				getline(plik,buf2);	//gatunek
				buf3 = "";
				while(true) {
					getline(plik,buf4);
					if(buf4[0] == 126 || buf4[1] == 126) break;
					buf3 += buf4;
					buf3 += "\n";
				}	//opis
				getline(plik,buf4);
				ibuf1 = stoi(buf4);	//ogrWiek
				getline(plik,buf4);
				ibuf2 = stoi(buf4);	//czasTrwania
				getline(plik,buf4);
				ibuf3 = stoi(buf4);	//cenaBiletu
				if(filmList == NULL) {
					filmList = new Film(buf1,buf2,buf3,ibuf1,ibuf2,ibuf3);
					filmAux = filmList;
				} else {
					filmAux->next = new Film(buf1,buf2,buf3,ibuf1,ibuf2,ibuf3);
					filmAux = filmAux->next;
				}
				if( find(gatList.begin(), gatList.end(), buf2) == gatList.end() ) gatList.push_back(buf2);
				getline(plik,buf4);
				while(true) {
					if(buf4 == "SEANS") {
						getline(plik,buf2);
						ibuf1 = stoi(buf2);	//godzRozp
						getline(plik,buf2);
						ibuf2 = stoi(buf2);	//minRozp
						getline(plik,buf2);
						ibuf3 = stoi(buf2);	//data.dzien
						getline(plik,buf2);
						ibuf4 = stoi(buf2);	//data.miesiac
						getline(plik,buf2);
						ibuf5 = stoi(buf2);	//data.rok
						filmAux->dodajSeans(ibuf1,ibuf2,{ibuf3,ibuf4,ibuf5});
						getline(plik,buf2);
						ibuf6 = stoi(buf2);
						for(i=0; i<POJEMNOSC; i++) {
							getline(plik,buf2);
							lVbuf.push_back(stol(buf2));
						}
						filmAux->setSeansFromFile({ibuf1,ibuf2,{ibuf3,ibuf4,ibuf5}}, ibuf6, lVbuf);
						lVbuf.clear();
						getline(plik,buf4);
					} else {
						break;
					}
				}
				//dodList.push_back({buf1,dbuf1,dbuf2});
			}
		}
	}
	plik.close();
	this->update();
}

Repertuar::~Repertuar() {
	this->update();
	ofstream plik;
	plik.open(REPERPLIK);
	if(plik.good()) {
		Film* filmAux = filmList;
		while(filmAux != NULL) {
			plik << filmAux->daneF.tytul << endl;
			plik << filmAux->daneF.gatunek << endl;
			plik << filmAux->daneF.opis;	//jesli to sie wysypuje, trzeba zastapic ponizszym
			//plik << filmAux->daneF.opis << endl;
			plik << "~" << endl;
			plik << filmAux->daneF.ogrWiek << endl;
			plik << filmAux->daneF.czasTrwania << endl;
			plik << filmAux->daneF.cenaBiletu << endl;
			if(filmAux->terminy != NULL) {
				plik.close();
				filmAux->printSeanseToFile();
				plik.open(REPERPLIK, ios::out | ios::app);
			}
			filmAux = filmAux->next;
		}
	} else {
		Film* filmAux;
		while(filmList != NULL) {
			filmAux = filmList->next;
			delete filmList;
			filmList = filmAux;
		}
		gatList.clear();
	}
	plik.close();
}

void Repertuar::update() {
	Film* filmAux = filmList;
	while(filmAux != NULL) {
		filmAux->update();
		filmAux = filmAux->next;
	}
}

bool Repertuar::dodajFilm(string s1, string s2, string s3, int i1, int i2, int i3) {
	this->update();
	if(s1.length() == 0 || s2.length() == 0 || i1 < 0 || i2 < 0 || i2 > MAXDLUGFILM || i3 < 0) {
		return false;
	}
	if(filmList) {
		Film* filmAux = filmList;
		while(filmAux != NULL) {
			if(filmAux->daneF.tytul == s1) return false;
			filmAux = filmAux->next;
		}
		filmAux = filmList;
		while(filmAux->next != NULL) filmAux = filmAux->next;
		filmAux->next = new Film(s1,s2,s3,i1,i2,i3);
	} else {
		filmList = new Film(s1,s2,s3,i1,i2,i3);
	}
	if( find(gatList.begin(), gatList.end(), s2) == gatList.end() ) gatList.push_back(s2);
	return true;
}

int Repertuar::dodajSeansDoFilmu(string naz, int g, int mi, dataPL dataS) {
	this->update();
	if(g < OTWARCIE || g > ZAMKNIECIE) return 3;	//kod bledu 3 - godzina poza otwarciem kina
	if(mi < 0 || mi > 59) return 4;	//kod bledu 4 - zle podana minuta
	if(filmList == NULL) return 1;	//kod bledu 1 - pusta lista filmow
	Film* filmAux = filmList;
	while(filmAux->daneF.tytul != naz && filmAux != NULL) filmAux = filmAux->next;
	if(filmAux == NULL) return 2;	//kod bledu 2 - nie znaleziono filmu
	return filmAux->dodajSeans(g,mi,dataS);	//kod bledu 5-8 jezeli filmy sie nakladaja, 0 jezeli dodane poprawnie
}

int Repertuar::modFilm(string naz, string s3, int i1, int i2, int i3) {
	this->update();
	if(i1 < 0 || i2 < 0 || i2 > MAXDLUGFILM || i3 < 0) return 1;
	Film* filmAux = filmList;
	while(filmAux->daneF.tytul != naz && filmAux != NULL) filmAux = filmAux->next;
	if(filmAux == NULL) return -1;	//kod bledu -1 - nie znaleziono filmu
	filmAux->daneF.opis = s3;
	filmAux->daneF.ogrWiek = i1;
	filmAux->daneF.czasTrwania = i2;
	filmAux->daneF.cenaBiletu = i3;
	return 0;
}

int Repertuar::usunSeansZFilmu(string naz, int num) {
	this->update();
	Film* filmAux = filmList;
	while(filmAux != NULL) {
		if(filmAux->daneF.tytul == naz) break;
		filmAux = filmAux->next;
	}
	if(filmAux!=NULL) return filmAux->usunSeans(num);	//1 dla seansu z biletami, 0 dla usunietego seansu, <=-2 dla innych bledow
	return -1;	//-1 dla nieznalezionego filmu
}

int Repertuar::usunFilm(string naz) {
	this->update();
	Film* filmAux = filmList;
	while(filmAux != NULL) {
		if(filmAux->daneF.tytul == naz) break;
		filmAux = filmAux->next;
	}
	if(filmAux!=NULL) {
		if(filmAux->terminy == NULL) {
			string gat = filmAux->daneF.gatunek;
			if(filmAux == filmList) {
				filmList = filmList->next;
				delete filmAux;
			} else {
				Film* filmAux2 = filmList;
				while(filmAux2->next != filmAux) filmAux2 = filmAux2->next;
				filmAux2->next = filmAux->next;
				delete filmAux;
			}
			bool gatDel = true;
			filmAux = filmList;
			while(filmAux != NULL) {
				if(filmAux->daneF.gatunek == gat) {
					gatDel = false;
					break;
				}
				filmAux = filmAux->next;
			}
			if(gatDel) {
				gatList.erase(find(gatList.begin(),gatList.end(),gat));
			}
			return 0;
		} else {
			return 1; //1 dla filmu z seansami
		}
	}
	return -1; //-1 dla nieznalezionego filmu
}

vector<daneSeansu> Repertuar::getSeanseZFilmu(string naz) {
	this->update();
	vector<daneSeansu> listForUser;
	Film* filmAux = filmList;
	while(filmAux != NULL) {
		if(filmAux->daneF.tytul == naz) break;
		filmAux = filmAux->next;
	}
	if(filmAux!=NULL) listForUser = filmAux->getSeanse();
	return listForUser;
}

vector<bool> Repertuar::getWolneMiejscaZFilmu(string naz, int num) {
	this->update();
	Film* filmAux = filmList;
	while(filmAux != NULL) {
		if(filmAux->daneF.tytul == naz) break;
		filmAux = filmAux->next;
	}
	if(filmAux!=NULL) return filmAux->getWolneMiejsca(num);
	vector<bool> placeholder;
	return placeholder;
}

daneFilmu Repertuar::getFilm(long user, string s) {
	this->update();
	dataPL dAkt = aktualnaData();
	dataPL dUr;
	if(user<0) {
		dUr = dAkt;
	}
	else {
		dUr = udbR->getWiek(user);
	}
	Film* filmAux = filmList;
	while(filmAux != NULL) {
		if(filmAux->daneF.tytul == s) break;
		filmAux = filmAux->next;
	}
	if(filmAux == NULL)	return {"","","",0,0,0};
	if(dataComp(dUr,dAkt,filmAux->daneF.ogrWiek)) return filmAux->daneF;
	return {"","","",0,0,0};
}

vector<string> Repertuar::listaFilmow(long user) {
	this->update();
	dataPL dAkt = aktualnaData();
	dataPL dUr;
	if(user<0) {
		dUr = dAkt;
	}
	else {
		dUr = udbR->getWiek(user);
	}
	Film* filmAux = filmList;
	vector<string> listForUser;
	while(filmAux != NULL) {
		if(dataComp(dUr,dAkt,filmAux->daneF.ogrWiek)) listForUser.push_back(filmAux->daneF.tytul);
		filmAux = filmAux->next;
	}
	return listForUser;
}

vector<string> Repertuar::listaFilmowG(long user, string gat) {
	this->update();
	dataPL dAkt = aktualnaData();
	dataPL dUr;
	if(user<0) {
		dUr = dAkt;
	}
	else {
		dUr = udbR->getWiek(user);
	}
	Film* filmAux = filmList;
	vector<string> listForUser;
	while(filmAux != NULL) {
		if(dataComp(dUr,dAkt,filmAux->daneF.ogrWiek) && filmAux->daneF.gatunek == gat) listForUser.push_back(filmAux->daneF.tytul);
		filmAux = filmAux->next;
	}
	return listForUser;
}

vector<string> Repertuar::listaGatunkow() {
	this->update();
	return gatList;
}

bool Repertuar::dodajBiletDoFilmu(long user, string naz, daneSeansu dS, int miejsce) {
	this->update();
	Film* filmAux = filmList;
	while(filmAux != NULL) {
		if(filmAux->daneF.tytul == naz) break;
		filmAux = filmAux->next;
	}
	if(filmAux!=NULL) return filmAux->dodajBilet(user, dS, miejsce);
	return false;
}

bool Repertuar::usunBiletZFilmu(long user, string naz, daneSeansu dS, int miejsce) {
	this->update();
	Film* filmAux = filmList;
	while(filmAux != NULL) {
		if(filmAux->daneF.tytul == naz) break;
		filmAux = filmAux->next;
	}
	if(filmAux!=NULL) return filmAux->usunBilet(user, dS, miejsce);
	return false;
}



// ======================
// === OBSLUGA SKLEPU ===
// ======================

struct dodatek {
	string opis;
	double skladnik1;
	double skladnik2;
};

struct znizka {
	string opis;
	double odjemnik1;
	double procent;
	double odjemnik2;
	int minWiek;
	int maxWiek;
};

struct bilet {
	long user;
	string nazwaFilmu;
	daneSeansu daneS;
	int miejsce;
	vector<dodatek> dodList;
	vector<znizka> znizList;
	double cena;
	unsigned long transakcjaID;
};

ostream& operator<<(ostream& stm, const dodatek& d) {
	stm << "Dodatek: " << d.opis << endl;
	stm << "Cena: ";
	if(d.skladnik1>0) stm << "+" << d.skladnik1 << "PLN przed uwzglednieniem promocji procentowej, ";
	if(d.skladnik2>0) stm << "+" << d.skladnik2 << "PLN po uwzglednieniu promocji procentowej.";
	stm << endl;
	return stm;
}

ostream& operator<<(ostream& stm, const znizka& z) {
	stm << "Znizka: " << z.opis << endl;
	stm << "Dozwolony wiek kupujacego: " << z.minWiek << "-" << z.maxWiek << endl;
	stm << "Promocja: ";
	if(z.odjemnik1>0) stm << "-" << z.odjemnik1 << "PLN przed uwzglednieniem promocji procentowej, ";
	if(z.procent>0) stm << "-" << z.procent << "% promocji procentowej, ";
	if(z.odjemnik2>0) stm << "-" << z.odjemnik2 << "PLN po uwzglednieniu promocji procentowej.";
	stm << endl;
	return stm;
}

ostream& operator<<(ostream& stm, const bilet& bil) {
	stm << "Uzytkownik: " << bil.user << endl;
	stm << "Nazwa filmu: " << bil.nazwaFilmu << endl;
	stm << "Termin seansu: " << bil.daneS.dataSeansu << ", " << bil.daneS.godzRozp << ":" << bil.daneS.minRozp << endl;
	stm << "Numer miejsca: " << bil.miejsce;
	if(bil.miejsce >= POJEMNOSC-LOZA) stm << " (loza)";
	unsigned int i;
	stm << endl << "Lista dodatkow: ";
	if(bil.dodList.size() == 0) stm << "brak" << endl;
	else {
		stm << endl;
		for(i=0; i<bil.dodList.size(); i++) stm << bil.dodList[i];
	}
	stm << "Lista znizek: ";
	if(bil.znizList.size() == 0) stm << "brak" << endl;
	else {
		stm << endl;
		for(i=0; i<bil.znizList.size(); i++) stm << bil.znizList[i];
	}
	stm << "Cena koncowa: " << bil.cena << "PLN" << endl;
	stm << "Numer transakcji: " << bil.transakcjaID;
	return stm;
}

class Sklep {
	SystemPlatnosci* sysPlat;
	Repertuar* reper;
	vector<dodatek> dodList;
	vector<znizka> znizList;
	vector<bilet> bilList;
	void archive();
public:
	Sklep(SystemPlatnosci*, Repertuar*);
	~Sklep();
	double getCena(string, int, vector<dodatek>, vector<znizka>);
	int kupBilet(long, string, daneSeansu, int, vector<dodatek>, vector<znizka>);
	vector<bilet> wyswietlBilety(long);
	int anulujBilet(long, string, int);
	bool dodajDodatek(string, double, double);
	bool dodajZnizke(string, double, double, double, int, int);
	bool usunDodatek(int);
	bool usunZnizke(int);
	vector<dodatek> getDodatki();
	vector<znizka> getZnizki();
};

Sklep::Sklep(SystemPlatnosci* ptrSP, Repertuar* ptrRP) {
	sysPlat = ptrSP;
	reper = ptrRP;
	
	ifstream plik;
	plik.open(DODPLIK);
	if(plik.is_open() && plik.good()) {
		string buf1;
		string buf2;
		double dbuf1 = 0;
		double dbuf2 = 0;
		while(true) {
			if(getline(plik,buf1)) {
				getline(plik,buf2);
				dbuf1 = stod(buf2);
				getline(plik,buf2);
				dbuf2 = stod(buf2);
				dodList.push_back({buf1,dbuf1,dbuf2});
			} else break;
		}
	}
	plik.close();
	
	plik.open(ZNIZPLIK);
	if(plik.is_open() && plik.good()) {
		string buf1;
		string buf2;
		double dbuf1 = 0.0;
		double dbuf2 = 0.0;
		double dbuf3 = 0.0;
		int ibuf1 = 0;
		int ibuf2 = 0;
		while(true) {
			if(getline(plik,buf1)) {
				getline(plik,buf2);
				dbuf1 = stod(buf2);
				getline(plik,buf2);
				dbuf2 = stod(buf2);
				getline(plik,buf2);
				dbuf3 = stod(buf2);
				getline(plik,buf2);
				ibuf1 = stoi(buf2);
				getline(plik,buf2);
				ibuf2 = stoi(buf2);
				znizList.push_back({buf1,dbuf1,dbuf2,dbuf3,ibuf1,ibuf2});
			} else break;
		}
	}
	plik.close();
	
	plik.open(BILPLIK);
	if(plik.is_open() && plik.good()) {
		string buf1;
		string buf2;
		string buf3;
		long lbuf = 0;
		int ibuf1 = 0;
		int ibuf2 = 0;
		int ibuf3 = 0;
		int ibuf4 = 0;
		int ibuf5 = 0;
		int ibuf6 = 0;
		double dbuf1 = 0.0;
		double dbuf2 = 0.0;
		double dbuf3 = 0.0;
		unsigned long ulbuf;
		daneSeansu dSbuf;
		vector<dodatek> vDbuf;
		vector<znizka> vZbuf;
		while(true) {
			vDbuf.clear();
			vZbuf.clear();
			if(getline(plik,buf1)) {
				lbuf = stol(buf1);	//user
				getline(plik,buf1);	//nazwaFilmu
				getline(plik,buf2);
				ibuf1 = stoi(buf2);	//daneS.godz
				getline(plik,buf2);
				ibuf2 = stoi(buf2);	//daneS.min
				getline(plik,buf2);
				ibuf3 = stoi(buf2);	//daneS.data.dzien
				getline(plik,buf2);
				ibuf4 = stoi(buf2);	//daneS.data.miesiac
				getline(plik,buf2);
				ibuf5 = stoi(buf2);	//daneS.data.rok
				getline(plik,buf2);
				ibuf6 = stoi(buf2);	//daneS.wolneMiejsca
				dSbuf = {ibuf1,ibuf2,{ibuf3,ibuf4,ibuf5},ibuf6};
				getline(plik,buf2);
				ibuf1 = stoi(buf2);	//miejsce
				getline(plik,buf2);
				while(true) {
					if(buf2 == "DODATEK") {
						getline(plik,buf2);
						getline(plik,buf3);
						dbuf1 = stoi(buf3);
						getline(plik,buf3);
						dbuf2 = stoi(buf3);
						vDbuf.push_back({buf2,dbuf1,dbuf2});
						getline(plik,buf2);
					} else break;
				}
				while(true) {
					if(buf2 == "ZNIZKA") {
						getline(plik,buf2);
						getline(plik,buf3);
						dbuf1 = stod(buf3);
						getline(plik,buf3);
						dbuf2 = stod(buf3);
						getline(plik,buf3);
						dbuf3 = stod(buf3);
						getline(plik,buf3);
						ibuf2 = stoi(buf3);
						getline(plik,buf3);
						ibuf3 = stoi(buf3);
						vZbuf.push_back({buf2,dbuf1,dbuf2,dbuf3,ibuf2,ibuf3});
						getline(plik,buf2);
					} else break;
				}
				dbuf1 = stod(buf2); //cena
				getline(plik,buf2);
				ulbuf = stoul(buf2); //transakcjaID
				bilList.push_back({lbuf,buf1,dSbuf,ibuf1,vDbuf,vZbuf,dbuf1,ulbuf});
			} else break;
		}
	}
	plik.close();
}

Sklep::~Sklep() {
	unsigned int i,j;
	ofstream plik;
	plik.open(DODPLIK);
	if(plik.good()) {
		for(i=0; i<dodList.size(); i++) {
			plik << dodList[i].opis << endl;
			plik << dodList[i].skladnik1 << endl;
			plik << dodList[i].skladnik2 << endl;
		}
	}
	plik.close();
	plik.open(ZNIZPLIK);
	if(plik.good()) {
		for(i=0; i<znizList.size(); i++) {
			plik << znizList[i].opis << endl;
			plik << znizList[i].odjemnik1 << endl;
			plik << znizList[i].procent << endl;
			plik << znizList[i].odjemnik2 << endl;
			plik << znizList[i].minWiek << endl;
			plik << znizList[i].maxWiek << endl;
		}
	}
	plik.close();
	plik.open(BILPLIK);
	if(plik.good()) {
		for(i=0; i<bilList.size(); i++) {
			plik << bilList[i].user << endl;
			plik << bilList[i].nazwaFilmu << endl;
			plik << bilList[i].daneS.godzRozp << endl;
			plik << bilList[i].daneS.minRozp << endl;
			plik << bilList[i].daneS.dataSeansu.dzien << endl;
			plik << bilList[i].daneS.dataSeansu.miesiac << endl;
			plik << bilList[i].daneS.dataSeansu.rok << endl;
			plik << bilList[i].daneS.wolneMiejsca << endl;
			plik << bilList[i].miejsce << endl;
			for(j=0; j<bilList[i].dodList.size(); j++) {
				plik << "DODATEK" << endl;
				plik << bilList[i].dodList[j].opis << endl;
				plik << bilList[i].dodList[j].skladnik1 << endl;
				plik << bilList[i].dodList[j].skladnik2 << endl;
			}
			for(j=0; j<bilList[i].znizList.size(); j++) {
				plik << "ZNIZKA" << endl;
				plik << bilList[i].znizList[j].opis << endl;
				plik << bilList[i].znizList[j].odjemnik1 << endl;
				plik << bilList[i].znizList[j].procent << endl;
				plik << bilList[i].znizList[j].odjemnik2 << endl;
				plik << bilList[i].znizList[j].minWiek << endl;
				plik << bilList[i].znizList[j].maxWiek << endl;
			}
			plik << bilList[i].cena << endl;
			plik << bilList[i].transakcjaID << endl;
		}
	}
	plik.close();
}

void Sklep::archive() {
	ofstream plik(BILARCHPLIK, ios::out | ios::app);
	if(plik.good()) {
		dataPL2 akt = aktualnaData2();
		dataPL2 wzor;
		for(unsigned int i=bilList.size()-1; i<bilList.size(); i--) {
			wzor.dzien = bilList[i].daneS.dataSeansu.dzien;
			wzor.miesiac = bilList[i].daneS.dataSeansu.miesiac;
			wzor.rok = bilList[i].daneS.dataSeansu.rok;
			wzor.godz = bilList[i].daneS.godzRozp;
			wzor.min = bilList[i].daneS.minRozp;
			if(dataComp2(wzor, akt)) {
				plik << bilList[i] << endl << endl;
				bilList.erase(bilList.begin() + i);
			}
		}
	}
	plik.close();
}

double Sklep::getCena(string nazwaFilmu, int miejsceR, vector<dodatek> dodListR, vector<znizka> znizListR) {
	this->archive();
	if(miejsceR < 0 || miejsceR >= POJEMNOSC) return -1.0; //zle wprowadzone dane
	daneFilmu dF = reper->getFilm(0, nazwaFilmu);
	double cena = dF.cenaBiletu;
	if(miejsceR >= POJEMNOSC-LOZA) cena += CENALOZY;
	unsigned int i;
	for(i=0; i<dodListR.size(); i++) cena += dodListR[i].skladnik1;
	for(i=0; i<znizListR.size(); i++) cena -= znizListR[i].odjemnik1;
	for(i=0; i<znizListR.size(); i++) cena *= (1.0 - znizListR[i].procent/100.0);
	for(i=0; i<dodListR.size(); i++) cena += dodListR[i].skladnik2;
	for(i=0; i<znizListR.size(); i++) cena -= znizListR[i].odjemnik2;
	return cena;
}

int Sklep::kupBilet(long user, string nazwaFilmu, daneSeansu daneSR, int miejsceR, vector<dodatek> dodListR, vector<znizka> znizListR) {
	this->archive();
	double cena = this->getCena(nazwaFilmu, miejsceR, dodListR, znizListR);
	if(cena < 0 || user < 0) return 1; //zle wprowadzone dane
	bool status = reper->dodajBiletDoFilmu(user, nazwaFilmu, daneSR, miejsceR);
	if(status) {
		unsigned long transID = sysPlat->autoryzacja(cena);
		if(transID == 0) {
			status = reper->usunBiletZFilmu(user, nazwaFilmu, daneSR, miejsceR);
			if(status) return -2; //odrzucenie transakcji, powrot do stanu wczesniejszego
			else return -1; //krytyczny blad po odrzuceniu transakcji, nie zwolnilo sie miejsce 
		}
		else {
			bilList.push_back({user, nazwaFilmu, daneSR, miejsceR, dodListR, znizListR, cena, transID});
			return 0;
		}
	}
	return 2; //miejsce nie jest wolne lub nie znaleziono filmu/seansu
}

vector<bilet> Sklep::wyswietlBilety(long user) {
	reper->update();
	this->archive();
	vector<bilet> retList;
	for(unsigned int i=0; i<bilList.size(); i++) if(bilList[i].user == user) retList.push_back(bilList[i]);
	return retList;
}

int Sklep::anulujBilet(long user, string nazwaFilmu, int indeks) { //nazwaFilmu jest tylko dodatkowym zabezpieczeniem
	reper->update();
	this->archive();
	unsigned int i;
	if(indeks<0) return -1;	//zly indeks
	for(i=0; i<bilList.size(); i++) {
		if(bilList[i].user == user) indeks--;
		if(indeks < 0) {
			if(nazwaFilmu != bilList[i].nazwaFilmu) return 1; //zla nazwa filmu
			else {
				bool status = sysPlat->anulowanie(bilList[i].transakcjaID);
				if(status) {
					status = reper->usunBiletZFilmu(user, nazwaFilmu, bilList[i].daneS, bilList[i].miejsce);
					ofstream plik(BILCANPLIK, ios::out | ios::app);
					if(plik.good()) {
						plik << bilList[i] << endl << endl;
					}
					plik.close();
					bilList.erase(bilList.begin() + i);
					if(status) {
						return 0;
					} else {
						return 3; //blad zwolnienia miejsca w seansie
					}
				} else {
					return 2; //anulowanie transakcji sie nie powiodlo
				}
			}
		}
	}
	return -1;	//zly indeks
}

bool Sklep::dodajDodatek(string str, double s1, double s2) {
	this->archive();
	if(s1 < 0 || s2 < 0 || (s1 == 0 && s2 == 0) || str.length() == 0) return false;
	dodList.push_back({str,s1,s2});
	return true;
}

bool Sklep::dodajZnizke(string str, double o1, double p, double o2, int min, int max) {
	this->archive();
	if(o1 < 0 || o2 < 0 || min<0 || max<0 || p < 0 || p > 100 || (o1 == 0 && o2 == 0 && p == 0) || str.length() == 0) return false;
	znizList.push_back({str, o1, p, o2, min, max});
	return true;
}

bool Sklep::usunDodatek(int i) {
	this->archive();
	if(i < 0 || i >= dodList.size()) return false;
	dodList.erase(dodList.begin() + i);
	return true;
}

bool Sklep::usunZnizke(int i) {
	this->archive();
	if(i < 0 || i >= znizList.size()) return false;
	znizList.erase(znizList.begin() + i);
	return true;
}

vector<dodatek> Sklep::getDodatki() {
	this->archive();
	return dodList;
}

vector<znizka> Sklep::getZnizki() {
	this->archive();
	return znizList;
}





// =================
// === INTERFEJS ===
// =================



class Interfejs {
	string bufor1;
	string bufor2;
	string bufor3;
	string bufor4;
	int ibufor1;
	int ibufor2;
	int ibufor3;
	double dbufor1;
	double dbufor2;
	double dbufor3;
	dataPL databufor;
	string nazwaFilmu;
	vector<bool> boolList;
	vector<string> dispFilmList;
	vector<daneSeansu> dispSeansList;
	vector<dodatek> dispDodList;
	vector<znizka> dispZnizList;
	vector<bilet> dispBilList;
	SystemPlatnosci* ptrSysPlat;
	UserDB* ptrUserDB;
	Repertuar* ptrReper;
	Sklep* ptrSklep;
	long user;
public:
	Interfejs(SystemPlatnosci*);
	~Interfejs();
	bool intro();					//tu sie zaczyna i zapetla program
private:
	void fillUserBuffers();
	void rejestracja();
	void login();
	void zarzadzanieKontem();
	bool fillVecListFilm();
	void przegladReper(bool);
	void printMiejsca();
	void sklepKup();
	void sklepDispBil();
	void sklepDispDod();
	void sklepDispZniz();
	void sklepAnuluj();
	void adminRFilmAdd();
	void adminRFilmMod();
	void adminRFilmDel();
	void adminRSeansAdd();
	void adminRSeansDel();
	void adminSDodAdd();
	void adminSDodDel();
	void adminSZnizAdd();
	void adminSZnizDel();
};

Interfejs::Interfejs(SystemPlatnosci* ptrSP) {
	ptrUserDB = new UserDB();
	ptrReper = new Repertuar(ptrUserDB);
	ptrSysPlat = ptrSP;
	ptrSklep = new Sklep(ptrSysPlat, ptrReper);
	user = -1;
}

Interfejs::~Interfejs() {
	delete ptrUserDB;
	delete ptrReper;
	delete ptrSklep;
}

void Interfejs::printMiejsca() {
	unsigned int i=0;
	cout << "Miejsca zwykle: " << endl;
	while(i<10 && i<(POJEMNOSC-LOZA)) {
		if(boolList[i]) cout << "0" << i << " ";
		else cout << "xx ";
		i++;
	}
	while(i<100 && i<(POJEMNOSC-LOZA)) {
		if( i%10 == 0 ) cout << endl;
		if(boolList[i]) cout << i << " ";
		else cout << "xx ";
		i++;
	}
	cout << endl << "Loza: " << endl;
	while(i<POJEMNOSC) {
		if(boolList[i]) cout << i << " ";
		else cout << "xxx ";
		i++;
		if( (i-100)%7 == 0 ) cout << endl;
	}
	cout << endl;
}

bool Interfejs::intro() {
	cout << endl;
	if(user == 0) {
		cout << "R. Modyfikacja repertuaru\tD. Modyfikacja dodatkow/znizek" << endl;
	}
	cout << "1. Oferta              2. Sklep        3. Logowanie" << endl << "4. Zarzadzanie kontem  5. Rejestracja  x. Wyjscie" << endl << "Podaj opcje: ";
	cin >> bufor1;
	
	if(bufor1[0] == 82 && user == 0) {
		cout << endl << "1. Dodaj film  2. Dodaj seans  3. Zmodyfikuj film  4. Usun seans  5. Usun film  x. Wroc do menu glownego: ";
		cin >> bufor1;
		if(bufor1[0] == 49) this->adminRFilmAdd();
		else if(bufor1[0] == 50) this->adminRSeansAdd();
		else if(bufor1[0] == 51) this->adminRFilmMod();
		else if(bufor1[0] == 52) this->adminRSeansDel();
		else if(bufor1[0] == 53) this->adminRFilmDel();
		return true;
	}
	if(bufor1[0] == 68 && user == 0) {
		cout << endl << "1. Dodaj dodatek  2. Dodaj znizke  3. Usun dodatek  4. Usun znizke  x. Wroc do menu glownego: ";
		cin >> bufor1;
		if(bufor1[0] == 49) this->adminSDodAdd();
		else if(bufor1[0] == 50) this->adminSZnizAdd();
		else if(bufor1[0] == 51) this->adminSDodDel();
		else if(bufor1[0] == 52) this->adminSZnizDel();
		return true;
	}
	bufor1[0] -= 49;
	if(bufor1[0] < 0 || bufor1[0] > 4) return false;
	if(bufor1[0] == 0) przegladReper(false);
	else if(bufor1[0] == 1) {
		if(user < 0) cout << "Nie jestes zalogowany." << endl;
		else {
			cout << endl << "1. Kup bilet       2. Wyswietl bilety 3. Wyswietl dodatki" << endl;
			cout << "4. Wyswietl znizki 5. Anuluj bilet    x. wyjscie" << endl << "Podaj opcje: ";
			cin >> bufor3;
			if(bufor3[0] == 49) this->przegladReper(true);
			else if(bufor3[0] == 50) this->sklepDispBil();
			else if(bufor3[0] == 51) this->sklepDispDod();
			else if(bufor3[0] == 52) this->sklepDispZniz();
			else if(bufor3[0] == 53) this->sklepAnuluj();
		}
	}
	else if(bufor1[0] == 2) login();
	else if(bufor1[0] == 3) zarzadzanieKontem();
	else if(bufor1[0] == 4) rejestracja();
	return true;
}

void Interfejs::fillUserBuffers() {
	bool small, large, digit, spec;
	unsigned int len;
	int status = 0;
	while(status == 0) {
		cout << "Podaj haslo (musi zawierac 1 mala litere, 1 duza litere, 1 cyfre i 1 znak specjalny): ";
		cin >> bufor2;
		small = large = digit = spec = false;
		len = bufor2.length();
		for(unsigned int i=0; i<len; i++) {
			if(bufor2[i]>=32 && bufor2[i]<=126) {
				if(bufor2[i]>=48 && bufor2[i]<=57) {
					digit = true;
				} else if(bufor2[i]>=65 && bufor2[i]<=90) {
					large = true;
				} else if(bufor2[i]>=97 && bufor2[i]<=122) {
					small = true;
				} else {
					spec = true;
				}
				if(small && large && digit && spec) {
					status = 1;
					break;
				}
			}
		}
	}
	cout << "Podaj imie: ";
	cin >> bufor3;
	cout << "Podaj nazwisko: ";
	cin >> bufor4;
	ibufor1 = 1;
	while(true) {
		cout << "Podaj rok urodzenia: ";
		cin >> databufor.rok;
		cout << "Podaj miesiac urodzenia: ";
		cin >> databufor.miesiac;
		cout << "Podaj dzien urodzenia: ";
		cin >> databufor.dzien;
		if(weryfikacjaDaty(databufor) == false) {
			cout << "Niepoprawna data urodzenia." << endl;
		} else {
			break;
		}
	}
}

bool Interfejs::fillVecListFilm() {			//uzyte bufory: bufor2, ibufor2
	unsigned int i;
	cout << endl << "1. Wyswietl pelna liste filmow 2. Wyswietl liste gatunkow x. Wroc do menu glownego: ";
	cin >> bufor2;
	if(bufor2[0] == 49) {
		dispFilmList = ptrReper->listaFilmow(user);
		if(dispFilmList.size() == 0) {
			cout << "Repertuar jest pusty." << endl;
			return false;
		}
		for(i=0; i<dispFilmList.size(); i++) cout << i+1 << ". " << dispFilmList[i] << endl;
		return true;
	} else if(bufor2[0] == 50) {
		dispFilmList = ptrReper->listaGatunkow();
		if(dispFilmList.size() == 0) {
			cout << "Repertuar jest pusty." << endl;
			return false;
		}
		for(i=0; i<dispFilmList.size(); i++) cout << i+1 << ". " << dispFilmList[i] << endl;
		cout << endl << "Wybierz numer gatunku (0 lub liczba ujemna by wrocic) by wyswietlic wszystkie filmy z gatunku: ";
		cin >> ibufor2;
		if(ibufor2 > 0 && ibufor2 <= dispFilmList.size()) {
			dispFilmList = ptrReper->listaFilmowG(user,dispFilmList[ibufor2-1]);
			if(dispFilmList.size() == 0) {
				cout << "Gatunek nie ma zadnych filmow lub wszystkie filmy maja ograniczenia wiekowe." << endl;
				return false;
			}
			for(i=0; i<dispFilmList.size(); i++) cout << i+1 << ". " << dispFilmList[i] << endl;
			return true;
		}
	}
	return false;
}

void Interfejs::przegladReper(bool b) {
	if( this->fillVecListFilm() ) {
		cout << endl << "Wybierz numer filmu (0 lub liczba ujemna by wrocic) by wyswietlic szczegolowy opis: ";
		cin >> ibufor1;
		if(ibufor1 > 0 && ibufor1 <= dispFilmList.size()) {
			nazwaFilmu = dispFilmList[ibufor1-1];
			cout << endl << ptrReper->getFilm(user,nazwaFilmu);
			dispSeansList = ptrReper->getSeanseZFilmu(nazwaFilmu);
			cout << "Terminy seansow:" << endl;
			for(unsigned int i=0; i<dispSeansList.size(); i++) cout << i+1 << ". " << dispSeansList[i];
			if(dispSeansList.size() == 0) {
				cout << "brak terminow" << endl;
			}
			else if(user > -1 && b) {
				cout << "Czy chcesz kupic bilet na seans? Wpisz numer seansu by kupic bilet na niego, inny znak by wrocic: ";
				cin >> ibufor3;
				if(ibufor3 > 0 && ibufor3 <= dispSeansList.size() && dispSeansList[ibufor3-1].wolneMiejsca <= 0) {
					cout << "Podany seans nie ma juz wolnych miejsc." << endl;
				}
				if(ibufor3 > 0 && ibufor3 <= dispSeansList.size()) {
					this->sklepKup();
				}
			}
		}
	}
}

void Interfejs::sklepKup() {
	//wypelnione: nazwaFilmu, dispSeansList, ibufor3(numer seansu z listy)
	unsigned int i;
	boolList = ptrReper->getWolneMiejscaZFilmu(nazwaFilmu, ibufor3-1);
	if(boolList.size() != POJEMNOSC) {
		cout << "Blad podczas sciagania listy wolnych miejsc." << endl;
		return;
	}
	cout << endl << "UWAGA: prosze sie upewnic, ze osoba dla ktorej kupuja Panstwo bilety jest w odpowiednim przedziale wiekowym - za rowno ze wzgledu na film jak i dodatki/znizki do biletu." << endl;
	cout << "Organizacja nie ponosi odpowiedzialnosci za zle dokonany zakup - jezeli osoba posiadajaca bilet przy kasie ma nieodpowiednie dane osobowe, bilet jest uznany za niewazny." << endl;
	this->printMiejsca();
	while(true) {
		cout << "Prosze wybrac wolne miejsce: ";
		cin >> ibufor2;
		if(ibufor2 >= 0 && ibufor2<POJEMNOSC) {
			if(boolList[ibufor2]) {
				break;
			} else {
				cout << "Miejsce jest juz zajete.";
			}
		} else {
			cout << "Miejsce poza zakresem. ";
		}
	} //ibufor2 zawiera odpowiednie miejsce
	dispDodList = ptrSklep->getDodatki();
	if(dispDodList.size() > 0) {
		boolList.resize(dispDodList.size());
		for(i=0; i<dispDodList.size(); i++) {
			cout << i+1 << ". " << dispDodList[i] << endl;
			boolList[i] = false;
		}
		while(true) {
			cout << "Lista wybranych dodatkow: ";
			for(i=0; i<dispDodList.size(); i++) if(boolList[i]) cout << i+1 << " ";
			cout << endl << "Podaj dodatek jaki chcesz dodac/usunac z listy (liczba poza zakresem przechodzi dalej): ";
			cin >> ibufor1;
			if(ibufor1 <= 0 || ibufor1 > dispDodList.size()) break;
			else boolList[ibufor1-1] = !boolList[ibufor1-1];
		}
		for(i=dispDodList.size()-1; i<dispDodList.size(); i--) {
			if(!boolList[i]) dispDodList.erase(dispDodList.begin() + i);
		}
	}
	dispZnizList = ptrSklep->getZnizki();
	if(dispZnizList.size() > 0) {
		boolList.resize(dispZnizList.size());
		for(i=0; i<dispZnizList.size(); i++) {
			cout << i+1 << ". " << dispZnizList[i] << endl;
			boolList[i] = false;
		}
		while(true) {
			cout << "Lista wybranych znizek: ";
			for(i=0; i<dispZnizList.size(); i++) if(boolList[i]) cout << i+1 << " ";
			cout << endl << "Podaj znizke jaka chcesz dodac/usunac z listy (liczba poza zakresem przechodzi dalej): ";
			cin >> ibufor1;
			if(ibufor1 <= 0 || ibufor1 > dispZnizList.size()) break;
			else boolList[ibufor1-1] = !boolList[ibufor1-1];
		}
		for(i=dispZnizList.size()-1; i<dispZnizList.size(); i--) {
			if(!boolList[i]) dispZnizList.erase(dispZnizList.begin() + i);
		}
	}
	cout << "Koncowy koszt biletu: " << ptrSklep->getCena(nazwaFilmu, ibufor2, dispDodList, dispZnizList) << "PLN, czy chcesz kontynuowac? T. tak, x. nie ";
	cin >> bufor1;
	if(bufor1[0] == 84) {
		int status = ptrSklep->kupBilet(user, nazwaFilmu, dispSeansList[ibufor3-1], ibufor2, dispDodList, dispZnizList);
		if(status == 0) cout << "Pomyslnie zakupiono bilet." << endl;
		else if(status == 1) cout << "Zle wprowadzone dane." << endl;
		else if(status == 2) cout << "BLAD: podane miejsce nie jest wolne." << endl;
		else if(status == -1) cout << "BLAD: krytyczny blad systemu po odrzuceniu transakcji." << endl;
		else if(status == -2) cout << "Transakcja zostala odrzucona przez system platnosci." << endl;
	} else {
		cout << "Anulowano zakup biletu." << endl;
	}
}

void Interfejs::sklepDispBil() {
	dispBilList = ptrSklep->wyswietlBilety(user);
	if(dispBilList.size() == 0) {
		cout << "Lista Twoich biletow jest pusta." << endl;
	} else {
		cout << endl;
		for(unsigned int i=0; i<dispBilList.size(); i++) cout << dispBilList[i] << endl << endl;
	}
}

void Interfejs::sklepDispDod() {
	dispDodList = ptrSklep->getDodatki();
	if(dispDodList.size() == 0) {
		cout << "Lista dodatkow jest pusta." << endl;
	} else {
		cout << endl;
		for(unsigned int i=0; i<dispDodList.size(); i++) cout << i+1 << ". " << dispDodList[i] << endl;
	}
}

void Interfejs::sklepDispZniz() {
	dispZnizList = ptrSklep->getZnizki();
	if(dispZnizList.size() == 0) {
		cout << "Lista znizek jest pusta." << endl;
	} else {
		cout << endl;
		for(unsigned int i=0; i<dispZnizList.size(); i++) cout << i+1 << ". " << dispZnizList[i] << endl;
	}
}

void Interfejs::sklepAnuluj() {
	dispBilList = ptrSklep->wyswietlBilety(user);
	if(dispBilList.size() == 0) {
		cout << "Lista Twoich biletow jest pusta." << endl;
	} else {
		cout << "Lista Twoich biletow: " << endl;
		for(unsigned int i=0; i<dispBilList.size(); i++) cout << "BILET " << i+1 << endl << dispBilList[i] << endl << endl;
		cout << "Wybierz bilet do anulowania (indeks poza zakresem anuluje operacje): ";
		cin >> ibufor1;
		int status = ptrSklep->anulujBilet(user, dispBilList[ibufor1-1].nazwaFilmu, ibufor1-1);
		if(status == 0) cout << "Pomyslnie anulowano bilet." << endl;
		else if(status == -1) cout << "BLAD: zly indeks biletu." << endl;
		else if(status == 1) cout << "BLAD: zla nazwa filmu." << endl;
		else if(status == 2) cout << "System platnosci odrzucil transakcje anulowania biletu." << endl;
		else if(status == 3) cout << "BLAD: krytyczny blad zwolnienia miejsca w seansie." << endl;
	}
}

void Interfejs::login() {
	cout << endl << "Podaj nazwe uzytkownika: ";
	cin >> bufor1;
	cout << "Podaj haslo: ";
	cin >> bufor2;
	user = ptrUserDB->logowanie(bufor1,bufor2);
	if(user<0) cout << "Nieudana proba logowania." << endl;
	if(user==0) cout << "Zalogowano jako administrator." << endl;
	if(user>0) cout << "Pomyslne logowanie uzytkownika." << endl;
}

void Interfejs::zarzadzanieKontem() {
	if(user<0) {
		cout << endl << "Uzytkownik nie jest zalogowany." << endl;
	} else {
		cout << endl << "1. Wyswietl aktualne dane\t2. Zmien aktualne dane\tx. Wroc do menu glownego: ";
		cin >> bufor2;
		if(bufor2[0] == 49) {
			dOs temp = ptrUserDB->getDaneOs(user);
			cout << temp;
		} else if(bufor2[0] == 50) {
			this->fillUserBuffers();
			ptrUserDB->modyfikacja(user, bufor2, bufor3, bufor4, databufor);
			cout << "Zmodyfikowano dane uzytkownika." << endl;
		}
	}
}

void Interfejs::rejestracja() {
	cout << endl << "Podaj nazwe uzytkownika: ";
	cin >> bufor1;
	if(ptrUserDB->istnieje(bufor1)) {
		cout << "Podany uzytkownik juz istnieje." << endl;
	} else {
		this->fillUserBuffers();
		ptrUserDB->dodaj(bufor1, bufor2, bufor3, bufor4, databufor);
		cout << "Pomyslna rejestracja uzytkownika." << endl;
	}
}

void Interfejs::adminRFilmAdd() {
	cin.clear();
	cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	cout << "Podaj tytul: ";
	getline(cin,bufor2);
	cout << "Podaj gatunek: ";
	getline(cin,bufor3);
	cout << "Podaj opis (~ na poczatku konczy zapis): ";
	bufor4 = "";
	while(true) {
		bufor1 = "";
		getline(cin,bufor1);
		if(bufor1[0] == 126 || bufor1[1] == 126) break;
		if(bufor1.length() > 0) {
			bufor4 += bufor1;
			bufor4 += "\n";
		}
	}
	cin.clear();
	cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	cout << "Podaj ograniczenie wiekowe (0 - brak): ";
	cin >> ibufor1;
	cout << "Podaj czas trwania (w minutach): ";
	cin >> ibufor2;
	cout << "Podaj cene (w zlotych): ";
	cin >> ibufor3;
	if ( ptrReper->dodajFilm(bufor2, bufor3, bufor4, ibufor1, ibufor2, ibufor3) ) {
		cout << "Pomyslnie dodano film." << endl;
	} else {
		cout << "Blad formatu danych - nie dodano filmu." << endl;
	}
}

void Interfejs::adminRSeansAdd() {
	if( this->fillVecListFilm() ) {
		cout << endl << "Wybierz numer filmu (0 lub liczba ujemna by wrocic) do dodania seansu: ";
		cin >> ibufor1;
		if(ibufor1 > 0 && ibufor1 <= dispFilmList.size()) {
			nazwaFilmu = dispFilmList[ibufor1-1];
			ibufor1 = 1;
			while(ibufor1) {
				cout << "Podaj rok seansu: ";
				cin >> databufor.rok;
				cout << "Podaj miesiac seansu: ";
				cin >> databufor.miesiac;
				cout << "Podaj dzien seansu: ";
				cin >> databufor.dzien;
				if(weryfikacjaDaty(databufor) == false) {
					cout << "Niepoprawna data seansu." << endl;
				} else {
					ibufor1 = 0;
				}
			}
			cout << "Podaj godzine seansu: ";
			cin >> ibufor2;
			cout << "Podaj minute seansu: ";
			cin >> ibufor3;
			int status = ptrReper->dodajSeansDoFilmu(nazwaFilmu, ibufor2, ibufor3, databufor);
			if(status == 0) cout << "Pomyslnie dodano seans do filmu." << endl;
			else if(status == 1) cout << "Pusta lista filmow." << endl;
			else if(status == 2) cout << "Nie znaleziono filmu." << endl;
			else if(status == 3) cout << "Godzina poza czasem otwarcia kina (" << OTWARCIE << "-" << ZAMKNIECIE << ")." << endl;
			else if(status == 4) cout << "Zle podana minuta." << endl;
			else if(status == 5) cout << "Seans bedzie trwal poza polnoc nastepnego dnia." << endl;
			else if(status > 5 && status < 9) cout << "Dodawany seans naklada sie z innym." << endl;
			else cout << "Nieznany blad." << endl;
		}
	}
}

void Interfejs::adminRFilmMod() {
	if( this->fillVecListFilm() ) {
		cout << endl << "Wybierz numer filmu (0 lub liczba ujemna by wrocic) do modyfikacji: ";
		cin >> ibufor1;
		if(ibufor1 > 0 && ibufor1 <= dispFilmList.size()) {
			nazwaFilmu = dispFilmList[ibufor1-1];
			cin.clear();
			cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			cout << "Podaj opis (~ na poczatku konczy zapis): ";
			bufor4 = "";
			while(true) {
				bufor1 = "";
				getline(cin,bufor1);
				if(bufor1[0] == 126 || bufor1[1] == 126) break;
				if(bufor1.length() > 0) {
					bufor4 += bufor1;
					bufor4 += "\n";
				}
			}
			cin.clear();
			cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			cout << "Podaj ograniczenie wiekowe (0 - brak): ";
			cin >> ibufor1;
			cout << "Podaj czas trwania (w minutach): ";
			cin >> ibufor2;
			cout << "Podaj cene (w zlotych): ";
			cin >> ibufor3;
			int status = ptrReper->modFilm(nazwaFilmu, bufor4, ibufor1, ibufor2, ibufor3);
			if(status == 0) cout << "Pomyslnie zmodyfikowano film." << endl;
			else if(status > 0) cout << "Blad formatu danych - nie zmodyfikowano filmu." << endl;
			else if(status < 0) cout << "BLAD: nie znaleziono filmu o wybranej nazwie." << endl; 
		}
	}
}

void Interfejs::adminRSeansDel() {
	if( this->fillVecListFilm() ) {
		cout << endl << "Wybierz numer filmu (0 lub liczba ujemna by wrocic) do usuniecia seansu: ";
		cin >> ibufor1;
		if(ibufor1 > 0 && ibufor1 <= dispFilmList.size()) {
			nazwaFilmu = dispFilmList[ibufor1-1];
			dispSeansList = ptrReper->getSeanseZFilmu(nazwaFilmu);
			cout << "Terminy seansow:" << endl;
			for(unsigned int i=0; i<dispSeansList.size(); i++) cout << i+1 << ". " << dispSeansList[i];
			if(dispSeansList.size() == 0) {
				cout << "brak terminow" << endl;
				return;
			} else {
				cout << endl << "Wybierz numer seansu do usuniecia: ";
				cin >> ibufor2;
				if(ibufor2 > 0 && ibufor2 <= dispSeansList.size()) {
					int status = ptrReper->usunSeansZFilmu(nazwaFilmu, ibufor2-1);
					if(status == 0) cout << "Pomyslnie usunieto seans z filmu." << endl;
					else if(status == 1) cout << "Seans ma wykupione bilety." << endl;
					else if(status == -1) cout << "BLAD: nie znaleziono filmu o podanej nazwie." << endl;
					else cout << "BLAD: zle podany indeks seansu.";
				}
			}
		}
	}
}

void Interfejs::adminRFilmDel() {
	if( this->fillVecListFilm() ) {
		cout << endl << "Wybierz numer filmu (0 lub liczba ujemna by wrocic) do usuniecia: ";
		cin >> ibufor1;
		if(ibufor1 > 0 && ibufor1 <= dispFilmList.size()) {
			nazwaFilmu = dispFilmList[ibufor1-1];
			int status = ptrReper->usunFilm(nazwaFilmu);
			if(status == 0) cout << "Film zostal pomyslnie usuniety." << endl;
			else if(status == 1) cout << "Film posiada seanse - trzeba je najpierw usunac." << endl;
			else cout << "BLAD: nie znaleziono filmu o podanej nazwie." << endl;
		}
	}
}

void Interfejs::adminSDodAdd() {
	cout << "Podaj cene obnizana przez znizki: ";
	cin >> dbufor1;
	cout << "Podaj cene nieobnizana przez znizki: ";
	cin >> dbufor2;
	cout << "Podaj opis: ";
	cin.clear();
	cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	getline(cin,bufor2);
	cin.clear();
	cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	if ( ptrSklep->dodajDodatek(bufor2, dbufor1, dbufor2) ) {
		cout << "Pomyslnie dodano dodatek." << endl;
	} else {
		cout << "Blad formatu danych - nie dodano dodatku." << endl;
	}
}

void Interfejs::adminSZnizAdd() {
	cout << "Podaj znizke przed aplikacja procentu: ";
	cin >> dbufor1;
	cout << "Podaj obnizke procentowa: ";
	cin >> dbufor2;
	cout << "Podaj znizke po aplikacji procentu: ";
	cin >> dbufor3;
	cout << "Podaj wiek minimalny: ";
	cin >> ibufor2;
	cout << "Podaj wiek maksymalny: ";
	cin >> ibufor3;
	cout << "Podaj opis: ";
	cin.clear();
	cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	getline(cin,bufor2);
	cin.clear();
	cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	if ( ptrSklep->dodajZnizke(bufor2, dbufor1, dbufor2, dbufor3, ibufor2, ibufor3) ) {
		cout << "Pomyslnie dodano znizke." << endl;
	} else {
		cout << "Blad formatu danych - nie dodano znizki." << endl;
	}
}

void Interfejs::adminSDodDel() {
	dispDodList = ptrSklep->getDodatki();
	if(dispDodList.size() == 0) {
		cout << "Lista dodatkow jest pusta." << endl;
	} else {
		cout << endl;
		for(unsigned int i=0; i<dispDodList.size(); i++) cout << i+1 << ". " << dispDodList[i] << endl;
		cout << "Wybierz numer dodatku (0 lub liczba ujemna by wrocic) do usuniecia: ";
		cin >> ibufor2;
		if(ibufor2 > 0 && ibufor2 <= dispDodList.size()) {
			if(ptrSklep->usunDodatek(ibufor2-1)) cout << "Pomyslnie usunieto dodatek." << endl;
			else cout << "BLAD: nie usunieto dodatku - zly indeks." << endl;
		}
	}
}

void Interfejs::adminSZnizDel() {
	dispZnizList = ptrSklep->getZnizki();
	if(dispZnizList.size() == 0) {
		cout << "Lista znizek jest pusta." << endl;
	} else {
		cout << endl;
		for(unsigned int i=0; i<dispZnizList.size(); i++) cout << i+1 << ". " << dispZnizList[i] << endl;
		cout << "Wybierz numer znizki (0 lub liczba ujemna by wrocic) do usuniecia: ";
		cin >> ibufor2;
		if(ibufor2 > 0 && ibufor2 <= dispZnizList.size()) {
			if(ptrSklep->usunZnizke(ibufor2-1)) cout << "Pomyslnie usunieto znizke." << endl;
			else cout << "BLAD: nie usunieto znizki - zly indeks." << endl;
		}
	}
}

int main() {
	srand(time(NULL));
	SystemPlatnosci* sysPlat = new SystemPlatnosci();
	Interfejs* inter = new Interfejs(sysPlat);
	bool status = true;
	while(status) {
		status = inter->intro();
	}
	delete sysPlat;
	delete inter;
	return 0;
}

#ifndef UTILITY_H
#define UTILITY_H

#include <time.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#define BUFFER_SIZE 3072 //Dimensione massima del buffer
#define DELIM " \n" //Utile per suddividere in parole i vari comandi
#define USER_SIZE 32 //Dimensione massima username
#define PASS_SIZE 16 //Dimensione massima password

//Struttura che caratterizza un utente
struct utente {
    char username[USER_SIZE];
    char password[PASS_SIZE];
    char sessionID[10];
};

//Struttura che caratterizza un indirizzo IP bloccato (da inserire nella blacklist)
struct bloccato {
    int indirizzo; //Indirizzo IP da bloccare (espresso formato numerico)
    time_t timestamp; //Orario nel quale e' avvenuto il terzo tentativo fallimentare di autenticazione
};

//Struttura che caratterizza una schedina giocata da un utente
struct schedina {
    time_t timestamp; //Orario in cui e' stata effettuata la giocata
    char utente[USER_SIZE]; //Username dell'utente che ha effettuato la giocata
    bool ruota[11]; //Ogni elemento dell'array rappresenta una ruota, in ordine alfabetico
                    //0- Bari, 1- Cagliari, 2- Firenze, 3- Genova, 4- Milano, 5- Napoli
                    //6- Palermo, 7- Roma, 8- Torino, 9- Venezia, 10- Nazionale
                    //Se ruota[i] == 1 --> la scommessa e' stata effettuata su quella ruota
    int numeri[10]; //Array contenente i numeri giocati (minimo 1, massimo 10)
    double importi[5]; //Ogni elemento dell'array rappresenta l'importo per il tipo di giocata
                       //0- estratto, 1- ambo, 2- terno, 3- quaterna, 4- cinquina
};

//Struttura che caratterizza un'estrazione
struct estrazione {
    time_t timestamp; //Orario in cui e' stata effettuata l'estrazione
    
    /*Ogni array contiene i 5 numeri estratti per la rispettiva ruota*/
    int bari[5];
    int cagliari[5];
    int firenze[5];
    int genova[5];
    int milano[5];
    int napoli[5];
    int palermo[5];
    int roma[5];
    int torino[5];
    int venezia[5];
    int nazionale[5];
};

void generatoreIDRandom(char* s);
int nonRuota(char* parola);
void generaListaEstrazioni(char* ritorno, char* ruota, struct estrazione es);
void generaListaGiocate(char* ritorno, struct schedina giocata);
void estraiNumeri(int* ruota);
void generaListaVincite(char* risposta, struct schedina vincente);
bool calcolaVincite(struct estrazione nuova, struct schedina giocata, struct schedina* output);

#endif /* UTILITY_H */


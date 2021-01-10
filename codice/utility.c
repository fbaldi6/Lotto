#include "utility.h"

//Genera una stringa di 10 caratteri alfanumerici casuali
void generatoreIDRandom(char* s) {
    int i;
    time_t t;
    static const char alphanum[] = "0123456789"
                                    "ABCDEFGHIJKLMNOPQRSTUVZ"
                                    "abcdefghijklmnopqrstuvz";
    
    srand((unsigned) time(&t));
    for(i = 0; i<10; ++i) {
        s[i] = alphanum[rand()%(sizeof(alphanum) -1)];
    }   
    s[10] = 0;
}

//Restituisce 1 se la parola passata come parametro non corrisponde 
//ad una delle possibili ruote, 0 altrimenti.
int nonRuota(char* parola) {
    if((strcmp(parola, "roma")!=0) && (strcmp(parola, "milano")!=0) 
            && (strcmp(parola, "bari")!=0) && (strcmp(parola, "cagliari")!=0)
            && (strcmp(parola, "firenze")!=0) && (strcmp(parola, "genova")!=0)
            && (strcmp(parola, "napoli")!=0) && (strcmp(parola, "palermo")!=0)
            && (strcmp(parola, "torino")!=0) && (strcmp(parola, "venezia")!=0)
            && (strcmp(parola, "nazionale")!=0) && (strcmp(parola, "tutte")!=0))
        return 1;
    return 0;
}

//Genera la lista di estrazioni relative a "ruota" e "estrazione" passate come parametri
void generaListaEstrazioni(char* ritorno, char* ruota, struct estrazione es) {  
    char t1[24], t2[24], t3[24], t4[24], t5[24], t6[24], t7[24], t8[24], t9[24], t10[24], t11[24];
    if(strcmp(ruota, "bari") == 0) {
        sprintf(ritorno, "Bari: %d %d %d %d %d\n", es.bari[0], es.bari[1], es.bari[2], es.bari[3], es.bari[4]);
    } else if(strcmp(ruota, "cagliari") == 0) {
        sprintf(ritorno, "Cagliari: %d %d %d %d %d\n", es.cagliari[0], es.cagliari[1], es.cagliari[2], es.cagliari[3], es.cagliari[4]);
    } else if(strcmp(ruota, "firenze") == 0) {
        sprintf(ritorno, "Firenze: %d %d %d %d %d\n", es.firenze[0], es.firenze[1], es.firenze[2], es.firenze[3], es.firenze[4]);
    } else if(strcmp(ruota, "genova") == 0) {
        sprintf(ritorno, "Genova: %d %d %d %d %d\n", es.genova[0], es.genova[1], es.genova[2], es.genova[3], es.genova[4]);
    } else if(strcmp(ruota, "milano") == 0) {
        sprintf(ritorno, "Milano: %d %d %d %d %d\n", es.milano[0], es.milano[1], es.milano[2], es.milano[3], es.milano[4]);
    } else if(strcmp(ruota, "napoli") == 0) {
        sprintf(ritorno, "Napoli: %d %d %d %d %d\n", es.napoli[0], es.napoli[1], es.napoli[2], es.napoli[3], es.napoli[4]);
    } else if(strcmp(ruota, "palermo") == 0) {
        sprintf(ritorno, "Palermo: %d %d %d %d %d\n", es.palermo[0], es.palermo[1], es.palermo[2], es.palermo[3], es.palermo[4]);
    } else if(strcmp(ruota, "roma") == 0) {
        sprintf(ritorno, "Roma: %d %d %d %d %d\n", es.roma[0], es.roma[1], es.roma[2], es.roma[3], es.roma[4]);
    } else if(strcmp(ruota, "torino") == 0) {
        sprintf(ritorno, "Torino: %d %d %d %d %d\n", es.torino[0], es.torino[1], es.torino[2], es.torino[3], es.torino[4]);
    } else if(strcmp(ruota, "venezia") == 0) {
        sprintf(ritorno, "Venezia: %d %d %d %d %d\n", es.venezia[0], es.venezia[1], es.venezia[2], es.venezia[3], es.venezia[4]);
    } else if(strcmp(ruota, "nazionale") == 0) {
        sprintf(ritorno, "Nazionale: %d %d %d %d %d\n", es.nazionale[0], es.nazionale[1], es.nazionale[2], es.nazionale[3], es.nazionale[4]);
    } else if(strcmp(ruota, "tutte") == 0) {
        generaListaEstrazioni(t1, "bari", es);
        generaListaEstrazioni(t2, "cagliari", es);
        generaListaEstrazioni(t3, "firenze", es);
        generaListaEstrazioni(t4, "genova", es);
        generaListaEstrazioni(t5, "milano", es);
        generaListaEstrazioni(t6, "napoli", es);
        generaListaEstrazioni(t7, "palermo", es);
        generaListaEstrazioni(t8, "roma", es);
        generaListaEstrazioni(t9, "torino", es);
        generaListaEstrazioni(t10, "venezia", es);
        generaListaEstrazioni(t11, "nazionale", es);
        sprintf(ritorno, "%s%s%s%s%s%s%s%s%s%s%s", t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11);
    }
}

//Genera una stringa relativa alla schedina passata come parametro
void generaListaGiocate(char* ritorno, struct schedina giocata) {
    int i;
    bool tutte;
    char temp[8];
    
    strcpy(ritorno, " ");
    
    for(i=0; i<11; i++) {
        if(giocata.ruota[i] == false) {
            tutte = false;
            break;
        }
        tutte = true;
    }
    if(tutte == true)
        strcat(ritorno, "Tutte ");
    else if(tutte == false) { 
        if(giocata.ruota[0] == true) 
            strcat(ritorno, "Bari ");
        if(giocata.ruota[1] == true)
            strcat(ritorno, "Cagliari ");
        if(giocata.ruota[2] == true)
            strcat(ritorno, "Firenze ");
        if(giocata.ruota[3] == true)
            strcat(ritorno, "Genova ");
        if(giocata.ruota[4] == true)
            strcat(ritorno, "Milano ");
        if(giocata.ruota[5] == true)
            strcat(ritorno, "Napoli ");
        if(giocata.ruota[6] == true)
            strcat(ritorno, "Palermo ");
        if(giocata.ruota[7] == true)
            strcat(ritorno, "Roma ");
        if(giocata.ruota[8] == true)
            strcat(ritorno, "Torino ");
        if(giocata.ruota[9] == true)
            strcat(ritorno, "Venezia ");
        if(giocata.ruota[1] == true)
            strcat(ritorno, "Nazionale ");
    }
    for(i=0; i<10; i++) {
        if(giocata.numeri[i] != 0) {
            sprintf(temp, "%d ", giocata.numeri[i]);
            strcat(ritorno, temp);
        }
    }
    for(i=0; i<5; i++) {
        if(giocata.importi[i] != 0) {
            sprintf(temp, "* %.2f ", giocata.importi[i]);
            strcat(ritorno, temp);
            if(i == 0)
                strcat(ritorno, "estratto ");
            if(i == 1)
                strcat(ritorno, "ambo ");
            if(i == 2) 
                strcat(ritorno, "terno ");
            if(i == 3)
                strcat(ritorno, "quaterna ");
            if(i == 4)
                strcat(ritorno, "cinquina ");
        }
    }
    strcat(ritorno, "\n");
}

//Prende in ingresso un intero k e restituisce k! (k fattoriale)
int fattoriale(int k) {
    if(k<0) return -1; //Il fattoriale non e' definito per interi negativi
    if(k==0) return 1;
    return k*fattoriale(k-1);
}

//Calcola quante combinazioni di k numeri esistono in una serie di n numeri
int calcolaCombinazioni(int n, int k) {
    int i;
    int combo = 1;
    int k_fattoriale = fattoriale(k);
    for(i = n; i> n-k; i--) {
        combo = combo*i;
    }
    combo = combo/k_fattoriale;
    return combo;
}

//Estrae 5 numeri casuali e li inserisce nell'array relativo alla ruota passata come parametro
void estraiNumeri(int* ruota) {
    int i, j;
    unsigned u = rand()%1000+1;
    srand((unsigned)time(NULL) +u);
    for(i = 0; i<5; i++) {
        ruota[i] = rand()%90 + 1;
        for(j=0;j<i; j++) {
            if(ruota[i] == ruota[j]) {
                i--;
                break;
            }
        }
    }
}

//Genera una stringa rappresentante un consuntivo delle vincite ottenute con la schedina passata come parametro
void generaListaVincite(char* risposta, struct schedina vincente) {
    int i;
    char temp[40];
    strcpy(risposta, "");
    
    if(vincente.ruota[0] == true)
        strcat(risposta, "Bari ");
    if(vincente.ruota[1] == true)
        strcat(risposta, "Cagliari ");
    if(vincente.ruota[2] == true)
        strcat(risposta, "Firenze ");
    if(vincente.ruota[3] == true)
        strcat(risposta, "Genova ");
    if(vincente.ruota[4] == true)
        strcat(risposta, "Milano ");
    if(vincente.ruota[5] == true)
        strcat(risposta, "Napoli ");
    if(vincente.ruota[6] == true)
        strcat(risposta, "Palermo ");
    if(vincente.ruota[7] == true)
        strcat(risposta, "Roma ");
    if(vincente.ruota[8] == true)
        strcat(risposta, "Torino ");
    if(vincente.ruota[9] == true)
        strcat(risposta, "Venezia ");
    if(vincente.ruota[10] == true)
        strcat(risposta, "Nazionale ");
    for(i=0; i<10; i++) {
        if(vincente.numeri[i] != 0) {
            sprintf(temp, "%d ", vincente.numeri[i]);
            strcat(risposta, temp);
        }
    }
    strcat(risposta, " >> ");
    for(i=0; i<5; i++) {
        if(vincente.importi[i] != 0) {
            if(i==0)
                strcat(risposta, "Estratto ");
            if(i==1)
                strcat(risposta, "Ambo ");
            if(i==2)
                strcat(risposta, "Terno ");
            if(i==3)
                strcat(risposta, "Quaterna ");
            if(i==4)
                strcat(risposta, "Cinquina ");
            sprintf(temp, "%.2f ", vincente.importi[i]);
            strcat(risposta, temp);
        }
    }
    strcat(risposta, "\n");
}

//Calcola l'importo della vincita relativo alla schedina giocata e passata come parametro, sapendo quanti numeri sono stati giocati
//e quanti numeri tra questi sono stati estratti. Le vincite vengono registrate sulla schedina "output"
void calcolaImportiVincita(int numeri_vincenti, int numeri_giocati, struct schedina giocata, struct schedina* output) {
    if(giocata.importi[0] != 0) 
        output->importi[0] += ((giocata.importi[0]*11.23)/calcolaCombinazioni(numeri_giocati, 1))*calcolaCombinazioni(numeri_vincenti, 1);
    if(numeri_vincenti == 1) return;
    if(giocata.importi[1] != 0)
        output->importi[1] += ((giocata.importi[1]*250)/calcolaCombinazioni(numeri_giocati, 2))*calcolaCombinazioni(numeri_vincenti, 2);
    if(numeri_vincenti == 2) return;
    if(giocata.importi[2] != 0)
        output->importi[2] += ((giocata.importi[2]*4500)/calcolaCombinazioni(numeri_giocati, 3))*calcolaCombinazioni(numeri_vincenti, 3);
    if(numeri_vincenti == 3) return;
    if(giocata.importi[3] != 0)
        output->importi[3] += ((giocata.importi[3]*120000)/calcolaCombinazioni(numeri_giocati, 4))*calcolaCombinazioni(numeri_vincenti, 4);
    if(numeri_vincenti == 4) return;
    if(giocata.importi[4] != 0) 
        output->importi[4] += ((giocata.importi[4]*6000000)/calcolaCombinazioni(numeri_giocati, 5))*calcolaCombinazioni(numeri_vincenti, 5);
}

//Date un'estrazione e una schedina passate come parametri, restituisce true se la schedina e' vincente, false altrimenti.
//La schedina "output" rappresenta la schedina vincente, con l'importo delle varie vincite
bool calcolaVincite(struct estrazione nuova, struct schedina giocata, struct schedina* output) {
    int i, j, q, num_ruote, numeri_giocati, numeri_vincenti;
    bool vincente = false;
    num_ruote = 0; //Su quante ruote si e' scommesso
    numeri_giocati = 0; //Quanti numeri si sono giocati
    
    //Calcolo quanti numeri sono stati giocati con la schedina
    for(i=0; i<10; i++) 
        if(giocata.numeri[i] != 0)
            numeri_giocati++;
    
    //Per ogni ruota, se l'utente vi ha scommesso, si calcola quanti dei numeri giocati sono stati estratti.
    //Di conseguenza di calcola l'importo relativo alla vincita.
    for(i=0; i<11; i++) {
        numeri_vincenti = 0;
        if(giocata.ruota[i] == true) {
            num_ruote++;
            if(i==0) { //Bari
                for(j=0; j<5;j++) 
                    for(q=0; q<10; q++) 
                        if(giocata.numeri[q] == nuova.bari[j]){
                            numeri_vincenti++;
                            break;
                        }
                if(numeri_vincenti != 0)
                    calcolaImportiVincita(numeri_vincenti, numeri_giocati, giocata, output);
            } else if(i==1){ //Cagliari
                for(j=0; j<5;j++) 
                    for(q=0; q<10; q++) 
                        if(giocata.numeri[q] == nuova.cagliari[j]){
                            numeri_vincenti++;
                            break;
                        }
                if(numeri_vincenti != 0)
                    calcolaImportiVincita(numeri_vincenti, numeri_giocati, giocata, output);
            } else if(i==2){ //Firenze
                for(j=0; j<5;j++) 
                    for(q=0; q<10; q++)
                        if(giocata.numeri[q] == nuova.firenze[j]){
                            numeri_vincenti++;
                            break;
                        }
                if(numeri_vincenti != 0)
                    calcolaImportiVincita(numeri_vincenti, numeri_giocati, giocata, output);
            } else if(i==3){ //Genova
                for(j=0; j<5;j++) 
                    for(q=0; q<10; q++)
                        if(giocata.numeri[q] == nuova.genova[j]){
                            numeri_vincenti++;
                            break;
                        }
                if(numeri_vincenti != 0)
                    calcolaImportiVincita(numeri_vincenti, numeri_giocati, giocata, output);
            } else if(i==4){ //Milano
                for(j=0; j<5;j++) 
                    for(q=0; q<10; q++)
                        if(giocata.numeri[q] == nuova.milano[j]){
                            numeri_vincenti++;
                            break;
                        }
                if(numeri_vincenti != 0)
                    calcolaImportiVincita(numeri_vincenti, numeri_giocati, giocata, output);
            } else if(i==5){ //Napoli
                for(j=0; j<5;j++) 
                    for(q=0; q<10; q++)
                        if(giocata.numeri[q] == nuova.napoli[j]){
                            numeri_vincenti++;
                            break;
                        }
                if(numeri_vincenti != 0)
                    calcolaImportiVincita(numeri_vincenti, numeri_giocati, giocata, output);
            } else if(i==6){ //Palermo
                for(j=0; j<5;j++) 
                    for(q=0; q<10; q++)
                        if(giocata.numeri[q] == nuova.palermo[j]){
                            numeri_vincenti++;
                            break;
                        }
                if(numeri_vincenti != 0)
                    calcolaImportiVincita(numeri_vincenti, numeri_giocati, giocata, output);
            } else if(i==7){ //Roma
                for(j=0; j<5;j++) 
                    for(q=0; q<10; q++)
                        if(giocata.numeri[q] == nuova.roma[j]){
                            numeri_vincenti++;
                            break;
                        }
                if(numeri_vincenti != 0)
                    calcolaImportiVincita(numeri_vincenti, numeri_giocati, giocata, output);
            } else if(i==8){ //Torino
                for(j=0; j<5;j++) 
                    for(q=0; q<10; q++)
                        if(giocata.numeri[q] == nuova.torino[j]){
                            numeri_vincenti++;
                            break;
                        }
                if(numeri_vincenti != 0)
                    calcolaImportiVincita(numeri_vincenti, numeri_giocati, giocata, output);
            } else if(i==9){ //Venezia
                for(j=0; j<5;j++) 
                    for(q=0; q<10; q++)
                        if(giocata.numeri[q] == nuova.venezia[j]){
                            numeri_vincenti++;
                            break;
                        }
                if(numeri_vincenti != 0)
                    calcolaImportiVincita(numeri_vincenti, numeri_giocati, giocata, output);
            } else if(i==10){ //Nazionale
                for(j=0; j<5;j++) 
                    for(q=0; q<10; q++)
                        if(giocata.numeri[q] == nuova.nazionale[j]){
                            numeri_vincenti++;
                            break;
                        }
                if(numeri_vincenti != 0)
                    calcolaImportiVincita(numeri_vincenti, numeri_giocati, giocata, output);
            }
        }
    }
    
    //Gli importi di vincita, se presenti, vanno poi divisi per il numero di ruote su cui si e' scommesso
    for(i=0; i<5; i++)
        if(output->importi[i] != 0) {
            output->importi[i]/= num_ruote;
            vincente = true;
        }
    return vincente;
}
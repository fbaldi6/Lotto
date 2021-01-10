#include "utility.h"

/*Configurazione*/
int periodo; //Ogni quanto verra' effettuata un'estrazione (espresso in secondi)

/*Connessione e comunicazione con i client*/
bool attivo = true; //Vale true se la connessione con un client e' attiva, false altrimenti
int sd; //Descrittore del socket usato per ricevere richieste di connessione
int new_sd; //Descrittore del socket usato per la comunicazione con il client
socklen_t addrlen; //Dimensione dell'indirizzo del client
struct sockaddr_in indirizzo_server, indirizzo_client; //Strutture per l'indirizzo di server e client
int len; //Dimensione del messaggio da inviare/ricevere
uint16_t lmsg; //Lunghezza del messaggio da inviare/ricevere espressa in network order
char buffer[BUFFER_SIZE]; //Buffer di invio/ricezione

/*Login utente*/
struct utente corrente; //Struttura contenente le informazioni dell'utente con il 
                        //quale si sta attualmente comunicando
int num_tentativi = 0; //Tentativi falliti di autenticazione. 
                       //Dopo 3 tentativi l'indirizzo IP deve essere bloccato per 30 minuti

//Crea un socket TCP e si pone in ascolto sulla porta passata come parametro
void creaSocketAscolto(int porta) {
    int ret;
    
    /*Creazione socket*/
    sd = socket(AF_INET, SOCK_STREAM, 0);
    if(sd == -1) {
        perror("Errore nella creazione del socket\n");
        exit(1);
    }
    
    /*Creazione indirizzo di bind*/
    memset(&indirizzo_server, 0, sizeof(indirizzo_server)); // Pulizia 
    indirizzo_server.sin_family = AF_INET;
    indirizzo_server.sin_port = htons(porta);
    indirizzo_server.sin_addr.s_addr = INADDR_ANY;
    
    //Assegno l'indirizzo del server al socket
    ret = bind(sd, (struct sockaddr*)&indirizzo_server, sizeof(indirizzo_server));
    if(ret == -1){
        perror("Errore in fase di bind\n");
        exit(1);
    }
    
    //Il socket viene utilizzato per ricevere richieste di connessione
    ret = listen(sd, 10);
    if(ret == -1) {
        perror("Errore nella listen\n");
        exit(1);
    }
    
    printf("Creazione del socket avvenuta con successo. Server in ascolto sulla porta %d\n", porta);
}

//Invia al client il messaggio passato come parametro
void inviaRisposta(char* risposta) {
    int ret;
    
    /*Invio di un messaggio al client*/
    //Invio al client la quantita' di dati
    len = strlen(risposta) + 1; // Voglio inviare anche il carattere di fine stringa
    lmsg = htons(len); 
    ret = send(new_sd, (void*) &lmsg, sizeof(uint16_t), 0);
        
    //Invio risposta
    ret = send(new_sd, (void*) risposta, len, 0);
    if(ret == -1){
        perror("Errore in fase di invio: \n");
        exit(1);
    }  
}

//Funzione invocata quando viene ricevuto il comando !signup
//Registra un nuovo utente caratterizzato da username e password ricevuti come parametri.
void signUp(char* parola) {
    FILE *user;
    char nomeFile[40];
    struct utente nuovo;
    
    /*Scorro il buffer parola per parola*/
    parola = strtok(NULL, DELIM); 
    strcpy(nuovo.username, parola);  //La prima parola dopo il comando rappresenta l'username
    parola = strtok(NULL, DELIM);
    strcpy(nuovo.password, parola); //La seconda parola dopo il comando rappresenta la password
    
    /*Apro il file relativo all'utente in scrittura "esclusiva"*/
    sprintf(nomeFile, "%s.bin", nuovo.username);
    user = fopen(nomeFile, "wbx");
    if(user == NULL) { //Se il file esiste gia', l'username e' gia' stato utilizzato
        inviaRisposta("Username gia' in uso, scegline un altro!\n");
    } else { //Se e' stato possibile creare il file, e' possibile procedere con la registrazione
        inviaRisposta("Registrazione effettuata con successo!\n");
        fwrite(&nuovo, sizeof(struct utente), 1, user);
        fclose(user);
        printf("Nuovo utente registrato. Username: %s\n", nuovo.username);
    }
}

//Toglie l'indirizzo IP del client che ha effettuato la connessione dalla blacklist
void sbloccaIndirizzoIP() {
    FILE *blacklist, *replica;
    struct bloccato input;
    
    blacklist = fopen("blacklist.txt", "r");
    replica = fopen("replica.txt", "w");
    
    if(blacklist == NULL || replica == NULL) {
        printf("Errore nell'apertura del file!\n");
        return;
    }
    
    /*Elimino dal file blacklist.txt l'indirizzo IP da sbloccare (corrispondente a quello
        del client attuale), lasciando invariati gli altri*/
    while(fread(&input, sizeof(struct bloccato), 1, blacklist)) {
        if(input.indirizzo != indirizzo_client.sin_addr.s_addr)
            fwrite(&input, sizeof(struct bloccato), 1, replica);
    }
    fclose(blacklist);
    fclose(replica);
    remove("blacklist.txt");
    rename("replica.txt", "blacklist.txt");
}

//Inserisce l'indirizzo IP del client che ha effettuato 3 tentativi fallimentari di 
//autenticazione nella blacklist e chiude la connessione
void bloccaIndirizzoIP() {
    FILE *blacklist;
    time_t timeval;
    
    time(&timeval); //Timestamp corrente
    struct bloccato input = {indirizzo_client.sin_addr.s_addr, timeval};
    blacklist = fopen("blacklist.txt", "a");
    if(blacklist == NULL) {
        printf("Errore nell'apertura del file\n");
    } else {
       fwrite(&input, sizeof(struct bloccato), 1, blacklist);
       fclose(blacklist);
	   printf(" Inserimento nella blacklist.\n");
       attivo = false;
    }
}

//Funzione invocata quando viene ricevuto il comando !login
//Autentica l'utente caratterizzato da username e password ricevuti come parametri.
void login(char* parola) {
    FILE *user;
    char temp[48];
    struct utente input;
    
    /*Scorro il buffer parola per parola*/
    parola = strtok(NULL, DELIM); 
    strcpy(corrente.username, parola); //La prima parola dopo il comando corrisponde allo username
    parola = strtok(NULL, DELIM);
    strcpy(corrente.password, parola); //La seconda parola dopo il comando corrisponde alla password
        
    /*Apro il file relativo all'utente*/
    sprintf(temp, "%s.bin", corrente.username);
    user = fopen(temp, "rb");
    if(user == NULL) { //Se il file non esiste, l'utente non e' registrato
        inviaRisposta("Username non presente\n");
    } else { //Se il file esiste occorre confrontare le password 
        fread(&input, sizeof(struct utente), 1, user);
        if(strcmp(input.password, corrente.password)!=0) { //Se le password sono diverse, l'autenticazione fallisce
            num_tentativi++;
            if(num_tentativi == 3) { //Se l'autenticazione fallisce per 3 volte occorre bloccare l'indirizzo IP del client per 30 minuti
				printf("Terzo tentativo di login errato per l'utente %s.", corrente.username);
                inviaRisposta("Autenticazione fallita per 3 volte. Riprova tra 30 minuti\n");
                bloccaIndirizzoIP();
            } else //altrimenti si segnala al client di ritentare
				printf("Login fallito per l'utente %s. Tentativi rimasti: %d\n", corrente.username, 3-num_tentativi);
                inviaRisposta("Password errata. Ritenta!\n");
        } else { //Se le password coincidono, l'autenticazione va a buon fine
            num_tentativi = 0;
            generatoreIDRandom(corrente.sessionID); //Viene generato il sessionID per l'utente corrente
            strcpy(temp, "Login effettuato con successo\n");
            strcat(temp, corrente.sessionID);
            strcat(temp, "\n");
            printf("L'utente %s ha effettuato il login. SessionID: %s\n", corrente.username, corrente.sessionID);
            inviaRisposta(temp);
        }
        fclose(user);
    }
}

//Funzione invocata quando viene ricevuto il comando !invia_giocata
//Registra la giocata rappresentata dalla schedina nel file relativo all'utente e in quello delle giocate in attesa di estrazione
void riceviGiocata(char* parola) {
    FILE *user, *giocate;
    int i;
    char temp[40];
    struct schedina giocata;
    time_t rawtime;
   
    //Assegno alla giocata il timestamp attuale
    time(&rawtime);
    giocata.timestamp = rawtime;
    
    /*Scorro il buffer parola per parola*/
    parola = strtok(NULL, DELIM);
    //Prima dell'opzione -n c'e' la lista delle ruote su cui l'utente ha scommesso.
    //Pongo la relativa entrata dell'array a true se l'utente ha scommesso su quella ruota.
    while(strcmp(parola, "-n") != 0) {
        if(strcmp(parola, "bari") == 0)
            giocata.ruota[0] = true;
        else if(strcmp(parola, "cagliari") == 0) 
            giocata.ruota[1] = true;
        else if(strcmp(parola, "firenze") == 0)
            giocata.ruota[2] = true;
        else if(strcmp(parola, "genova") == 0) 
            giocata.ruota[3] = true;
        else if(strcmp(parola, "milano") == 0)
            giocata.ruota[4] = true;
        else if(strcmp(parola, "napoli") == 0)
            giocata.ruota[5] = true;
        else if(strcmp(parola, "palermo") == 0)
            giocata.ruota[6] = true;
        else if(strcmp(parola, "roma") == 0)
            giocata.ruota[7] = true;
        else if(strcmp(parola, "torino") == 0)
            giocata.ruota[8] = true;
        else if(strcmp(parola, "venezia") == 0)
            giocata.ruota[9] = true;
        else if(strcmp(parola, "nazionale") == 0)
            giocata.ruota[10] = true;
        else if(strcmp(parola, "tutte") == 0)
            for(i = 0; i<11; i++)
                giocata.ruota[i] = true;
        parola = strtok(NULL, DELIM);
    }
    
    //Dopo l'opzione -n (e prima dell'opzione -i) c'e' la lista dei numeri giocati
    //Inserisco ogni numero giocato nella rispettiva posizione dell'array
    parola = strtok(NULL, DELIM);
    i = 0;
    while(strcmp(parola, "-i") != 0) {
        giocata.numeri[i] = atoi(parola);
        i++;
        parola = strtok(NULL, DELIM);
    }
    
    //Dopo l'opzione -i c'e' la lista di importi giocati per ogni tipologia
    //Inserisco ogni importo nella rispettiva posizione dell'array
    parola = strtok(NULL, DELIM);
    i = 0;
    while(atof(parola) != 0 || strcmp(parola, "0") == 0) {
        giocata.importi[i] = atof(parola);
        i++;
        parola = strtok(NULL, DELIM);
    }
    
    //L'utente che ha effettuato la giocata e' quello con cui e' attiva la connessione
    strcpy(giocata.utente, corrente.username);
    
    //Se il messaggio non contenesse un sessionID valido, la registrazione della giocata non andrebbe a buon fine
    if((strcmp(parola, corrente.sessionID)) != 0)
        inviaRisposta("SessionID non valido!\n");
    else { //Altrimenti si registra la giocata e si invia un messaggio di conferma al client
        sprintf(temp, "%s.bin", corrente.username);
        user = fopen(temp, "ab");
        fwrite(&giocata, sizeof(struct schedina), 1, user);
        fclose(user);
        giocate = fopen("Giocate.bin", "ab");
        fwrite(&giocata, sizeof(struct schedina), 1, giocate);
        fclose(giocate);
        printf("Registrata giocata dell'utente %s\n", corrente.username);
        inviaRisposta("Giocata inviata con successo!\n");
    }
}

//Funzione invocata quando viene ricevuto il comando !vedi_estrazione
//Invia al client la lista delle ultime n estrazioni sulla ruota eventualmente richiesta
void inviaEstrazioni(char* parola) {
    FILE *estrazioni;
    struct estrazione input;
    int n; //contatore
    int numero; //numero di estrazioni richieste dall'utente
    char ruota[10]; //ruota per la quale visualizzare le estrazioni
    int max_estrazioni = 0; //se il numero di estrazioni dovesse superare il numero di estrazioni   
                        //effettuate, dovranno essere visualizzate solo queste ultime
    char risposta[BUFFER_SIZE];
    char ritorno[BUFFER_SIZE];
    
    strcpy(risposta, "");
    
    /*Scorro il buffer parola per parola*/
    parola = strtok(NULL, DELIM);
    numero = atoi(parola); //La prima parola dopo il comando rappresenta quante estrazioni visualizzare
    parola = strtok(NULL, DELIM);
    //La seconda parola dopo il comando e' opzionale. Se presente rappresenta la ruota per la quale visualizzare le estrazioni
    if(!nonRuota(parola)) { 
        strcpy(ruota, parola);
        printf("L'utente %s ha richiesto le ultime %d estrazioni per la ruota di %s\n", corrente.username, numero, ruota);
        parola = strtok(NULL, DELIM);
    } else { //Se non e' presente, vengono visualizzate le estrazioni per tutte le ruote
        strcpy(ruota, "tutte");  
        printf("L'utente %s ha richiesto le ultime %d estrazioni per tutte le ruote\n", corrente.username, numero);
    }
     
    //Se il sessionID contenuto nel messaggio non dovesse essere valido, il comando non andrebbe a buon fine
    if(strcmp(parola, corrente.sessionID) != 0) {
        inviaRisposta("SessionID non valido!\n");
        return;
    }
    
    /*Viene aperto il file contenente tutte le estrazioni e viene letto dal basso verso l'alto 
    (dalla piu' recente alla piu' vecchia)*/
    estrazioni = fopen("Estrazioni.bin", "rb");
    if(estrazioni == NULL) {
        inviaRisposta("Non sono presenti estrazioni\n");
    } else {
        while(fread(&input, sizeof(struct estrazione), 1, estrazioni))
            max_estrazioni++;
        fseek(estrazioni, -sizeof(struct estrazione), SEEK_END);
        for(n=0; n<numero && n<max_estrazioni; n++) {
            fread(&input, sizeof(struct estrazione), 1, estrazioni);
            generaListaEstrazioni(ritorno, ruota, input); //Viene generato il messaggio di risposta
            strcat(risposta, ritorno);
            strcat(risposta, "*************************\n");
            fseek(estrazioni, -2*sizeof(struct estrazione), SEEK_CUR);
        }
        fclose(estrazioni);
        
        //Se e' stato possibile leggere il file, viene inviata la risposta al client
        inviaRisposta(risposta);
    }
}

//Funzione invocata quando viene ricevuto il comando !vedi_giocate
//Invia al client la lista delle giocate passate o attive, in base al parametro tipo
void inviaGiocate(char* parola) {
    FILE *es, *gi;
    int tipo;
    int i = 0; //contatore
    struct estrazione ultima;
    struct schedina giocata;
    char nomeFile[40];
    char ritorno[BUFFER_SIZE];
    char risposta[BUFFER_SIZE];
    
    strcpy(risposta, "");

    /*Scorro il buffer parola per parola*/
    parola = strtok(NULL, DELIM);
    tipo = atoi(parola); //La prima parola dopo il comando rappresenta il tipo di giocate da visualizzare
    parola = strtok(NULL, DELIM);   
    if(strcmp(parola, corrente.sessionID) != 0) { //La seconda parola dopo il comando rappresenta il sessionID dell'utente
        inviaRisposta("SessionID non valido!\n"); //Se il sessionID non dovesse essere valido, il comando non andrebbe a buon fine
        return;
    }
    
    if(tipo == 1) {
        printf("L'utente %s ha richiesto le sue giocate attive\n", corrente.username);
    } else {
        printf("L'utente %s ha richiesto le sue giocate relative a estrazioni gia' effettuate\n", corrente.username);
    }
    
    /*Apro il file delle estrazioni per vedere quando e' stata effettuata l'ultima e apro il file 
        relativo all'utente per vedere le schedine che ha giocato*/
    es = fopen("Estrazioni.bin", "rb");
    sprintf(nomeFile, "%s.bin", corrente.username);
    gi = fopen(nomeFile, "rb");
    fseek(gi, sizeof(struct utente), SEEK_SET); //Le giocate si trovano dopo la struttura con le informazioni relative all'utente
    if(es == NULL && tipo == 0){ //Se non c'e' stata ancora un'estrazione, non esistono giocate passate
        inviaRisposta("Non sono presenti giocate di questo tipo\n");
        return;
    } else if(es == NULL && tipo ==1) { //Se non c'e' stata ancora un'estrazione, tutte le giocate sono attive (in attesa di estrazione)
        while(fread(&giocata, sizeof(struct schedina), 1, gi)) {
            i++;
            sprintf(ritorno, "%d)", i);
            strcat(risposta, ritorno);
            generaListaGiocate(ritorno, giocata);
            strcat(risposta, ritorno);
        }
    } else { //Se c'e' gia' stata almeno un'estrazione, ricavo il timestamp di quest'ultima
        fseek(es, -sizeof(struct estrazione), SEEK_END);
        fread(&ultima, sizeof(struct estrazione), 1, es);
        fclose(es);
        while(fread(&giocata, sizeof(struct schedina), 1, gi)) { 
            if(tipo == 0) { //Se l'utente ha richiesto le giocate passate, gli invio solo le giocate il cui timestamp e' precedente a quello dell'ultima estrazione
                if((int)giocata.timestamp <= (int)ultima.timestamp) {
                    i++; //Aumento il contatore per tenere conto di quante giocate devo inviare
                    sprintf(ritorno, "%d)", i);
                    strcat(risposta, ritorno);
                    generaListaGiocate(ritorno, giocata);
                    strcat(risposta, ritorno);
                }
            } else { //Se l'utente ha richiesto le giocate attive, gli invio solo le giocate il cui timestamp e' successivo a quello dell'ultima estrazione
                if((int)giocata.timestamp > (int)ultima.timestamp) {
                    i++; //Aumento il contatore per tenere conto di quante giocate devo inviare
                    sprintf(ritorno, "%d)", i);
                    strcat(risposta, ritorno);
                    generaListaGiocate(ritorno, giocata);
                    strcat(risposta, ritorno);
                } 
            }
        }
    }    
    fclose(gi);

    if(i==0) //Se il contatore e' a 0 significa che non c'erano giocate del tipo richiesto
        inviaRisposta("Non sono presenti giocate di questo tipo\n");
    else //Altrimenti invio le giocate del tipo richiesto al client
        inviaRisposta(risposta);
}


//Funzione invocata quando viene ricevuto il comando !esci
//Invia al client un messaggio di conferma e chiude il socket per la comunicazione
void esci(char* parola) {
    char indirizzo[INET_ADDRSTRLEN];
    parola = strtok(NULL, DELIM);
    if(strcmp(parola, corrente.sessionID) == 0) { //Se il sessionID e' presente, l'utente aveva effettuato il login
        printf("Disconnessione dell'utente %s\n", corrente.username);
        inviaRisposta("Logout effettuato con successo!\n");
    } else { //Login non ancora effettuato
        inet_ntop(AF_INET, &indirizzo_client.sin_addr.s_addr, indirizzo, sizeof(indirizzo));
        printf("Disconnessione del client con indirizzo IP %s\n", indirizzo);
        inviaRisposta("Disconnessione dal server avvenuta\n");
    }
    attivo = false;
}


//Funzione invocata quando viene ricevuto il comando !vedi_vincite
//Legge le vincite del client e ne invia un consuntivo
void inviaVincite(char* parola) {
    FILE *vincite;
    struct schedina vincente;
    char risposta[BUFFER_SIZE];
    char temp[BUFFER_SIZE];
    double estratto=0, ambo=0, terno=0, quaterna=0, cinquina=0;
    bool vinto = false;
    
    strcpy(risposta, "");
    
    parola = strtok(NULL, DELIM); //La parola che segue il comando e' il sessionID
    if(strcmp(parola, corrente.sessionID) != 0) { //Se non e' valido, il comando non puo' essere eseguito
        inviaRisposta("SessionID non valido!\n");
        return;
    } 
    
    printf("L'utente %s ha richiesto le sue vincite\n", corrente.username);
    
    /*Si apre il file relativo a tutte le vincite registrate*/
    vincite = fopen("Vincite.bin", "rb");
    if(vincite == NULL) { //Se ancora non sono state registrate vincite, l'utente non puo' aver vinto
        inviaRisposta("Non hai ancora mai vinto\n");
    } else { //Se sono state registrate vincite, si scorre il file alla ricerca di schedine appartenenti all'utente
        while(fread(&vincente, sizeof(struct schedina), 1, vincite)) {
            if(strcmp(vincente.utente, corrente.username) == 0) {  //Se si trova una schedina appartenente all'utente gli si invia
                vinto = true;
                sprintf(temp, "Estrazione del %s", ctime(&vincente.timestamp));
                strcat(risposta, temp);
                generaListaVincite(temp, vincente);
                strcat(risposta, temp);
                strcat(risposta, "*********************************************\n");
                estratto+=vincente.importi[0];
                ambo+=vincente.importi[1];
                terno+=vincente.importi[2];
                quaterna+=vincente.importi[3];
                cinquina+=vincente.importi[4];
            }
        }
        fclose(vincite);
    }
    if(vinto == true) { //Se l'utente ha giocato almeno una schedina vincente, gli si invia anche un consuntivo per tipologia di giocata
        sprintf(temp, "Vincite su ESTRATTO: %.2f\n"
                    "Vincite su AMBO: %.2f\n"
                    "Vincite su TERNO: %.2f\n"
                    "Vincite su QUATERNA: %.2f\n"
                    "Vincite su CINQUINA %.2f\n", estratto, ambo, terno, quaterna, cinquina);
        strcat(risposta, temp);
        inviaRisposta(risposta);
    } else { //Altrimenti si segnala al cliente che non ha ancora mai vinto
        inviaRisposta("Non hai ancora mai vinto\n");
    }
}

//Attende che il client invii un comando e invoca la rispettiva funzione di gestione
void riceviComando() {
    int ret;
    char* parola;
    
    // Attendo dimensione del messaggio                
    ret = recv(new_sd, (void*)&lmsg, sizeof(uint16_t), 0);
    len = ntohs(lmsg); // Riconverto in formato host
    
    //Attendo il comando
    ret = recv(new_sd, (void*)buffer, len, 0);
    if(ret == 0){
        printf("Il client si e' disconnesso improvvisamente\n");
        exit(1);
    }
    if(ret < 0){
        perror("Errore in fase di ricezione: \n");
        exit(1);
    }
          
    /*Estraggo la prima parola ricevuta cosi' da poter discriminare i vari comandi*/
    parola = strtok(buffer, DELIM);
    if(strcmp(parola, "!signup") == 0) {
        signUp(parola);
    } else if(strcmp(parola, "!login") == 0) {
        login(parola);
    } else if(strcmp(parola, "!invia_giocata") == 0) {
        riceviGiocata(parola);
    } else if(strcmp(parola, "!vedi_giocate") == 0) {
        inviaGiocate(parola);
    } else if(strcmp(parola, "!vedi_estrazione") == 0) {
        inviaEstrazioni(parola);
    } else if(strcmp(parola, "!vedi_vincite") == 0) {
        inviaVincite(parola);
    } else if(strcmp(parola, "!esci") == 0) {
        esci(parola);
    }
}

//Data l'estrazione passata come parametro, calcola se tra le schedine in attesa di estrazione ce n'e' qualcuna vincente
void schedineVincenti(struct estrazione nuova) {
    FILE *giocate, *vincite;
    int i;
    struct schedina input, output;

    /*Apro il file relativo alle giocate in attesa di estrazione*/
    giocate = fopen("Giocate.bin", "rb");
    if(giocate != NULL) { //Solo se il file esiste procedo a verificare la presenza o meno di schedine vincenti
        vincite = fopen("Vincite.bin", "ab");
        while(fread(&input, sizeof(struct schedina), 1, giocate)) {
            fseek(giocate, -sizeof(struct schedina), SEEK_CUR);
            fread(&output, sizeof(struct schedina), 1, giocate);
            for(i=0;i<5;i++)
                output.importi[i] = 0;
            if(calcolaVincite(nuova, input, &output)) {
                output.timestamp = nuova.timestamp;
                fwrite(&output, sizeof(struct schedina), 1, vincite);
            }
        }
        fclose(giocate);
        fclose(vincite);
        remove("Giocate.bin");
    }
}

//Effettua una nuova estrazione su tutte le ruote
void nuovaEstrazione(int s) {
    FILE *estrazioni;
    time_t rawtime;
    struct estrazione nuova;
    alarm(periodo); //Si reimposta la nuova estrazione tra "periodo" secondi
    
    time(&rawtime);
    nuova.timestamp = rawtime; //La nuova estrazione ha il time stamp attuale
    
    /*Si estraggono i numeri per ogni ruota*/
    estraiNumeri(nuova.bari);
    estraiNumeri(nuova.cagliari);
    estraiNumeri(nuova.firenze);
    estraiNumeri(nuova.genova);
    estraiNumeri(nuova.milano);
    estraiNumeri(nuova.napoli);
    estraiNumeri(nuova.nazionale);
    estraiNumeri(nuova.palermo);
    estraiNumeri(nuova.roma);
    estraiNumeri(nuova.torino);
    estraiNumeri(nuova.venezia);
    
    /*Si inserisce la nuova estrazione nel file relativo alle estrazioni*/
    estrazioni = fopen("Estrazioni.bin", "ab");
    fwrite(&nuova, sizeof(struct estrazione), 1, estrazioni);
    fclose(estrazioni);
    
    printf("Estrazione effettuata, orario: %s", ctime(&rawtime));
    
    schedineVincenti(nuova); //Si verifica se ci sono schedine vincenti tra quelle in attesa di estrazione
}

//Legge il file contenente gli indirizzi bloccati, confronta le entrate con l'indirizzo del client
// che ha attualmente effettuato la connessione e restituisce true se l'indirizzo e' bloccato, false altrimenti.
bool indirizzoIPBloccato() {
    FILE* blacklist;
    struct bloccato input;
    int t = time(NULL);
    
    blacklist = fopen("blacklist.txt", "r");
    if(blacklist != NULL) {
        while(fread(&input, sizeof(struct bloccato), 1, blacklist)) {
            if(input.indirizzo == indirizzo_client.sin_addr.s_addr) //Se l'indirizzo del client e' presente nel file
                if(t - 30*60 >= (int)input.timestamp){ //Se sono gia' passati 30 minuti l'indirizzo IP viene sbloccato
                    sbloccaIndirizzoIP();
                    return false;
                }   
                return true; // Altrimenti l'indirizzo IP resta bloccato
        }
    }
    return false; //Se l'indirizzo del client non e' presente nel file sicuramente non e' bloccato
}

int main(int argc, char* argv[]){ 
    pid_t pid;
    char indirizzo[INET_ADDRSTRLEN];
    
    //Controllo parametri
    if(argc != 2 && argc != 3) {
        fprintf(stderr, "Usa: ./lotto_server <porta> oppure ./lotto_server <porta> <periodo>\n");
        exit(1);
    }
    
    //Se non e' stato indicato il periodo, si imposta il periodo di default (5 minuti)
    if(argc == 2) {
        periodo = 300;
    } else {
        periodo = atoi(argv[2])*60;
    }
    
    //Crea un socket TCP in ascolto sulla porta passata come parametro
    creaSocketAscolto(atoi(argv[1])); 

	printf("Le estrazioni verranno effettuate ogni %d minuti\n", periodo/60);
    
    signal(SIGALRM, nuovaEstrazione); //Ogni volta che verra' ricevuto un segnale di tipo SIGALRM verra' effettuata un'estrazione
    alarm(periodo); //Si imposta la prossima estrazione fra "periodo" secondi
    
    while(1){  
        addrlen = sizeof(indirizzo_client);
    
        // Accetto nuove connessioni
        new_sd = accept(sd, (struct sockaddr*) &indirizzo_client, &addrlen);
        //Se l'indirizzo IP e' tra quelli bloccati si chiude la connessione
        if(indirizzoIPBloccato()) {
            inviaRisposta("Autenticazione fallita per 3 volte. Riprova piu' tardi\n");
            close(new_sd);
            continue;
        }
        
        inet_ntop(AF_INET, &indirizzo_client.sin_addr.s_addr, indirizzo, sizeof(indirizzo));
        inviaRisposta("Connessione al server effettuata con successo!\n");
        printf("Accettata nuova connessione. Indirizzo IP del client: %s\n", indirizzo);
        
        pid = fork(); //Server multiprocesso
        
        if(pid == 0) {// Sono nel processo figlio
            close(sd);
            
            //Finche' il server e' attivo resta in attesa ricorsivamente di nuovi comandi
            while(attivo){
                riceviComando();
            }   
            
            close(new_sd);
            exit(0);
        } else {
            // Processo padre  
            close(new_sd);
        }
    }
}

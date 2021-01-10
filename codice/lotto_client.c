#include "utility.h"

bool attivo; //Vale true se la connessione e' attiva, false altrimenti
char sessionID[] = "----------"; //Stringa alfanumerica casuale che il server associa al client una volta
                                 //che questo ha effettuato il login. La stringa verra' poi inserita in ogni 
                                 //messaggio che contenga un comando per cui e' necessario essere loggati

/*Connessione e comunicazione con il server*/
int sd; //Descrittore del socket locale
int len; //Dimensione del messaggio da inviare/ricevere
uint16_t lmsg; //Lunghezza del messaggio da inviare/ricevere espressa in network order
struct sockaddr_in indirizzo_server; //Struttura per l'indirizzo del server
char buffer[BUFFER_SIZE]; //Buffer di invio/ricezione


//Crea un socket TCP e si connette al server con indirizzo "indirizzo" sulla porta "porta"
void creaSocketTCP(const char* indirizzo, int porta) {
    int ret;
    
    /* Creazione socket */
    sd = socket(AF_INET, SOCK_STREAM, 0);
    if(sd == -1) { //La primitiva socket() restituisce -1 in caso di errore
        perror("Errore nella creazione del socket\n");
        exit(1);
    }
    
    /* Creazione indirizzo del server */
    memset(&indirizzo_server, 0, sizeof(indirizzo_server)); // Pulizia 
    indirizzo_server.sin_family = AF_INET;
    indirizzo_server.sin_port = htons(porta);
    inet_pton(AF_INET, indirizzo, &indirizzo_server.sin_addr);
    
    /* Connessione del socket locale a quello remoto del server*/
    ret = connect(sd, (struct sockaddr*)&indirizzo_server, sizeof(indirizzo_server));
    if(ret == -1){
        perror("Errore in fase di connessione: \n");
        exit(1);
    } 
    
    /*Il server inviera' un messaggio di avvenuta connessione o meno*/
    //Attendo dimensione del messaggio
    ret = recv(sd, (void*)&lmsg, sizeof(uint16_t), 0);
    len = ntohs(lmsg); // Riconverto in formato host
        
    //Attendo messaggio
    ret = recv(sd, (void*)buffer, len, 0);
    if(ret == 0){
        printf("Il server si e' disconnesso improvvisamente\n");
        exit(1);
    }
    if(ret < 0){
        perror("Errore in fase di ricezione: \n");
        exit(1);
    }
    
    /*Stampa a video del messaggio ricevuto dal server*/
    printf("%s\n", buffer);
    //Se in precedenza il client ha tentato di effettuare il login fallendo per tre volte
    //il server rifiutera' la connessione. In questo caso e' necessario terminare il client
    if(strcmp(buffer, "Autenticazione fallita per 3 volte. Riprova piu' tardi\n") == 0)
        attivo = false;
    else {
        printf("******************************* GIOCO DEL LOTTO *******************************\n");
        attivo = true;
    }
}

//Stampa a video i comandi disponibili
void messaggioDiHelp() {
    printf("Sono disponibili i seguenti comandi:\n\n"
    "1) !help <comando> --> mostra i dettagli di un comando\n"
    "2) !signup <username> <password> --> crea un nuovo utente\n"
    "3) !login <username> <password> --> autentica un utente\n"
    "4) !invia_giocata g --> invia una giocata g al server\n"
    "5) !vedi_giocate tipo --> visualizza le giocate precedenti dove tipo = {0,1}\n"
    "                          e permette di visualizzare le giocate passate '0'\n"
    "                          oppure le giocate attive '1' (ancora non estratte)\n"
    "6) !vedi_estrazione <n> <ruota> --> mostra i numeri delle ultime n estrazioni\n"
    "                                    sulla ruota specificata\n"
    "7) !vedi_vincite --> visualizza tutte le vincite dell'utente\n"
    "8) !esci --> termina il client\n");
}

//Invia il comando passato come parametro al server
void inviaComando(char* comando) {
    int ret;
    char* parola;
    char ricevuto[BUFFER_SIZE];
    
    /* Invio un messaggio al server attraverso il socket connesso*/
    // Invio al server la quantita' di dati
    len = strlen(comando) + 1; // Voglio inviare anche il carattere di fine stringa
    lmsg = htons(len);
    ret = send(sd, (void*) &lmsg, sizeof(uint16_t), 0);
    
    //Invio al server il comando
    ret = send(sd, (void*) comando, len, 0);
        
    /*Il server rispondera' inviando un messaggio di conferma*/
    //Attendo dimensione della risposta
    ret = recv(sd, (void*)&lmsg, sizeof(uint16_t), 0);
    len = ntohs(lmsg); // Riconverto in formato host
        
    //Attendo risposta
    ret = recv(sd, (void*)buffer, len, 0);
    if(ret == 0){
        printf("Il server si e' disconnesso improvvisamente\n");
        exit(1);
    }
    if(ret < 0){
        perror("Errore in fase di ricezione: \n");
        exit(1);
    }
    
    strcpy(ricevuto, buffer);
    
    //Scomponiamo il messaggio ricevuto per discriminare i vari casi
    parola = strtok(buffer, "\n"); 
    
    //Se il client ha specificato delle credenziali valide, il server genera un sessionID e lo invia al client
    if(strcmp(parola, "Login effettuato con successo") == 0) {
        printf("%s\n", parola);
        parola = strtok(NULL, "\n");
        strcpy(sessionID, parola);
        return;
    }
    
    //Se il client ha fallito l'autenticazione per 3 volte, oppure ha invocato il comando !esci
    //e' necessario terminare il client e stampare a video il messaggio ricevuto
    if((strcmp(parola, "Autenticazione fallita per 3 volte. Riprova tra 30 minuti") == 0)
                || (strcmp(parola, "Logout effettuato con successo!") == 0)
                || (strcmp(parola, "Disconnessione dal server avvenuta")) == 0){
        attivo = false; 
    }
      
    //In tutti gli altri casi semplicemente si stampa il contenuto dell'intero messaggio ricevuto
    printf("%s", ricevuto);   
}

//Funzione invocata quando il client digita il comando !help. 
//Mostra una breve descrizione del comando digitato di seguito.
void help(char* parola) {
    parola = strtok(NULL, DELIM); 
  
    //Se l'utente non digita nessun comando viene mostrato nuovamente a video il messaggio di help
    if(parola == NULL) {
        messaggioDiHelp();
        return;
    }
    
    /*Stampa a video della descrizione corrispondente al comando digitato*/
    if((strcmp(parola, "!signup") == 0) || (strcmp(parola, "signup") == 0)) {
        printf("Registra un nuovo utente caratterizzato da username e password ricevuti come parametri.\n"
                "L'username deve contenere al massimo 32 caratteri e la password 16.\n");
    } else if((strcmp(parola, "!login") == 0) || (strcmp(parola, "login") == 0)){
        printf("Autentica l'utente con le credenziali passate come parametri.\n"
                "Se si inseriscono username o password errati per 3 volte consecutive verra' negato"
                " l'accesso al server per 30 minuti.\n");
    } else if((strcmp(parola, "!invia_giocata") == 0) ||(strcmp(parola, "invia_giocata") == 0)) {
        printf("Se l'utente e' loggato, invia al server la giocata descritta dalla schedina, passata come parametro\n"
                "La schedina e' formata da:\n"
                "1) l'elenco delle ruote, preceduto dall'opzione -r\n"
                "2) i numeri giocati, preceduti dall'opzione -n\n"
                "3) gli importi per ogni tipologia di giocata (in ordine dall'estratto alla cinquina)"
                ", preceduti dall'opzione -i\n");
    } else if((strcmp(parola, "!vedi_giocate") == 0) || (strcmp(parola, "vedi_giocate") == 0)) {
        printf("Se l'utente e' loggato, richiede al server le giocate effettuate.\n"
                "Se il parametro tipo vale 0, vengono visualizzate le giocate relative a estrazioni gia' effettuate,\n"
                "se il parametro tipo vale 1, vengono visualizzate le giocate attive, in attesa della prossima estrazione.\n");
    } else if((strcmp(parola, "!vedi_estrazione") == 0) ||(strcmp(parola, "vedi_estrazione") == 0)) {
        printf("Se l'utente e' loggato, richiede al server i numeri estratti nelle ultime estrazioni.\n"
                "Ha due parametri:\n"
                "* n --> di quante estrazioni visualizzare i numeri estratti\n"
                "* ruota (opzionale) --> per quale ruota visualizzare le estrazioni.\n"
                "                       Se non viene specificata, vengono visualizzati i numeri\n"
                "                       estratti su tutte le ruote.\n");
    } else if((strcmp(parola, "!vedi_vincite") == 0) || (strcmp(parola, "vedi_vincite") == 0)){
        printf("Se l'utente e' loggato, vengono visualizzate tutte le sue vincite, l'estrazione in "
                "cui sono state realizzate e un consuntivo per tipo di giocata\n");
    } else if((strcmp(parola, "!esci") == 0) || (strcmp(parola, "esci") == 0)){
        printf("Viene effettuato il logout dal server e il client viene disconnesso.\n");
    } else if((strcmp(parola, "!help") == 0) || (strcmp(parola, "help") == 0)) {
        printf("Mostra i dettagli del comando specificato come parametro\n");
    } else {
        printf("Comando non riconosciuto. ");
        messaggioDiHelp();
    }
}

//Funzione invocata quando il client digita il comando !signup.
//Registra un nuovo utente caratterizzato da username e password digitati di seguito al comando.
void signUp(char* parola) {
    char comando[BUFFER_SIZE];
    int num_parole = 0;
    int formatoErrato = 0;
    
    strcpy(comando, "!signup ");
    
    /*Scorro il buffer parola per parola*/
    parola = strtok(NULL, DELIM); 
    while(parola != NULL) {
        num_parole++;
        if(num_parole == 1) { //La prima parola che segue il comando rappresenta l'username
            if(strlen(parola) > USER_SIZE) { //L'username inserito supera il numero massimo di caratteri
                printf("Username troppo lungo! Usa al massimo %d caratteri.\n", USER_SIZE);
                formatoErrato++;
                break;
            }
            strcat(comando, parola);
            strcat(comando, " ");
        } else if(num_parole == 2) { //La seconda parola che segue il comando rappresenta la password
            if(strlen(parola) > PASS_SIZE) { //La password inserita supera il numero massimo di caratteri
                printf("Password troppo lunga! Usa al massimo %d caratteri.\n", PASS_SIZE);
                formatoErrato++;
                break;
            }
          strcat(comando, parola);
          strcat(comando, "\n");
        } 
        parola = strtok(NULL, DELIM);
    }
    
    //Se username e/o password sono della giusta lunghezza ma le parole che seguono il comando
    // non sono due e' necessario chiedere al client di reinserire il comando.
    //Altrimenti il comando viene inviato al server.
    if(formatoErrato == 0) {
        if(num_parole != 2) {
            printf("Sono richiesti due parametri: username e password\n");
        } else 
        inviaComando(comando); 
    }
}

//Funzione invocata quando il client digita il comando !login.
//Autentica l'utente caratterizzato da username e password digitati di seguito al comando.
void login(char* parola) {
    char comando[BUFFER_SIZE];
    int num_parole = 0;
    int formatoErrato = 0;
    
    //Se il sessionID e' gia' stato modificato significa che il client e' gia' loggato e 
    //non puo' effettuare un nuovo login. 
    if(strcmp(sessionID, "----------") != 0) {
        printf("Hai gia' effettuato il login!\n");
        return;
    } 
    
    strcpy(comando, "!login ");
        
    /*Scorro il buffer parola per parola*/
    parola = strtok(NULL, DELIM); 
    while(parola != NULL) {
        num_parole++;
        if(num_parole == 1) { //La prima parola che segue il comando rappresenta l'username
            if(strlen(parola) > USER_SIZE) { //L'username inserito supera il numero massimo di caratteri
                printf("Username troppo lungo! Usa al massimo %d caratteri.\n", USER_SIZE);
                formatoErrato++;
                break;
            }
            strcat(comando, parola);
            strcat(comando, " ");    
        } else if(num_parole == 2) { //La seconda parola che segue il comando rappresenta la password
            if(strlen(parola) > PASS_SIZE) {//La password inserita supera il numero massimo di caratteri
                printf("Password troppo lunga! Usa al massimo %d caratteri.\n", PASS_SIZE);
                formatoErrato++;
                break;
            }
            strcat(comando, parola);
            strcat(comando, "\n");
        } 
        parola = strtok(NULL, DELIM);
    }
        
    
    //Se username e/o password sono della giusta lunghezza ma le parole che seguono il comando
    //non sono due e' necessario chiedere al client di reinserire il comando.
    //Altrimenti il comando viene inviato al server.
    if(formatoErrato == 0) {
        if(num_parole != 2) 
            printf("Sono richiesti due parametri: username e password\n");
        else 
            inviaComando(comando);
        
    }   
}

//Funzione invocata quando il client digita il comando !invia_giocata.
//Invia al server la giocata caratterizzata dalla schedina inserita di seguito al comando.
void inviaGiocata(char* parola) {
    char comando[BUFFER_SIZE];
	int i;
    int num_parole = 0;
    int numeri_giocati = 0;
    int num_importi = 0;
    double scommessa = 0;
	int giocati[10];
    bool ruote, numeri, importi;
    
    //Posso inviare il comando solo se ho effettuato il login
    if(strcmp(sessionID, "----------") == 0){
        printf("Devi prima effettuare il login!\n");
        return;
    } 
    
    strcpy(comando, "!invia_giocata ");
    
    /*Scorro il buffer parola per parola*/
    parola = strtok(NULL, DELIM);
    ruote = true;
    while(parola != NULL) {
        num_parole++;
        if(num_parole == 1) {
            if(ruote == true){
                if(strcmp(parola, "-r") != 0) { //La prima parola che segue il comando deve essere -r
                    printf("L'opzione -r deve precedere l'elenco delle ruote\n");
                    return;
                } else {
                    strcat(comando, parola);
                    strcat(comando, " ");
                }
            } 
        } else if(num_parole == 2) {
            if(ruote == true) {
                if(nonRuota(parola)) { //Dopo l'opzione -r deve esserci almeno una ruota
                    printf("L'opzione -r deve essere seguita da almeno una ruota\n");
                    return;
                } else {
                    strcat(comando, parola);
                    strcat(comando, " ");
                }
            } else if(numeri == true) {
                if(atoi(parola) == 0) { //Dopo l'opzione -n deve esserci almeno un numero
                    printf("L'opzione -n deve essere seguita da almeno un numero\n");
                    return;
                } else if(atoi(parola) < 1 || atoi(parola)>90) { //E' possibile scommettere soltanto su numeri compresi tra 1 e 90
                    printf("Scegli numeri compresi tra 1 e 90!\n");
                    return;                    
                } else {
					for(i = 0; i<numeri_giocati; i++) {
						if(atoi(parola) == giocati[i]) {
							printf("Non puoi giocare piu' volte lo stesso numero\n");
							return;	
						}
					}
					giocati[numeri_giocati] = atoi(parola);
                    numeri_giocati++;
                    strcat(comando, parola);
                    strcat(comando, " ");
                }
            }
        } else {
            if(ruote == true) {
                if(nonRuota(parola)) {
                    if(strcmp(parola, "-n") == 0) { //Dopo la lista di ruote deve essere digitata l'opzione -n
                        ruote = false;
                        numeri = true;
                        num_parole = 1;
                        strcat(comando, parola);
                        strcat(comando, " ");
                    } else {
                        printf("L'opzione -n deve precedere i numeri giocati\n");
                        return;
                    }
                } else {
                    strcat(comando, parola);
                    strcat(comando, " ");
                }
            } else if(numeri == true) {
                if(atoi(parola) == 0) { //Dopo la lista di numeri deve essere digitata l'opzione -i
                    if(strcmp(parola, "-i") == 0) {
                        numeri = false;
                        importi = true;
                        num_parole = 2;
                        strcat(comando, parola);
                        strcat(comando, " ");
                    } else {
                        printf("L'opzione -i deve precedere gli importi per tipologia di giocata\n");
                        return;
                    }
                } else if(atoi(parola) < 1 || atoi(parola) > 90) {
                    printf("Scegli numeri compresi tra 1 e 90!\n");
                    return;    
                } else if(numeri_giocati >= 10){ 
                    printf("Puoi scommettere su al massimo 10 numeri!\n");
                    return;                    
                } else {
		        	for(i = 0; i<numeri_giocati; i++) {
						if(atoi(parola) == giocati[i]) {
							printf("Non puoi giocare piu' volte lo stesso numero\n");
							return;	
						}
					}
					giocati[numeri_giocati] = atoi(parola);
                    numeri_giocati++;
                    strcat(comando, parola);
                    strcat(comando, " ");
                }
            } else if(importi == true) {
                if(atof(parola) == 0 && (strcmp(parola, "0") != 0)) { //Dopo l'opzione -i deve essere indicato almeno un importo
                    printf("L'opzione -i deve essere seguita da almeno un numero\n");
                    return;
                } else { 
                    num_importi++;
                    scommessa = scommessa + atof(parola);
                    strcat(comando, parola);
                    strcat(comando, " ");
                }
            }    
        }
        parola = strtok(NULL, DELIM);
    }
    
    if(num_parole < 3) { //Se il numero di parole e' minore di 3 sicuramente il formato del comando e' errato
        printf("Formato errato\n");
    } else if(num_importi > numeri_giocati) { //Non si puo' scommettere sull'uscita di piu' numeri di quanti se ne siano giocati
        printf("Le tipologie di giocata non possono superare la quantita' di numeri giocati\n");
    } else if(scommessa < 1 || scommessa > 200) { //Non si possono giocare piu' di 200 euro o meno di 1 euro per una schedina
        printf("Per ogni schedina puoi giocare minimo 1 euro, massimo 200 euro\n");
    } else { //Altrimenti il comando e' corretto e puo' essere inviato al server
        strcat(comando, "\n");
        strcat(comando, sessionID);
        strcat(comando, "\n");
        inviaComando(comando);
    }
}

//Funzione invocata quando il client digita il comando !vedi_estrazione
//Richiede al server le ultime estrazioni sulla ruota eventualmente indicata di seguito
void vediEstrazione(char* parola) {
    char comando[BUFFER_SIZE];
    int num_parole = 0;
    
    //Posso inviare il comando solo se ho effettuato il login
    if(strcmp(sessionID, "----------") == 0){
        printf("Devi prima effettuare il login!\n");
        return;
    } 
    
    strcpy(comando, "!vedi_estrazione ");
    
    /*Scorro il buffer parola per parola*/
    parola = strtok(NULL, DELIM);
    while(parola != NULL) {
        num_parole++;
        if(num_parole == 1) { //La prima parola dopo il comando indica quante estrazioni vogliono essere visualizzate
            if(atoi(parola) == 0) {
                printf("Indica quante estrazioni vuoi visualizzare!\n");
                return;
            } else if(atoi(parola) > 10) { //Il numero di estrazioni che si vogliono visualizzare deve essere minore del massimo
                printf("Indica un numero minore o uguale a 10\n");
                return;
            } else
                strcat(comando, parola);
        } else if(num_parole == 2) { //La seconda parola dopo il comando (se presente) deve corrispondere a una delle ruote
            if(nonRuota(parola)) {
                printf("Il secondo parametro deve essere una delle ruote\n");
                return;
            } else {
                strcat(comando, " ");
                strcat(comando, parola);
            }
        }
        parola = strtok(NULL, DELIM);
    }
    
    //Se il comando e' seguito da un numero di parole diverso da 1 o 2, il client dovra' reinserire il comando con un formato corretto
    if(num_parole != 1 && num_parole != 2) {
        printf("Il comando richiede il numero di estrazioni e per quale ruota (opzionale)\n");
        return;
    }
    
    //Altrimenti il comando puo' essere inviato al server
    strcat(comando, "\n");
    strcat(comando, sessionID);
    strcat(comando, "\n");
    inviaComando(comando);
}


//Funzione invocata quando il client digita il comando !vedi_giocate
//Richiede al server giocate passate oppure attive in base al valore del parametro tipo digitato di seguito al comando.
void vediGiocate(char* parola) {
    char comando[BUFFER_SIZE];
    
    //Posso inviare il comando solo se ho effettuato il login
    if(strcmp(sessionID, "----------") == 0){
        printf("Devi prima effettuare il login!\n");
        return;
    } 
    
    parola = strtok(NULL, DELIM);
    
    //Il comando deve essere seguito dal tipo, che puo' assumere i valori 0 o 1
    if(parola == NULL){
        printf("Inserisci il tipo di giocate da visualizzare!\n");
        return;
    } 
    if((strcmp(parola, "1")!=0) && (strcmp(parola, "0")!=0)) {
        printf("Il parametro tipo puo' assumere solo due valori: 0 o 1\n");
        return;
    }
    
    //Se il formato e' corretto, il comando puo' essere inviato al server
    sprintf(comando, "!vedi_giocate %s\n%s\n", parola, sessionID);
    inviaComando(comando);
}

//Funzione invocata quando il client digita il comando !vedi_vincite
//Richiede al server un consuntivo delle vincite del client.
void vediVincite() {
    char comando[40];
    
        
    //Posso inviare il comando solo se ho effettuato il login
    if(strcmp(sessionID, "----------") == 0){
        printf("Devi prima effettuare il login!\n");
        return;
    } 
    
    //Se il cliente ha effettuato il login, il comando viene inviato al server
    sprintf(comando, "!vedi_vincite\n%s\n", sessionID);
    inviaComando(comando);
}


//Funzione invocata quando il client digita il comando !esci
//Disconnette il client dal server e termina il client.
void esci() {
    char comando[40];
    sprintf(comando, "!esci\n%s\n", sessionID);
    inviaComando(comando);
}

//Attende che l'utente digiti un comando e invoca la rispettiva funzione di gestione
void leggiComando() {
    char* parola;
    
    printf("> ");
    
    /*Attendo input da tastiera*/
    fgets(buffer, BUFFER_SIZE, stdin);
    
    /*Estraggo la prima parola digitata cosi' da poter discriminare i vari comandi*/
    parola = strtok(buffer, DELIM);
    if(strcmp(parola, "!signup") == 0) {
        signUp(parola);
    } else if(strcmp(parola, "!login") == 0) {
        login(parola);
    } else if(strcmp(parola, "!invia_giocata") == 0) {
        inviaGiocata(parola);
    } else if(strcmp(parola, "!vedi_giocate") == 0) {
        vediGiocate(parola);
    } else if(strcmp(parola, "!vedi_estrazione") == 0) {
        vediEstrazione(parola);
    } else if(strcmp(parola, "!vedi_vincite") == 0) {
        vediVincite();
    } else if(strcmp(parola, "!esci") == 0) {
        esci();
    } else if(strcmp(parola, "!help") == 0) {
        help(parola);
    } else {
        printf("Comando non riconosciuto. ");
        messaggioDiHelp();
    }
}

int main(int argc, char* argv[]){
    //Controllo parametri
    if(argc != 3) {
        fprintf(stderr, "Usa: ./lotto_client <indirizzo> <porta>\n");
        exit(1);
    }
    
    //Crea il socket TCP collegandosi al server con indirizzo passato come primo parametro 
    //e alla porta passata come secondo parametro
    creaSocketTCP(argv[1], atoi(argv[2]));
    
    //Se la connessione e' riuscita viene stampato il messaggio contenente il riepilogo dei comandi disponibili
    if(attivo == true)
        messaggioDiHelp();
    
    //Finche' il client e' attivo legge ricorsivamente i comandi dallo standard input
    while(attivo){
        leggiComando();
    }
    
    close(sd);     
    return 0;
}

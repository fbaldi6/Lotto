# Lotto
Applicazione distribuita client-server in C che riproduce il gioco del Lotto, sviluppata per l'esame di Reti Informatiche del Corso di Laurea di Ingegneria Informatica dell'Università di Pisa.

## Implementazione
Il server consente ai client di fare giocate per la prossima estrazione prevista, visualizzare le ultime giocate e le ultime vincite. Il server è anche in grado di gestire richieste concorrenti. 
D’altra parte, il client permette di inviare le giocate e utilizzare le altre funzionalità descritte nel seguito.

### Server
Il server **lotto_server** riceve le richieste dai vari client e le serve, eseguendo il comando ricevuto. Il server gestisce la concorrenza tramite l’approccio **multi-processo**. Il server è mandato in esecuzione come segue:

`./lotto_server <porta> <periodo>`

dove:

`<porta>` è la porta su cui il server è in ascolto. Il server è raggiungibile da tutti gli indirizzi IP della macchina su cui è in esecuzione;

`<periodo>` parametro opzionale che specifica il tempo fra un’estrazione e la successiva. Se non si specifica il parametro, le estrazioni sono effettuate ogni 5 minuti.

Una volta eseguito, lotto_server mostra a video le informazioni sullo stato (creazione socket di ascolto, connessioni ricevute, operazioni richieste dai client ecc.). Per ogni client, il server memorizza su un file locale le informazioni sulle giocate e sulle vincite, in modo da poterle inviare al client quando richieste.

#### Dettaglio dei comandi
I comandi accettati incondizionatamente dal server sono:

`!signup username password`

`!login username password`

Dopo il login, il server accetta i comandi per le giocate di seguito, solo se inviati tramite messaggi contenenti un session id valido:

`!invia_giocata schedina`

`!vedi_giocate tipo`

`!vedi_estrazione n ruota`

`!vedi_vincite`

`!esci`

### Client
Il client si avvia con la seguente sintassi:

`./lotto_client <IP server> <porta server>`

dove:

`<IP server>` è l’indirizzo dell’host su cui è in esecuzione il server;
`<porta server>` è la porta su cui il server è in ascolto;

#### Dettaglio comandi
All’avvio, i comandi disponibili sono:
* `!help <comando`
* `!signup <username> <password>`
* `!login <username> <password>`
* `!invia_giocata <schedina>`
* `!vedi_giocate <tipo>`
* `!vedi_estrazione <n> <ruota>`
* `!vedi_vincite`
* `!esci`

Il client mostra messaggi relativi agli errori che si verificano durante l’esecuzione. I messaggi spegano brevemente la natura dell’errore. 

### Requisiti
* Client e server si scambiano dati tramite socket TCP. Prima di ogni scambio, il ricevente deve essere informato su quanti byte deve leggere dal socket. Non possono essere inviati numeri arbitrari di byte sui socket.

* Per gestire le schedine, le giocate e le vincite, è possibile utilizzare le strutture dati a piacere. Gli aspetti che non sono dettagliatamente specificati in questo documento possono essere implementati liberamente.

* Usare gli autotools (comando make) per la compilazione del progetto.

* Il codice deve essere indentato e commentato in ogni sua parte: significato delle variabili, descrizione delle strutture dati introdotte, processazioni e così via. I commenti possono essere evitati nelle parti banali del codice.

* Il salvataggio delle informazioni di giocate, vincite ed estrazioni sui file registro è opzionale. Nel caso si decida di non implementarlo, tali informazioni sono semplicemente mantenute in memoria.




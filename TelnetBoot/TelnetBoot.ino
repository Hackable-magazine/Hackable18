#include <SPI.h>
#include <Ethernet.h>

// port IP à tester
#define TESTPORT 22
// broche de la led relais/erreur
#define RLEDPIN 7
// broche de la led online
#define OLEDPIN 6
// broche de test +5V
#define TESTPIN 5
// broche du relais
#define RELAIS A0
// temps de pression
#define RELDELAY 500
// taille max de commande
#define BUFSIZE 16

// Adresse MAC
byte mac[] = { 0x90, 0xA2, 0xDA, 0x0F, 0x0F, 0xA0};
// Adresse IP du PC à contrôler
byte testip[] = { 192, 168, 10, 166 };

// hôte distant (cible)
EthernetClient testclient;
// hôte distant (client telnet)
EthernetClient client;

// serveur telnet
EthernetServer server(23);

// Affichage IP
void printIPAddress(){
  Serial.print("Mon adresse est ");
  for (byte i = 0; i < 4; i++) {
    Serial.print(Ethernet.localIP()[i], DEC);
    Serial.print(".");
  }
  Serial.println();
}

// Test de connexion
bool isOnline(int essais, int msec) {
  int i = 0;
  client.println("Test du +5V sur la machine");
  // si on ne détecte pas de 5V on ne va pas plus loin
  if(digitalRead(TESTPIN))
    return 0;
  client.print("Test de la machine sur le port ");
  client.print(TESTPORT);
  // on boucle autant de d'essais demandés
  while(i < essais) {
    client.print(".");
    if(testclient.connect(testip, TESTPORT)==1) {
      // le serveur répond
      Serial.println("Connexion ok");
      testclient.stop();
      client.println();
      // on s'arrête mà
      return 1;
    } else {
      // essai suivant après pause
      Serial.print("Erreur ");
      Serial.println(i);
      delay(msec);
      i++;
    }
  }
  client.println();
  // On arrive ici uniquement si le nombre
  // d'essais est dépassé
  return 0;
}

void setup() {
  digitalWrite(RELAIS, HIGH);
  pinMode(RELAIS, OUTPUT);
  
  pinMode(RLEDPIN, OUTPUT);
  pinMode(OLEDPIN, OUTPUT);
  pinMode(TESTPIN, INPUT);
  
  Serial.begin(115200);
  Serial.println("Demande DHCP...");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Erreur de configuration via DHCP");
    while (1) {
      // erreur DHCP
      digitalWrite(RLEDPIN, HIGH);
      delay(250);
      digitalWrite(RLEDPIN, LOW);
      delay(250);
    }
  }
  printIPAddress();
  server.begin();
  digitalWrite(OLEDPIN, HIGH);
}

void loop() {
  // stockage des données reçues
  char clientline[BUFSIZE];

  // recupération de la connexion d'un client
  client = server.available();

  // on a un client ?
  if (client && client.connected()) {
    // oui. Combien d'octets à lire ?
    int toread=client.available();
    if(toread > 0) {
      // plus d'octet que la taille dispo ?
      if(toread>=BUFSIZE) {
        // trop long !
        while(client.available()) {
          // on lit sans rien garder
          client.read();
        }
      } else {
        // effactement du tampon
        memset(clientline, 0, sizeof(clientline));
        // lecture des données
        client.read(clientline, toread);
        // conversion en chaîne
        String commande = String(clientline);
        // Suppression des blancs et CRLF en fin
        commande.trim();
        // Affichage de l'ordre reçu
        Serial.print("Commande lue: [");
        Serial.print(commande);
        Serial.println("]");

        // ordre vide après suppression CRLF ou espace ?
        if(commande.length()==0) {
        } else if(commande.equals("bye") || commande.equals("quit")) {
          // fin de connexion
          client.println("\e[1;32mBye.\e[0m");
          client.flush();
          client.stop();
          Serial.println("Client quit");
        } else if(commande.equals("sta") || commande.equals("status")) {
          if(isOnline(2,1000))
            // test de connexion réussi
            client.println("\e[1;32mMachine en fonction.\e[0m");
          else
            client.println("\e[1;31mMachine silencieuse!\e[0m");
        } else if(commande.equals("boot")) {
          // teste de connexion
          if(isOnline(1,1)) {
            client.println("\e[1;31mMachine deja en fonction!\e[0m");
          } else {
            // l'hôte ne répond pas il doit être éteint
            client.println("\e[1;32mMise en route de la machine.\e[0m");
            // allumage 
            digitalWrite(RELAIS, LOW);
            digitalWrite(RLEDPIN, HIGH);
            delay(RELDELAY);
            digitalWrite(RELAIS, HIGH);
            digitalWrite(RLEDPIN, LOW);
          }
        } else if(commande.equals("bootw")) {
          if(isOnline(1,1)) {
            client.println("\e[1;31mMachine deja en fonction!\e[0m");
          } else {
            // Allumage
            client.println("\e[1;32mMise en route de la machine.\e[0m");
            digitalWrite(RELAIS, LOW);
            digitalWrite(RLEDPIN, HIGH);
            delay(RELDELAY);
            digitalWrite(RELAIS, HIGH);
            digitalWrite(RLEDPIN, LOW);
            client.println("Attente de la mise en route de la machine");
            delay(3000);
            // vérification du démarrage par connexion
            if(isOnline(5,1000)) {
              client.println("\e[1;32mOK. Machine en fonction.\e[0m");
            } else {
              client.println("\e[1;31mErreur! La machine est silencieuse!\e[0m");
            }
          }
        } else {
          // commande inconnue
          client.println("Hein?");
        }
      }
      // invite de commandes
      client.print("mon> ");
    }
  }

  // gestion du bail DHCP
  switch (Ethernet.maintain()) {
    case DHCP_CHECK_RENEW_FAIL:
      Serial.println("Error lors du renouvellement!");
      digitalWrite(OLEDPIN, LOW);
      break;
    case DHCP_CHECK_RENEW_OK:
      Serial.println("Renouvellement ok");
      printIPAddress();
      digitalWrite(OLEDPIN, HIGH);
      break;
    case DHCP_CHECK_REBIND_FAIL:
      Serial.println("Error lors de la demande");
      digitalWrite(OLEDPIN, LOW);
      break;
    case DHCP_CHECK_REBIND_OK:
      Serial.println("Demande ok");
      printIPAddress();
      digitalWrite(OLEDPIN, HIGH);
      break;
    default:
      // rien
      break;
  }
}


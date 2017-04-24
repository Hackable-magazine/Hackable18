#include <VGAX.h>

// Objet représentant l'écran
VGAX vga;

void setup() {
  // initialisation
  vga.begin();
}

// Compteur
uint8_t cnt;

void loop() {
  // affichage d'un pixel aléatoire
  vga.putpixel(rand()%VGAX_WIDTH, rand()%VGAX_HEIGHT, cnt%4);
  cnt++;
}

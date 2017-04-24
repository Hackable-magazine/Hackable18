#include <VGAX.h>
// fichier de police
#include "font.h"

// objet écran
VGAX vga;

// notre chaîne de caractères
static const char str0[] PROGMEM="VGA AVEC ARDUINO\nDANS HACKABLE 18";

void setup() {
  // initialisation
  vga.begin();
  // effacement écran
  vga.clear(0);

  // affichage du texte
  vga.printPROGMEM(
    (byte*)fnt_nanofont_data,            // police
    FNT_NANOFONT_SYMBOLS_COUNT,          // nombre de symboles
    FNT_NANOFONT_HEIGHT,                 // hauteur caractères
    2,                                   // séparation horizontale
    1,                                   // séparation verticale
    str0,                                // chaîne à afficher
    0,                                   // emplacement X
    VGAX_HEIGHT/2-FNT_NANOFONT_HEIGHT/2, // emplacement Y
    3);                                  // couleur
}

void loop() {
  vga.delay(100);
}

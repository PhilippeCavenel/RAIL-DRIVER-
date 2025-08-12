# Optimisation de l’UART sur PIC18F4680 — Note d’implémentation

**Projet** : Console UART + Bus ECAN  
**Cible** : Microchip PIC18F4680 @ `_XTAL_FREQ = 32 MHz`  
**Auteur** : (vous) + patch proposé par l’assistant  
**Date** : (à compléter)

---

## 1) Contexte et problème initial
- Le code d’origine utilise une émission UART **bloquante** : la fonction `sendUSART()` attend successivement `TRMT` puis `TXIF` à **chaque octet** avant de continuer.  
- L’application appelle fréquemment `printf` et renvoie un **echo** caractère par caractère, ce qui **surcharge** l’UART et **bloque** le CPU.  
- Le générateur de baud est configuré en **8 bits** (`BRG16=0`), ce qui **limite la précision** et les débits atteignables proprement à 32 MHz.

**Effets observés** : interface console “lente”, goulots d’étranglement, CPU occupé à attendre plutôt qu’à traiter (et potentiellement moins de marge pour le CAN).

---

## 2) Objectifs
1. **Supprimer le TX bloquant** : remplacer le “busy-wait” par un envoi **piloté par interruption** avec **buffer circulaire**.  
2. **Améliorer la précision du baud rate** : activer le **BRG 16 bits** pour réduire l’erreur et permettre des débits plus élevés (115200 / 230400).  
3. **Préserver la priorité du bus CAN** : faire passer l’**UART TX en basse priorité** d’interruption, laisser **CAN (et UART RX) en haute**.

---

## 3) Changements principaux (résumé)
- **BRG 16 bits** : `BAUDCONbits.BRG16 = 1` et calcul de `SPBRGH:SPBRG` via la formule **Fosc / (4 * (SPBRG+1))** (avec `BRGH=1`).  
- **TX non bloquant** : ajout d’un **buffer circulaire** (`gl_txBuf`) côté émission. `sendUSART()` **met en file** l’octet et **active** `TXIE`.  
- **ISR TX courte** : l’IT `TXIF` charge **un seul** octet de `TXREG` puis ressort. Quand le buffer est vide ⇒ **désactivation** de `TXIE`.  
- **Priorités** : activation des niveaux d’interruptions, **UART TX en low priority**, **CAN RX (et optionnellement UART RX) en high priority**.  
- **Variante retenue** : `sendUSART()` **ne bloque jamais**. Si le buffer est plein, l’octet est **droppé** et un compteur `gl_txOverruns` est **incrémenté**.

---

## 4) Détails d’implémentation

### 4.1) Nouvelles données globales
```c
#define TXBUFFERSIZE 128

static volatile unsigned char  gl_txBuf[TXBUFFERSIZE];
static volatile unsigned char  gl_txHead = 0;
static volatile unsigned char  gl_txTail = 0;
static volatile unsigned int   gl_txOverruns = 0; // octets perdus (buffer plein)
```

### 4.2) Initialisation UART (extrait)
```c
TXSTAbits.SYNC = 0;     // Async
TXSTAbits.BRGH = 1;     // High speed
RCSTAbits.SPEN = 1;     // Enable serial port
RCSTAbits.CREN = 1;     // RX enable
BAUDCONbits.BRG16 = 1;  // BRG 16 bits

// Calcule SPBRGH:SPBRG pour Fosc=32 MHz, _BAUD (ex. 115200)
unsigned int spbrg = (_XTAL_FREQ / (4UL * _BAUD)) - 1U;
SPBRGH = (spbrg >> 8) & 0xFF;
SPBRG  = (spbrg     ) & 0xFF;

TXSTAbits.TXEN = 1;     // TX ON (après SPBRG)
```

### 4.3) Priorités d’interruptions
```c
// Activer les niveaux de priorité
RCONbits.IPEN = 1;
INTCONbits.GIEH = 1;  // high global
INTCONbits.GIEL = 1;  // low global

// Priorités : CAN et UART RX en high, UART TX en low
IPR1bits.RCIP  = 1;   // UART RX high (optionnel mais conseillé)
IPR1bits.TXIP  = 0;   // UART TX low
IPR3bits.RXB0IP = 1;  // CAN RXB0 high
IPR3bits.RXB1IP = 1;  // CAN RXB1 high
```

### 4.4) Envoi non bloquant
```c
void sendUSART(unsigned char data){
    unsigned char next;
    if (gl_master==FALSE) return;

    next = (unsigned char)(gl_txHead + 1);
    if (next >= TXBUFFERSIZE) next = 0;

    if (next == gl_txTail){
        // Buffer plein: on droppe et on compte
        gl_txOverruns++;
        return;
    }
    gl_txBuf[gl_txHead] = data;
    gl_txHead = next;

    PIE1bits.TXIE = 1; // Assure l’armement de l’IT TX
}
```

### 4.5) Interruptions
- **ISR haute priorité** : traite CAN (et UART RX si choisi).  
- **ISR basse priorité** : **UART TX uniquement**.

Exemple `low_isr()` :  
```c
#pragma interruptlow low_isr
void low_isr(void){
    // UART TX interrupt: envoi du prochain octet si dispo
    if (PIE1bits.TXIE && PIR1bits.TXIF){
        if (gl_txHead != gl_txTail){
            TXREG = gl_txBuf[gl_txTail++];
            if (gl_txTail >= TXBUFFERSIZE) gl_txTail = 0;
        } else {
            PIE1bits.TXIE = 0; // plus rien à envoyer
        }
    }
}

#pragma code low_vector=0x18
void low_vector(void){
    _asm goto low_isr _endasm
}
#pragma code
```

---

## 5) Paramétrage registres — synthèse
- **Baud / Horloge** : `BRG16=1`, `BRGH=1`; `SPBRGH:SPBRG = (Fosc/(4*BAUD))-1`.  
- **UART** : `SPEN=1`, `CREN=1`, `TXEN=1`.  
- **Interruptions** : `IPEN=1`, `GIEH=1`, `GIEL=1`.  
- **Priorités** : `TXIP=0` (bas), `RCIP=1` (haut, optionnel), `RXB0IP=1`, `RXB1IP=1`.  
- **Masques** : `PIE1bits.RCIE=1` (RX), `PIE1bits.TXIE` **activé dynamiquement** par `sendUSART()`.

---

## 6) Impact sur le bus CAN
- **Modules indépendants** : EUSART et ECAN ne se perturbent pas.  
- **TX low-priority + ISR courte** : l’UART ne **préempte pas** la réception CAN.  
- **RX CAN vidé rapidement** : les ISR hautes priorité conservent la réactivité nécessaire.

Conclusion : **pas d’impact négatif** sur le CAN, et même plus de marge CPU car on supprime le busy-wait UART.

---

## 7) Variantes et réglages
- **Taille du buffer TX** : `TXBUFFERSIZE` par défaut 128. Adapter selon trafic.  
- **Politique en cas de plein** :  
  - Variante 1 (retenue) : **drop + compteur** (`gl_txOverruns`).  
  - Variante 2 : **attente active** (bloquante) jusqu’à la place libre — déconseillée si CAN prioritaire.  
- **Débit** : 115200 sûr; **230400** possible si le host et le câblage le supportent (BRG16 garde une erreur faible).

---

## 8) Intégration pas à pas
1. Remplacer vos `main.c` / `main.h` par la variante **low priority** fournie (`main_uart_lowpri.c/.h`).  
2. Vérifier qu’aucun autre module ne définit le **vecteur low priority** `0x18`.  
3. Recompiler, flasher.  
4. Optionnel : **désactiver l’écho** caractère par caractère si la console reste bruyante.  
5. Monitorer `gl_txOverruns` pour dimensionner correctement `TXBUFFERSIZE`.

---

## 9) Tests de validation (checklist)
- [ ] À 115200 bauds, envoi d’un flux texte long **sans blocage** visible du main.  
- [ ] **CAN** : réception/transmission inchangée sous charge (rafale de trames).  
- [ ] `gl_txOverruns == 0` en régime nominal (ou faible et acceptable).  
- [ ] Passage à **230400** (si support PC) : console stable, pas d’erreurs apparentes.  
- [ ] Echo optionnel : pas d’influence notable sur la latence CAN.

---

## 10) Dépannage
- **Baud incorrect** : vérifier `_XTAL_FREQ`, `_BAUD`, et la formule `SPBRGH:SPBRG`.  
- **Pas de TX** : `TXEN=1`, `SPEN=1`, `GIEL=1`, `PIE1bits.TXIE` activé après `sendUSART()`.  
- **Overflow CAN** : revoir priorités (CAN en high), réduire la charge console, couper l’écho.  
- **Beaucoup d’overruns** : augmenter `TXBUFFERSIZE`, réduire la verbosité, baisser BAUD si nécessaire, ou mettre une file d’attente côté application.

---

## 11) Formules baud & erreurs (rappel)
En mode `BRGH=1`, `BRG16=1` :  
- **SPBRG = Fosc / (4 * Baud) - 1**  
- **Baud réel = Fosc / (4 * (SPBRG + 1))**  
- **Erreur (%) ≈ 100 * (Baud_réel - Baud_ciblé) / Baud_ciblé**

Pour **Fosc = 32 MHz, Baud = 115200** :  
`SPBRG ≈ 32e6 / (4 * 115200) - 1 ≈ 69.4 → 69`  
Baud réel ≈ `32e6 / (4 * (69+1)) = 114285.7` (erreur ≈ −0,79 %), **nettement mieux** que BRG 8 bits.

---

## 12) Extraits utiles
**Déclenchement TX à la volée** :  
```c
gl_txBuf[gl_txHead] = data;
gl_txHead = next;
PIE1bits.TXIE = 1; // démarre / continue l’émission
```

**Vidage du buffer en IT** :  
```c
if (gl_txHead != gl_txTail){
    TXREG = gl_txBuf[gl_txTail++];
    if (gl_txTail >= TXBUFFERSIZE) gl_txTail = 0;
} else {
    PIE1bits.TXIE = 0;
}
```

---

## 13) Fichiers livrés
- **Variante standard (TX même priorité)** : `main_uart_nonblocking.c`, `main_uart_nonblocking.h`  
- **Variante recommandée (TX basse priorité)** : `main_uart_lowpri.c`, `main_uart_lowpri.h`

---

## 14) Historique (à compléter)
- v1.0 — Passage BRG16, TX non bloquant, TX low priority, compteur d’overruns.
- v1.1 — (exemple) Ajustement taille buffer, suppression echo.

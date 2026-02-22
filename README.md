# Battleship (C)
Proiectul reprezinta o implementare în consola a jocului clasic Battleship, realizata în limbajul C.

STRUCTURA CODULUI

Programul reprezinta tabla de joc printr-o matrice bidimensionala. 
Acesta contine functii care au rolul de a initializa tabla de joc, mai exact de a plasa navele si a verifica daca pozitia respectiva este valida.
De asemenea, are functii pentru gestionarea atacurilor, functii care verifica daca o nava a fost distrusa si are o functie pentru determinarea conditiei de castig.
Functiile au fost folosite pentru a evita duplicarea codului si pentru a permite refractorizarea ulterioară în mai multe fișiere sursă.

## Structură
.
├── include/
│   └── battleship.h
├── src/
│   ├── battleship.c
│   └── main.c
└── Makefile


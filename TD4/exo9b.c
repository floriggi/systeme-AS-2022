#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX 100

void erreur(const char *message) {
    printf("Erreur durant : %s\n", message);
    exit(EXIT_FAILURE);
}

int ouvrirFichier(char *nom) {
    int fd;
    if ((fd = open(nom, O_RDONLY)) < 0) {
        erreur("ouverture fichier");
    }

    return fd;
}

void fermerFichier(int fichier) {
    close(fichier);
}

void effacerFichier(char const *nomFichier) {
    if (unlink(nomFichier) != 0) {
        erreur("effacer fichier");
    }
}

int main(int argc, char const *argv[]) {
    int fichier;
    char *nomFifo = "/tmp/fifo";
    char buffer[MAX];

    printf("Exercice 9 - 2\n");

    fichier = ouvrirFichier(nomFifo);
    read(fichier, buffer, MAX);

    printf("Message recu : %s\n", buffer);

    fermerFichier(fichier);
    effacerFichier(nomFifo);

    return EXIT_SUCCESS;
}

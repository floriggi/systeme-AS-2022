#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX 100

void erreur(const char *message) {
    printf("Erreur durant : %s\n", message);
    exit(EXIT_FAILURE);
}

void lireMessage(char tampon[]) {
    printf("Saisir un message à envoyer :\n");
    fgets(tampon, MAX, stdin);
}

void creerFichier(const char *nom) {
    if (mkfifo(nom, 0644) != 0) {
        erreur("mkfifo");
    }
}

int ouvrirFichier(char *nom) {
    int fd;
    if ((fd = open(nom, O_WRONLY)) < 0) {
        erreur("ouverture fichier");
    }

    return fd;
}

void fermerFichier(int fichier) {
    close(fichier);
}

int main(int argc, char const *argv[]) {
    int fichier;
    char *nomFifo = "/tmp/fifo";
    char buffer[MAX];

    printf("Exercice 9 - 1\n");

    creerFichier(nomFifo);
    fichier = ouvrirFichier(nomFifo);

    lireMessage(buffer);

    write(fichier, buffer, strlen(buffer) + 1);

    fermerFichier(fichier);

    return EXIT_SUCCESS;
}

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX 100
#define NB_TOUR 4

void erreur(const char *message) {
    printf("Erreur durant : %s\n", message);
    exit(EXIT_FAILURE);
}

void lireMessage(char tampon[]) {
    printf("Saisir un message à envoyer :\n");
    fgets(tampon, MAX, stdin);
}

int ouvrirFichierEcriture(char *nom) {
    int fd;
    if ((fd = open(nom, O_WRONLY)) < 0) {
        erreur("ouverture fichier ecriture");
    }

    return fd;
}

int ouvrirFichierLecture(char *nom) {
    int fd;
    if ((fd = open(nom, O_RDONLY)) < 0) {
        erreur("ouverture fichier lecture");
    }

    return fd;
}

void fermerFichier(int fichier) {
    close(fichier);
}

int fichierExiste(char const *nomFichier) {
    struct stat buffer;
    return (stat(nomFichier, &buffer) == 0);
}

void effacerFichier(char const *nomFichier) {
    if (fichierExiste(nomFichier)) {
        if (unlink(nomFichier) != 0) {
            erreur("effacer fichier");
        }
    }
}

int main(int argc, char const *argv[]) {
    int compteur = 0;
    int fichierDepuis1;
    int fichierVers1;
    char *nomFifoDepuis1 = "/tmp/fifoVers2";
    char *nomFifoVers1 = "/tmp/fifoVers1";
    char buffer[MAX];

    printf("Exercice 10 - 2\n");

    fichierDepuis1 = ouvrirFichierLecture(nomFifoDepuis1);
    fichierVers1 = ouvrirFichierEcriture(nomFifoVers1);

    int nbCaracteresRecus = 0;

    for (compteur = 0; compteur < NB_TOUR; compteur++) {
        nbCaracteresRecus = read(fichierDepuis1, buffer, MAX);
        buffer[nbCaracteresRecus] = 0;

        printf("Message recu : %s\n", buffer);

        lireMessage(buffer);
        write(fichierVers1, buffer, strlen(buffer) + 1);
    }

    fermerFichier(fichierDepuis1);
    fermerFichier(fichierVers1);
    effacerFichier(nomFifoVers1);
    effacerFichier(nomFifoDepuis1);

    printf("Fin du programme\n");

    return EXIT_SUCCESS;
}

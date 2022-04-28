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

void creerFichier(const char *nom) {
    if (mkfifo(nom, 0644) != 0) {
        erreur("mkfifo");
    }
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
    return (stat (nomFichier, &buffer) == 0);
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
    int fichierVers2;
    int fichierDepuis2;
    char *nomFifoVers2 = "/tmp/fifoVers2";
    char *nomFifoDepuis2 = "/tmp/fifoVers1";
    char buffer[MAX];

    printf("Exercice 10 - 1\n");

    effacerFichier(nomFifoVers2);
    effacerFichier(nomFifoDepuis2);

    creerFichier(nomFifoDepuis2);
    creerFichier(nomFifoVers2);

    fichierVers2 = ouvrirFichierEcriture(nomFifoVers2);
    fichierDepuis2 = ouvrirFichierLecture(nomFifoDepuis2);

    int nbCaracteresRecus = 0;

    for (compteur = 0; compteur < NB_TOUR; compteur++) {
        lireMessage(buffer);

        write(fichierVers2, buffer, strlen(buffer) + 1);

        nbCaracteresRecus = read(fichierDepuis2, buffer, MAX);
        buffer[nbCaracteresRecus] = 0;
        printf("Message recu : %s\n", buffer);
    }

    fermerFichier(fichierVers2);
    fermerFichier(fichierDepuis2);

    printf("Fin du programme\n");

    return EXIT_SUCCESS;
}

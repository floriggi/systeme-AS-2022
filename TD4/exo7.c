#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int fils1;
int fils2;

void lireMessage(char tampon[]) {
    printf("Saisir un message à envoyer :\n");
    fgets(tampon, 99, stdin);
}

void erreur(const char *message) {
    printf("Erreur durant : %s\n", message);
    exit(EXIT_FAILURE);
}

void ouvrirTube(int tube[]) {
    if (pipe(tube) == -1) {
        erreur("ouverture tube");
    }
}

void fermerTube(int tube[]) {
    if (close(tube[0]) == -1) {
        erreur("fermeture tube 1");
    }
    if (close(tube[1]) == -1) {
        erreur("fermeture tube 2");
    }
}

void ecrireDansTube(const int tube[], int *pid) {
    int taille = sizeof(int);

    if (write(tube[1], pid, taille) < taille) {
        erreur("écriture dans tube");
    }
}

void lireDansTube(const int tube[], int *pid) {
    if (read(tube[0], pid, sizeof(int)) == -1) {
        erreur("lecture dans tube");
    }
}

int main(int argc, char const *argv[]) {
    char tampon[100];
    int tubeVersFils1[2];
    int tubeVersFils2[2];

    ouvrirTube(tubeVersFils1);
    ouvrirTube(tubeVersFils2);

    switch (fils1 = fork()) {
        case -1:
            erreur("fork fils 1");
        case 0:
            lireMessage(tampon);

            write(tubeVersFils2[1], tampon, strlen(tampon) + 1);

            read(tubeVersFils1[0], tampon, 100);
            printf("Je suis le fils 1, j'ai recu : %s\n", tampon);

            fermerTube(tubeVersFils1);
            fermerTube(tubeVersFils2);

            exit(EXIT_SUCCESS);
    }

    switch (fils2 = fork()) {
        case -1:
            erreur("fork fils 2");
        case 0:
            read(tubeVersFils2[0], tampon, 100);
            printf("Je suis le fils 2, j'ai recu : %s\n", tampon);

            sleep(1);

            printf("Je suis le fils 2, j'envoie %s\n", tampon);
            write(tubeVersFils1[1], tampon, strlen(tampon) + 1);

            fermerTube(tubeVersFils1);
            fermerTube(tubeVersFils2);

            exit(EXIT_SUCCESS);
    }

    waitpid(fils1, NULL, 0);
    waitpid(fils2, NULL, 0);

    fermerTube(tubeVersFils1);
    fermerTube(tubeVersFils2);

    printf("Je suis le père. Fin du programme\n");

    return EXIT_SUCCESS;
}

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define TEMPS 15

int fils1;
int fils2;

void erreur(const char *message);
void ouvrirTube(int tube[]);
void fermerTube(int tube[]);
void ecrireDansTube(const int tube[], int *pid);
void lireDansTube(const int tube[], int *pid);
void capturerSignalFils1(const int sig);
void capturerSignalFils2(const int sig);
void attendreSignalFils1();
void attendreSignalFils2();
void envoyerSignal(const int pid, const int numeroSignal);
void envoyerSignalAuFils1();
void envoyerSignalAuFils2();
void tuerFils(int pid);


int main(int argc, char const *argv[]) {
    int tubeVersFils1[2];
    int tubeVersFils2[2];

    ouvrirTube(tubeVersFils1);
    ouvrirTube(tubeVersFils2);

    /***** le code du fils 1 *****/
    switch (fils1 = fork()) {
        case -1:
            printf("Erreur fork. Arret du programme.\n");
            exit(EXIT_FAILURE);
        case 0:
            fermerTube(tubeVersFils2);

            lireDansTube(tubeVersFils1, &fils2);
            printf("Fils 1 a recu le pid : %d\n", fils2);

            fermerTube(tubeVersFils1);

            attendreSignalFils1();
            while (1) {}
            break; //inutile, mais bon...
    }

    /***** le code du fils 2 *****/
    switch (fils2 = fork()) {
        case -1:
            printf("Erreur fork. Arret du programme.\n");
            exit(EXIT_FAILURE);
        case 0:
            fermerTube(tubeVersFils1);

            lireDansTube(tubeVersFils2, &fils1);
            printf("Fils 2 a recu le pid : %d\n", fils1);

            fermerTube(tubeVersFils2);

            attendreSignalFils2();
            while (1) {}
            break; //inutile, mais bon...
    }

    /******** le code du père ********/
    ecrireDansTube(tubeVersFils2, &fils1);
    ecrireDansTube(tubeVersFils1, &fils2);

    sleep(2);

    envoyerSignalAuFils1();

    sleep(TEMPS);

    // on tue les fils
    tuerFils(fils1);
    tuerFils(fils2);

    waitpid(fils1, NULL, 0);
    waitpid(fils2, NULL, 0);

    fermerTube(tubeVersFils1);
    fermerTube(tubeVersFils2);

    printf("Je suis le père. Fin du programme\n");

    return EXIT_SUCCESS;
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
        tuerFils(*pid);
        erreur("écriture dans tube");
    }
}

void lireDansTube(const int tube[], int *pid) {
    if (read(tube[0], pid, sizeof(int)) == -1) {
        erreur("lecture dans tube");
    }
}

void capturerSignalFils1(const int sig) {
    // ping
    printf("Fils 1. Signal %d reçu\n", sig);
    envoyerSignalAuFils2();
}

void capturerSignalFils2(const int sig) {
    // pong
    printf("Fils 2. Signal %d reçu\n", sig);
    envoyerSignalAuFils1();
}

void attendreSignalFils1() {
    if (signal(3, capturerSignalFils1) == SIG_ERR) {
        erreur("attente signal");
    }
    printf("Fils 1, pret\n");
}

void attendreSignalFils2() {
    if (signal(4, capturerSignalFils2) == SIG_ERR) {
        erreur("attente signal");
    }
    printf("Fils 2, pret\n");
}

void envoyerSignal(const int pid, const int numeroSignal) {
    usleep(300 * 1000);     // on attend en microseconde -> 1000 microsecondes = 1 milliseconde
    printf("On envoie le signal %d à %d\n", numeroSignal, pid);
    kill(pid, numeroSignal);
}

void envoyerSignalAuFils1() {
    envoyerSignal(fils1, 3);
}

void envoyerSignalAuFils2() {
    envoyerSignal(fils2, 4);
}

void tuerFils(int pid) {
    envoyerSignal(pid, 9);
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define N 4
#define MAX 100

void erreur(const char *message) {
    printf("Erreur durant : %s\n", message);
    exit(EXIT_FAILURE);
}

void lireMessage(char tampon[]) {
    printf("Saisir un message à envoyer :\n");
    fgets(tampon, 99, stdin);
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

int main(int argc, char const *argv[]) {
    int pid[N];
    int tube[N][2];
    char tampon[MAX];

    for (int i = 0; i < N; i++) {
        ouvrirTube(tube[i]);
    }

    for (int i = 0; i < N; i++) {
        switch (pid[i] = fork()) {
            case -1:
                erreur("fork");
                break;
            case 0:
                printf("Je suis le fils %d, mon pid = %d\n", i + 1, getpid());

                read(tube[i][0], tampon, MAX);
                printf("Je suis le fils %d (%d), j'ai recu : %s\n", i + 1, getpid(), tampon);

                fermerTube(tube[i]);

                exit(EXIT_SUCCESS);
        }
    }

    // ici le père
    sleep(1);

    lireMessage(tampon);

    for (int i = 0; i < N; i++) {
        write(tube[i][1], tampon, strlen(tampon) + 1);
        wait(&pid[i]);
    }

    for (int j = 0; j < N; j++) {
        fermerTube(tube[j]);
    }

    printf("Je suis ton père. Fin du programme.\n");
    return EXIT_SUCCESS;
}

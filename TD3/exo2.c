#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#define N 2

void erreur(const char *message) {
    printf("Erreur durant : %s\n", message);
    exit(EXIT_FAILURE);
}

void afficherPere() {
    printf("Je suis un père ! PID = %d\n", getpid());
    printf("PID de mon pere = %d\n\n", getppid());
}

void afficherFils() {
    printf("Je suis un fils : %d\n", getpid());
    printf("Mon père est : %d\n\n", getppid());
}

void creerFils(const int index) {
    int pid;

    switch (pid = fork()) {
        case -1:
            erreur("fork");
        case 0:
            afficherFils();

            if (index == N) {
                creerFils(0);
            } else {
                exit(EXIT_SUCCESS);
            }
        default:
            break;
    }
}

int main(int argc, char const *argv[]) {
    int i;
    int j;
    int fils1;
    int pidPere = getpid();

    fils1 = fork();

    if (fils1 == -1) {
        erreur("fork fils 1\n");
    }

    if (fils1 == 0) {
        afficherFils();

        for (i = 1; i <= N; i++) {
            creerFils(i);
        }

        for (j = 1; j <= N; ++j) {
            wait(NULL);
        }
    }

    waitpid(fils1, NULL, 0);
    afficherPere();

    return EXIT_SUCCESS;
}

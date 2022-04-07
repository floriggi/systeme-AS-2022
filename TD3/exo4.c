#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#define N 2
#define MAX 10

int fils[N];
int compteur = 0;
int saisie;

void erreur(const char *message) {
    printf("Erreur durant : %s\n", message);
    exit(EXIT_FAILURE);
}

void saisirEntier(const int numeroFils) {
    printf("Fils %d. Saisir un entier\n", numeroFils);
    scanf("%d", &saisie);
}

void creerFils(const int index) {
    switch (fils[index] = fork()) {
        case -1:
            erreur("fork");
        case 0:
            while (compteur < MAX) {
                saisirEntier(index + 1);
                printf("Je suis le fils %d : %d, vous avez saisie : %d\n", index + 1, getpid(), saisie);
                compteur++;
                saisie = 0; //on réinitialise, pas obligé
                sleep(2);
            }
            exit(EXIT_SUCCESS);
        default:
            break;
    }
}

int main(const int argc, char const *argv[]) {
    int i;
    int j;

    for (i = 0; i < N; i++) {
        creerFils(i);
    }

    for (j = 0; j < N; j++) {
        waitpid(fils[j], NULL, 0);
    }

    printf("Je suis le père ! PID = %d\n", getpid());
    printf("Fin du programme exo4\n");

    return EXIT_SUCCESS;
}

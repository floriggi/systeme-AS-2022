#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#define N 3
#define MAX 5

int compteur = 0;
int pidFils[N];
int total = 0;
int saisie;
int ancien;


void erreur(const char *message) {
    printf("Erreur durant : %s\n", message);
    exit(EXIT_FAILURE);
}

void saisirEntier() {
    printf("Saisir un entier\n");
    scanf("%d", &saisie);
}

void creerFils(const int index) {
    switch (pidFils[index] = fork()) {
        case -1:
            erreur("fork");
        case 0:
            while (compteur < MAX) {
                saisirEntier();
                ancien = total;
                total += saisie;

                printf("\nTour %d. "
                       "Je suis le fils %d, vous avez saisie : %d\n"
                       "Mon ancien total était de %d, il vaut maintenant %d\n",
                       compteur + 1, index + 1, saisie, ancien, total);
                compteur++;
                sleep(1);
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
        waitpid(pidFils[j], NULL, 0);
    }

    printf("\nJe suis ton père.\n");
    printf("Fin du programme exo8\n");

    return EXIT_SUCCESS;
}

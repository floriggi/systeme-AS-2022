#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MIN 5
#define MAX 100
#define NB_PLACES 10
#define NB_CLIENTS 100
#define TAILLE_PANNEAU 200
#define NB_SECONDES 15

typedef struct {
    int affiches[TAILLE_PANNEAU];
    int compteur;
    double tempsMax;
} Panneau;

void initialiserTableau(int tab[]);
void *lirePanneau(void *arg);
int randomNumber();


pthread_mutex_t verrou = PTHREAD_MUTEX_INITIALIZER;
sem_t semaphore; // déclaration du semaphore


int main(const int argc, char const *argv[]) {
    pthread_t clients[NB_CLIENTS];
    Panneau panneau;
    panneau.compteur = 1;
    panneau.tempsMax = (double) (time(NULL) + NB_SECONDES);

    printf("Initialisation du tableau\n");
    initialiserTableau(panneau.affiches);

    //initialisation du semaphore
    if (sem_init(&semaphore, 0, NB_PLACES) != 0) {
        printf("erreur de creation de semaphore\n");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < NB_CLIENTS; ++i) {
        if (pthread_create(&clients[i], NULL, lirePanneau, &panneau) != 0) {
            printf("erreur de creation de thread\n");
            return EXIT_FAILURE;
        }
    }

    printf("Creation du thread -> ok\n");

    for (int i = 0; i < NB_CLIENTS; ++i) {
        pthread_join(clients[i], NULL);
    }

    sem_destroy(&semaphore);
    pthread_mutex_destroy(&verrou);

    printf("Fin du programme TD8-1");
    return EXIT_SUCCESS;
}

void initialiserTableau(int tab[]) {
    for (int i = 0; i < TAILLE_PANNEAU; i++) {
        tab[i] = randomNumber();
    }
}

void *lirePanneau(void *arg) {
    Panneau *panneau = (Panneau *) arg;

    pthread_mutex_lock(&verrou);
    const int numero = panneau->compteur++;
    pthread_mutex_unlock(&verrou);

    while (time(NULL) < panneau->tempsMax) {
        sem_wait(&semaphore);
        int numPanneau = rand() % TAILLE_PANNEAU;
        printf("[Lecteur %d], affichage du panneau[%d] = %d\n\n",
                numero,
                numPanneau,
                panneau->affiches[numPanneau]);
        sleep(1);
        sem_post(&semaphore);
    }

    pthread_exit(NULL);
}

int randomNumber() {
    return (rand() % (MAX - MIN)) + MIN;
}

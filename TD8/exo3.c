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
#define NB_PANNEAUX 5
#define NB_SECONDES 15

typedef struct {
    int affiches[TAILLE_PANNEAU];
    sem_t semaphore; // déclaration du semaphore, cette fois à l'intérieur de la structure
} Panneau;

void initialiserTableaux(Panneau panneaux[]);
void initialiserTableau(int tab[]);
void *lirePanneau(void *arg);
void *modifierTableau(void *arg);
int randomNumber();


pthread_mutex_t verrou = PTHREAD_MUTEX_INITIALIZER;
int compteur = 0; //utilisé pour que chaque client dispose d'un numéro unique
double tempsMax;


int main(const int argc, char const *argv[]) {
    pthread_t colleur;
    pthread_t clients[NB_CLIENTS];
    Panneau panneaux[NB_PANNEAUX];

    printf("Initialisation des panneaux\n");
    initialiserTableaux(panneaux);

    tempsMax = (double) (time(NULL) + NB_SECONDES);

    if (pthread_create(&colleur, NULL, modifierTableau, &panneaux) != 0) {
        printf("erreur de creation de thread\n");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < NB_CLIENTS; ++i) {
        if (pthread_create(&clients[i], NULL, lirePanneau, &panneaux) != 0) {
            printf("erreur de creation de thread\n");
            return EXIT_FAILURE;
        }
    }

    printf("Creation du thread -> ok\n");

    pthread_join(colleur, NULL);

    for (int i = 0; i < NB_CLIENTS; ++i) {
        pthread_join(clients[i], NULL);
    }

    pthread_mutex_destroy(&verrou);
    for (int j = 0; j < NB_PANNEAUX; ++j) {
        sem_destroy(&(panneaux[j].semaphore));
    }

    printf("Fin du programme TD8-3");
    return EXIT_SUCCESS;
}

void initialiserTableaux(Panneau panneaux[]) {
    for (int i = 0; i < NB_PANNEAUX; ++i) {
        //initialisation du semaphores
        if (sem_init(&(panneaux[i].semaphore), 0, NB_PLACES) != 0) {
            printf("erreur de creation de semaphore\n");
            exit(EXIT_FAILURE);
        }

        initialiserTableau(panneaux[i].affiches);
    }
}

void initialiserTableau(int tab[]) {
    for (int i = 0; i < TAILLE_PANNEAU; ++i) {
        tab[i] = randomNumber();
    }
}

void *lirePanneau(void *arg) {
    Panneau *panneaux = (Panneau *) arg;
    int numeroTableau = 0;

    pthread_mutex_lock(&verrou);
    const int numero = compteur++;
    pthread_mutex_unlock(&verrou);

    while (time(NULL) < tempsMax) {
        numeroTableau = rand() % NB_PANNEAUX;
        sem_wait(&(panneaux[numeroTableau].semaphore));
        printf("[Lecteur %d], affichage du panneau %d\n", numero, numeroTableau);
        for (int i = 0; i < TAILLE_PANNEAU; ++i) {
            printf("%d ", panneaux[numeroTableau].affiches[i]);
        }
        printf("\n\n");
        sem_post(&(panneaux[numeroTableau].semaphore));
        sleep(1);
    }

    pthread_exit(NULL);
}

void *modifierTableau(void *arg) {
    Panneau *panneaux = (Panneau *) arg;

    while (time(NULL) < tempsMax) {
        for (int numeroPanneau = 0; numeroPanneau < NB_PANNEAUX; ++numeroPanneau) {
            Panneau *panneauCourant = panneaux + numeroPanneau;

            // On occupe toutes les places du panneau
            for (int j = 0; j < NB_PLACES; ++j) {
                sem_wait(&(panneauCourant->semaphore));
            }

            printf("[Colleur d'affiches], je modifie le panneau %d\n\n", numeroPanneau);
            initialiserTableau(panneauCourant->affiches);
            sleep(1);
            printf("[Colleur d'affiches], travail sur panneau %d terminé\n\n", numeroPanneau);

            // On libère toutes les places du panneau
            for (int j = 0; j < NB_PLACES; ++j) {
                sem_post(&(panneauCourant->semaphore));
            }
        }

        sleep(3);
    }

    pthread_exit(NULL);
}

int randomNumber() {
    return (rand() % (MAX - MIN)) + MIN;
}

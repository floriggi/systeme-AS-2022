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
#define NB_COLLEURS NB_PANNEAUX
#define NB_SECONDES 15

typedef struct {
    int affiches[TAILLE_PANNEAU];
    sem_t semaphore; // déclaration du semaphore
} Panneau;

void initialiserTableaux(Panneau panneaux[]);
void initialiserTableau(int tab[]);
void *lirePanneau(void *arg);
void *modifierTableau(void *arg);
int randomNumber();


pthread_mutex_t verrou = PTHREAD_MUTEX_INITIALIZER;
int compteurClient = 0;     //utilisé pour que chaque client dispose d'un numéro unique
int compteurAfficheur = 0;  //utilisé pour que chaque colleur dispose d'un numéro unique
double tempsMax;


int main(const int argc, char const *argv[]) {
    pthread_t colleurs[NB_COLLEURS];
    pthread_t clients[NB_CLIENTS];
    Panneau panneaux[NB_PANNEAUX];

    printf("Initialisation des panneaux\n");
    initialiserTableaux(panneaux);

    tempsMax = (double) (time(NULL) + NB_SECONDES);

    for (int i = 0; i < NB_COLLEURS; ++i) {
        if (pthread_create(&colleurs[i], NULL, modifierTableau, &panneaux) != 0) {
            printf("erreur de creation de thread\n");
            return EXIT_FAILURE;
        }
    }

    for (int i = 0; i < NB_CLIENTS; ++i) {
        if (pthread_create(&clients[i], NULL, lirePanneau, &panneaux) != 0) {
            printf("erreur de creation de thread\n");
            return EXIT_FAILURE;
        }
    }

    printf("Creation du thread -> ok\n");

    for (int i = 0; i < NB_COLLEURS; ++i) {
        pthread_join(colleurs[i], NULL);
    }

    for (int i = 0; i < NB_CLIENTS; ++i) {
        pthread_join(clients[i], NULL);
    }

    pthread_mutex_destroy(&verrou);
    for (int j = 0; j < NB_PANNEAUX; ++j) {
        sem_destroy(&panneaux[j].semaphore);
    }

    printf("Fin du programme TD8-4");
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
    const int numero = compteurClient++;
    pthread_mutex_unlock(&verrou);

    while (time(NULL) < tempsMax) {
        numeroTableau = rand() % NB_PANNEAUX;

        sem_wait(&(panneaux[numeroTableau].semaphore));
        printf("[Lecteur %d], affichage du tableau %d\n", numero, numeroTableau);
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

    pthread_mutex_lock(&verrou);
    const int numero = compteurAfficheur++;
    pthread_mutex_unlock(&verrou);

    while (time(NULL) < tempsMax) {
        // On occupe toutes les places du panneau
        for (int i = 0; i < NB_PLACES; ++i) {
            sem_wait(&panneaux[numero].semaphore);
        }

        printf("[Colleur d'affiches panneau %d], je modifie toutes les cases\n\n", numero);
        initialiserTableau(panneaux[numero].affiches);
        sleep(1);
        printf("[Colleur d'affiches panneau %d], travail terminé\n\n", numero);

        // On libère toutes les places du panneau
        for (int j = 0; j < NB_PLACES; ++j) {
            sem_post(&panneaux[numero].semaphore);
        }

        sleep(3);
    }

    pthread_exit(NULL);
}

int randomNumber() {
    return (rand() % (MAX - MIN)) + MIN;
}

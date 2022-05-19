#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NB_ACHETEURS 4
#define NB_MIN 1
#define NB_MAX 10
#define NB_SECONDES 25

typedef struct {
    int quantite;
    int numeroClient;
    double tempsMax;
} Magasin;

void *fonctionAcheteur(void *arg);
void *fonctionVendeur(void *arg);
int randomNumber();


pthread_mutex_t mutex;
pthread_mutex_t mutexCompteur;
pthread_cond_t conditionStockOk;


int main(const int argc, char const *argv[]) {
    Magasin magasin;
    magasin.quantite = 100;
    magasin.numeroClient = 1;
    // On calcule le moment où la boucle infinie devra s'arrêter (pour les threads)
    magasin.tempsMax = (double) (time(NULL) + NB_SECONDES);

    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&mutexCompteur, NULL);
    pthread_cond_init(&conditionStockOk, NULL);

    pthread_t vendeur;
    pthread_t acheteurs[NB_ACHETEURS];

    if (pthread_create(&vendeur, NULL, fonctionVendeur, &magasin) != 0) {
        printf("erreur de creation de thread vendeur\n");
        return EXIT_FAILURE;
    }

    for (int j = 0; j < NB_ACHETEURS; ++j) {
        if (pthread_create(&acheteurs[j], NULL, fonctionAcheteur, &magasin) != 0) {
            printf("erreur de creation de thread client\n");
            return EXIT_FAILURE;
        }
    }

    pthread_join(vendeur, NULL);

    for (int i = 0; i < NB_ACHETEURS; ++i) {
        pthread_join(acheteurs[i], NULL);
    }

    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&mutexCompteur);
    pthread_cond_destroy(&conditionStockOk);

    printf("Fin du programme TD7-1");
    return 0;
}

void *fonctionVendeur(void *arg) {
    Magasin *magasin = (Magasin *) arg;

    while (time(NULL) < magasin->tempsMax) {
        sleep(10);
        pthread_mutex_lock(&mutex);
        magasin->quantite += 1000;
        printf("Je suis le vendeur, je rajoute 1000 unités. Total = %d\n", magasin->quantite);
        pthread_cond_broadcast(&conditionStockOk);
        pthread_mutex_unlock(&mutex);
    }

    usleep(300 * 1000);
    pthread_cond_broadcast(&conditionStockOk);

    pthread_exit(NULL);
}

void *fonctionAcheteur(void *arg) {
    Magasin *magasin = (Magasin *) arg;
    int achat = 0;

    pthread_mutex_lock(&mutexCompteur);
    const int numero = magasin->numeroClient++;
    pthread_mutex_unlock(&mutexCompteur);

    while (time(NULL) < magasin->tempsMax) {
        pthread_mutex_lock(&mutex);
        achat = randomNumber();
        printf("Je suis l'acheteur %d, la quantité actuelle = %d\n", numero, magasin->quantite);

        if (magasin->quantite < achat) {
            printf("(acheteur %d) Le stock est insuffisant, je dois attendre le vendeur\n", numero);
            pthread_cond_wait(&conditionStockOk, &mutex);
        }

        magasin->quantite -= achat;
        printf("Je suis l'acheteur %d, achat de %d. Nouvelle quantité = %d\n", numero, achat, magasin->quantite);
        pthread_mutex_unlock(&mutex);
        usleep(50 * 1000);
    }

    pthread_exit(NULL);
}

int randomNumber() {
    return (rand() % (NB_MAX - NB_MIN)) + NB_MIN;
}

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NB_ACHETEURS 5
#define NB_ARTICLES 4
#define NB_MIN 1
#define NB_MAX 10
#define NB_SECONDES 15

typedef struct {
    int quantite[NB_ARTICLES];
    int numeroClient;
    int numeroVendeur;
    double tempsMax;
} Magasin;

void *fonctionAcheteur(void *arg);
void *fonctionVendeur(void *arg);
int randomNumber();


pthread_mutex_t mutexCompteur;
pthread_mutex_t mutex[NB_ARTICLES];
pthread_cond_t conditionStockOk[NB_ARTICLES];
pthread_cond_t conditionStockEpuise[NB_ARTICLES];


void initialiserMagasin(Magasin *magasin) {
    magasin->numeroClient = 1;
    magasin->numeroVendeur = 0;
    // On calcule le moment où la boucle infinie devra s'arrêter (pour les threads)
    magasin->tempsMax = (double) (time(NULL) + NB_SECONDES);
    for (int i = 0; i < NB_ARTICLES; ++i) {
        magasin->quantite[i] = 20;
    }
}

int main(const int argc, char const *argv[]) {
    Magasin magasin;
    initialiserMagasin(&magasin);

    pthread_mutex_init(&mutexCompteur, NULL);
    for (int i = 0; i < NB_ARTICLES; ++i) {
        pthread_mutex_init(&mutex[i], NULL);
        pthread_cond_init(&conditionStockOk[i], NULL);
        pthread_cond_init(&conditionStockEpuise[i], NULL);
    }

    pthread_t vendeurs[NB_ARTICLES];
    pthread_t acheteurs[NB_ACHETEURS];

    for (int i = 0; i < NB_ARTICLES; ++i) {
        if (pthread_create(&vendeurs[i], NULL, fonctionVendeur, &magasin) != 0) {
            printf("erreur de creation de thread vendeur\n");
            return EXIT_FAILURE;
        }
    }

    for (int j = 0; j < NB_ACHETEURS; ++j) {
        if (pthread_create(&acheteurs[j], NULL, fonctionAcheteur, &magasin) != 0) {
            printf("erreur de creation de thread client\n");
            return EXIT_FAILURE;
        }
    }

    // cette partie pour forcer tous les threads à se reveiller et sortir de leur boucle
    sleep(NB_SECONDES + 1);
    printf("\nbroadcast\n");
    for (int k = 0; k < NB_ARTICLES; ++k) {
        pthread_cond_broadcast(&conditionStockOk[k]);
        pthread_cond_broadcast(&conditionStockEpuise[k]);
    }

    for (int i = 0; i < NB_ARTICLES; ++i) {
        pthread_join(vendeurs[i], NULL);
    }

    for (int j = 0; j < NB_ACHETEURS; ++j) {
        pthread_join(acheteurs[j], NULL);
    }

    pthread_mutex_destroy(&mutexCompteur);
    for (int i = 0; i < NB_ARTICLES; ++i) {
        pthread_mutex_destroy(&mutex[i]);
        pthread_cond_destroy(&conditionStockOk[i]);
        pthread_cond_destroy(&conditionStockEpuise[i]);
    }

    printf("Fin du programme TD7-3");
    return 0;
}

void *fonctionAcheteur(void *arg) {
    Magasin *magasin = (Magasin *) arg;
    int achat = 0;
    int numArticle = 0;

    pthread_mutex_lock(&mutexCompteur);
    const int numero = magasin->numeroClient++;
    pthread_mutex_unlock(&mutexCompteur);

    while (time(NULL) < magasin->tempsMax) {
        numArticle = rand() % NB_ARTICLES;

        pthread_mutex_lock(&mutex[numArticle]);
        achat = randomNumber();
        printf("Je suis l'acheteur %d, la quantité[%d] actuelle = %d\n", numero, numArticle,
               magasin->quantite[numArticle]);

        if (magasin->quantite[numArticle] < achat) {
            printf("(acheteur %d) Le stock est insuffisant, je dois attendre le vendeur %d\n", numero, numArticle);
            pthread_cond_signal(&conditionStockEpuise[numArticle]);
            pthread_cond_wait(&conditionStockOk[numArticle], &mutex[numArticle]);
        }

        magasin->quantite[numArticle] -= achat;
        printf("Je suis l'acheteur %d, achat de %d. Nouvelle quantité[%d] = %d\n", numero, achat, numArticle,
               magasin->quantite[numArticle]);
        pthread_mutex_unlock(&mutex[numArticle]);
        usleep(100 * 1000);
    }

    pthread_exit(NULL);
}

void *fonctionVendeur(void *arg) {
    Magasin *magasin = (Magasin *) arg;

    pthread_mutex_lock(&mutexCompteur);
    const int numero = magasin->numeroVendeur++;
    pthread_mutex_unlock(&mutexCompteur);

    while (time(NULL) < magasin->tempsMax) {
        pthread_mutex_lock(&mutex[numero]);
        pthread_cond_wait(&conditionStockEpuise[numero], &mutex[numero]);
        magasin->quantite[numero] += 100;
        printf("Je suis le vendeur %d, je rajoute 100 unités. Total = %d\n", numero, magasin->quantite[numero]);
        sleep(1);
        pthread_cond_signal(&conditionStockOk[numero]);
        pthread_mutex_unlock(&mutex[numero]);
    }

    pthread_exit(NULL);
}

int randomNumber() {
    return (rand() % (NB_MAX - NB_MIN)) + NB_MIN;
}

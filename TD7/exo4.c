#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NB_ACHETEURS 5
#define NB_ARTICLES 4
#define NB_MAGASINS 3
#define NB_MIN 1
#define NB_MAX 10
#define NB_SECONDES 15

typedef struct {
    int quantite[NB_ARTICLES];
    pthread_mutex_t mutex[NB_ARTICLES];
    pthread_cond_t conditionStockOk[NB_ARTICLES];
    pthread_cond_t conditionStockEpuise[NB_ARTICLES];
} Magasin;

void *fonctionAcheteur(void *arg);
void *fonctionVendeur(void *arg);
int randomNumber();


pthread_mutex_t mutexCompteur;
int numeroClient;
int numeroMagasin;
int numeroVendeur;
double tempsMax;

void initialiserMagasin(Magasin *magasin) {
    for (int i = 0; i < NB_ARTICLES; ++i) {
        magasin->quantite[i] = 20;
        pthread_mutex_init(&magasin->mutex[i], NULL);
        pthread_cond_init(&magasin->conditionStockOk[i], NULL);
        pthread_cond_init(&magasin->conditionStockEpuise[i], NULL);
    }
}

void initialiserMagasins(Magasin magasins[]) {
    for (int i = 0; i < NB_MAGASINS; ++i) {
        initialiserMagasin(&magasins[i]);
    }
}

int main(const int argc, char const *argv[]) {
    Magasin magasins[NB_MAGASINS];
    initialiserMagasins(magasins);

    numeroClient = 0;
    numeroMagasin = 0;
    numeroVendeur = 0;
    // On calcule le moment où la boucle infinie devra s'arrêter (pour les threads)
    tempsMax = (double) (time(NULL) + NB_SECONDES);

    pthread_mutex_init(&mutexCompteur, NULL);

    pthread_t acheteurs[NB_ACHETEURS];
    pthread_t vendeurs[NB_MAGASINS][NB_ARTICLES];

    for (int i = 0; i < NB_MAGASINS; ++i) {
        for (int j = 0; j < NB_ARTICLES; ++j) {
            if (pthread_create(&vendeurs[i][j], NULL, fonctionVendeur, &magasins) != 0) {
                printf("erreur de creation de thread vendeur\n");
                return EXIT_FAILURE;
            }
        }
    }

    for (int j = 0; j < NB_ACHETEURS; ++j) {
        if (pthread_create(&acheteurs[j], NULL, fonctionAcheteur, &magasins) != 0) {
            printf("erreur de creation de thread client\n");
            return EXIT_FAILURE;
        }
    }

    // cette partie pour forcer tous les threads à se reveiller et sortir de leur boucle
    sleep(NB_SECONDES + 1);
    printf("\nbroadcast\n");
    for (int i = 0; i < NB_MAGASINS; ++i) {
        for (int j = 0; j < NB_ARTICLES; ++j) {
            pthread_cond_broadcast(&magasins[i].conditionStockOk[j]);
            pthread_cond_broadcast(&magasins[i].conditionStockEpuise[j]);
        }
    }

    for (int i = 0; i < NB_MAGASINS; ++i) {
        for (int j = 0; j < NB_ARTICLES; ++j) {
            pthread_join(vendeurs[i][j], NULL);
        }
    }

    for (int j = 0; j < NB_ACHETEURS; ++j) {
        pthread_join(acheteurs[j], NULL);
    }

    pthread_mutex_destroy(&mutexCompteur);
    for (int i = 0; i < NB_MAGASINS; ++i) {
        for (int j = 0; j < NB_ARTICLES; ++j) {
            pthread_mutex_destroy(&magasins[i].mutex[j]);
            pthread_cond_destroy(&magasins[i].conditionStockOk[j]);
            pthread_cond_destroy(&magasins[i].conditionStockEpuise[j]);
        }
    }

    printf("Fin du programme TD7-3");
    return 0;
}

void *fonctionAcheteur(void *arg) {
    Magasin *magasins = (Magasin *) arg;
    int achat = 0;
    int numArticle = 0;
    int numMagasin = 0;

    pthread_mutex_lock(&mutexCompteur);
    const int numero = numeroClient++;
    pthread_mutex_unlock(&mutexCompteur);

    while (time(NULL) < tempsMax) {
        numArticle = rand() % NB_ARTICLES;
        numMagasin = rand() % NB_MAGASINS;

        pthread_mutex_lock(&magasins[numMagasin].mutex[numArticle]);
        achat = randomNumber();
        printf("Je suis l'acheteur %d, la quantité[%d][%d] actuelle = %d\n", numero, numMagasin, numArticle,
               magasins[numMagasin].quantite[numArticle]);

        if (magasins[numMagasin].quantite[numArticle] < achat) {
            printf("(acheteur %d) Le stock est insuffisant, je dois attendre le vendeur %d-%d\n", numero, numMagasin,
                   numArticle);
            pthread_cond_signal(&magasins[numMagasin].conditionStockEpuise[numArticle]);
            pthread_cond_wait(&magasins[numMagasin].conditionStockOk[numArticle], &magasins[numMagasin].mutex[numArticle]);
        }

        magasins[numMagasin].quantite[numArticle] -= achat;
        printf("Je suis l'acheteur %d, achat de %d. Nouvelle quantité[%d][%d] = %d\n", numero, achat, numMagasin,
               numArticle,
               magasins[numMagasin].quantite[numArticle]);
        pthread_mutex_unlock(&magasins[numMagasin].mutex[numArticle]);
        usleep(100 * 1000);
    }

    pthread_exit(NULL);
}

void *fonctionVendeur(void *arg) {
    Magasin *magasins = (Magasin *) arg;

    pthread_mutex_lock(&mutexCompteur);
    const int numeroMag = numeroMagasin;
    const int numeroVend = numeroVendeur++;

    if (numeroVendeur == NB_ARTICLES) {
        numeroVendeur = 0;
        numeroMagasin++;
    }
    pthread_mutex_unlock(&mutexCompteur);

    while (time(NULL) < tempsMax) {
        pthread_mutex_lock(&magasins[numeroMag].mutex[numeroVend]);
        pthread_cond_wait(&magasins[numeroMag].conditionStockEpuise[numeroVend], &magasins[numeroMag].mutex[numeroVend]);

        magasins[numeroMag].quantite[numeroVend] += 100;
        printf("Je suis le vendeur %d-%d, je rajoute 100 unités. Total = %d\n", numeroMag, numeroVend,
               magasins[numeroMag].quantite[numeroVend]);
        sleep(1);

        pthread_cond_signal(&magasins[numeroMag].conditionStockOk[numeroVend]);
        pthread_mutex_unlock(&magasins[numeroMag].mutex[numeroVend]);
    }

    pthread_exit(NULL);
}

int randomNumber() {
    return (rand() % (NB_MAX - NB_MIN)) + NB_MIN;
}

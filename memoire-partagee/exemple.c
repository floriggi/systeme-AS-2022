#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <unistd.h>
#include <wait.h>

#define NB_PLACE_SEMAPHORE 1

typedef struct {
    int nombre;
} Donnees;


void ouvrirMemoirePartagee();
void fermerMemoirePartagee();
void ouvrirSemaphore();
void prendrePlaceSemaphore(int semid);
void libererPlaceSemaphore(int semid);


int segment_id; // id mémoire partagée
int semaphore;

Donnees *donnees;


int main(const int argc, const char *argv[]) {
    ouvrirMemoirePartagee();

    donnees->nombre = 10;

    int child_pid = fork();

    if (child_pid == -1) {
        perror("fork");
    }

    if (child_pid == 0) {
        printf("Execution du fils\n");

        prendrePlaceSemaphore(semaphore);
        printf("Valeur dans mémoire partagée avant = %d\n", donnees->nombre);
        donnees->nombre = 8;
        printf("Valeur dans mémoire partagée après = %d\n", donnees->nombre);
        libererPlaceSemaphore(semaphore);

        printf("Fin du fils - %d\n", getpid());
        exit(EXIT_SUCCESS);
    }

    printf("Execution du père\n");
    prendrePlaceSemaphore(semaphore);
    printf("Valeur dans mémoire partagée avant = %d\n", donnees->nombre);
    donnees->nombre = 5;
    printf("Valeur dans mémoire partagée après = %d\n", donnees->nombre);
    libererPlaceSemaphore(semaphore);

    if (waitpid(child_pid, NULL, 0) == -1) {
        perror("waitpid");
    }

    printf("Fin du père - %d\n", getpid());

    fermerMemoirePartagee();

    exit(EXIT_SUCCESS);
}


void ouvrirMemoirePartagee() {
    segment_id = shmget(IPC_PRIVATE, sizeof(Donnees), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    donnees = (Donnees *) shmat(segment_id, 0, 0);
}

void fermerMemoirePartagee() {
    shmdt(donnees);
    shmctl(segment_id, IPC_RMID, 0);
}

void ouvrirSemaphore() {
    const key_t clefsem = ftok(".", 20);
    semaphore = semget(clefsem, 1, IPC_CREAT | 0666); //initialiser semaphore
    semctl(semaphore, 0, SETVAL, NB_PLACE_SEMAPHORE);

    if (semaphore == -1) {
        printf("erreur de creation de semaphore\n");
        exit(EXIT_FAILURE);
    }
}

void prendrePlaceSemaphore(const int semid) {
    int rep;
    struct sembuf sb = {0, -1, 0};
    rep = semop(semid, &sb, 1); //1 = un seul semaphore concerne
}

void libererPlaceSemaphore(const int semid) {
    int rep;
    struct sembuf sb = {0, 1, 0};
    rep = semop(semid, &sb, 1);
}

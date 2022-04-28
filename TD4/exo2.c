#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int pid;

void lirePid() {
    printf("Saisir un PID\n");
    scanf("%d", &pid);
}

void envoyerSignal(const int signal) {
    printf("Envoi du signal %d\n", signal);
    kill(pid, signal);
}

int main(int argc, char const *argv[]) {
    lirePid();

    for (int signal = 1; signal < NSIG; signal++) {
        if (signal != 9 && signal != 19) {
           envoyerSignal(signal);
           usleep(500 * 1000);
        }
    }

    // ceci pour tuer l'exercice 1
    envoyerSignal(9);

    return EXIT_SUCCESS;
}

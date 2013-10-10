#include <iostream>
#include <pthread.h>
#include <cstdlib>

using namespace std;

// Funktioner
void *carFunc(void* carID);
void *entryFunc(void*);
void *exitFunc(void*);

// Konstanter
const int CAR_AMOUNT = 1;   // Amount of cars

// Globale variabler
pthread_mutex_t entryLock, exitLock;
pthread_cond_t entrySignal, exitSignal;

bool entryWaiting = false;
bool exitWaiting = false;
bool entryIsOpen = false;
bool exitIsOpen = false;

int main()
{
    cout << "PLCS started." << endl;

    // Fejl variable
    int err = 0;

    // Initialiserer mutexes
    if ( (err = pthread_mutex_init(&entryLock, NULL)) )
    {
        cout << "Could not initialize entryLock, ERROR: " << err << endl;
        return EXIT_FAILURE;
    }
    if ( (err = pthread_mutex_init(&exitLock, NULL)) )
    {
        cout << "Could not initialize exitLock, ERROR: " << err << endl;
        return EXIT_FAILURE;
    }

    // Initialiserer cond signaler
    if ( (err = pthread_cond_init(&entrySignal, NULL)) )
    {
        cout << "Could not initialize cond signal entry, ERROR: " << err << endl;
        return EXIT_FAILURE;
    }
    if ( (err = pthread_cond_init(&exitSignal, NULL)) )
    {
        cout << "Could not initialize cond signal exit, ERROR: " << err << endl;
        return EXIT_FAILURE;
    }

    // Initialiserer entry og exit threads
    pthread_t entryThread, exitThread;

    if ( (err = pthread_create(&entryThread, NULL, entryFunc, NULL)) )
    {
        cout << "Could not create entryThread, ERROR: " << err << endl;
        return EXIT_FAILURE;
    }
    if ( (err = pthread_create(&exitThread, NULL, exitFunc, NULL)) )
    {
        cout << "Could not create exitThread, ERROR: " << err << endl;
        return EXIT_FAILURE;
    }

    // Initialiserer bil threads
    pthread_t carThread[CAR_AMOUNT];
    int carID[CAR_AMOUNT];  // Array to pass by reference
    for (int i = 0; i < CAR_AMOUNT; i++)
    {
        carID[i] = i+1; // ID array should start with 1 not 0
        if ( (err = pthread_create(&carThread[i], NULL, carFunc, (void*)(carID+i))) )
        {
            cout << "Could not create carThread with ID " << carID+i << ", ERROR: " << err << endl;
            return EXIT_FAILURE;
        }
    }

    // Threads join
    if ( (err = pthread_join(entryThread, NULL)) )
    {
        cout << "Could not join entryThread, ERROR: " << err << endl;
        return EXIT_FAILURE;
    }
    if ( (err = pthread_join(exitThread, NULL)) )
    {
        cout << "Could not join exitThread, ERROR: " << err << endl;
        return EXIT_FAILURE;
    }
    for (int i = 0; i < CAR_AMOUNT; i++)
    {
        if ( (err = pthread_join(carThread[i], NULL)) )
        {
            cout << "Could not join carThread with ID " << carID+i << ", ERROR: " << err << endl;
            return EXIT_FAILURE;
        }
    }

    // Nedlægger mutexes
    if ( (err = pthread_mutex_destroy(&entryLock)) )
    {
        cout << "Could not destroy entryLock, ERROR: " << err << endl;
        return EXIT_FAILURE;
    }
    if ( (err = pthread_mutex_destroy(&exitLock)) )
    {
        cout << "Could not destroy exitLock, ERROR: " << err << endl;
        return EXIT_FAILURE;
    }

    // Nedlægger mutexes
    if ( (err = pthread_cond_destroy(&entrySignal)) )
    {
        cout << "Could not destroy entrySignal, ERROR: " << err << endl;
        return EXIT_FAILURE;
    }
    if ( (err = pthread_cond_destroy(&exitSignal)) )
    {
        cout << "Could not destroy exitSignal, ERROR: " << err << endl;
        return EXIT_FAILURE;
    }

    cout << "PLCS ended." << endl;
    return EXIT_SUCCESS;
}

void *carFunc(void* carID)
{
    // Gemmer bilens ID
    int ID = *((int*)carID);

    // Indkørsel
    cout << "Car " << ID << " is approaching the entry gate." << endl;
    pthread_mutex_lock(&entryLock);
    entryWaiting = true;
    pthread_cond_signal(&entrySignal);
    while(!entryIsOpen)
    {
        pthread_cond_wait(&entrySignal, &entryLock);
    }
    cout << "Car " << ID << " is now inside the parking lot." << endl;
    entryWaiting = false;
    pthread_cond_signal(&entrySignal);
    pthread_mutex_unlock(&entryLock);

    // Udkørsel
    cout << "Car " << ID << " is approaching exit gate." << endl;
    pthread_mutex_lock(&exitLock);
    exitWaiting = true;
    pthread_cond_signal(&exitSignal);
    while(!exitIsOpen)
    {
        pthread_cond_wait(&exitSignal, &exitLock);
    }
    cout << "Car " << ID << " is now outside the parking lot." << endl;
    exitWaiting = false;
    pthread_cond_signal(&exitSignal);
    pthread_mutex_unlock(&exitLock);

    // Slutter thread
    pthread_exit((void*)&ID);
}

void *entryFunc(void*)
{
    pthread_mutex_lock(&entryLock);
    while(!entryWaiting)
    {
        pthread_cond_wait(&entrySignal, &entryLock);
    }
    cout << "The entry gate is now open." << endl;
    entryIsOpen = true;
    pthread_cond_signal(&entrySignal);
    while(entryWaiting)
    {
        pthread_cond_wait(&entrySignal, &entryLock);
    }
    cout << "The entry gate is now closed." << endl;
    entryIsOpen = false;
    pthread_mutex_unlock(&entryLock);

    // Slutter thread
    pthread_exit(NULL);
}

void *exitFunc(void*)
{
    pthread_mutex_lock(&exitLock);
    while(!exitWaiting)
    {
        pthread_cond_wait(&exitSignal, &exitLock);
    }
    cout << "The exit gate is now open." << endl;
    exitIsOpen = true;
    pthread_cond_signal(&exitSignal);
    while(exitWaiting)
    {
        pthread_cond_wait(&exitSignal, &exitLock);
    }
    cout << "The exit gate is now closed." << endl;
    exitIsOpen = false;
    pthread_mutex_unlock(&exitLock);

    // Slutter thread
    pthread_exit(NULL);
}


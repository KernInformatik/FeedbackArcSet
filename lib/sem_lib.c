/**
 * @file sem_lib.c
 * @author kernkraftwerk (kernkraftdev@hotmail.com)
 * @brief for documentation see the sem_lib.c
 * @version 0.1
 * @date 2026-09-29
 *
 * @copyright Copyright (c) 2026
 *
 */
#include "sem_lib.h"
volatile sig_atomic_t quit = false;
void
onSignal (int sig, siginfo_t *si, void *unused)
{
  quit = true;
}

void
initSignalHandler (void)
{
  struct sigaction sa;
  sa.sa_flags = SA_SIGINFO;
  sigemptyset (&sa.sa_mask);
  sa.sa_sigaction = onSignal;
  if (sigaction (SIGINT, &sa, NULL) == -1)
    {
      error_exit ("sigaction");
    }
  if (sigaction (SIGTERM, &sa, NULL) == -1)
    {
      error_exit ("sigaction");
    }
  if (sigaction (SIGQUIT, &sa, NULL) == -1)
    {
      error_exit ("sigaction");
    }
}

/*Begin helper functions */
static inline struct shm *
sharedMemory (int *shmfd, bool isServer)
{
  struct shm *rv = NULL;

  *shmfd = isServer ? shm_open (SHM_NAME, SHM_OFLAG_SERVER, SHM_MODE)
                    : shm_open (SHM_NAME, SHM_OFLAG_CLIENT, SHM_MODE);

  error_exit_failure (*shmfd, (isServer ? (OPENING_SHM_ERROR_SERVER)
                                        : (OPENING_SHM_ERROR_CLIENT)));

  if (isServer)
    {
      error_exit_failure (ftruncate (*shmfd, sizeof (struct shm)),
                          TRUNCATING_SHM_ERROR);
    }

  rv = (struct shm *)mmap (NULL, sizeof (struct shm), MMP_PROT_FLAGS,
                           MMP_FLAGS, *shmfd, MMP_OFFSET);

  if (rv == MAP_FAILED)
    error_exit (MAPPING_SHM_ERROR);

  if (isServer)
    {
      memset (rv, 0, sizeof (*rv));
    }
  return rv;
}

static inline int
cleanSharedMemory (struct shm *shm, int *shmfd, bool isServer)
{

  error_exit_failure (munmap (shm, sizeof (*shm)), UNMAPPING_SHM_ERROR);
  error_exit_failure (close (*shmfd), CLOSING_SHM_ERROR);

  if (isServer)
    {
      error_exit_failure (shm_unlink (SHM_NAME), UNLINKING_SHM_ERROR);
    }

  return 0;
}

static inline sem_t *
initializeSemaphore (const char *sem_name, unsigned int init_value,
                     bool isServer)
{
  sem_t *rv = isServer ? sem_open (sem_name, OPEN_SEM_MODE_SERVER,
                                   OPEN_SEM_MODE, init_value)
                       : sem_open (sem_name, OPEN_SEM_MODE_CLIENT,
                                   OPEN_SEM_MODE, init_value);
  if (rv == SEM_FAILED)
    {
      error_exit (OPENING_SEM_ERROR);
    }

  return rv;
}

static inline void
cleanSemaphore (const char *sem_name, sem_t *semaphore, bool isServer)
{
  error_exit_failure (sem_close (semaphore), CLOSING_SEM_ERROR);

  if (isServer)
    {
      error_exit_failure (sem_unlink (sem_name), UNLINKING_SEM_ERROR);
    }
}

/*End helper functions */

/*API REGION*/
/*See semaphore.h for documentation */

struct shm *
sharedMemory_Server (int *shmfd)
{
  return sharedMemory (shmfd, true);
}

/*See semaphore.h for documentation */

struct shm *
sharedMemory_Client (int *shmfd)
{

  return sharedMemory (shmfd, false);
}

/*See semaphore.h for documentation */

void
cleanSharedMemory_Server (struct shm *shm, int shmfd)
{
  cleanSharedMemory (shm, &shmfd, true);
}

/*See semaphore.h for documentation */

void
cleanSharedMemory_Client (struct shm *shm, int shmfd)
{
  cleanSharedMemory (shm, &shmfd, false);
}

/*See semaphore.h for documentation */
sem_t *
initializeSemaphore_Server (const char *sem_name, unsigned int init_value)
{
  return initializeSemaphore (sem_name, init_value, true);
}

/*See semaphore.h for documentation */

sem_t *
initializeSemaphore_Client (const char *sem_name)
{

  return initializeSemaphore (sem_name, 0, false);
}

/*See semaphore.h for documentation */
void
cleanSemaphore_Server (sem_t *semaphore, const char *sem_name)
{
  cleanSemaphore (sem_name, semaphore, true);
}

/*See semaphore.h for documentation */

void
cleanSemaphore_Client (sem_t *semaphore)
{
  cleanSemaphore (NULL, semaphore, false);
}

/*END API REGION */
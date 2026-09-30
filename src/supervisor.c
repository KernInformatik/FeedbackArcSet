/**
 * @file supervisor.c
 * @author kernkraftwerk (kernkraftdev@hotmail.com)
 * @brief This is the supervisor, reading from the circular buffer located in
 * /dev/shm
 * @version 0.1
 * @date 2026-09-29
 *
 * @copyright Copyright (c) 2026
 *
 */
#include "../lib/common.h"
#include "../lib/sem_lib.h"
#define SOLUTION_CONGRATULATIONS                                              \
  "Congrats buddy, the given graph is acyclic!\n"
extern volatile sig_atomic_t quit;

/**
 * @brief reads the solution from the circular buffer, prints a congratulations
 * message
 *
 * @param buffer the circular buffer storing the edges of a graph, see graph.h
 * and sem_lib.h
 */
void
readSolution (struct shm *buffer)
{
  struct GRAPH_EDGE_LIST solution = buffer->data[buffer->readhead];
  buffer->readhead = (buffer->readhead + 1) % MAX_BUFF_SIZE;

  if (solution.length == 0)
    {
      printf ("%s", SOLUTION_CONGRATULATIONS);
    }

  static long currentBestSolution = ULONG_MAX;

  if (solution.length < currentBestSolution)
    {
      currentBestSolution = solution.length;
      printf ("[./supervisor] Solution with %zu edges:", solution.length);
      for (size_t i = 0; i < solution.length; i++)
        {
          printf (" %d-%d", solution.edgeList[i].from.name,
                  solution.edgeList[i].to.name);
        }
      printf ("\n");
    }
}

/**
 * @brief
 *
 * @param argc argument counter
 * @param argv  argument vectors
 * @return int retuns 0 on success and -1 on failure
 */
int
main (int argc, char **argv)
{
  int shmfd;
  struct shm *buffer = sharedMemory_Server (&shmfd);
  sem_t *free, *used, *write;

  initSignalHandler ();

  free = initializeSemaphore_Server (FREE_SPACE_SEMAPHORE,
                                     FREE_SPACE_SEMAPHORE_SIZE);

  write = initializeSemaphore_Server (WRITE_SPACE_SEMAPHORE,
                                      WRITE_SPACE_SEMAPHORE_SIZE);

  used = initializeSemaphore_Server (USED_SPACE_SEMAPHORE,
                                     USED_SPACE_SEMAPHORE_SIZE);

  while (!quit)
    {
      if (sem_wait (used) == -1)
        {
          fprintf (stderr, "supervisor: woke up, readhead=%zu\n",
                   buffer->readhead);
          if (quit)
            break;
          if (errno == EINTR)
            {
              continue;
            }
          error_exit ("Space is full");
        }
      readSolution (buffer);

      if (sem_post ((free)) == -1)
        {
          error_exit ("sem_post");
        }
    }

  cleanSemaphore_Server (free, FREE_SPACE_SEMAPHORE);
  cleanSemaphore_Server (used, USED_SPACE_SEMAPHORE);
  cleanSemaphore_Server (write, WRITE_SPACE_SEMAPHORE);
  cleanSharedMemory_Server (buffer, shmfd);
}

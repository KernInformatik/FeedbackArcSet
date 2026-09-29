/**
 * @file generator.c
 * @author kernkraftwerk (kernkraftdev@hotmail.com)
 * @brief This is the generator, parsing the input, coloring the vertexes and
 * storing it accordingly into the shared memory/circular buffer
 * @version 0.1
 * @date 2026-09-29
 *
 * @copyright Copyright (c) 2026
 *
 */
#include "../lib/graph.h"
#include "../lib/sem_lib.h"

/**
 * @brief Parses the input and stores it
 *
 * @param argc argument counter
 * @param argv argument vectors
 * @return struct GRAPH_EDGE_LIST
 */
struct GRAPH_EDGE_LIST
parseInput (int argc, char **argv)
{

  struct GRAPH_EDGE_LIST rv;

  /* Error checking*/
  if ((argc - 1) < 1)
    {
      error_exit ("too few arguments");
    }

  if ((argc - 1) > 1028)
    {
      error_exit ("too many arguments");
    }

  for (int i = 1; i < argc; i++)
    {
      for (int j = i + 1; j < argc; j++)
        {
          if (strcmp (argv[i], argv[j]) == 0)
            {
              error_exit ("duplicate arguments");
            }
        }
    }

  for (int i = 1; i < argc; i++)
    {
      if (strchr (argv[i], '-') == NULL)
        {
          error_exit ("no dash in argument");
        }
    }

  for (int i = 1; i < argc; i++)
    {
      if (strchr (argv[i], '-') != strchr (argv[i], '-'))
        {
          error_exit ("to many dashes in argument");
        }
    }

  for (int i = 1; i < argc; i++)
    {
      char *string = strdup (argv[i]);
      assert (string != NULL);

      char *savePtr;
      char *first = strtok_r (string, "-", &savePtr);
      char *second = strtok_r (NULL, "-", &savePtr);

      if ((first == NULL) || (second == NULL))
        {
          error_exit ("missing nodes in arguments");
        }

      for (size_t i = 0; i < strlen (first); i++)
        {
          if (!isdigit (first[i]))
            {
              error_exit ("No digits in argument node");
            }
        }

      for (size_t i = 0; i < strlen (second); i++)
        {
          if (!isdigit (second[i]))
            {
              error_exit ("No digits in argument node");
            }
        }

      free (string);
    }

  /* i = 1 because argc also counts program name which is mentally deranged,
   * took me a while to realize it and argv[0] is the program name damn it*/
  for (int i = 1; i < argc; i++)
    {
      char *savePtr;
      char *first = strdup (argv[i]);
      strtok_r (first, "-", &savePtr);
      char *second = strtok_r (NULL, "-", &savePtr);
      rv.edgeList[i - 1].from.name = strtol (first, NULL, 10);
      rv.edgeList[i - 1].to.name = strtol (second, NULL, 10);

      free (first);
    }
  rv.length = (size_t)argc - 1;

  return rv;
}

/**
 * @brief Colorizes the vertexes randomly
 *
 * @param edgeList
 */
static void
shuffle (struct GRAPH_EDGE_LIST *edgeList)
{
  for (int i = 0; i < edgeList->length; i++)
    {
      edgeList->edgeList[i].from.order = rand () % 2;

      edgeList->edgeList[i].to.order = rand () % 2;
    }
}

/**
 * @brief Writes only the illegal solutions to the shm, if there are none to
 * store, the graph is 3 colorable, if there are any, the optimum solution,
 * found with the randomized coloring of the vertexes will be printed to stdout
 *
 * @param edgeList
 * @return struct GRAPH_EDGE_LIST
 */
struct GRAPH_EDGE_LIST
generateSolution (struct GRAPH_EDGE_LIST edgeList)
{
  struct GRAPH_EDGE_LIST solution;

  int counter = 0;

  shuffle (&edgeList);

  for (int i = 0; i < edgeList.length; i++)
    {
      if (edgeList.edgeList[i].from.order > edgeList.edgeList[i].to.order)
        {
          solution.edgeList[counter++] = edgeList.edgeList[i];
        }
    }
  solution.length = counter;
  return solution;
}

/**
 * @brief writes the graph into shm
 *
 * @param edgeList the stored graph, broken down in GRAPH_EDGE_LIST
 * @param sharedMemory the shared memory in /dev/shm
 * @param free parsing the free semaphore
 * @param used parsing the used semaphore
 * @param writeparsing the used semaphore
 */
void
writeSolution (struct GRAPH_EDGE_LIST edgeList, struct shm *sharedMemory,
               sem_t *free, sem_t *used, sem_t *write)
{
  sem_wait (free);
  sem_wait (write);
  sharedMemory->data[sharedMemory->writehead] = edgeList;
  sem_post (write);
  sem_post (used);
  sharedMemory->writehead++;
  sharedMemory->writehead %= MAX_BUFF_SIZE;
}

int
main (int argc, char **argv)
{
  int shmfd;
  struct shm *possibleSolution = sharedMemory_Client (&shmfd);
  sem_t *free, *used, *write;

  free = initializeSemaphore_Client (FREE_SPACE_SEMAPHORE);
  write = initializeSemaphore_Client (WRITE_SPACE_SEMAPHORE);
  used = initializeSemaphore_Client (USED_SPACE_SEMAPHORE);

  struct GRAPH_EDGE_LIST test = parseInput (argc, argv);
  writeSolution (generateSolution (test), possibleSolution, free, used, write);

  cleanSemaphore_Client (free);
  cleanSemaphore_Client (used);
  cleanSemaphore_Client (write);
  cleanSharedMemory_Client (possibleSolution, shmfd);
}

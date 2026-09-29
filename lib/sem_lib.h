/**
 * @file semaphore.h
 * @author Kernkraftwerk (kernkraftwerk@hotmail.com)
 * @brief Provides functions to set up and tear down shared memory and semaphores for IPC between a server and client
process
 * @details This module wraps shm_open(3), mmap(2), and sem_open(3) into a small
 *          set of create/connect/destroy/disconnect functions. A server process
 *          must call the *_create functions before any client calls *_connect,
 *          since the server is responsible for allocating the underlying
 *          resources. There must be exactly one server; there may be many
 *          clients.
 * @version 0.1

 * @date 2026-09-16
 *
 * @copyright Copyright (c) 2026
 *
 */
#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include "common.h"
#include "graph.h"

#define MAX_BUFF_SIZE 32

/**
 * @brief Layout of the shared memory.
 * @details Each slot of the circular buffer holds one complete solution
 *          (a set of at most GRAPH_MAX_SOLUTION_EDGES edges). The graph itself
 *          is NOT stored here: every generator keeps its own copy.
 *          alive is set to 1 by the supervisor and to 0 to make all generators terminate.
 */
struct shm {
	struct GRAPH_EDGE_LIST data[MAX_BUFF_SIZE];
	size_t readhead;
	size_t writehead;
};

/*SHARED MEMORY OPTIONS */
#define SHM_NAME "/eXXXXXXXX_shm"
#define SHM_OFLAG_SERVER (O_RDWR | O_CREAT | O_EXCL)
#define SHM_OFLAG_CLIENT O_RDWR
#define SHM_MODE S_IRWXU
#define MMP_PROT_FLAGS (PROT_READ | PROT_WRITE)
#define MMP_FLAGS MAP_SHARED
#define MMP_OFFSET 0
/*END SHARED MEMORY OPTIONS*/

/*SEMAPHORE OPTIONS */
/**
 * @brief For the circular buffer use 3 semaphores, sem_t *free, sem_t *used, sem_t *write. Each of these semphores are
needed for proper IPC communication between processess
 *
 */
#define FREE_SPACE_SEMAPHORE "/eXXXXXXXX_sem_free"
#define FREE_SPACE_SEMAPHORE_SIZE MAX_BUFF_SIZE

#define USED_SPACE_SEMAPHORE "/eXXXXXXXX_sem_used"
#define USED_SPACE_SEMAPHORE_SIZE 0

#define WRITE_SPACE_SEMAPHORE "/eXXXXXXXX_sem_write" // mutex
#define WRITE_SPACE_SEMAPHORE_SIZE 1

#define OPEN_SEM_MODE S_IRWXU
#define OPEN_SEM_MODE_SERVER (O_RDWR | O_CREAT | O_EXCL)
#define OPEN_SEM_MODE_CLIENT O_RDWR
/*END SEMAPHORE OPTIONS */

/*SEMAPHORE ERROR*/
#define OPENING_SHM_ERROR_SERVER "Creating shared memory failed"
#define OPENING_SHM_ERROR_CLIENT "Opening shared memory failed. Ensure a supervisor is running"

#define TRUNCATING_SHM_ERROR "Initializing shared memory failed"
#define MAPPING_SHM_ERROR "Mapping shared memory failed"
#define CLOSING_SHM_ERROR "Closing shared memory file failed"
#define UNMAPPING_SHM_ERROR "Unmapping shared memory failed"
#define UNLINKING_SHM_ERROR "Unlinking shared memory failed"

#define OPENING_SEM_ERROR "Opening semaphore failed"
#define CLOSING_SEM_ERROR "Closing semaphore failed"
#define UNLINKING_SEM_ERROR "Unlinking semaphore failed"
/*END SEMAPHORE ERROR*/

/* Semaphore functions*/
/**
 * @brief Creates and maps the shared memory segment (server side)
 * @details Calls shm_open(3) with O_CREAT | O_EXCL, ftruncate(2) to size it,
 *          and mmap(2) to map it into the process's address space.
 *          Terminates the program with EXIT_FAILURE and prints the
 *          corresponding error message if any of these calls fail.
 *
 * @param shmfd Pointer to an int that will receive the shared memory file
 *              descriptor. Caller must keep it for cleanSharedMemory_Server.
 * @return struct shm*
 */
struct shm *sharedMemory_Server(int *shmfd);

/**
 * @brief Opens and maps an existing shared memory segment (client side)
 * @details Calls shm_open(3) without O_CREAT, and mmap(2) to map the
 *          segment created by a server.
 *          Terminates the program with EXIT_FAILURE and prints the
 *          corresponding error message if any of these calls fail.
 *
 * @param shmfd Pointer to an int that will receive the shared memory file
 *              descriptor. Caller must keep it for cleanSharedMemory_Client.
 * @return struct shm*
 */
struct shm *sharedMemory_Client(int *shmfd);

/**
 * @brief Unmaps, closes, and unlinks the shared memory segment (server side)
 * @details Calls munmap(2), close(2), and shm_unlink(3), in that order.
 *          After this call, shm points to invalid memory and must not be
 *          dereferenced.
 *
 * @param shm   Pointer to the shared memory, as returned by initializeSharedMemory_Server.
 * @param shmfd The file descriptor returned via initializeSharedMemory_Server's shmfd param.
 */
void cleanSharedMemory_Server(struct shm *shm, int shmfd);

/**
 * @brief Unmaps and closes the shared memory segment (client side)
 * @details Calls munmap(2) and close(2). Does NOT unlink the segment,
 *          since the server or other clients may still be using it.
 *          After this call, shm points to invalid memory and must not be
 *          dereferenced.
 *
 * @param shm   Pointer to the shared memory, as returned by sharedMemory_Client.
 * @param shmfd The file descriptor returned via sharedMemory_Client's shmfd param.
 */
void cleanSharedMemory_Client(struct shm *shm, int shmfd);

/**
 * @brief Creates and initializes a named semaphore (server side)
 * @details Calls sem_open(3) with O_CREAT | O_EXCL and the given initial
 *          value. Terminates the program with EXIT_FAILURE and prints the
 *          corresponding error message on failure.
 *
 * @param sem_name   The semaphore's name, as used by sem_open(3);
 *                    see sem_overview(7) for naming rules.
 * @param init_value The initial value of the semaphore.
 * @return sem_t*
 */
sem_t *initializeSemaphore_Server(const char *sem_name,
    unsigned int init_value);

/**
 * @brief Opens an existing named semaphore (client side)
 * @details Calls sem_open(3) without O_CREAT. Terminates the program
 *          with EXIT_FAILURE if no semaphore with that name exists.
 *
 * @param sem_name The semaphore's name, matching one used by the server.
 * @return sem_t*
 */
sem_t *initializeSemaphore_Client(const char *sem_name);

/**
 * @brief Closes and unlinks a semaphore (server side)
 * @details Calls sem_close(3) and sem_unlink(3), in that order.
 *
 * @param semaphore Pointer to the semaphore, as returned by initializeSemaphore_Server.
 * @param sem_name  The semaphore's name (needed for sem_unlink).
 */
void cleanSemaphore_Server(sem_t *semaphore, const char *sem_name);

/**
 * @brief Closes a semaphore without unlinking it (client side)
 * @details Calls sem_close(3) only, since the server or other clients
 *          may still hold the semaphore open.
 *
 * @param semaphore Pointer to the semaphore, as returned by semaphore_Client.
 */
void cleanSemaphore_Client(sem_t *semaphore);

void onSignal(int sig, siginfo_t *si, void *unused);
void initSignalHandler(void);
#endif
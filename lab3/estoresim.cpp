#include <cstring>
#include <cstdlib>
#include <stdio.h>

#include "EStore.h"
#include "TaskQueue.h"
#include "RequestGenerator.h"

class Simulation
{
    public:
    TaskQueue supplierTasks;
    TaskQueue customerTasks;
    EStore store;

    int maxTasks;
    int numSuppliers;
    int numCustomers;

    explicit Simulation(bool useFineMode) : store(useFineMode) { }
};

/*
 * ------------------------------------------------------------------
 * supplierGenerator --
 *
 *      The supplier generator thread. The argument is a pointer to
 *      the shared Simulation object.
 *
 *      Enqueue arg->maxTasks requests to the supplier queue, then
 *      stop all supplier threads by enqueuing arg->numSuppliers
 *      stop requests.
 *
 *      Use a SupplierRequestGenerator to generate and enqueue
 *      requests.
 *
 *      This thread should exit when done.
 *
 * Results:
 *      Does not return. Exit instead.
 *
 * ------------------------------------------------------------------
 */
static void*
supplierGenerator(void* arg)
{
    // TODO: Your code here.
    Simulation * sim = (Simulation*)arg;
    SupplierRequestGenerator generator(&(sim->supplierTasks));
    generator.enqueueTasks(sim->maxTasks, &(sim->store));
    
    sthread_exit();
}

/*
 * ------------------------------------------------------------------
 * customerGenerator --
 *
 *      The customer generator thread. The argument is a pointer to
 *      the shared Simulation object.
 *
 *      Enqueue arg->maxTasks requests to the customer queue, then
 *      stop all customer threads by enqueuing arg->numCustomers
 *      stop requests.
 *
 *      Use a CustomerRequestGenerator to generate and enqueue
 *      requests.  For the fineMode argument to the constructor
 *      of CustomerRequestGenerator, use the output of
 *      store.fineModeEnabled() method, where store is a field
 *      in the Simulation class.
 *
 *      This thread should exit when done.
 *
 * Results:
 *      Does not return. Exit instead.
 *
 * ------------------------------------------------------------------
 */
static void*
customerGenerator(void* arg)
{
    // TODO: Your code here.
    return NULL; // Keep compiler happy.
}

/*
 * ------------------------------------------------------------------
 * supplier --
 *
 *      The main supplier thread. The argument is a pointer to the
 *      shared Simulation object.
 *
 *      Dequeue Tasks from the supplier queue and execute them.
 *
 * Results:
 *      Does not return.
 *
 * ------------------------------------------------------------------
 */
static void*
supplier(void* arg)
{
    // TODO: Your code here.
    Simulation * sim = (Simulation*)arg;
    Task task = sim->supplierTasks.dequeue();
    (*task.handler)(arg);
}

/*
 * ------------------------------------------------------------------
 * customer --
 *
 *      The main customer thread. The argument is a pointer to the
 *      shared Simulation object.
 *
 *      Dequeue Tasks from the customer queue and execute them.
 *
 * Results:
 *      Does not return.
 *
 * ------------------------------------------------------------------
 */
static void*
customer(void* arg)
{
   Simulation * sim = (Simulation*)arg;
    Task task = sim->customerTasks.dequeue();
    (*task.handler)(arg);
}

/*
 * ------------------------------------------------------------------
 * startSimulation --
 *      Create a new Simulation object. This object will serve as
 *      the shared state for the simulation. 
 *
 *      Create the following threads:
 *          - 1 supplier generator thread.
 *          - 1 customer generator thread.
 *          - numSuppliers supplier threads.
 *          - numCustomers customer threads.
 *
 *      After creating the worker threads, the main thread
 *      should wait until all of them exit, at which point it
 *      should return.
 *
 *      Hint: Use sthread_join.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
/*
static void * test1(void* arg){
	printf("1\n");
}
static void * test2(void* arg){
	printf("2\n");
}
static void * test3(void* arg){
	printf("3\n");
}
static void * test4(void* arg){
	printf("4\n");
}
*/
static void
startSimulation(int numSuppliers, int numCustomers, int maxTasks, bool useFineMode)
{	
	
    // TODO: Your code here.
	Simulation sim(useFineMode);
	sim.maxTasks = maxTasks;
	sim.numSuppliers = numSuppliers;
	sim.numCustomers = numCustomers;
	sthread_t sup_gen;
	sthread_t cus_gen; 
	sthread_t Suppliers_t;
	sthread_t Customers_t;
	sthread_create(&sup_gen,supplierGenerator, &sim);
	sthread_create(&cus_gen,customerGenerator, &sim);
	sthread_create(&Suppliers_t,supplier, &sim);
	sthread_create(&Customers_t,customer, &sim);
	sthread_join(sup_gen);
	sthread_join(cus_gen);
	sthread_join(Suppliers_t);
	sthread_join(Customers_t);
	
}

int main(int argc, char **argv)
{
    bool useFineMode = false;

    // Seed the random number generator.
    // You can remove this line or set it to some constant to get deterministic
    // results, but make sure you put it back before turning in.
    srand(time(NULL));

    if (argc > 1)
        useFineMode = strcmp(argv[1], "--fine") == 0;
    startSimulation(10, 10, 100, useFineMode);
    return 0;
}


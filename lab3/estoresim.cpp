#include <cstring>
#include <cstdlib>
#include <iostream>

#include "EStore.h"
#include "TaskQueue.h"
#include "RequestGenerator.h"

using namespace std;

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
    cout << "Running supplier generator...\n";

    Simulation *sim = (Simulation *)arg;
    //create an generator
    SupplierRequestGenerator generator(&sim->supplierTasks);

    //enqueue Tasks
    generator.enqueueTasks(sim->maxTasks, &(sim->store));

    //enqueue stop requests
    generator.enqueueStops(sim->numSuppliers);

    sthread_exit();

    return NULL;
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
    cout << "Running customer generator...\n";

    Simulation *sim = (Simulation *)arg;
    //create generator
    CustomerRequestGenerator  generator(&sim->customerTasks, sim->store.fineModeEnabled());

    //enqueue tasks 
    generator.enqueueTasks(sim->maxTasks, &(sim->store));

    //enque stop requests
    generator.enqueueStops(sim->numCustomers);

    sthread_exit();

    return NULL;
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
   
    Simulation *sim = (Simulation *)arg;
    //constantly consuming taks if there's one
    while (true) {
	 //consume a task
        Task task = sim->supplierTasks.dequeue();
	//execute the task
        task.handler(task.arg);
    }

    return NULL; // Keep compiler happy.
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
    // TODO: Your code here.
    
    Simulation *sim = (Simulation *)arg;
    //constantly consuming taks if there's one
    while (true) {
         //consume the task
        Task task = sim->customerTasks.dequeue();
	//execute the task
        task.handler(task.arg);
    }

    return NULL; // Keep compiler happy.
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
static void
startSimulation(int numSuppliers, int numCustomers, int maxTasks, bool useFineMode)
{
    // TODO: Your code here.
    cout << "Simulation starts...\n";

    Simulation *simulation = new Simulation(useFineMode);
    simulation->maxTasks = maxTasks;
    simulation->numSuppliers = numSuppliers;
    simulation->numCustomers = numCustomers;

    
    sthread_t supplier_generator_worker;
    sthread_t customer_generator_worker;
    sthread_t supplier_workers[numSuppliers];
    sthread_t customer_workers[numCustomers];
    //creat provider and consumer
    sthread_create(&supplier_generator_worker, supplierGenerator, simulation);
    sthread_create(&customer_generator_worker, customerGenerator, simulation);

    for (int i = 0; i < numCustomers; i++) {
        sthread_create(&customer_workers[i], customer, simulation);
    }

    for (int i = 0; i < numSuppliers; i++) {
        sthread_create(&supplier_workers[i], supplier, simulation);
    }

    //wait for all the threads to finish
    sthread_join(supplier_generator_worker);

    sthread_join(customer_generator_worker);
	
        for (int i = 0; i < numSuppliers; i++) {
        sthread_join(supplier_workers[i]);
    }    

    for (int i = 0; i < numCustomers; i++) {
        sthread_join(customer_workers[i]);
    }

    

    
    cout << "Simulation ends...\n";
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



#include "TaskQueue.h"

TaskQueue::
TaskQueue()
{
    // TODO: Your code here.
    smutex_init(&this->mutex);
    scond_init(&this->waiting);
}

TaskQueue::
~TaskQueue()
{
    // TODO: Your code here.
    smutex_destroy(&this->mutex);
    scond_destroy(&this->waiting);
}

/*
 * ------------------------------------------------------------------
 * size --
 *
 *      Return the current size of the queue.
 *
 * Results:
 *      The size of the queue.
 *
 * ------------------------------------------------------------------
 */
int TaskQueue::
size()
{
    // TODO: Your code here.
    smutex_lock(&this->mutex);

    int size = this->q.size();

    smutex_unlock(&this->mutex);

    return size; // Keep compiler happy until routine done.
}

/*
 * ------------------------------------------------------------------
 * empty --
 *
 *      Return whether or not the queue is empty.
 *
 * Results:
 *      The true if the queue is empty and false otherwise.
 *
 * ------------------------------------------------------------------
 */
bool TaskQueue::
empty()
{
    // TODO: Your code here.
    smutex_lock(&(this->mutex));

    bool is_empty = this->q.empty();

    smutex_unlock(&(this->mutex));

    return is_empty; // Keep compiler happy until routine done.
}

/*
 * ------------------------------------------------------------------
 * enqueue --
 *
 *      Insert the task at the back of the queue.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void TaskQueue::
enqueue(Task task)
{
    // TODO: Your code here.
    smutex_lock(&this->mutex);

    this->q.push(task);

    scond_signal(&this->waiting, &this->mutex);
    smutex_unlock(&this->mutex);
}

/*
 * ------------------------------------------------------------------
 * dequeue --
 *
 *      Remove the Task at the front of the queue and return it.
 *      If the queue is empty, block until a Task is inserted.
 *
 * Results:
 *      The Task at the front of the queue.
 *
 * ------------------------------------------------------------------
 */
Task TaskQueue::
dequeue()
{
    // TODO: Your code here.
    smutex_lock(&this->mutex);

    while (this->q.empty()) {
        scond_wait(&this->waiting, &this->mutex);
    }

    Task task = this->q.front();
    this->q.pop();

    smutex_unlock(&this->mutex);

    return task; 
}


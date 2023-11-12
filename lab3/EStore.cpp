#include <cassert>
#include <cstdio>
#include "EStore.h"

using namespace std;


Item::
Item() : valid(false)
{
}

Item::
~Item()
{
}


EStore::
EStore(bool enableFineMode)
    : fineMode(enableFineMode)
{
    // TODO: Your code here.
    this->store_discount = 0;
    this->shipping_cost = 3;
    
    //initialize the lock and CV
    smutex_init(&this->mutex);
    scond_init(&this->avl);
    //initialize item list
    for(int i = 0; i < INVENTORY_SIZE; i++){
	inventory[i].valid = false;
        inventory[i].quantity = 0;
        inventory[i].price = 0;
        inventory[i].discount = 0;
    }
    //set mode
    if (this->fineMode) {
        for (int i = 0; i < INVENTORY_SIZE; i++) {
            smutex_init(&this->mutexs[i]);
        }
    }
}

EStore::
~EStore()
{
    // TODO: Your code here.
    //delete lock and CV
    smutex_destroy(&this->mutex);
    scond_destroy(&this->avl);
    //check mode
    if (this->fineMode) {
        for (int i = 0; i < INVENTORY_SIZE; i++) {
            smutex_destroy(&this->mutexs[i]);
        }
    }
}

/*
 * ------------------------------------------------------------------
 * buyItem --
 *
 *      Attempt to buy the item from the store.
 *
 *      An item can be bought if:
 *          - The store carries it.
 *          - The item is in stock.
 *          - The cost of the item plus the cost of shipping is no
 *            more than the budget.
 *
 *      If the store *does not* carry this item, simply return and
 *      do nothing. Do not attempt to buy the item.
 *
 *      If the store *does* carry the item, but it is not in stock
 *      or its cost is over budget, block until both conditions are
 *      met (at which point the item should be bought) or the store
 *      removes the item from sale (at which point this method
 *      returns).
 *
 *      The overall cost of a purchase for a single item is defined
 *      as the current cost of the item times 1 - the store
 *      discount, plus the flat overall store shipping fee.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void EStore::
buyItem(int item_id, double budget)
{
    assert(!fineModeEnabled());
	
    // TODO: Your code here.
    if (item_id < 0 || item_id >= INVENTORY_SIZE) {
        return;
    }

    smutex_lock(&this->mutex);

    Item item = this->inventory[item_id];
    //check whethter the store has it
    if (!item.valid) {
        smutex_unlock(&this->mutex);
	//printf("No such a item!\n");
        return;
    }
    //if not available or not enought money
    while (item.quantity == 0 ||  (item.price * (1 - item.discount) * (1 - this->store_discount) + this->shipping_cost) - budget > 0.0000001 ) {
        scond_wait(&this->avl, &this->mutex);
    }

    this->inventory[item_id].quantity--;

    smutex_unlock(&this->mutex);
}

/*
 * ------------------------------------------------------------------
 * buyManyItem --
 *
 *      Attempt to buy all of the specified items at once. If the
 *      order cannot be bought, give up and return without buying
 *      anything. Otherwise buy the entire order at once.
 *
 *      The entire order can be bought if:
 *          - The store carries all items.
 *          - All items are in stock.
 *          - The cost of the the entire order (cost of items plus
 *            shipping for each item) is no more than the budget.
 *
 *      If multiple customers are attempting to buy at the same
 *      time and their orders are mutually exclusive (i.e., the
 *      two customers are not trying to buy any of the same items),
 *      then their orders must be processed at the same time.
 *
 *      For the purposes of this lab, it is OK for the store
 *      discount and shipping cost to change while an order is being
 *      processed.
 *
 *      The cost of a purchase of many items is the sum of the
 *      costs of purchasing each item individually. The purchase
 *      cost of an individual item is covered above in the
 *      description of buyItem.
 *
 *      Challenge: For bonus points, implement a version of this
 *      method that will wait until the order can be fulfilled
 *      instead of giving up. The implementation should be efficient
 *      in that it should not wake up threads unecessarily. For
 *      instance, if an item decreases in price, only threads that
 *      are waiting to buy an order that includes that item should be
 *      signaled (though all such threads should be signaled).
 *
 *      Challenge: For bonus points, ensure that the shipping cost
 *      and store discount does not change while processing an
 *      order.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void EStore::
buyManyItems(vector<int>* item_ids, double budget)
{
    assert(fineModeEnabled());
     
    // TODO: Your code here.
    double total = 0;

    for (unsigned int i = 0; i < item_ids->size(); i++) {
        int item_id = (*item_ids)[i];
    //validation
        if (item_id < 0 || item_id >= INVENTORY_SIZE) {
            return;
        }

        smutex_lock(&this->mutexs[item_id]);

        Item item = this->inventory[item_id];
    //in stock or not available
        if (item.quantity == 0 || !item.valid ) {
            smutex_unlock(&this->mutexs[item_id]);

            return;
        }
    //add to total price
        total += item.price * (1 - item.discount) *  (1 - this->store_discount) + this->shipping_cost;
    //see whether we have enough money
        if (total > budget) {
            smutex_unlock(&this->mutexs[item_id]);
            return;
        }

	
        smutex_unlock(&this->mutexs[item_id]);
    }
    //reduce the amount, if the entire order can be processed
    for (unsigned int i = 0; i < item_ids->size(); i++) {
        int item_id = item_ids->data()[i];

        smutex_lock(&this->mutexs[item_id]);
    //update item amount
        this->inventory[item_id].quantity--;

        smutex_unlock(&this->mutexs[item_id]);
    }
    
}

/*
 * ------------------------------------------------------------------
 * addItem --
 *
 *      Add the item to the store with the specified quantity,
 *      price, and discount. If the store already carries an item
 *      with the specified id, do nothing.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void EStore::
addItem(int item_id, int quantity, double price, double discount)
{
    // TODO: Your code here.
    if (item_id < 0 || item_id >= INVENTORY_SIZE) {
        return ;
    }

    smutex_t *mutex = this->fineMode ? &this->mutexs[item_id] : &this->mutex;

    smutex_lock(mutex);
    //if it's already in the store
    if (this->inventory[item_id].valid) {
        smutex_unlock(mutex);
        return;
    }
    //add information
    this->inventory[item_id].valid = true;
    this->inventory[item_id].quantity = quantity;
 this->inventory[item_id].discount = discount;
    this->inventory[item_id].price = price;
   
    //wake up cusomters
    scond_broadcast(&this->avl, mutex);
    smutex_unlock(mutex);
}

/*
 * ------------------------------------------------------------------
 * removeItem --
 *
 *      Remove the item from the store. The store no longer carries
 *      this item. If the store is not carrying this item, do
 *      nothing.
 *
 *      Wake any waiters.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void EStore::
removeItem(int item_id)
{
    // TODO: Your code here.
    if (item_id < 0 || item_id >= INVENTORY_SIZE) {
        return;
    }

    smutex_t *mutex = this->fineMode ? &this->mutexs[item_id] : &this->mutex;

    smutex_lock(mutex);
    //if store doesn't have it
    if (!(this->inventory[item_id].valid)) {
        smutex_unlock(mutex);
        return;
    }
    //set to not valid
    this->inventory[item_id].valid = false;

    smutex_unlock(mutex);
}

/*
 * ------------------------------------------------------------------
 * addStock --
 *
 *      Increase the stock of the specified item by count. If the
 *      store does not carry the item, do nothing. Wake any waiters.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void EStore::
addStock(int item_id, int count)
{
    // TODO: Your code here.
    if (item_id < 0 || item_id >= INVENTORY_SIZE) {
        return;
    }
    //check mode
    smutex_t *mutex = this->fineMode ? &this->mutexs[item_id] : &this->mutex;

    smutex_lock(mutex);
    //not valid
    if (!(this->inventory[item_id].valid)) {
        smutex_unlock(mutex);

        return;
    }
    //add items
    this->inventory[item_id].quantity += count;
    //wake up costumers
    scond_broadcast(&this->avl, mutex);
    smutex_unlock(mutex);
}

/*
 * ------------------------------------------------------------------
 * priceItem --
 *
 *      Change the price on the item. If the store does not carry
 *      the item, do nothing.
 *
 *      If the item price decreased, wake any waiters.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void EStore::
priceItem(int item_id, double price)
{
    // TODO: Your code here.
    if (item_id < 0 || item_id >= INVENTORY_SIZE) {
        return;
    }

    smutex_t *mutex = this->fineMode ? &this->mutexs[item_id] : &this->mutex;

    smutex_lock(mutex);

    //if the store doesn't have it
    if (!(this->inventory[item_id].valid)) {
        smutex_unlock(mutex);

        return;
    }
    
    double original = this->inventory[item_id].price;

    this->inventory[item_id].price = price;
    //wake up costumers if there's a lower price
    if (original > price) 
        scond_broadcast(&this->avl, mutex);
    

    smutex_unlock(mutex);
}

/*
 * ------------------------------------------------------------------
 * discountItem --
 *
 *      Change the discount on the item. If the store does not carry
 *      the item, do nothing.
 *
 *      If the item discount increased, wake any waiters.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void EStore::
discountItem(int item_id, double discount)
{
    // TODO: Your code here.
    if (item_id < 0 || item_id >= INVENTORY_SIZE) {
        return;
    }
    //check mode
    smutex_t *mutex = this->fineMode ? &this->mutexs[item_id] : &this->mutex;

    smutex_lock(mutex);
    //if not in store
    if (!(this->inventory[item_id].valid)) {
        smutex_unlock(mutex);
        return;
    }

    double original= this->inventory[item_id].discount;

    this->inventory[item_id].discount = discount;
    //wake up customer if lower price
    if (original< discount) 
        scond_broadcast(&this->avl, mutex);
    
    smutex_unlock(mutex);
}

/*
 * ------------------------------------------------------------------
 * setShippingCost --
 *
 *      Set the per-item shipping cost. If the shipping cost
 *      decreased, wake any waiters.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void EStore::
setShippingCost(double cost)
{
    // TODO: Your code here.
    smutex_lock(&this->mutex);

    double original = this->shipping_cost;

    this->shipping_cost = cost;
    //wake up customer if lower price
    if (original > cost) {
        scond_broadcast(&this->avl, &this->mutex);
    }

    smutex_unlock(&this->mutex);
}

/*
 * ------------------------------------------------------------------
 * setStoreDiscount --
 *
 *      Set the store discount. If the discount increased, wake any
 *      waiters.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void EStore::
setStoreDiscount(double discount)
{
    // TODO: Your code here.
    smutex_lock(&this->mutex);

    double original = this->store_discount;

    this->store_discount = discount;
    //wake up customer if lower price
    if (original < discount) {
        scond_broadcast(&this->avl, &this->mutex);
    }

    smutex_unlock(&this->mutex);
}



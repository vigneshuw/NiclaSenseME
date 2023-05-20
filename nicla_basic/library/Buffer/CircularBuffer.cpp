#include "CircularBuffer.h"
#include <zephyr/sys/util.h>


CircularBufferFIFO::CircularBufferFIFO(uint16_t size) : _full(false), _head(0), _size(size) {
    // Initialize the FIFO
    k_fifo_init(&_buf_fifo);


}

CircularBufferFIFO::~CircularBufferFIFO()
{

}

int CircularBufferFIFO::put(void *data) {

    // Check if FIFO is already full
    if(_full) {
        void *fifo_data = k_fifo_get(&_buf_fifo, K_NO_WAIT);
        _head--;
        k_free(fifo_data);
        _full = false;
    }

    // Put the data into buffer
    int ret = k_fifo_alloc_put(&_buf_fifo, data);
    _head++;

    if(_head == _size) {
        // FIFO is full
        _full = true;
    }

    return ret;

}

bool CircularBufferFIFO::get(void *data) {
    // Get the data 
    void *fifo_data = k_fifo_get(&_buf_fifo, K_NO_WAIT);
    if(fifo_data) {
        // Ensure the item has been removed 
        _head--;
        bytecpy(data, fifo_data, sizeof(data));

        // Free the heap
        k_free(fifo_data);

        // Reset full
        _full = false;

        return true;
    } else {
        return false;
    }
    
}

bool CircularBufferFIFO::is_full() {
    return _full;

}

bool CircularBufferFIFO::is_empty() {
    int ret = k_fifo_is_empty(&_buf_fifo);
    if (ret == 0) {
        return true;
    } else {
        return false;
    }
}








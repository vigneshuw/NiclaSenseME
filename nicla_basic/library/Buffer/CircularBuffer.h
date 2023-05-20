#ifndef _CIRCULAR_BUFFER_H_
#define _CIRCULAR_BUFFER_H_

#include <zephyr/types.h>
#include <zephyr/kernel.h>

#ifdef __cplusplus
extern "C" {
#endif


class CircularBufferFIFO {
    public:
        CircularBufferFIFO(uint16_t size);     
        ~CircularBufferFIFO();

        /** @brief Put data into the buffer. Old data is overwritten if over size.
         * 
         * @param   data    Pointer to the data that is to be put in the buffer
         * 
         * @return  0, if success;  -ENOMEM, if there isn't sufficient RAM in the caller's resource pool
        */ 
        int put(void *data);
        
        /** @brief Get data from the buffer by copy into the presented buffer. Once data is read, the memory is freed.
         * 
         * @param   data    Pointer to the data that is to be put in the buffer
         * 
         * @return  1, if data is returned; 0, otherwise
        */
        bool get(void *data);

        /** @brief Check if the circular buffer is full.
         * 
         * @return true, if full 
        */
        bool is_full(void);

        /** @brief Check if the buffer is empty 
         * 
         * @return true, if empty
        */
       bool is_empty(void);



    private:
        // Size to set for FIFO
        uint16_t _size;

        struct k_fifo _buf_fifo;
        uint16_t _head;
        bool _full;


};







#ifdef __cplusplus
}
#endif

#endif
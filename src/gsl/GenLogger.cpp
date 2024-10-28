#include "GenLogger.hpp"

#include "ring_buffer_on_shmem.hpp" 
#include "shared_mem_manager.hpp" 

SharedMemRingBuffer gSharedMemRingBuffer (SLEEPING_WAIT);

namespace Verdi
{

namespace GSL {

int Init(void)
{
    int MAX_RBUFFER_CAPACITY = 1024*8; //should be same as consumer
    int MAX_RAW_MEM_BUFFER_SIZE = 1000000; //should be same as consumer

    if(! gSharedMemRingBuffer.Init(123456,
                                   MAX_RBUFFER_CAPACITY, 
                                   923456,
                                   MAX_RAW_MEM_BUFFER_SIZE ) ) { 
        //Error!
        return -1; 
    }
    return 0;
}

void DisruptorMessage(std::string message)
{
    char raw_data[1024];
    int64_t my_index = -1;

    PositionInfo my_data;
    size_t write_position = 0;

    my_data.len = snprintf(raw_data, sizeof(raw_data), "%s", message.c_str());

    my_index = gSharedMemRingBuffer.ClaimIndex(my_data.len, & write_position );

    my_data.start_position = write_position  ; 
    my_data.offset_position = write_position + my_data.len;
    my_data.status = DATA_EXISTS ;
         
    gSharedMemRingBuffer.SetData( my_index, &my_data, write_position, raw_data );

    gSharedMemRingBuffer.Commit( my_index); 
}

} // namespace GSL
} // namespace Verdi

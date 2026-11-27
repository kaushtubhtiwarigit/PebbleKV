#include "thread_compat.h"

#ifdef _WIN32

#include <winsock2.h>
typedef SOCKET socket_t;

namespace pebblekv {

// Forward declare Server
class Server;

unsigned __stdcall Thread::threadFunc(void* param) {
    struct ThreadData {
        void* func;
        void* obj;
        void* arg;
    };
    
    ThreadData* data = static_cast<ThreadData*>(param);
    
    // Cast function pointer and call it
    typedef void (*FuncPtr)(void*, socket_t);
    FuncPtr func = reinterpret_cast<FuncPtr>(data->func);
    socket_t socket = reinterpret_cast<socket_t>(data->arg);
    
    func(data->obj, socket);
    
    delete data;
    return 0;
}

} // namespace pebblekv

#endif

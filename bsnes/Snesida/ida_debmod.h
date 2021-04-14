#pragma once

//
//
//      This is the base debmod_t class definition
//      From this class all debugger code must inherite and specialize
//
//      Some OS specific functions must be implemented:
//        bool init_subsystem();
//        bool term_subsystem();
//        debmod_t *create_debug_session();
//        int create_thread(thread_cb_t thread_cb, void *context);
//

#undef INLINE

#include <deque>
#include <ida.hpp>
#include <idd.hpp>

//--------------------------------------------------------------------------
// Very simple class to store pending events
enum queue_pos_t
{
    IN_FRONT,
    IN_BACK
};

struct eventlist_t : public std::deque<debug_event_t>
{
private:
    bool synced;
public:
    // save a pending event
    void enqueue(const debug_event_t &ev, queue_pos_t pos)
    {
        if (pos != IN_BACK)
            push_front(ev);
        else
            push_back(ev);
    }

    // retrieve a pending event
    bool retrieve(debug_event_t *event)
    {
        if (empty())
            return false;
        // get the first event and return it
        *event = front();
        pop_front();
        return true;
    }
};

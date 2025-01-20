#include "tick.h"
#include <time.h>

// static tick_t tick = 0;

// void TickIncrement(void)
// {
//     tick++;
// }

tick_t TickGetTicks()
{

    struct timespec ts;
    clock_gettime(0, &ts); // would have preffered CLOCK_REALTIME or similar but
                           // 0 works. May have issue on Nanomind platform
    return (tick_t)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000L);
    // #ifndef ARDUINO_VERSION
    // return (tick_t)millis();
    // #else
    // // TICK_INTERRUPT_DISABLE;

    // tick_t retval = tick;//ensures that the interrupt doesn't corrupt the
    // tick value during update

    // // TICK_INTERRUPT_ENABLE;
    // // return retval;
    // #endif
}

tick_t TicksSince(tick_t previousTick)
{
    tick_t protectedTick = TickGetTicks();
    if (previousTick > protectedTick)
    {
        // rollover so deal with it
        return (tick_t_MAX - previousTick + protectedTick + 1);
    }
    else
    {
        // no rollover
        return (protectedTick - previousTick);
    }
}

bool TicksHavePassed(tick_t* tickVariable, tick_t duration)
{
    // If you get a Segmentation fault attempting to enter this method
    // you may have forgotten the & on the tickVariable
    tick_t elapsedTicks = TicksSince(*tickVariable);
    if (elapsedTicks >= duration)
    {
        *tickVariable += elapsedTicks;
        return true;
    }
    return false;
}

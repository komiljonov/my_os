#include <timer.h>
#include <common/common.h>
#include <hardwarecommunication/interrupts.h>

using namespace myos::common;
using namespace myos::hardwarecommunication;

namespace myos
{

    static volatile common::uint64_t g_ticks = 0;
    static common::uint32_t g_hz = 0;

    static Timer::Event g_events[Timer::MAX_EVENTS];
    static int g_nextId = 1;

    void Timer::Init(common::uint32_t hz)
    {
        g_hz = hz;
        for (int i = 0; i < MAX_EVENTS; i++)
            g_events[i].active = false;
    }

    common::uint64_t Timer::NowTicks() { return g_ticks; }
    common::uint32_t Timer::Hz() { return g_hz; }

    int Timer::After(common::uint32_t ticks, TimerCallback cb, void *ctx)
    {
        if (ticks == 0 || cb == nullptr)
            return -1;

        for (int i = 0; i < MAX_EVENTS; i++)
        {
            if (!g_events[i].active)
            {
                g_events[i].active = true;
                g_events[i].periodic = false;
                g_events[i].remaining = ticks;
                g_events[i].period = 0;
                g_events[i].cb = cb;
                g_events[i].ctx = ctx;
                g_events[i].id = g_nextId++;
                return g_events[i].id;
            }
        }
        return -1;
    }

    int Timer::Every(common::uint32_t periodTicks, TimerCallback cb, void *ctx)
    {
        if (periodTicks == 0 || cb == nullptr)
            return -1;

        for (int i = 0; i < MAX_EVENTS; i++)
        {
            if (!g_events[i].active)
            {
                g_events[i].active = true;
                g_events[i].periodic = true;
                g_events[i].remaining = periodTicks;
                g_events[i].period = periodTicks;
                g_events[i].cb = cb;
                g_events[i].ctx = ctx;
                g_events[i].id = g_nextId++;
                return g_events[i].id;
            }
        }
        return -1;
    }

    void Timer::Cancel(int id)
    {
        if (id <= 0)
            return;
        for (int i = 0; i < MAX_EVENTS; i++)
            if (g_events[i].active && g_events[i].id == id)
                g_events[i].active = false;
    }

    void Timer::OnTick()
    {
        g_ticks++;

        // Fire due events
        for (int i = 0; i < MAX_EVENTS; i++)
        {
            if (!g_events[i].active)
                continue;

            if (g_events[i].remaining > 0)
                g_events[i].remaining--;

            if (g_events[i].remaining == 0)
            {
                // call
                TimerCallback cb = g_events[i].cb;
                void *ctx = g_events[i].ctx;

                if (g_events[i].periodic)
                    g_events[i].remaining = g_events[i].period;
                else
                    g_events[i].active = false;

                // Keep callbacks short! (or just set flags)
                cb(ctx);
            }
        }
    }

    TimerInterruptHandler::TimerInterruptHandler(uint8_t interruptNumber, InterruptManager *manager)
        : InterruptHandler(interruptNumber, manager)
    {
    }

    uint32_t TimerInterruptHandler::HandleInterrupt(uint32_t esp)
    {
        myos::Timer::OnTick();
        return esp;
    }

}

#pragma once

#pragma once
#include <common/types.h>
#include <hardwarecommunication/interrupts.h>

namespace myos
{

    using TimerCallback = void (*)(void *);

    class Timer
    {
    public:
        static void Init(common::uint32_t hz);
        static void OnTick(); // called from IRQ handler
        static common::uint64_t NowTicks();
        static common::uint32_t Hz();

        static int After(common::uint32_t ticks, TimerCallback cb, void *ctx);
        static int Every(common::uint32_t periodTicks, TimerCallback cb, void *ctx);
        static void Cancel(int id);

        struct Event
        {
            bool active;
            bool periodic;
            common::uint32_t remaining;
            common::uint32_t period;
            TimerCallback cb;
            void *ctx;
            int id;
        };

        static constexpr int MAX_EVENTS = 64;
    };

    class TimerInterruptHandler : public hardwarecommunication::InterruptHandler
    {
    public:
        TimerInterruptHandler(common::uint8_t interruptNumber, hardwarecommunication::InterruptManager *manager);

        virtual common::uint32_t HandleInterrupt(common::uint32_t esp) override;
    };

}

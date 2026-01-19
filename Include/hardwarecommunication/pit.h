#ifndef __MYOS__HARDWARECOMMUNICATION_PIT
#define __MYOS__HARDWARECOMMUNICATION_PIT

#include <common/types.h>
#include <hardwarecommunication/port.h>

namespace myos
{
    namespace hardwarecommunication
    {

        class PIT
        {
            Port8Bit commandPort;
            Port8Bit channel0Port;

        public:
            PIT() : commandPort(0x43), channel0Port(0x40) {}

            void SetFrequency(common::uint32_t hz)
            {
                if (hz < 19)
                    hz = 19;
                if (hz > 1000)
                    hz = 1000;

                common::uint32_t divisor = 1193182 / hz;
                if (divisor == 0)
                    divisor = 1;
                if (divisor > 0xFFFF)
                    divisor = 0xFFFF;

                commandPort.Write(0x36); // ch0, lo/hi, mode3, binary
                channel0Port.Write((common::uint8_t)(divisor & 0xFF));
                channel0Port.Write((common::uint8_t)((divisor >> 8) & 0xFF));
            }
        };

    }
}

#endif
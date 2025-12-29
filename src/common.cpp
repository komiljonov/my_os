
#include <common/common.h>

#include <common/types.h>

namespace myos
{
    namespace common
    {

        void printf(char *str)
        {
            uint16_t *VideoMemory = (uint16_t *)0xb8000;

            static uint8_t x = 0, y = 0;

            for (int i = 0; str[i] != '\0'; ++i)
            {

                switch (str[i])
                {
                case '\n':
                    y++;
                    x = 0;
                    break;
                default:
                    VideoMemory[80 * y + x] = (VideoMemory[80 * y + x] & 0xFF00) | str[i];
                    x++;
                    break;
                }

                if (x >= 80)
                {
                    y++;
                    x = 0;
                }

                if (y >= 25)
                {
                    for (y = 0; y < 25; y++)
                        for (x = 0; x < 80; x++)
                            VideoMemory[80 * y + x] = (VideoMemory[80 * y + x] & 0xFF00) | ' ';

                    x = 0;
                    y = 0;
                }
            }
        }

        void printHex(uint8_t key)
        {
            char *foo = "00";
            char *hex = "0123456789ABCDEF";

            foo[0] = hex[(key >> 4) & 0xf];
            foo[1] = hex[key & 0xf];
            printf(foo);
        }

    }
}
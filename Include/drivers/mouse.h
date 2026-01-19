
#ifndef __MYOS__DRIVERS__MOUSE_H
#define __MYOS__DRIVERS__MOUSE_H

#include <common/types.h>
#include <hardwarecommunication/port.h>
#include <drivers/driver.h>
#include <hardwarecommunication/interrupts.h>
#include <drivers/vga.h>

namespace myos
{
    namespace drivers
    {

        constexpr myos::common::uint8_t BLACK = 0x00;
        constexpr myos::common::uint8_t BLUE = 0x01;
        constexpr myos::common::uint8_t GREEN = 0x02;
        constexpr myos::common::uint8_t CYAN = 0x03;
        constexpr myos::common::uint8_t RED = 0x04;
        constexpr myos::common::uint8_t MAGENTA = 0x05;
        constexpr myos::common::uint8_t BROWN = 0x06;
        constexpr myos::common::uint8_t LIGHTGRAY = 0x07;
        constexpr myos::common::uint8_t DARKGRAY = 0x08;
        constexpr myos::common::uint8_t LIGHTBLUE = 0x09;
        constexpr myos::common::uint8_t LIGHTGREEN = 0x0A;
        constexpr myos::common::uint8_t LIGHTCYAN = 0x0B;
        constexpr myos::common::uint8_t LIGHTRED = 0x0C;
        constexpr myos::common::uint8_t LIGHTMAGENTA = 0x0D;
        constexpr myos::common::uint8_t YELLOW = 0x0E;
        constexpr myos::common::uint8_t WHITE = 0x0F;

        class MouseEventHandler
        {

        public:
            MouseEventHandler();

            virtual void OnActivate();
            virtual void OnMouseDown(myos::common::uint8_t button);
            virtual void OnMouseUp(myos::common::uint8_t button);
            virtual void OnMouseMove(myos::common::int8_t xoffset, myos::common::int8_t yoffset);
        };

        class MouseDriver : public myos::hardwarecommunication::InterruptHandler, public Driver
        {
            myos::hardwarecommunication::Port8Bit dataport;
            myos::hardwarecommunication::Port8Bit commandport;
            myos::common::uint8_t buffer[3];
            myos::common::uint8_t offset;
            myos::common::uint8_t buttons;

            myos::common::int8_t x, y;

            MouseEventHandler *handler;

        public:
            MouseDriver(myos::hardwarecommunication::InterruptManager *manager, MouseEventHandler *handler);
            ~MouseDriver();
            virtual myos::common::uint32_t HandleInterrupt(myos::common::uint32_t esp);
            virtual void Activate();
        };

        class MouseToConsole : public MouseEventHandler
        {
            myos::common::int8_t x, y;

        public:
            MouseToConsole();
            virtual void OnMouseMove(myos::common::int8_t xoffset, myos::common::int8_t yoffset);
        };

        class MouseToVGAScreen : public MouseEventHandler
        {

            volatile  myos::common::int32_t prevX, prevY;
            volatile  myos::common::int32_t x, y;

            bool dirty;

            
            myos::common::uint8_t color_index = 0;
            
            myos::common::uint8_t COLORS[4] = {RED, YELLOW, GREEN, BROWN};
            
            VideoGraphicsArray *vga;

        public:
            MouseToVGAScreen(VideoGraphicsArray *vga);
            virtual void OnMouseMove(myos::common::int8_t xoffset, myos::common::int8_t yoffset);
            virtual void OnMouseDown(myos::common::uint8_t button);


            bool Dirty() {
                return dirty;
            }

            bool Render();

            void NextColor();
        };

    }
}
#endif

#ifndef __MYOS_DRIVERS__VGA_H
#define __MYOS_DRIVERS__VGA_H

#include <common/types.h>
#include <hardwarecommunication/port.h>
#include <drivers/driver.h>

namespace myos
{
    namespace drivers
    {

        class VideoGraphicsArray
        {

        private:
            myos::common::uint8_t backBuffer[320 * 200];

        protected:
            hardwarecommunication::Port8Bit miscPort;
            hardwarecommunication::Port8Bit crtcIndexPort;
            hardwarecommunication::Port8Bit crtcDataPort;
            hardwarecommunication::Port8Bit sequencerIndexPort;
            hardwarecommunication::Port8Bit sequencerDataPort;
            hardwarecommunication::Port8Bit graphicsControllerIndexPort;
            hardwarecommunication::Port8Bit graphicsControllerDataPort;
            hardwarecommunication::Port8Bit attributeControllerIndexPort;
            hardwarecommunication::Port8Bit attributeControllerReadPort;
            hardwarecommunication::Port8Bit attributeControllerWritePort;
            hardwarecommunication::Port8Bit attributeControllerResetPort;

            void WriteRegisters(myos::common::uint8_t *registers);
            myos::common::uint8_t *GetFrameBufferSegment();

            virtual myos::common::uint8_t GetColorIndex(myos::common::uint8_t r, myos::common::uint8_t g, myos::common::uint8_t b);

            myos::common::uint8_t *frameBufferSegment;

            void SetFrameBufferSegment();

        public:
            VideoGraphicsArray();
            ~VideoGraphicsArray();

            virtual bool SupportsMode(myos::common::uint32_t width, myos::common::uint32_t height, myos::common::uint32_t colorDepth);

            virtual bool SetMode(myos::common::uint32_t width, myos::common::uint32_t height, myos::common::uint32_t colorDepth);

            virtual void PutPixel(myos::common::uint32_t x, myos::common::uint32_t y, myos::common::uint8_t r, myos::common::uint8_t g, myos::common::uint8_t b);
            virtual void PutPixel(myos::common::uint32_t x, myos::common::uint32_t y, myos::common::uint8_t colorIndex);

            virtual void DrawRect(
                myos::common::uint32_t x,
                myos::common::uint32_t y,
                myos::common::uint32_t width,
                myos::common::uint32_t height,
                myos::common::uint8_t colorIndex);

            virtual void DrawRect(
                myos::common::uint32_t x,
                myos::common::uint32_t y,
                myos::common::uint32_t width,
                myos::common::uint32_t height,
                myos::common::uint8_t r,
                myos::common::uint8_t g,
                myos::common::uint8_t b);

            virtual void Present();

            void Clear(myos::common::uint8_t color);
        };

        class VGARenderScheduler
        {
            VideoGraphicsArray *vga;
            volatile bool renderFlag;

        public:
            VGARenderScheduler(VideoGraphicsArray *vga);

            static void Tick(void *ctx);

            bool Flag()
            {
                return renderFlag;
            }

            void SetFlag(bool value)
            {
                renderFlag = value;
            }
        };

    }
}

#endif
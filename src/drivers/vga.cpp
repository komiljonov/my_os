#include <drivers/vga.h>

using namespace myos::common;
using namespace myos::drivers;

namespace myos
{
    namespace drivers
    {

        VideoGraphicsArray::VideoGraphicsArray()
            : miscPort(0x3c2),
              crtcIndexPort(0x3d4),
              crtcDataPort(0x3d5),
              sequencerIndexPort(0x3c4),
              sequencerDataPort(0x3c5),
              graphicsControllerIndexPort(0x3ce),
              graphicsControllerDataPort(0x3cf),
              attributeControllerIndexPort(0x3c0),
              attributeControllerReadPort(0x3c1),
              attributeControllerWritePort(0x3c0),
              attributeControllerResetPort(0x3da)
        {
        }

        VideoGraphicsArray::~VideoGraphicsArray()
        {
        }

        void VideoGraphicsArray::WriteRegisters(myos::common::uint8_t *registers)
        {
            // misc
            miscPort.Write(*(registers++));

            // sequencer
            for (uint8_t i = 0; i < 5; i++)
            {
                sequencerIndexPort.Write(i);
                sequencerDataPort.Write(*(registers++));
            }

            // cathode ray tube controller

            crtcIndexPort.Write(0x03);
            crtcDataPort.Write(crtcDataPort.Read() | 0x80);

            crtcIndexPort.Write(0x11);
            crtcDataPort.Write(crtcDataPort.Read() & ~0x80);

            registers[0x03] = registers[0x03] | 0x80;
            registers[0x11] = registers[0x11] & ~0x80;

            for (uint8_t i = 0; i < 25; i++)
            {
                crtcIndexPort.Write(i);
                crtcDataPort.Write(*(registers++));
            }

            // graphics controller
            for (uint8_t i = 0; i < 9; i++)
            {
                graphicsControllerIndexPort.Write(i);
                graphicsControllerDataPort.Write(*(registers++));
            }

            // attribute controller
            for (uint8_t i = 0; i < 21; i++)
            {
                attributeControllerResetPort.Read();
                attributeControllerIndexPort.Write(i);
                attributeControllerWritePort.Write(*(registers++));
            }

            attributeControllerResetPort.Read();
            attributeControllerIndexPort.Write(0x20);
        }

        bool VideoGraphicsArray::SupportsMode(myos::common::uint32_t width, myos::common::uint32_t height, myos::common::uint32_t colorDepth)
        {
            return width == 320 && height == 200 && colorDepth == 8;
        }

        bool VideoGraphicsArray::SetMode(myos::common::uint32_t width, myos::common::uint32_t height, myos::common::uint32_t colorDepth)
        {

            if (!SupportsMode(width, height, colorDepth))
                return false;

            uint8_t g_320x200x256[] = {
                /* MISC
                 *
                 * 0x63 => 01100011
                 * 7 6 5 4 3 2 1 0
                 * 1 1 0 0 0 1 1 0
                 * VSP HSP - - CS CS ERAM IOS
                 * 7,6 - 480 lines
                 * 5,4 - free
                 * 3,2 - 28,322 MHZ Clock
                 * 1 - Enable Ram
                 * 0 - Map 0x3d4 to 0x3b4
                 */
                0x63,
                /* SEQ */
                /**
                 * index 0x00 - Reset
                 * 0x03 = 11
                 * Bits 1,0 Synchronous reset
                 */
                0x03,
                /**
                 * index 0x01
                 * Clocking mode register
                 * 8/9 Dot Clocks
                 */
                0x01,
                /**
                 * Map Mask Register, 0x02
                 * 0x0F = 1111
                 * Enable all 4 Maps Bits 0-3
                 * chain 4 mode
                 */
                0x0F,
                /**
                 * map select register, 0x03
                 * no character map enabled
                 */
                0x00,
                /**
                 * memory mode register 0x04
                 * enables ch4,odd/even,extended memory
                 */
                0x0E,
                /* CRTC */
                0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F,
                0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x9C, 0x0E, 0x8F, 0x28, 0x40, 0x96, 0xB9, 0xA3,
                0xFF,
                /* GC */
                0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F,
                0xFF,
                /* AC */
                0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
                0x41, 0x00, 0x0F, 0x00, 0x00};

            WriteRegisters(g_320x200x256);

            SetFrameBufferSegment();

            return true;
        }

        void VideoGraphicsArray::SetFrameBufferSegment()
        {
            graphicsControllerIndexPort.Write(0x06);
            uint8_t segmentNumber = (graphicsControllerDataPort.Read() >> 2) & 0x03;
            switch (segmentNumber)
            {
            default:
            case 0:
                frameBufferSegment = (uint8_t *)0x00000;
                break;
            case 1:
                frameBufferSegment = (uint8_t *)0xA0000;
                break;
            case 2:
                frameBufferSegment = (uint8_t *)0xB0000;
                break;
            case 3:
                frameBufferSegment = (uint8_t *)0xB8000;
                break;
            }
        }

        uint8_t *VideoGraphicsArray::GetFrameBufferSegment()
        {
            return frameBufferSegment;
        }

        void VideoGraphicsArray::PutPixel(myos::common::uint32_t x, myos::common::uint32_t y, myos::common::uint8_t colorIndex)
        {
            uint8_t *pixelAddress = frameBufferSegment + 320 * y + x;
            *pixelAddress = colorIndex;
        }

        uint8_t VideoGraphicsArray::GetColorIndex(
            myos::common::uint8_t r,
            myos::common::uint8_t g,
            myos::common::uint8_t b)
        {
            // Blue
            if (r == 0x00 && g == 0x00 && b == 0xA8)
                return 0x01;

            // Red
            if (r == 0xFF && g == 0x00 && b == 0x00)
                return 0x04;

            // Default: black
            return 0x00;
        }

        void VideoGraphicsArray::PutPixel(myos::common::uint32_t x, myos::common::uint32_t y, myos::common::uint8_t r, myos::common::uint8_t g, myos::common::uint8_t b)
        {
            PutPixel(x, y, GetColorIndex(r, g, b));
        }

        void VideoGraphicsArray::DrawRect(
            myos::common::uint32_t x,
            myos::common::uint32_t y,
            myos::common::uint32_t width,
            myos::common::uint32_t height,
            myos::common::uint8_t colorIndex)
        {
            for (uint32_t iy = 0; iy < height; iy++)
            {
                for (uint32_t ix = 0; ix < width; ix++)
                {
                    PutPixel(x + ix, y + iy, colorIndex);
                }
            }
        }

        void VideoGraphicsArray::DrawRect(
            myos::common::uint32_t x,
            myos::common::uint32_t y,
            myos::common::uint32_t width,
            myos::common::uint32_t height,
            myos::common::uint8_t r,
            myos::common::uint8_t g,
            myos::common::uint8_t b)
        {
            DrawRect(x, y, width, height, GetColorIndex(r, g, b));
        }
    }
}

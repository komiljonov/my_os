
#include <drivers/mouse.h>

using namespace myos::common;
using namespace myos::drivers;
using namespace myos::hardwarecommunication;

MouseEventHandler::MouseEventHandler()
{
}

void MouseEventHandler::OnActivate()
{
}

void MouseEventHandler::OnMouseDown(uint8_t button)
{
}
void MouseEventHandler::OnMouseUp(uint8_t button)
{
}
void MouseEventHandler::OnMouseMove(myos::common::int8_t xoffset, myos::common::int8_t yoffset)
{
}

MouseDriver::MouseDriver(InterruptManager *manager, MouseEventHandler *handler)
    : InterruptHandler(0x2C, manager),
      dataport(0x60),
      commandport(0x64)
{
    this->handler = handler;
}

MouseDriver::~MouseDriver()
{
}

void MouseDriver::Activate()
{
    offset = 0;
    buttons = 0;

    commandport.Write(0xA8);
    commandport.Write(0x20); // command 0x60 = read controller command byte
    uint8_t status = dataport.Read() | 2;
    commandport.Write(0x60); // command 0x60 = set controller command byte
    dataport.Write(status);

    commandport.Write(0xD4);
    dataport.Write(0xF4);
    dataport.Read();
}

uint32_t MouseDriver::HandleInterrupt(uint32_t esp)
{
    uint8_t status = commandport.Read();

    if (!(status & 0x20))
        return esp;

    buffer[offset] = dataport.Read();

    if (handler == 0)
        return esp;

    offset = (offset + 1) % 3;

    if (offset == 0)
    {
        if (buffer[1] != 0 || buffer[2] != 0)
        {

            handler->OnMouseMove((int8_t)buffer[1], (int8_t)buffer[2]);
        }

        for (uint8_t i = 0; i < 3; i++)
        {
            if ((buffer[0] & (0x1 << i)) != (buttons & (0x1 << i)))
            {
                if (buttons & (0x1 << i))
                    handler->OnMouseDown(i + 1);
                else
                    handler->OnMouseUp(i + 1);
            }
        }
        buttons = buffer[0];
    }
    return esp;
}

MouseToConsole::MouseToConsole()
{
    uint16_t *VideoMemory = (uint16_t *)0xb8000;

    x = 40;
    y = 12;
    VideoMemory[80 * y + x] = (VideoMemory[80 * y + x] & 0x0F00) << 4 | (VideoMemory[80 * y + x] & 0xF000) >> 4 | (VideoMemory[80 * y + x] & 0x00FF);
}

void MouseToConsole::OnMouseMove(myos::common::int8_t xoffset, myos::common::int8_t yoffset)
{
    static uint16_t *VideoMemory = (uint16_t *)0xb8000;
    VideoMemory[80 * y + x] = (VideoMemory[80 * y + x] & 0x0F00) << 4 | (VideoMemory[80 * y + x] & 0xF000) >> 4 | (VideoMemory[80 * y + x] & 0x00FF);

    x += xoffset;

    if (x >= 80)
        x = 79;
    if (x < 0)
        x = 0;

    y -= yoffset;
    if (y >= 25)
        y = 24;
    if (y < 0)
        y = 0;

    VideoMemory[80 * y + x] = (VideoMemory[80 * y + x] & 0x0F00) << 4 | (VideoMemory[80 * y + x] & 0xF000) >> 4 | (VideoMemory[80 * y + x] & 0x00FF);
}

MouseToVGAScreen::MouseToVGAScreen(VideoGraphicsArray *vga) : vga(vga)
{
    x = 155;
    y = 95;

    vga->DrawRect(
        x, y, 10, 10, 0xff, 0, 0);

    vga->Present();
}

// void MouseToVGAScreen::OnMouseMove(int8_t xoffset, int8_t yoffset)
// {

//     vga->DrawRect(x, y, 10, 10, BLUE);

//     x += xoffset;

//     if (x >= 310)
//         x = 310;

//     if (x < 0)
//         x = 0;

//     y -= yoffset;

//     if (y >= 190)
//         y = 190;

//     if (y < 0)
//         y = 0;

//     vga->DrawRect((uint32_t)x, (uint32_t)y, 10, 10, COLORS[color_index]);
//     // vga->Present();
// }

void MouseToVGAScreen::OnMouseMove(int8_t xoffset, int8_t yoffset)
{
    x += xoffset;
    if (x < 0)
        x = 0;
    if (x > 310)
        x = 310;

    y -= yoffset;
    if (y < 0)
        y = 0;
    if (y > 190)
        y = 190;

    dirty = true;
}

bool  MouseToVGAScreen::Render()
{
    if (!dirty)
    {
        return false;
    }
    int32_t cx, cy;

    // snapshot x/y atomically w.r.t IRQ updates
    // asm volatile("cli");
    cx = x;
    cy = y;
    // asm volatile("sti");

    // erase old cursor
    vga->DrawRect((uint32_t)prevX, (uint32_t)prevY, 10, 10, BLUE);

    // draw new cursor
    vga->DrawRect((uint32_t)cx, (uint32_t)cy, 10, 10, COLORS[color_index]);

    prevX = cx;
    prevY = cy;

    dirty = false;
    return true;
}

void MouseToVGAScreen::OnMouseDown(uint8_t button)
{
    NextColor();
    // vga->DrawRect((uint32_t)x, (uint32_t)y, 10, 10, COLORS[color_index]);
}

void MouseToVGAScreen::NextColor()
{

    color_index += 1;

    if (color_index >= 4)
    {
        color_index = 0;
    }
    dirty = true;
    // vga->DrawRect((uint32_t)x, (uint32_t)y, 10, 10, COLORS[color_index]);
}
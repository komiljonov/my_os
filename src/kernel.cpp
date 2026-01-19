#include <common/types.h>
#include <common/common.h>

#include <gdt.h>
#include <hardwarecommunication/interrupts.h>
#include <hardwarecommunication/pci.h>
#include <drivers/driver.h>
#include <drivers/keyboard.h>
#include <drivers/mouse.h>
#include <drivers/vga.h>
#include <hardwarecommunication/pit.h>
#include <timer.h>

using namespace myos;
using namespace myos::common;
using namespace myos::hardwarecommunication;
using namespace myos::drivers;

typedef void (*constructor)();
extern "C" constructor start_ctors;
extern "C" constructor end_ctors;

class PrintfKeyboardEventHandler : public KeyboardEventHandler
{
    virtual void OnKeyDown(char c)
    {
        char *foo = " ";
        foo[0] = c;
        printf(foo);
    }
};

extern "C" void
callConstructors()
{
    for (constructor *i = &start_ctors; i != &end_ctors; i++)
        (*i)();
}

static void ChangeColor(void *ctx)
{
    MouseToVGAScreen *mouse = (MouseToVGAScreen *)ctx;
    mouse->NextColor();
}

extern "C" void kernelMain(void *multiboot_structure, uint32_t magicnumber)
{

    printf("Hello World --- Komiljonov\n");

    GlobalDescriptorTable gdt;
    InterruptManager interrupts(0x20, &gdt);

    printf("Initializing Hardware, Stage 1\n");

    VideoGraphicsArray vga;

    DriverManager drvManager;

    PrintfKeyboardEventHandler kbHandler;
    KeyboardDriver keyboard(&interrupts, &kbHandler);

    TimerInterruptHandler timer(0x20, &interrupts);

    PIT pit;
    pit.SetFrequency(60);

    Timer::Init(60);

    drvManager.AddDriver(&keyboard);

    printf("Keyboard driver added to driver manager\n");

    // MouseToConsole mouseHandler;
    MouseToVGAScreen mouseHandler(&vga);

    MouseDriver mouse(&interrupts, &mouseHandler);

    drvManager.AddDriver(&mouse);
    printf("Mouse driver added to driver manager\n");

    PeripheralComponentInterconnectController PCIController;

    printf("PCI Initialized\n");

    PCIController.SelectDrivers(&drvManager, &interrupts);

    printf("Initializing Hardware, Stage 2\n");

    drvManager.ActivateAll();

    printf("Initializing Hardware, Stage 3\n");

    interrupts.Activate();

    vga.SetMode(320, 200, 8);

    // for (uint32_t y = 0; y < 200; y++)
    // {
    //     for (uint32_t x = 0; x < 320; x++)
    //     {
    //         vga.PutPixel(x, y, BLUE);
    //     }
    // }
    vga.DrawRect(0, 0, 320, 200, BLUE);
    vga.Present();

    VGARenderScheduler vgaScheduler(&vga);

    Timer::Every(1, &ChangeColor, (void *)&mouseHandler);
    Timer::Every(1, &VGARenderScheduler::Tick, &vgaScheduler);

    printf("Initializing Hardware, Active\n");

    while (1)
    {
        if (vgaScheduler.Flag())
        {
            vgaScheduler.SetFlag(false);

            bool res = mouseHandler.Render();

            // if(res){
            //     vga.Present();
            // }
            // vga.Present();
        }

        asm volatile("hlt");
    }
}

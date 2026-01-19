

#include <hardwarecommunication/interrupts.h>

#include <common/common.h>

using namespace myos::common;

namespace myos
{
    namespace hardwarecommunication
    {

        InterruptHandler::InterruptHandler(uint8_t interruptNumber, InterruptManager *interruptManager)
        {
            this->interruptNumber = interruptNumber;
            this->interruptManager = interruptManager;
            interruptManager->handlers[interruptNumber] = this;
        }

        InterruptHandler::~InterruptHandler()
        {
            if (interruptManager->handlers[interruptNumber] == this)
                interruptManager->handlers[interruptNumber] = 0;
        }

        uint32_t InterruptHandler::HandleInterrupt(uint32_t esp)
        {
            return esp;
        }

        InterruptManager::GateDescriptor InterruptManager::interruptDescriptorTable[256];

        InterruptManager *InterruptManager::ActiveInterruptManager = 0;

        void InterruptManager::SetInterruptDescriptionTableEntry(
            uint8_t interruptNumber,
            uint16_t CodeSegment,
            void (*handler)(),
            uint8_t DescriptionPrivilegeLevel,
            uint8_t DescriptorType)
        {

            interruptDescriptorTable[interruptNumber].handlerAddressLowBits = ((uint32_t)handler) & 0xFFFF;
            interruptDescriptorTable[interruptNumber].handlerAddressHighBits = (((uint32_t)handler) >> 16) & 0xFFFF;
            interruptDescriptorTable[interruptNumber].gdt_codeSegmentSelector = CodeSegment;

            const uint8_t IDT_DESC_PRESENT = 0x80;

            interruptDescriptorTable[interruptNumber].access = IDT_DESC_PRESENT | ((DescriptionPrivilegeLevel & 3) << 5) | DescriptorType;
            interruptDescriptorTable[interruptNumber].reserved = 0;
        }

        InterruptManager::InterruptManager(uint16_t hardwareInterruptOffset, GlobalDescriptorTable *gdt)
            : picMasterCommand(0x20),
              picMasterData(0x21),
              picSlaveCommand(0xA0),
              picSlaveData(0xA1)
        {

            uint16_t CodeSegment = gdt->CodeSegmentSelector();
            const uint8_t IDT_INTERRUPT_GATE = 0xE;

            for (uint16_t i = 255; i > 0; --i)
            {
                handlers[i] = 0;
                SetInterruptDescriptionTableEntry(i, CodeSegment, &InterruptIgnore, 0, IDT_INTERRUPT_GATE);
            }

            SetInterruptDescriptionTableEntry(0, CodeSegment, &InterruptIgnore, 0, IDT_INTERRUPT_GATE);

            SetInterruptDescriptionTableEntry(0x20, CodeSegment, &HandleInterruptRequest0x00, 0, IDT_INTERRUPT_GATE);
            SetInterruptDescriptionTableEntry(0x21, CodeSegment, &HandleInterruptRequest0x01, 0, IDT_INTERRUPT_GATE);
            SetInterruptDescriptionTableEntry(0x2C, CodeSegment, &HandleInterruptRequest0x0C, 0, IDT_INTERRUPT_GATE);

            picMasterCommand.Write(0x11);
            picSlaveCommand.Write(0x11);

            picMasterData.Write(hardwareInterruptOffset);
            picSlaveData.Write(hardwareInterruptOffset + 8);

            picMasterData.Write(0x04);
            picSlaveData.Write(0x02);

            picMasterData.Write(0x01);
            picSlaveData.Write(0x01);

            picMasterData.Write(0x00);
            picSlaveData.Write(0x00);

            // picMasterData.Write(0xF9);
            picMasterData.Write(0xF8);
            picSlaveData.Write(0xEF);

            interruptDescriptorTablePointer idt_pointer;
            idt_pointer.size = 256 * sizeof(GateDescriptor) - 1;
            idt_pointer.base = (uint32_t)interruptDescriptorTable;

            asm volatile("lidt %0" : : "m"(idt_pointer));
        }

        void InterruptManager::Activate()
        {
            if (ActiveInterruptManager != 0)
                ActiveInterruptManager->Deactivate();
            ActiveInterruptManager = this;
            asm("sti");
        }

        void InterruptManager::Deactivate()
        {
            if (ActiveInterruptManager == this)
            {
                ActiveInterruptManager = 0;
                asm("cli");
            }
        }

        InterruptManager::~InterruptManager()
        {
        }

        uint32_t InterruptManager::HandleInterrupt(uint8_t interruptNumnber, uint32_t esp)
        {

            if (ActiveInterruptManager != 0)
            {
                return ActiveInterruptManager->DoHandleInterrupt(interruptNumnber, esp);
            }

            return esp;
        }

        uint32_t InterruptManager::DoHandleInterrupt(uint8_t interruptNumber, uint32_t esp)
        {
            // char foo[] = "AAHANDLED INTERRUPT 0x00\n";
            // const char *hex = "0123456789ABCDEF";
            // foo[22] = hex[(interruptNumber >> 4) & 0x0F];
            // foo[23] = hex[interruptNumber & 0x0F];
            // printf(foo);

            if (handlers[interruptNumber])
            {
                // printf("PRE INTERRUPT\n");
                esp = handlers[interruptNumber]->HandleInterrupt(esp);
                // printf("INTERRUPT\n"); // optional, but can spam badly
            }
            else if (interruptNumber != 0x20)
            {

                // printf("UNHANDLED INTERRUPT 0x");
                // printHex(interruptNumber);
            }

            if (0x20 <= interruptNumber && interruptNumber < 0x30)
            {
                if (interruptNumber >= 0x28)
                    picSlaveCommand.Write(0x20);
                picMasterCommand.Write(0x20);
            }

            return esp;
        }

    }
}
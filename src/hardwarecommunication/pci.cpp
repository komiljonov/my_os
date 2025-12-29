#include <common/types.h>
#include <common/common.h>
#include <hardwarecommunication/pci.h>

using namespace myos::common;
using namespace myos::drivers;

namespace myos
{
    namespace hardwarecommunication
    {

        PeripheralComponentInterconnectDeviceDescriptor::PeripheralComponentInterconnectDeviceDescriptor()
        {
        }

        PeripheralComponentInterconnectDeviceDescriptor::~PeripheralComponentInterconnectDeviceDescriptor()
        {
        }

        PeripheralComponentInterconnectController::PeripheralComponentInterconnectController()
            : dataPort(0xCFC),
              commandPort(0xCF8)
        {
        }

        PeripheralComponentInterconnectController::~PeripheralComponentInterconnectController()
        {
        }

        uint32_t PeripheralComponentInterconnectController::Read(common::uint16_t bus, common::uint16_t device, common::uint16_t function, common::uint32_t registeroffset)
        {
            uint32_t id = 0x1 << 31 | ((bus & 0xff) << 16) // BUS
                          | ((device & 0x1f) << 11)        // DEVICE
                          | ((function & 0x07) << 8)       // FUNCTION
                          | (registeroffset & 0xFC);       // ADDRESS

            commandPort.Write(id);
            uint32_t result = dataPort.Read();

            return result >> (8 * (registeroffset % 4));
        }

        void PeripheralComponentInterconnectController::Write(common::uint16_t bus, common::uint16_t device, common::uint16_t function, common::uint32_t registeroffset, common::uint32_t value)
        {
            uint32_t id = 0x1 << 31 | ((bus & 0xff) << 16) // BUS
                          | ((device & 0x1f) << 11)        // DEVICE
                          | ((function & 0x07) << 8)       // FUNCTION
                          | (registeroffset & 0xFC);       // ADDRESS
            commandPort.Write(id);
            dataPort.Write(value);
        }

        bool PeripheralComponentInterconnectController::DeviceHasFunction(uint16_t bus, uint16_t device)
        {
            return Read(bus, device, 0, 0x0E) & (1 << 7);
        }

        void PeripheralComponentInterconnectController::SelectDrivers(myos::drivers::DriverManager *driverManager, myos::hardwarecommunication::InterruptManager *interrupts)
        {

            printf("Selecting drivers\n");

            for (int bus = 0; bus < 8; bus++)
            {
                // printf("Checking bus ");

                // printHex(bus & 0xFF);
                // printf("\n");

                for (int device = 0; device < 32; device++)
                {
                    // printf("Checking Device ");

                    // printHex(device & 0xFF);
                    // printf("\n");

                    int numFunctions = DeviceHasFunction(bus, device) ? 8 : 1;
                    for (int function = 0; function < numFunctions; function++)
                    {

                        // printf(", FUNCTION ");
                        // printHex(function & 0xFF);

                        PeripheralComponentInterconnectDeviceDescriptor dev = GetDeviceDescriptor(bus, device, function);

                        // printf("Got device descriptor");

                        if (dev.vendor_id == 0x0000 || dev.vendor_id == 0xFFFF)
                            continue;

                        for (int barNum = 0; barNum < 6; barNum++)
                        {
                            BaseAddressRegister bar = GetBaseAddressRegister(bus, device, function, barNum);
                            if (bar.address && (bar.type == InputOutput))
                                dev.portBase = (uint32_t)bar.address;

                            Driver *driver = GetDriver(dev, interrupts);
                            if (driver != 0)
                                driverManager->AddDriver(driver);
                        }

                        printf("PCI BUS 0x");
                        printHex(bus & 0xFF);

                        printf(", DEVICE 0x");
                        printHex(device & 0xFF);

                        printf(", FUNCTION 0x");
                        printHex(function & 0xFF);

                        printf(" = VENDER 0x");
                        printHex((dev.vendor_id & 0xFF00) >> 8);
                        printHex(dev.vendor_id & 0xFF);

                        printf(", DEVICE 0x");
                        printHex((dev.device_id & 0xFF00) >> 8);
                        printHex(dev.device_id & 0xFF);

                        printf("\n");
                    }
                }
            }
        }

        PeripheralComponentInterconnectDeviceDescriptor PeripheralComponentInterconnectController::GetDeviceDescriptor(uint16_t bus, uint16_t device, uint16_t function)
        {
            // printf("Getting device descriptor \n");
            PeripheralComponentInterconnectDeviceDescriptor result;

            result.bus = bus;
            result.device = device;
            result.function = function;

            // printf("1");

            result.vendor_id = Read(bus, device, function, 0x00);
            // printf("2");
            result.device_id = Read(bus, device, function, 0x02);
            // printf("3");

            result.class_id = Read(bus, device, function, 0x0b);
            // printf("4");
            result.subclass_id = Read(bus, device, function, 0x0a);
            // printf("5");
            result.interface_id = Read(bus, device, function, 0x09);
            // printf("6");

            result.revision = Read(bus, device, function, 0x08);
            // printf("7");
            result.interrupt = Read(bus, device, function, 0x3c);
            // printf("8\n");

            return result;
        }

        BaseAddressRegister PeripheralComponentInterconnectController::GetBaseAddressRegister(uint16_t bus, uint16_t device, uint16_t function, uint16_t bar)
        {
            BaseAddressRegister result;

            uint32_t headerType = Read(bus, device, function, 0x0E) & 0x7F;
            int maxBARs = 6 - (4 * headerType);

            if (bar >= maxBARs)
                return result;

            uint32_t bar_value = Read(bus, device, function, 0x10 + 4 * bar);

            result.type = (bar_value & 0x1) ? InputOutput : MemoryMapping;

            uint32_t temp;

            if (result.type == MemoryMapping)
            {
                switch ((bar_value >> 1) & 0x3)
                {
                case 0: // 32 Bit Mode
                case 1: // 20 Bit Mode
                case 2: // 64 Bit Mode
                    break;
                }
                result.prefetchable = ((bar_value >> 3) & 0x1) == 0x1;
            }
            else
            {
                result.address = (uint8_t *)(bar_value & ~0x3);
                result.prefetchable = false;
            }

            return result;
        }

        Driver *PeripheralComponentInterconnectController::GetDriver(PeripheralComponentInterconnectDeviceDescriptor dev, InterruptManager *interrupts)
        {
            switch (dev.vendor_id)
            {
            case 0x1022: // AMD
                switch (dev.device_id)
                {
                case 0x2000: // am79c973
                    /* code */
                    break;

                default:
                    break;
                }
                break;
            case 0x8086:
                break;

            default:
                break;
            }

            switch (dev.class_id)
            {
            case 0x03: // graphics
                switch (dev.subclass_id)
                {
                case 0x00: // VGA
                    /* code */
                    break;

                default:
                    break;
                }
                break;

            default:
                break;
            }

            return 0;
        }
    }
}
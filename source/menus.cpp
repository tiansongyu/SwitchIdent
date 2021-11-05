#include <cstdio>
#include <unistd.h>

#include "common.hpp"
#include "gui.hpp"
#include "menus.hpp"

namespace Menus {
    static u32 g_item_height = 0;
    static bool g_is_sd_inserted = false, g_is_gamecard_inserted = false;
    static const int g_item_dist = 67;
    static const int g_start_x = 450;
    static const int g_start_y = 250;
    
    enum MenuState {
        STATE_KERNEL_INFO = 0,
        STATE_SYSTEM_INFO,
        STATE_POWER_INFO,
        STATE_STORAGE_INFO,
        STATE_MISC_INFO,
        MAX_ITEMS
    };
    
    static void DrawItem(int x, int y, const char *title, const char *text) {
        u32 title_width = 0;
        GUI::GetTextDimensions(25, title, &title_width, nullptr);
        GUI::DrawText(x, y, 25, MENU_INFO_TITLE_COLOUR, title);
        GUI::DrawText(x + title_width + 20, y, 25, MENU_INFO_DESC_COLOUR, text);
    }

    static void DrawItemf(int x, int y, const char *title, const char *text, ...) {
        u32 title_width = 0;
        GUI::GetTextDimensions(25, title, &title_width, nullptr);
        GUI::DrawText(x, y, 25, MENU_INFO_TITLE_COLOUR, title);
        
        char buffer[256];
        va_list args;
        va_start(args, text);
        std::vsnprintf(buffer, 256, text, args);
        GUI::DrawText(x + title_width + 20, y, 25, MENU_INFO_DESC_COLOUR, buffer);
        va_end(args);
    }

    void KernelInfo(void) {
        SetSysFirmwareVersion ver = SwitchIdent::GetFirmwareVersion();
        Menus::DrawItemf(g_start_x, g_start_y + ((g_item_dist - g_item_height) / 2) + 50, "Firmware version:", 
            "%u.%u.%u-%u%u", ver.major, ver.minor, ver.micro, ver.revision_major, ver.revision_minor);
        Menus::DrawItem(g_start_x, g_start_y + ((g_item_dist - g_item_height) / 2) + 100, "Hardware:", SwitchIdent::GetHardwareType());
        Menus::DrawItem(g_start_x, g_start_y + ((g_item_dist - g_item_height) / 2) + 150, "Unit:", SwitchIdent::GetUnit());
        Menus::DrawItem(g_start_x, g_start_y + ((g_item_dist - g_item_height) / 2) + 200, "Serial:", SwitchIdent::GetSerialNumber().number);
        Menus::DrawItem(g_start_x, g_start_y + ((g_item_dist - g_item_height) / 2) + 250, "DRAM ID:", SwitchIdent::GetDramDesc());
        Menus::DrawItemf(g_start_x, g_start_y + ((g_item_dist - g_item_height) / 2) + 300, "Device ID:", "%llu", SwitchIdent::GetDeviceID());
    }

    void SystemInfo(void) {
        Menus::DrawItem(g_start_x, g_start_y + ((g_item_dist - g_item_height) / 2) + 50, "Region:",  SwitchIdent::GetRegion());
        Menus::DrawItemf(g_start_x, g_start_y + ((g_item_dist - g_item_height) / 2) + 100, "CPU clock:", "%lu MHz", SwitchIdent::GetClock(PcvModule_CpuBus));
        Menus::DrawItemf(g_start_x, g_start_y + ((g_item_dist - g_item_height) / 2) + 150, "GPU clock:", "%lu MHz", SwitchIdent::GetClock(PcvModule_GPU));
        Menus::DrawItemf(g_start_x, g_start_y + ((g_item_dist - g_item_height) / 2) + 200, "EMC clock:", "%lu MHz", SwitchIdent::GetClock(PcvModule_EMC));
        Menus::DrawItemf(g_start_x, g_start_y + ((g_item_dist - g_item_height) / 2) + 250, "Wireless LAN:", 
            "%s (RSSI: %d) (Quality: %lu)", SwitchIdent::GetWirelessLanEnableFlag()? "Enabled" : "Disabled", SwitchIdent::GetWlanRSSI(), SwitchIdent::GetWlanQuality(SwitchIdent::GetWlanRSSI()));
        Menus::DrawItem(g_start_x, g_start_y + ((g_item_dist - g_item_height) / 2) + 300, "Bluetooth:", SwitchIdent::GetBluetoothEnableFlag()? "Enabled" : "Disabled");
        Menus::DrawItem(g_start_x, g_start_y + ((g_item_dist - g_item_height) / 2) + 350, "NFC:", SwitchIdent::GetNfcEnableFlag()? "Enabled" : "Disabled");
    }

    void PowerInfo(void) {
        Menus::DrawItemf(g_start_x, g_start_y + ((g_item_dist - g_item_height) / 2) + 50, "Battery percentage:",  "%lu %% (%s)", SwitchIdent::GetBatteryPercentage(), SwitchIdent::IsCharging()? "charging" : "not charging");
        Menus::DrawItem(g_start_x, g_start_y + ((g_item_dist - g_item_height) / 2) + 100, "Battery voltage state:", SwitchIdent::GetVoltageState());
        Menus::DrawItem(g_start_x, g_start_y + ((g_item_dist - g_item_height) / 2) + 150, "Battery charger type:", SwitchIdent::GetChargerType());
        Menus::DrawItem(g_start_x, g_start_y + ((g_item_dist - g_item_height) / 2) + 200, "Battery charging enabled:", SwitchIdent::IsChargingEnabled()? "Yes" : "No");
        Menus::DrawItem(g_start_x, g_start_y + ((g_item_dist - g_item_height) / 2) + 250, "Battery ample power supplied:", SwitchIdent::IsEnoughPowerSupplied()? "Yes" : "No");
        Menus::DrawItem(g_start_x, g_start_y + ((g_item_dist - g_item_height) / 2) + 300, "Battery lot number:", SwitchIdent::GetBatteryLot().lot);
    }

    void StorageInfo(void) {
        u64 sd_used = SwitchIdent::GetUsedStorage(NcmStorageId_SdCard);
        u64 sd_total = SwitchIdent::GetTotalStorage(NcmStorageId_SdCard);
        
        u64 nand_u_used = SwitchIdent::GetUsedStorage(NcmStorageId_BuiltInUser);
        u64 nand_u_total = SwitchIdent::GetTotalStorage(NcmStorageId_BuiltInUser);
        
        u64 nand_s_used = SwitchIdent::GetUsedStorage(NcmStorageId_BuiltInSystem);
        u64 nand_s_total = SwitchIdent::GetTotalStorage(NcmStorageId_BuiltInSystem);
        
        char sd_total_str[16], sd_free_str[16], sd_used_str[16];
        SwitchIdent::GetSizeString(sd_total_str, sd_total);
        SwitchIdent::GetSizeString(sd_free_str, SwitchIdent::GetFreeStorage(NcmStorageId_SdCard));
        SwitchIdent::GetSizeString(sd_used_str, sd_used);
        
        char nand_u_total_str[16], nand_u_free_str[16], nand_u_used_str[16];
        SwitchIdent::GetSizeString(nand_u_total_str, nand_u_total);
        SwitchIdent::GetSizeString(nand_u_free_str, SwitchIdent::GetFreeStorage(NcmStorageId_BuiltInUser));
        SwitchIdent::GetSizeString(nand_u_used_str, nand_u_used);
        
        char nand_s_total_str[16], nand_s_free_str[16], nand_s_used_str[16];
        SwitchIdent::GetSizeString(nand_s_total_str, nand_s_total);
        SwitchIdent::GetSizeString(nand_s_free_str, SwitchIdent::GetFreeStorage(NcmStorageId_BuiltInSystem));
        SwitchIdent::GetSizeString(nand_s_used_str, nand_s_used);
        
        GUI::DrawRect(400, 50, 880, 670, BACKGROUND_COLOUR);
        
        GUI::DrawDriveIcon(450, 88);
        GUI::DrawRect(450, 226, 128, 25, STATUS_BAR_COLOUR);
        GUI::DrawRect(452, 228, 124, 21, BACKGROUND_COLOUR);
        GUI::DrawRect(452, 228, (((double)sd_used / (double)sd_total) * 124.0), 21, MENU_SELECTOR_COLOUR);
        
        GUI::DrawDriveIcon(450, 296);
        GUI::DrawRect(450, 434, 128, 25, STATUS_BAR_COLOUR);
        GUI::DrawRect(452, 436, 124, 21, BACKGROUND_COLOUR);
        GUI::DrawRect(452, 436, (((double)nand_u_used / (double)nand_u_total) * 124.0), 21, MENU_SELECTOR_COLOUR);
        
        GUI::DrawDriveIcon(450, 504);
        GUI::DrawRect(450, 642, 128, 25, STATUS_BAR_COLOUR);
        GUI::DrawRect(452, 644, 124, 21, BACKGROUND_COLOUR);
        GUI::DrawRect(452, 644, (((double)nand_s_used / (double)nand_s_total) * 124.0), 21, MENU_SELECTOR_COLOUR);
        
        GUI::DrawText(600, 38 + ((g_item_dist - g_item_height) / 2) + 50, 25, MENU_INFO_DESC_COLOUR, "SD");
        Menus::DrawItem(600, 38 + ((g_item_dist - g_item_height) / 2) + 88, "Total storage capacity:",  sd_total_str);
        Menus::DrawItem(600, 38 + ((g_item_dist - g_item_height) / 2) + 126, "Free storage capacity:", sd_free_str);
        Menus::DrawItem(600, 38 + ((g_item_dist - g_item_height) / 2) + 164, "Used storage capacity:", sd_used_str);
        
        GUI::DrawText(600, 246 + ((g_item_dist - g_item_height) / 2) + 50, 25, MENU_INFO_DESC_COLOUR, "NAND User");
        Menus::DrawItem(600, 246 + ((g_item_dist - g_item_height) / 2) + 88, "Total storage capacity:",  nand_u_total_str);
        Menus::DrawItem(600, 246 + ((g_item_dist - g_item_height) / 2) + 126, "Free storage capacity:", nand_u_free_str);
        Menus::DrawItem(600, 246 + ((g_item_dist - g_item_height) / 2) + 164, "Used storage capacity:", nand_u_used_str);
        
        GUI::DrawText(600, 454 + ((g_item_dist - g_item_height) / 2) + 50, 25, MENU_INFO_DESC_COLOUR, "NAND System");
        Menus::DrawItem(600, 454 + ((g_item_dist - g_item_height) / 2) + 88, "Total storage capacity:",  nand_s_total_str);
        Menus::DrawItem(600, 454 + ((g_item_dist - g_item_height) / 2) + 126, "Free storage capacity:", nand_s_free_str);
        Menus::DrawItem(600, 454 + ((g_item_dist - g_item_height) / 2) + 164, "Used storage capacity:", nand_s_used_str);
    }

    void MiscInfo(void) {
        char hostname[128];
        Result ret = gethostname(hostname, sizeof(hostname));

        SetCalBdAddress bd_addr = SwitchIdent::GetBluetoothBdAddress();
        SetCalMacAddress mac_addr = SwitchIdent::GetWirelessLanMacAddress();

        Menus::DrawItem(g_start_x, g_start_y + ((g_item_dist - g_item_height) / 2) + 50, "IP:",  R_SUCCEEDED(ret)? hostname : nullptr);
        Menus::DrawItem(g_start_x, g_start_y + ((g_item_dist - g_item_height) / 2) + 100, "State:", SwitchIdent::GetOperationMode());
        Menus::DrawItem(g_start_x, g_start_y + ((g_item_dist - g_item_height) / 2) + 150, "Automatic update:", SwitchIdent::GetAutoUpdateEnableFlag()? "Enabled" : "Disabled");
        Menus::DrawItem(g_start_x, g_start_y + ((g_item_dist - g_item_height) / 2) + 200, "Console information upload:", SwitchIdent::GetConsoleInformationUploadFlag()? "Enabled" : "Disabled");
        Menus::DrawItem(g_start_x, g_start_y + ((g_item_dist - g_item_height) / 2) + 250, "SD card status:", g_is_sd_inserted? "Inserted" : "Not inserted");
        Menus::DrawItem(g_start_x, g_start_y + ((g_item_dist - g_item_height) / 2) + 300, "Game card status:", g_is_gamecard_inserted? "Inserted" : "Not inserted");
        Menus::DrawItemf(g_start_x, g_start_y + ((g_item_dist - g_item_height) / 2) + 350, "BT address:",
            "%02X:%02X:%02X:%02X:%02X:%02X", bd_addr.bd_addr[0], bd_addr.bd_addr[1], bd_addr.bd_addr[2], bd_addr.bd_addr[3], bd_addr.bd_addr[4], bd_addr.bd_addr[5]);
        Menus::DrawItemf(g_start_x, g_start_y + ((g_item_dist - g_item_height) / 2) + 400, "WLAN address:", 
            "%02X:%02X:%02X:%02X:%02X:%02X", mac_addr.addr[0], mac_addr.addr[1], mac_addr.addr[2], mac_addr.addr[3], mac_addr.addr[4], mac_addr.addr[5]);
    }

    void Main(void) {
        u32 title_height = 0;
        GUI::GetTextDimensions(25, "SwitchIdent", nullptr, &title_height);
        GUI::GetTextDimensions(25, "Item", nullptr, &g_item_height);
        
        int banner_width = 200;
        int selection = STATE_KERNEL_INFO;
        Result ret = 0;
        
        FsDeviceOperator fsDeviceOperator;
        if (R_FAILED(ret = fsOpenDeviceOperator(&fsDeviceOperator)))
            std::printf("fsOpenDeviceOperator() failed: 0x%x.\n\n", ret);
            
        g_is_sd_inserted = SwitchIdent::IsSDCardInserted(&fsDeviceOperator);
        g_is_gamecard_inserted = SwitchIdent::IsGameCardInserted(&fsDeviceOperator);
        fsDeviceOperatorClose(&fsDeviceOperator);
        
        padConfigureInput(1, HidNpadStyleSet_NpadStandard);
        PadState pad;
        padInitializeDefault(&pad);

        const char *items[] = {
            "Kernel",
            "System",
            "Power",
            "Storage",
            "Misc",
            "Exit"
        };

        while(appletMainLoop()) {
            GUI::ClearScreen(BACKGROUND_COLOUR);
            GUI::DrawRect(0, 0, 1280, 50, STATUS_BAR_COLOUR);
            GUI::DrawRect(0, 50, 400, 670, MENU_BAR_COLOUR);
            
            GUI::DrawTextf(30, ((50 - title_height) / 2), 25, BACKGROUND_COLOUR, "SwitchIdent v%d.%d", VERSION_MAJOR, VERSION_MINOR);
            GUI::DrawBanner(400 + ((880 - (banner_width)) / 2),  80);
            
            GUI::DrawRect(0, 50 + (g_item_dist * selection), 400, g_item_dist, MENU_SELECTOR_COLOUR);

            for (int i = 0; i < MAX_ITEMS + 1; i++)
                GUI::DrawText(30, 50 + ((g_item_dist - g_item_height) / 2) + (g_item_dist * i), 25, selection == i? ITEM_SELECTED_COLOUR : ITEM_COLOUR, items[i]);
            
            padUpdate(&pad);
            u32 kDown = padGetButtonsDown(&pad);
            
            if (kDown & HidNpadButton_AnyDown)
                selection++;
            else if (kDown & HidNpadButton_AnyUp)
                selection--;
                
            if (selection > MAX_ITEMS) 
                selection = 0;
            if (selection < 0) 
                selection = MAX_ITEMS;
                
            switch (selection) {
                case STATE_KERNEL_INFO:
                    Menus::KernelInfo();
                    break;
                
                case STATE_SYSTEM_INFO:
                    Menus::SystemInfo();
                    break;
                
                case STATE_POWER_INFO:
                    Menus::PowerInfo();
                    break;
                
                case STATE_STORAGE_INFO:
                    Menus::StorageInfo();
                    break;
                    
                case STATE_MISC_INFO:
                    Menus::MiscInfo();
                    break;

                default:
                    break;
            }
            
            GUI::Render();
            
            if ((kDown & HidNpadButton_Plus) || ((kDown & HidNpadButton_A) && (selection == MAX_ITEMS)))
                break;
        }
    }
}

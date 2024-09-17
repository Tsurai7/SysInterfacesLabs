#include <iostream>
#include <windows.h>
#include <powrprof.h>
#include <chrono>
#include <thread>
#include <conio.h>

using namespace std;

SYSTEM_POWER_STATUS getPowerStatus() {
    SYSTEM_POWER_STATUS sps;
    GetSystemPowerStatus(&sps);
    return sps;
}

void printPowerInfo(const SYSTEM_POWER_STATUS& sps) {
    cout << "AC Line Status: " << (sps.ACLineStatus == 0 ? "Battery" : "AC Power") << endl;

    cout << "Battery type: ";
    switch (sps.BatteryFlag & 0xF) {
        case 0x1: cout << "Lithium-ion"; break;
        case 0x2: cout << "Nickel-metal hydride"; break;
        case 0x4: cout << "Lead-acid"; break;
        case 0x8: cout << "Unknown"; break;
        default: cout << "Unknown";
    }
    cout << endl;

    cout << "Battery charge level: " << static_cast<int>(sps.BatteryLifePercent) << "%" << endl;

    cout << "Power saving mode: " << (sps.SystemStatusFlag ? "Enabled" : "Disabled") << endl;
}

void goToSleep() {
    SetSuspendState(FALSE, TRUE, TRUE);
}

void goToHibernate() {
    SetSuspendState(TRUE, TRUE, TRUE);
}

void printBatteryRuntime(const chrono::steady_clock::time_point& start) {
    auto duration = chrono::duration_cast<chrono::seconds>(chrono::steady_clock::now() - start);
    auto hours = duration.count() / 3600;
    auto minutes = (duration.count() % 3600) / 60;
    auto seconds = duration.count() % 60;

    cout << "Battery runtime since unplugged: "
         << hours << " hours "
         << minutes << " minutes "
         << seconds << " seconds" << endl;
}

void printRemainingBatteryLife(const SYSTEM_POWER_STATUS& sps) {
    if (sps.ACLineStatus == 0 && sps.BatteryLifeTime != (DWORD)-1) {
        auto hours = sps.BatteryLifeTime / 3600;
        auto minutes = (sps.BatteryLifeTime % 3600) / 60;
        auto seconds = sps.BatteryLifeTime % 60;
        cout << "Remaining battery runtime after unplugging: "
             << hours << " hours " << minutes << " minutes " << seconds << " seconds" << endl;
    } else {
        cout << "Computer is connected to AC power or time is not available." << endl;
    }
}

int main() {
    auto unpluggedTime = chrono::steady_clock::now();
    bool isUnplugged = false;

    while (true) {
        // Clear screen
        system("cls");

        // Update battery status
        auto sps = getPowerStatus();
        cout << "-----------------" << endl;
        printPowerInfo(sps);

        if (sps.ACLineStatus == 0) {
            if (!isUnplugged) {
                unpluggedTime = chrono::steady_clock::now();
                isUnplugged = true;
            }
            printBatteryRuntime(unpluggedTime);
            printRemainingBatteryLife(sps);
        } else {
            isUnplugged = false;
            cout << "Connected to AC power." << endl;
        }

        // Display choices
        cout << "\nChoose an action:" << endl;
        cout << "1. Go to sleep mode" << endl;
        cout << "2. Go to hibernate mode" << endl;
        cout << "3. Exit the program" << endl;

        // Handle user input
        if (_kbhit()) {
            auto choice = _getch();

            switch (choice) {
                case '1':
                    goToSleep();
                    return 0;
                case '2':
                    goToHibernate();
                    return 0;
                case '3':
                    return 0;
                default:
                    cout << "Invalid choice. Press Enter to continue.";
                    cin.ignore();
                    cin.get();
                    continue;
            }
        }

        // Delay before next update
        this_thread::sleep_for(chrono::seconds(1));
    }

    return 0;
}

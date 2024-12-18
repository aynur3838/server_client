#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 9090

void display_menu() {
    printf("\n--- E-Commerce System ---\n");
    printf("Commands:\n");
    printf("1. LIST - View all available products\n");
    printf("2. ADD <product_name> <size> - Add a product to your cart\n");
    printf("3. REMOVE <product_name> <size> - Remove a product from your cart\n");
    printf("4. SEARCH <keyword> - Search for products\n");
    printf("5. SHOW CART - View items in your cart\n");
    printf("6. CHECKOUT - Purchase all items in the cart\n");
    printf("7. CLEAR CART - Remove all items from your cart\n");
    printf("8. VIEW PRODUCT <product_name> - View details of a specific product\n");
    printf("9. CART TOTAL - Get the total price of items in the cart\n");
    printf("10. FILTER PRODUCTS <min_price> <max_price> - Filter products by price range\n");
    printf("11. EXIT - Disconnect from the server\n");
    printf("----------------------------\n");
}

int main() {
    WSADATA wsaData;
    SOCKET sock;
    struct sockaddr_in server_addr;
    char buffer[1024] = {0};

    WSAStartup(MAKEWORD(2, 2), &wsaData);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        printf("Socket creation failed.\n");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Connection to server failed.\n");
        return 1;
    }

    printf("Connected to the server.\n");
    display_menu();

    while (1) {
        printf("\nEnter a command: ");
        fgets(buffer, 1024, stdin);
        send(sock, buffer, strlen(buffer), 0);

        if (strncmp(buffer, "EXIT", 4) == 0) {
            printf("Disconnected from the server.\n");
            break;
        }

        memset(buffer, 0, sizeof(buffer));
        recv(sock, buffer, 1024, 0);
        printf("Server: %s\n", buffer);
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}
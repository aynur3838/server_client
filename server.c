#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 9090
#define MAX_CLIENTS 10

typedef struct {
    char name[200];
    float price;
    int stock[3]; // Stock for small, medium, and large sizes
} Product;

typedef struct {
    char name[50];
    char size[10];
    int quantity;
} CartItem;
typedef struct {
    SOCKET client_socket;
    CartItem cart[100];
    int cart_count;
} ClientSession;

Product products[] = {
    {"Tshirt", 150.0, {10, 15, 20}}, 
    {"Pants", 300.0, {5, 10, 8}}, 
    {"Skirt", 200.0, {7, 12, 9}}, 
    {"Dress", 300.0, {3, 5, 2}}, 
    {"Sweater", 300.0, {6, 11, 4}},
    {"Shirt", 150.0, {10, 15, 20}},
    {"Jeans", 150.0, {10, 15, 20}},
    {"Jogger", 300.0, {3, 5, 2}},
    {"Coat", 300.0, {3, 5, 2}},
    {"Tights", 300.0, {3, 5, 2}},
    {"Jacket", 400.0, {10, 20, 8}},
    {"Cardigan", 300.0, {12, 8, 10}}
};

CartItem cart[100];
int cart_count = 0;
int i, j;

void add_to_cart(ClientSession *session,char *product_name, char *size);
void remove_from_cart(ClientSession *session,char *product_name, char *size);
void filter_products(float min_price, float max_price, SOCKET client_socket);
void clear_cart();
void checkout(ClientSession *session);
void view_product(char *product_name, SOCKET client_socket);
void search_product(char *keyword, SOCKET client_socket);
float calculate_cart_total(SOCKET client_socket);
void clear_cart() {
    cart_count = 0;
}

void add_to_cart(ClientSession *session, char *product_name, char *size) {
    for ( i = 0; i < sizeof(products) / sizeof(Product); i++) {
        if (strcmp(products[i].name, product_name) == 0) {
            for ( j = 0; j < session->cart_count; j++) {
                if (strcmp(session->cart[j].name, product_name) == 0 && strcmp(session->cart[j].size, size) == 0) {
                    session->cart[j].quantity++;
                    return;
                }
            }
            strcpy(session->cart[session->cart_count].name, product_name);
            strcpy(session->cart[session->cart_count].size, size);
            session->cart[session->cart_count].quantity = 1;
            session->cart[session->cart_count++];
            return;
        }
    }
}

void view_product(char *product_name, SOCKET client_socket) {
    char response[1024] = "Product Details:\n";
    for (i = 0; i < sizeof(products) / sizeof(Product); i++) {
        if (strcmp(products[i].name, product_name) == 0) {
            char temp[200];
            sprintf(temp, "Name: %s\nPrice: $%.2f\nStock (S: %d, M: %d, L: %d)\n",
                    products[i].name,
                    products[i].price,
                    products[i].stock[0],
                    products[i].stock[1],
                    products[i].stock[2]);
            strcat(response, temp);
            send(client_socket, response, strlen(response), 0);
            return;
        }
    }
    send(client_socket, "Product not found.\n", 20, 0);
}

void search_product(char *keyword, SOCKET client_socket) {
    char response[1024] = "Search Results:\n";
    int found = 0;
    int i;
    for ( i = 0; i < sizeof(products) / sizeof(Product); i++) {
        if (strstr(products[i].name, keyword) != NULL) {
            char temp[100];
            sprintf(temp, "%s - $%.2f (S:%d M:%d L:%d)\n", 
                    products[i].name, 
                    products[i].price, 
                    products[i].stock[0], 
                    products[i].stock[1], 
                    products[i].stock[2]);
            strcat(response, temp);
            found = 1;
        }
    }
    if (found) {
        send(client_socket, response, strlen(response), 0);
    } else {
        send(client_socket, "No products found.\n", 20, 0);
    }
}

float calculate_cart_total(SOCKET client_socket) {
    float total = 0;
    for (i = 0; i < cart_count; i++) {
        for (j = 0; j < sizeof(products) / sizeof(Product); j++) {
            if (strcmp(cart[i].name, products[j].name) == 0) {
                total += cart[i].quantity * products[j].price;
            }
        }
    }
    char response[50];
    sprintf(response, "Total Cart Price: $%.2f\n", total);
    send(client_socket, response, strlen(response), 0);
    return total;
}



void checkout(ClientSession *session) {
    for ( i = 0; i < session->cart_count; i++) {
        for ( j = 0; j < sizeof(products) / sizeof(Product); j++) {
            if (strcmp(session->cart[i].name, products[j].name) == 0) {
                int size_index = 0;
                if (strcmp(session->cart[i].size, "small") == 0) size_index = 0;
                else if (strcmp(session->cart[i].size, "medium") == 0) size_index = 1;
                else if (strcmp(session->cart[i].size, "large") == 0) size_index = 2;

                products[j].stock[size_index] -= session->cart[i].quantity;
                if (products[j].stock[size_index] < 0) products[j].stock[size_index] = 0;
            }
        }
    }
    session->cart_count = 0;
    send(session->client_socket, "Checkout complete.\n", 20, 0);
}

void remove_from_cart(ClientSession *session, char *product_name, char *size) {
    for (i = 0; i < session->cart_count; i++) {
        if (strcmp(session->cart[i].name, product_name) == 0 && strcmp(session->cart[i].size, size) == 0) {
            for (j = i; j < session->cart_count - 1; j++) {
                session->cart[j] = session->cart[j + 1];
            }
            session->cart_count--;
            return;
        }
    }
}
void filter_products(float min_price, float max_price, SOCKET client_socket) {
    Product filtered_products[sizeof(products) / sizeof(Product)];
    int filtered_count = 0;

    // Filtrelenen ürünleri yeni bir diziye ekle
    for ( i = 0; i < sizeof(products) / sizeof(Product); i++) {
        if (products[i].price >= min_price && products[i].price <= max_price) {
            filtered_products[filtered_count++] = products[i];
        }
    }

    // Ürünleri fiyatlarına göre sıralama (Bubble Sort)
    for ( i = 0; i < filtered_count - 1; i++) {
        for ( j = 0; j < filtered_count - i - 1; j++) {
            if (filtered_products[j].price > filtered_products[j + 1].price) {
                Product temp = filtered_products[j];
                filtered_products[j] = filtered_products[j + 1];
                filtered_products[j + 1] = temp;
            }
        }
    }

    // Yanıt oluştur ve kullanıcıya gönder
    if (filtered_count > 0) {
        char response[1024] = "Filtered Products (Sorted by Price):\n";
        for ( i = 0; i < filtered_count; i++) {
            char temp[200];
            sprintf(temp, "%s - $%.2f (S:%d M:%d L:%d)\n",
                    filtered_products[i].name,
                    filtered_products[i].price,
                    filtered_products[i].stock[0],
                    filtered_products[i].stock[1],
                    filtered_products[i].stock[2]);
            strcat(response, temp);
        }
        send(client_socket, response, strlen(response), 0);
    } else {
        send(client_socket, "No products found in the specified range.\n", 42, 0);
    }
}
void handle_client(SOCKET client_socket) {
    char buffer[1024];
    ClientSession *session = (ClientSession *)malloc(sizeof(ClientSession));
    session->client_socket = client_socket;
    session->cart_count = 0;
    memset(session->cart, 0, sizeof(session->cart)); 

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        recv(client_socket, buffer, sizeof(buffer), 0);

        if (strncmp(buffer, "EXIT", 4) == 0) {
            send(client_socket, "Disconnected.\n", 15, 0);
            closesocket(client_socket);
            free(session);
            break;
        } 
        else if (strncmp(buffer, "LIST", 4) == 0) {
            char product_list[1024] = "Available Products:\n";
            for (i = 0; i < sizeof(products) / sizeof(Product); i++) {
                char temp[100];
                sprintf(temp, "%s - $%.2f (S:%d M:%d L:%d)\n", 
                        products[i].name, 
                        products[i].price, 
                        products[i].stock[0], 
                        products[i].stock[1], 
                        products[i].stock[2]);
                strcat(product_list, temp);
            }
            send(client_socket, product_list, strlen(product_list), 0);
        } 
        else if (strncmp(buffer, "ADD", 3) == 0) {
            char product_name[50], size[10];
            sscanf(buffer, "ADD %s %s", product_name, size);
            add_to_cart(session,product_name, size);
            send(client_socket, "Product added to cart.\n", 24, 0);
        } 
        else if (strncmp(buffer, "REMOVE", 6) == 0) {
            char product_name[50], size[10];
            sscanf(buffer, "REMOVE %s %s", product_name, size);
            remove_from_cart(session,product_name, size);
            send(client_socket, "Product removed from cart.\n", 28, 0);
        } 
        else if (strncmp(buffer, "SHOW CART", 9) == 0) {
            char cart_info[1024] = "Cart Items:\n";
            if (session->cart_count == 0) {
                strcat(cart_info, "Your cart is empty.\n");
            } else {
                for ( i = 0; i < session->cart_count; i++) {
                    char temp[200];
                    sprintf(temp, "%d x %s (%s)\n", session->cart[i].quantity, session->cart[i].name, session->cart[i].size);
                    strcat(cart_info, temp);
                }
            }
            send(client_socket, cart_info, strlen(cart_info), 0);
        } 

        else if (strncmp(buffer, "CLEAR CART", 10) == 0) {
    // Tüm sepet bilgilerini sıfırla
    memset(session->cart, 0, sizeof(session->cart));
    session->cart_count = 0;
    // İstemciye bilgi gönder
    send(client_socket, "Your cart has been cleared.\n", 29, 0);
} 

        else if (strncmp(buffer, "CHECKOUT", 8) == 0) {
            checkout(session);
        }
        else if (strncmp(buffer, "VIEW PRODUCT", 12) == 0) {
            char product_name[50];
            sscanf(buffer, "VIEW PRODUCT %s", product_name);
            view_product(product_name, client_socket);
        } 
        
        else if (strncmp(buffer, "SEARCH", 6) == 0) {
            char keyword[50];
            sscanf(buffer, "SEARCH %s", keyword);
            search_product(keyword, client_socket);
        } 
          
       else if (strncmp(buffer, "CART TOTAL", 10) == 0) {
            float total = 0;
            for ( i = 0; i < session->cart_count; i++) {
                for ( j = 0; j < sizeof(products) / sizeof(Product); j++) {
                    if (strcmp(session->cart[i].name, products[j].name) == 0) {
                        total += session->cart[i].quantity * products[j].price;
                    }
                }
            }
            char response[50];
            sprintf(response, "Total Cart Price: $%.2f\n", total);
            send(client_socket, response, strlen(response), 0);
        } 

        else if (strncmp(buffer, "FILTER PRODUCTS", 15) == 0) {
            float min_price, max_price;
            sscanf(buffer, "FILTER PRODUCTS %f %f", &min_price, &max_price);
            filter_products(min_price, max_price, client_socket);
}
        else {
            send(client_socket, "Unknown command.\n", 17, 0);
        }
    }
}

int main() {
    WSADATA wsaData;
    SOCKET server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    int addr_len = sizeof(client_addr);

    WSAStartup(MAKEWORD(2, 2), &wsaData);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        printf("Socket creation failed.\n");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Bind failed.\n");
        return 1;
    }

    listen(server_socket, MAX_CLIENTS);
    printf("Server is running on port %d\n", PORT);

    while (1) {
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &addr_len);
        printf("New client connected.\n");

        if (client_socket == INVALID_SOCKET) {
            printf("Client connection failed.\n");
            continue;
        }

        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)handle_client, (LPVOID)client_socket, 0, NULL);
    }

    closesocket(server_socket);
    WSACleanup();
    return;
    }
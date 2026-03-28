#include <stdio.h>
#include <windows.h>
#include <winsock2.h>
#include <string.h>

#define Max_Clients 100

CRITICAL_SECTION lock;
int client_count = 0;

struct Client
{
    SOCKET socket;
    char name[50];
};

struct Client clients[Max_Clients];

void removeClient(SOCKET client_socket_parameter)
{
    for (int i = 0; i < client_count; i++)
    {
        if (clients[i].socket == client_socket_parameter)
        {
            for (int j = i; j < client_count - 1; j++)
            {
                clients[j] = clients[j + 1];
            }
            client_count--;
            break;
        }
    }
}

void broadcast(char *message, SOCKET sender)
{
    for (int i = 0; i < client_count; i++)
    {
        if (clients[i].socket != sender)
        {
            EnterCriticalSection(&lock);

            send(clients[i].socket, message, strlen(message), 0);
            LeaveCriticalSection(&lock);
        }
    }
}

DWORD WINAPI handle_client(LPVOID client_socket_parameter)
{
    char name[50];
    SOCKET client_socket = (SOCKET)client_socket_parameter;
    char buffer[1024];
    EnterCriticalSection(&lock);
    send(client_socket, "Enter Your Name: ", 18, 0);
    LeaveCriticalSection(&lock);

    int total = 0;
    while (1)
    {
        int n = recv(client_socket, name + total, sizeof(name) - 1, 0);
        if (n <= 0)
            break;
        total += n;
        if (name[total - 1] == '\n' || name[total - 1] == '\r')
            break;
    }

    name[total] = '\0';
    name[strcspn(name, "\n\r")] = '\0';

    for (int i = 0; i < client_count; i++)
    {
        if (clients[i].socket == client_socket)
        {
            strcpy(clients[i].name, name);
            break;
        }
    }

    while (1)
    {
        int total = 0;
        while (1)
        {
            int n = recv(client_socket, buffer + total, sizeof(buffer) - total - 1, 0);
            if (n <= 0)
                break;
            total += n;
            if (buffer[total - 1] == '\n' || buffer[total - 1] == '\r')
                break;
        }
        buffer[total] = '\0';
        buffer[strcspn(buffer, "\r\n")] = '\0';

        if (total <= 0)
        {
            printf("Client Disconnected!\n");
            removeClient(client_socket);
            break;
        }

        if (strlen(buffer) == 0)
        {
            continue;
        }

        printf("[Server]: %s: %s\n", name, buffer);
        char msg[1100];
        sprintf(msg, "%s : %s", name, buffer);
        if (strncmp(buffer, "/msg", 4) == 0)
        {
            char target_name[50];
            char private_message[1100];

            sscanf(buffer, "/msg %s %[^\n]", target_name, private_message);
            int found = 0;
            for (int i = 0; i < client_count; i++)
            {
                if (strcmp(clients[i].name, target_name) == 0)
                {
                    char msg[1300];
                    sprintf(msg, "[PM from %s]: %s", name, private_message);
                    EnterCriticalSection(&lock);

                    send(clients[i].socket, msg, strlen(msg), 0);
                    LeaveCriticalSection(&lock);

                    found = 1;
                    break;
                }
            }

            if (!found)
            {
                char *err = "User Not Found!\n";
                EnterCriticalSection(&lock);

                send(client_socket, err, strlen(err), 0);
                LeaveCriticalSection(&lock);
            }
        }
        else if (strncmp(buffer, "/list", 5) == 0)
        {
            char list_message[1100] = "\n--- Connected Users ---\n";
            for (int i = 0; i < client_count; i++)
            {
                sprintf(list_message + strlen(list_message), "%s\n", clients[i].name);
            }
            strcat(list_message, "------------------------\n");
            char final_msg[1200];
            sprintf(final_msg, "\n%s\n", list_message);
            EnterCriticalSection(&lock);
            send(client_socket, final_msg, strlen(final_msg), 0);
            LeaveCriticalSection(&lock);
        }
        else if (strncmp(buffer, "/exit", 5) == 0)
        {
            printf("%s left the chat!\n", name);
            removeClient(client_socket);
            break;
        }
        else
        {
            broadcast(msg, client_socket);
        }
    }

    closesocket(client_socket);
    return 0;
}

int main()
{
    // Initialize Winsock
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
    // Creating Server Socket
    SOCKET server_socket;
    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (server_socket == INVALID_SOCKET) // if Socket fails
    {
        printf("There is Error in Sockets");
        return 1;
    }
    printf("Server Socket Working..\n");
    // Seting Up server address (IP + Port)
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8080);

    if (bind(server_socket, (struct sockaddr *)&server, sizeof(server)) < 0) // if Binding Fails
    {
        printf("Bind Failed\n");
        return 1;
    }

    listen(server_socket, 3); // Making Server to Accept Clients Max 3
    printf("Server Listening on Port 8080...\n");

    // Creating Socket for client
    SOCKET client_socket;
    struct sockaddr_in client;
    int c = sizeof(struct sockaddr_in); // Storing Size of the structure
    InitializeCriticalSection(&lock);
    while (1)
    {
        client_socket = accept(server_socket, (struct sockaddr *)&client, &c); // accepts incoming client connection
        if (client_socket == INVALID_SOCKET)
        {
            printf("Client Acception Failed.\n");
            continue;
        }
        clients[client_count].socket = client_socket;
        client_count++;
        printf("Client Connected!\n");
        CreateThread(NULL, 0, handle_client, (LPVOID)client_socket, 0, NULL);
    }

    return 0;
}

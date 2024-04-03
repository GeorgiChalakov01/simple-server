#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<stdlib.h>
#include<netinet/in.h>
#include<unistd.h>
#include<fcntl.h>
#include <sys/stat.h>
#include <errno.h>


#define ROOT_DIRECTORY "/home/gchalakovmmi/www/"

#define DEFAULT_PORT "8080"
#define MESSAGE_LENGTH 99999
#define MAX_QUEUE_LENGTH 1000
#define REQUEST_LENGTH 7777777
#define MAX_FILE_PATH 7777777
#define SERVER_ROOT_DIRECTORY ROOT_DIRECTORY "%s"
#define HEADER_SIZE 77777
#define BUFFER_SIZE 77777

void write_message(char *in_message, char type);
void send_response(int client_socket, const char *status, const char *content_type, const char *body, long body_length);
void handle_get_request(int client_socket, char *file_path);
void handle_head_request(int client_socket, char *file_path);
void handle_delete_request(int client_socket, char *file_path);

int main(int argc, char *argv[]) {
	int port;
	
	// Allocate memmory for the message buffer
	char *message = malloc(MESSAGE_LENGTH * sizeof(char));

	if(message == NULL) {
		write_message("Memory allocation failed.", 'e');
	}

	// Allocate memmory for the request buffer
	char *request = malloc(REQUEST_LENGTH * sizeof(char));

	if(request == NULL) {
		write_message("Memory allocation failed.", 'e');
	}

	// Inform the user on which ip:port will the server be hosted.
	strcpy(message, "Hosting on localhost:");

	if(argc == 2) {
		strcat(message, argv[1]);
		port = atoi(argv[1]);
	} else {
		strcat(message, DEFAULT_PORT);
		port = atoi(DEFAULT_PORT);
	}

	write_message(message, 'i');

	// Create the socket file descriptor
	int server_fd;
	if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		write_message("Couldn't create the socket!\nExiting...", 'e');

	// Attaching the socket to the port
	int opt = 1;
	struct sockaddr_in address;
	if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
		write_message("Couldn't attach the socket to the port! Probably the port is in use by another program.\n", 'e');
	}

	address.sin_family = AF_INET;
		address.sin_addr.s_addr = INADDR_ANY;
		address.sin_port = htons(port);


	// Bind to the port
	if(bind(server_fd, (struct sockaddr *) &address, sizeof(address)) < 0){
		write_message("Could not bind to the port!", 'e');
	}

	// Listen for connections
		if (listen(server_fd, MAX_QUEUE_LENGTH) < 0) {
		write_message("listen: Failed to listen on socket", 'e');
				perror("listen");
		}

	while(1) {
		// Check if accept fails
		int client_socket;
		int addrlen = sizeof(address);
				if ((client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
			// If it does print the error to the console
			write_message("Accept failed!", 'e');
						perror("accept");
						continue;
				}

		// Read the incoming request
		long request_length = read(client_socket, request, REQUEST_LENGTH);
		if(request_length < 0) {
			write_message("Could not read the client request.", 'i');
			continue;
		}
		else if(request_length == 0) {
			write_message("Empty request sent by the browser.", 'i');
			continue;
		}
		else if(request_length != 0){
			snprintf(message, MESSAGE_LENGTH, "\tIncoming\n%s", request);
			write_message(message, 'i');
		}

		// Parse the request
		char *method = strtok(request, " ");
		char *requested_file_path = strtok(NULL, " ");

		// Check if the request has a path
		//if (requested_file_path == NULL || strlen(requested_file_path) == 0) {
		//	continue;
		//}

		char local_file_path[MAX_FILE_PATH];
		snprintf(local_file_path, MAX_FILE_PATH,SERVER_ROOT_DIRECTORY, requested_file_path);

		if(strncmp(method, "GET", 3) == 0) {
						handle_get_request(client_socket, local_file_path);
				} else if(strncmp(method, "HEAD", 4) == 0) {
			handle_head_request(client_socket, local_file_path);
				} else if(strncmp(method, "DELETE", 6) == 0) {
			handle_delete_request(client_socket, local_file_path);
				} else {
			strcpy(message, "<html><body><h1>501 Not Implemented</h1></body></html>");
						send_response(client_socket, "501 Not Implemented", "text/html", message, strlen(message));
				}
	}

	// Free allocated memmory
	free(message);
	free(request);

	return 0;
}

void write_message(char *in_message, char type) {
	char message[MESSAGE_LENGTH] = "**************************\n";

	switch (type){
		case 's': 
			strcat(message, "SUCCESS: \n");
		break;

		case 'i': 
			strcat(message, "INFO: \n");
		break;

		case 'e': 
			strcat(message, "ERROR: \n");
		break;

		default:
			printf("Error! Wrong message type given to void write_message(char *in_message, char type); Please supply one of the following values 's', 'i', 'e'.\n You supplied: '%c'\n", type);
			exit(EXIT_FAILURE);
	}
	
	strcat(message, in_message);
	strcat(message, "\nEND\n**************************\n\n");

	printf("%s", message);

	if(type == 'e')
		exit(EXIT_FAILURE);
}

void send_response(int client_socket, const char *status, const char *content_type, const char *body, long body_length) {
	char message[MESSAGE_LENGTH];
	char header[HEADER_SIZE];

	snprintf(header, HEADER_SIZE, "HTTP/1.1 %s\r\nContent-Type: %s\r\nContent-Length: %ld\r\n\r\n", status, content_type, body_length);
	send(client_socket, header, strlen(header), 0);

	snprintf(message, MESSAGE_LENGTH, "\tOutgoing Header\n%s", header);
	write_message(message, 'i');

	if (body && body_length > 0) {
		// Log the response body
	snprintf(message, MESSAGE_LENGTH, "\tOutgoing Body\n%.*s", MESSAGE_LENGTH - 20, body);

		write_message(message, 'i');

		// Send the response body
		send(client_socket, body, body_length, 0);
	}
}

void handle_get_request(int client_socket, char *file_path) {
	// Open the file
	int filefd = open(file_path, O_RDONLY);

	// File not found, send 404 Not Found response
	if (filefd == -1) {
		send_response(client_socket, "404 Not Found", "text/html", "Not Found", 9);
		return;
	}

	// Get the file size
	long file_size = lseek(filefd, 0, SEEK_END);

	// Reset the file pointer to the beginning
	lseek(filefd, 0, SEEK_SET);

	// Allocate a buffer to hold the file contents
	char *buffer = malloc(file_size + 1);

	// Read the file into the buffer
	read(filefd, buffer, file_size);

	// Null-terminate the buffer
	buffer[file_size] = '\0';

	// Send the response
	send_response(client_socket, "200 OK", "text/html", buffer, file_size);

	// Free the buffer
	free(buffer);

	// Close the file
	close(filefd);
}


void handle_head_request(int client_socket, char *file_path) {
	struct stat file_stat;
	if (stat(file_path, &file_stat) < 0) {
		// File not found, send 404 Not Found response
		send_response(client_socket, "404 Not Found", "text/html", "", 0);
		return;
	}

	// File found, send headers only
	send_response(client_socket, "200 OK", "text/html", NULL, file_stat.st_size);
}

void handle_delete_request(int client_socket, char *file_path) {
	// Attempt to delete the file
	if (remove(file_path) == 0) {
		// File successfully deleted, send 200 OK response
		send_response(client_socket, "200 OK", "text/html", "", 0);
	} else {
		// File could not be deleted, check the type of error
		if (errno == ENOENT) {
			// File not found, send 404 Not Found response
			send_response(client_socket, "404 Not Found", "text/html", "", 0);
		} else {
			// Other error, send 500 Internal Server Error response
			send_response(client_socket, "500 Internal Server Error", "text/html", "", 0);
		}
	}
}


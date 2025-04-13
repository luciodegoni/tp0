#include"utils.h"

t_log* logger;

int iniciar_servidor(void)
{

	int socket_servidor;

	struct addrinfo hints, *servinfo, *p;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(NULL, PUERTO, &hints, &servinfo);

// Recorremos la lista de direcciones hasta encontrar una válida
for (p = servinfo; p != NULL; p = p->ai_next) {
	socket_servidor = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
	if (socket_servidor == -1)
		continue;

	if (bind(socket_servidor, p->ai_addr, p->ai_addrlen) == 0)
		break; // ¡Éxito!

	close(socket_servidor); // Falló el bind, cerramos y probamos el siguiente
}

if (p == NULL) {
	log_error(logger, "No se pudo bindear el socket");
	exit(EXIT_FAILURE);
}

// Ponemos el socket en modo escucha
if (listen(socket_servidor, SOMAXCONN) == -1) {
	perror("listen");
	exit(EXIT_FAILURE);
}

freeaddrinfo(servinfo);
log_trace(logger, "Listo para escuchar a mi cliente");

return socket_servidor;
}

int esperar_cliente(int socket_servidor)
{
	struct sockaddr_in direccion_cliente;
	socklen_t tamanio_direccion = sizeof(struct sockaddr_in);

	// Aceptamos un nuevo cliente
	int socket_cliente = accept(socket_servidor, (void*)&direccion_cliente, &tamanio_direccion);

	if (socket_cliente == -1) {
		perror("accept");
		exit(EXIT_FAILURE);
	}

	log_info(logger, "¡Se conectó un cliente!");

	return socket_cliente;
}

int recibir_operacion(int socket_cliente)
{
	int cod_op;
	if(recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0)
		return cod_op;
	else
	{
		close(socket_cliente);
		return -1;
	}
}

void* recibir_buffer(int* size, int socket_cliente)
{
	void * buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

void recibir_mensaje(int socket_cliente)
{
	int size;
	char* buffer = recibir_buffer(&size, socket_cliente);
	log_info(logger, "Me llego el mensaje %s", buffer);
	free(buffer);
}

t_list* recibir_paquete(int socket_cliente)
{
	int size;
	int desplazamiento = 0;
	void * buffer;
	t_list* valores = list_create();
	int tamanio;

	buffer = recibir_buffer(&size, socket_cliente);
	while(desplazamiento < size)
	{
		memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		char* valor = malloc(tamanio);
		memcpy(valor, buffer+desplazamiento, tamanio);
		desplazamiento+=tamanio;
		list_add(valores, valor);
	}
	free(buffer);
	return valores;
}

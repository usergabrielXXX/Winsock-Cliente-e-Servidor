/* APLICATIVO CLIENTE */

/*
Aqui estou usando C majoritariamente, mas depois refatoro e mundo para C++.
Também pretendo adicionar um canal seguro(criptografia), mas só farei isso quando tiver mais experiência :)

Me desculpem pelo código sujo.
*/

#include <stdio.h>
#include <iostream> // std::cin E/S padrão C++
#include <stdlib.h>
#include <string.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
// Precisa vincular com Ws2_32.lib, Mswsock.lib e Advapi32.lib
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "Advapi32.lib")

//-------------------------------------------------- Exemplos e explicações da documentação WinSock2.h -----------------------------------------------------------------------------
//	A função WSAStartup é chamada para iniciar o uso de WS2_32.dll.
//	A estrutura WSADATA contém informações sobre a implementação do Windows Sockets.
//	O parâmetro MAKEWORD(2,2) do WSAStartup faz uma solicitação para a versão 2.2 do Winsock no sistema e define a versão
//	passada como a versão mais alta do Suporte do Windows Sockets que o chamador pode usar.
//	--------------------------------------------------------------------------------------------------------------------------------
//	Declare um objeto addrinfo que contém uma estrutura sockaddr e inicialize esses valores.
//	Para esse aplicativo, a família de endereços da Internet não é especificada para que um endereço IPv6 ou IPv4 possa ser retornado.
//	O aplicativo solicita que o tipo de soquete seja um soquete de fluxo para o protocolo TCP
//-----------------------------------------Estrutura de 'struct addrinfo'---------------------------------------------------------
//	struct addrinfo* result = NULL,
//	* ptr = NULL,
//	hints;
//
//	ZeroMemory(&hints, sizeof(hints));
//	hints.ai_family = AF_INET;
//	hints.ai_socktype = SOCK_STREAM;
//	hints.ai_protocol = IPPROTO_TCP;
//--------------------------------------------------------------------------------------------------------------------------------
//	Chame a função getaddrinfo solicitando o endereço IP para o nome do servidor passado na linha de comando.
//	A porta TCP no servidor ao qual o cliente se conectará é definida por PORTA como 27015 neste exemplo.
//	A função getaddrinfo retorna seu valor como um inteiro que é verificado quanto a erros.
// 
//#define PORTA "27015"
//
//Resolva o endereço e a porta do servidor
//	iResult = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result);
//	if (iResult != 0) {
//		printf("getaddrinfo failed: %d\n", iResult);
//		WSACleanup();
//		return 1;
//	}

#define BUFFER_MAX 512
#define BUFFER_MSG 1024

int main(int argc, char* argv[])
{
	WSADATA wsadata;
	int iResultado;
	char buffer_msg[BUFFER_MSG];
	// INET_ADDRSTRLEN é definida como 16 e é usada para garantir que o buffer de string alocado seja grande o suficiente para armazenar qualquer endereço IPv4.
	char ip[INET_ADDRSTRLEN] = { 0 };
	char PORTA[6] = { 0 };
	//--------------------------------------
	//O erro ocorre porque a função getaddrinfo espera um ponteiro para um ponteiro de addrinfo 
	//como o último argumento,
	//mas você está passando um int.
	//O correto seria utilizar um ponteiro para um ponteiro de addrinfo, e não um int.
	addrinfo* resultado = NULL;
	//--------------------------------------
	addrinfo soquete;
	ZeroMemory(&soquete, sizeof(soquete));
	soquete.ai_family = AF_INET;
	soquete.ai_socktype = SOCK_STREAM;
	soquete.ai_protocol = IPPROTO_TCP;

	printf("Insira o IP do servidor (Ex. 192.158.1.38):  ");
	scanf_s("%s", ip, (unsigned)_countof(ip));
	printf("Insira a porta (Ex. 4565): ? ");
	scanf_s("%s", PORTA, (unsigned)_countof(PORTA));
	// vetor para recebimento de mensagens


	//  scanf_s("%s", ip, (unsigned)_countof(ip));
	//if (argc != 2) {
	//	puts("----- CLIENTE -----");
	//	fprintf(stderr, "Uso: %s ip-do-servidor\n", argv[0]);
	//	printf("Exemplo: %s 127.0.0.1\n", argv[0]);
	//	return 1;
	//}
	iResultado = WSAStartup(MAKEWORD(2, 2), &wsadata);
	if (iResultado != 0) {
		printf("WSAStartup falhou e seu erro e: %d\n", iResultado);
		return 1;
	}
	iResultado = getaddrinfo(ip, PORTA, &soquete, &resultado);
	if (iResultado != 0) {
		printf("getaddrinfo falhou: %d\n", iResultado);
		//WSAStartup();
		WSACleanup();
		return 1;
	}
	//Crie um objeto SOCKET chamado ConnectSocket.
	SOCKET ConectarSocket = INVALID_SOCKET;
	// Chame a função de soquete e retorne seu valor para a variável ConectarSocket.
		//getaddrinfo* ptr = &resultado;
	addrinfo* ptr = resultado;
	ConectarSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
	//verifique se há erros para garantir que o soquete seja um soquete válido.
	if (ConectarSocket == INVALID_SOCKET) {
		printf("Erro no socket: %ld\n", WSAGetLastError());
		freeaddrinfo(resultado);
		WSACleanup();
		return 1;
	}
	//--------------------------Conectar ao servidor--------------------------
	//Chame a função connect, passando o soquete criado e a estrutura sockaddr como parâmetros.
	iResultado = connect(ConectarSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
	//Verifique se há erros gerais
	if (iResultado == SOCKET_ERROR) {
		closesocket(ConectarSocket);
		//ConectarSocket(INVALID_SOCKET);
		ConectarSocket = INVALID_SOCKET;
	}
	// Libera a memória alocada por getaddrinfo
	freeaddrinfo(resultado);
	//--------------------------implementação de CHAT--------------------------
		// loop
	while (true) {
		// envio de mensagens
		printf("Insira sua mensagem(ou 'sair' para sair do chat): ");
		std::cin >> buffer_msg;
		//fgets(buffer_msg, BUFFER_MSG, stdin);
		// lidando com caracteres
		size_t rmv = strlen(buffer_msg);
		if (rmv > 0 && buffer_msg[rmv - 1] == '\n') {
			buffer_msg[rmv - 1] = '\0';
		}
		iResultado = send(ConectarSocket, buffer_msg, (int)strlen(buffer_msg), 0);
		if (iResultado == SOCKET_ERROR) {
			fprintf(stderr, "Erro ao enviar sua mensagem: %ld\n", WSAGetLastError());
			break;
		}
		// se o usuario quiser sair, pode-se verificar dessa forma
		// para sair basta digitar: sair
		if (strcmp(buffer_msg, "sair") == 0) {
			break;
		}
		// receber alguma resposta do servidor
		iResultado = recv(ConectarSocket, buffer_msg, BUFFER_MSG - 1, 0);
		if (iResultado > 0) {
			buffer_msg[iResultado] = '\0'; // adicionar caracter nulo no fim da string
			printf("Resposta: %s\n", buffer_msg);
		}
		else if (iResultado == 0) {
			printf("Conexao fechada pelo servidor.\n");
		}
		else {
			printf("Erro ao receber dados: %ld\n", WSAGetLastError());
			break;
		}
	}

	if (ConectarSocket == INVALID_SOCKET) {
		printf("Nao foi possivel conectar ao socket.\n");
		WSACleanup();
		return 1;
	}
	//O código a seguir demonstra as funções send e recv
	//usadas pelo cliente depois que uma conexão é estabelecida.
	int recvBuffer = BUFFER_MAX;
	//const char* enviarBuffer = "Essa e a mensagem.\n";
	char recvBuf[BUFFER_MAX];
	//---------------------enviar o buffer inicial---------------------
	//As funções SEND e RECV retornam um valor inteiro do número de bytes enviados ou recebidos,
	//respectivamente, ou um erro.Cada função também usa os mesmos parâmetros : o soquete ativo, um buffer char,
	//o número de bytes a serem enviados ou recebidos e
	//quaisquer sinalizadores a serem usados.
	iResultado = send(ConectarSocket, buffer_msg, (int)strlen(buffer_msg), 0);
	if (iResultado == SOCKET_ERROR) {
		printf("O envio(send) falhou: %d\n", WSAGetLastError());
		closesocket(ConectarSocket);
		WSACleanup();
		return 1;
	}
	printf("Bytes enviados: %ld\n", iResultado);
	// encerra a conexão para envio já que nenhum dado será enviado
	// o cliente ainda pode usar o ConectarSocket para receber dados
	iResultado = shutdown(ConectarSocket, SD_SEND);
	if (iResultado == SOCKET_ERROR) {
		printf("O desligamento(shutdown) falhou: %d\n", WSAGetLastError());
		closesocket(ConectarSocket);
		WSACleanup();
		return 1;
	}
	//Receba dados até que o servidor feche a conexão
	do {
		iResultado = recv(ConectarSocket, recvBuf, recvBuffer, 0);
		if (iResultado > 0) {
			printf("Bytes recebidos: %d\n", iResultado);
		}
		else if (iResultado == 0) {
			printf("Conexao fechada.\n");
		}
		else {
			printf("recvBuf falhou.\n");
		}
	} while (iResultado > 0);
	//---------------------Desconectando o cliente---------------------
	//Depois que o cliente é concluído enviando e recebendo dados,
	// o cliente se desconecta do servidor e desliga o soquete.
	// desliga a metade de envio da conexão já que nenhum dado será enviado
	//Quando o aplicativo cliente termina de receber dados,
	// a função closesocket é chamada para fechar o soquete.
	if (iResultado == SOCKET_ERROR) {
		printf("Shutdown falhou: %d\n", WSAGetLastError());
		closesocket(ConectarSocket);
		WSACleanup();
		return 1;
	}
	//Quando o aplicativo cliente é concluído usando a DLL do Windows Sockets,
	// fechando o socket
	closesocket(ConectarSocket);
	// a função WSACleanup é chamada para liberar recursos
	WSACleanup();
	return 0;
}

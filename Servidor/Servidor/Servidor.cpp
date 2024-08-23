/* APLICATIVO SERVIDOR */

#include <iostream>
#include <cstring>
#include <thread>
#include <vector>
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#define BUFFER_TAMANHO_MAXIMO	1024
#define PORTA "4565"

typedef struct {
	SOCKET socket_cliente;
	sockaddr_in addr_cliente;
}Cliente_Dados;
// prototipando a função pra tentar o codigo bonito kkk
void Comunicacao(Cliente_Dados cliente);

int main(void) {

	WSADATA servidor_socket;
	int recebe_informacoes;
	SOCKET Ouvindo_cliente	= { 0 };
	SOCKET socket__			= { 0 };
	std::vector<std::thread> cliente_threads;
	// a estrutura 'addrinfo' é usada pela função 'getaddrinfo' para armazenar informações de endereço do host.
	addrinfo* estrutura = NULL, soquete;
	recebe_informacoes = WSAStartup(MAKEWORD(2,2), &servidor_socket);
	// verificando se WinSock pode iniciar suas operações
	if (recebe_informacoes != 0) {
		std::cerr << "Falha ao iniciar o WinSock: " << recebe_informacoes << '\n';
		return 1;
	}
	ZeroMemory(&soquete, sizeof(soquete));
	soquete.ai_family	= AF_INET;
	soquete.ai_socktype = SOCK_STREAM;
	soquete.ai_protocol = IPPROTO_TCP;
	soquete.ai_flags	= AI_PASSIVE;
	// a função 'getaddrinfo' fornece tradução independente de protocolo de um nome de host ANSI para um endereço.
	recebe_informacoes = getaddrinfo(nullptr, PORTA, &soquete, &estrutura);
	if (recebe_informacoes != 0) {
		std::cerr << "getaddrinfo falhou: " << recebe_informacoes << '\n';
		WSACleanup();
		return 1;
	}
	// A função de 'socket' cria um soquete associado a um provedor de serviços de transporte específico.
	Ouvindo_cliente = socket(estrutura->ai_family, estrutura->ai_socktype, estrutura->ai_protocol);
	if (Ouvindo_cliente == INVALID_SOCKET) {
		std::cerr << "Erro ao criar o socket: " << WSAGetLastError() << '\n';
		freeaddrinfo(estrutura);
		WSACleanup();
		return 1;
	}
	// A função 'bind' associa um endereço local a um soquete ou seja, escuta o cliente.
	recebe_informacoes = bind(Ouvindo_cliente, estrutura->ai_addr, (int)estrutura->ai_addrlen);
	if (recebe_informacoes == SOCKET_ERROR) {
		std::cerr << "a ligacao falhou: " << WSAGetLastError() << '\n';
		freeaddrinfo(estrutura);
		closesocket(Ouvindo_cliente);
		WSACleanup();
		return 1;
	}
	// liberando a memoria alocada
	freeaddrinfo(estrutura);
	// A função 'listen' coloca um socket em um estado no qual ele está escutando uma conexão de entrada
	recebe_informacoes = listen(Ouvindo_cliente, SOMAXCONN);
	if (recebe_informacoes == SOCKET_ERROR) {
		std::cerr << "Nao foi possivel obter conexoes: " << WSAGetLastError() << '\n';
		closesocket(Ouvindo_cliente);
		WSACleanup();
		return 1;
	}
	std::cout << "Esperando por conexoes...";
//-------------------------------------- UMA TENTATIVA DE IMPLEMENTAR UM CHAT TODO ZOADO --------------------------------------
	while (true) {
		Cliente_Dados cliente_dados = { 0 };
		// A função 'accept' permite uma tentativa de conexão de entrada em um soquete.
		cliente_dados.socket_cliente = accept(Ouvindo_cliente, (sockaddr*)&cliente_dados.addr_cliente, nullptr);
		if (cliente_dados.socket_cliente == INVALID_SOCKET) {
			std::cerr << "Nao deu pra aceitar o bagulho da conexao meu rei: " << WSAGetLastError() << '\n';
			closesocket(Ouvindo_cliente);
			WSACleanup();
			return 1;
		}
		cliente_threads.push_back(std::thread(Comunicacao, cliente_dados));

	}
	// fim do bagulho
	closesocket(Ouvindo_cliente);

	for (auto& bolsonaro : cliente_threads) {
		bolsonaro.join();
	}
	// limpa tudo e deixa só o ouro e a prata
	WSACleanup();
	return 0;
}




// ----------------------------------- FUNÇAOZINHA PARA CUIDAR DA PARTE CHATA DA COISA -----------------------------------
// esta misera função tem como proposito, cuidar dos bytes recebidos pelo usuário.
// 
void Comunicacao (Cliente_Dados cliente) {
	char tamanho_buffer[BUFFER_TAMANHO_MAXIMO];
	char INET_FODA[INET_ADDRSTRLEN];
	int funsock;

	if (inet_ntop(AF_INET, &(cliente.addr_cliente), INET_FODA, sizeof(INET_FODA)) == NULL) {
		//perror("inet_ntop falhou");
		std::cerr << "inet_ntop falhou\n";
		return;
	}

	while (true) {
		// receber a mensagem do cliente
		funsock = recv(cliente.socket_cliente, tamanho_buffer, BUFFER_TAMANHO_MAXIMO - 1, 0);
		if (funsock > 0) {
			tamanho_buffer[funsock] = '\0'; // terminador nulo da string
			// processar a mensagem recebida, enviar de volta ou retransmitir
			std::cout << "Cliente: " << INET_FODA << '\n';
			// responder o cliente
			funsock = send(cliente.socket_cliente, tamanho_buffer, funsock, 0);
			if (funsock == SOCKET_ERROR) {
				std::cerr << "Houve um erro ao responder o cliente." << INET_FODA;
				break;
			}
			// se caso o usuario digitar "/sair", o socket será encerrado e fim de papo.
			if (strcmp(tamanho_buffer, "/sair") == 0) {
				std::cout << "o cliente " << INET_FODA << " encerrou a conexao.\n";
				break;
			}
		}
		else {
			std::cout << "Erro ao receber dados do cliente " << INET_FODA << '\n';
			break;
		}
	}

}
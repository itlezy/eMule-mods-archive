#include "stdafx.h"
//#include <winsock2.h>
//#include <windows.h>
#include <stdio.h>
#include <string>
#include "HttpComm.h"

using namespace std;

// 16k di buffer
#define RECV_BUF_SIZE	(16*1024)

HttpComm::HttpComm(char *host, unsigned short port)
{
	_connected = false;
	_sd = INVALID_SOCKET;
	strncpy(_host, host, MAX_PATH-1);
	_port = port;
	// Cerco di inizializzare la socket e la apro col server
	IN_ADDR		iaHost;
	LPHOSTENT	lpHostEntry;
	SOCKADDR_IN saServer;

	iaHost.s_addr = inet_addr(_host);
	if (iaHost.s_addr == INADDR_NONE)
	{
		// Wasn't an IP address string, assume it is a name
		lpHostEntry = gethostbyname(_host);
	}
	else
	{
		// It was a valid IP address string
		lpHostEntry = gethostbyaddr((const char *)&iaHost, sizeof(struct in_addr), AF_INET);
	}
	if (lpHostEntry == NULL) return;

	_sd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == _sd) return;

	saServer.sin_port = htons(_port);
	saServer.sin_family = AF_INET;
	saServer.sin_addr = *((LPIN_ADDR)*lpHostEntry->h_addr_list);
	// imposto il timeout di 30 secondi sulla connessione
	int timeout = 30000;
	setsockopt(_sd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(int));
	if (SOCKET_ERROR == connect(_sd, (LPSOCKADDR)&saServer, sizeof(SOCKADDR_IN)))
	{
		closesocket(_sd);
		_sd = INVALID_SOCKET;
	}
	else _connected = true;
}

HttpComm::~HttpComm()
{
	if (_connected)
	{
		if (INVALID_SOCKET != _sd)
		{
			closesocket(_sd);
			_sd = INVALID_SOCKET;
		}
		_connected = false;
	}
}

// Implementazione dei metodi privati

bool HttpComm::sendAll(const char *buf, int size)
{
	int i = 0;
	while(i < size)
	{
		int rv = send(_sd, buf+i, size-i, 0);
		if (rv <= 0) return false;
		i += rv;
	}
	return true;
}

bool HttpComm::recvAll(char *buf, int size)
{
	int i = 0;
	while(i < size)
	{
		int rv = recv(_sd, buf+i, size-i, 0);
		if (rv <= 0) return false;
		i += rv;
	}
	return true;
}

int HttpComm::getHeader(char *bufHeader, int sizeIn, int &sizeOut)
{
	int i = 0;
	while(true)
	{
		int rv = recv(_sd, bufHeader+i, sizeIn-i, 0);
		if (rv <= 0) return -1;
		i += rv;
		// Cerco la seuqenza di escape
		char *ptr = strstr(bufHeader, "\r\n\r\n");
		if (ptr)
		{
			sizeOut = i;
			i = ptr-bufHeader+2;
			break;
		}
		if (i == sizeIn) return -1;
	}
	return i;
}

void HttpComm::getHeaderInfos(char *header, int headerSize, HttpComm::LPHTTPHeaderInfo info)
{
	char	line[MAX_PATH];
	int		i = 0;
	char	*curHeader = header;

	// Inizializzo i parametri di output
	info->response = 0;
	info->contentLength = 0;
	info->chunked = false;

	while(curHeader < header+headerSize)
	{
		char *ptr = strstr(curHeader, "\r\n");
		if (ptr && ptr-curHeader < MAX_PATH-1)
		{
			memcpy(line, curHeader, ptr-curHeader);
			line[ptr-curHeader] = '\0';
			printf("%s\n", line);
			// Esegui il parse della linea
			// HTTP/1.1
			if (strstr(line, "HTTP/1.1 "))
			{
				info->response = atoi(line+9);
			}
			// Content-length
			else if (strstr(line, "Content-Length:"))
			{
				info->contentLength = atoi(line+15);
			}
			// Transfer-Encoding: chunked
			else if (strstr(line, "Transfer-Encoding: chunked"))
			{
				info->chunked = true;
			}
		}
		curHeader = ptr+2;
	}
}

int HttpComm::getChunkedSize(char *chunkedInfo, int &chunkedInfoLength)
{
	int		ret = 0;
	int		exitFromCycle = false;
	char	*ptr = strstr(chunkedInfo, "\r\n");
	if (!ptr) return -1;
	chunkedInfoLength = ptr-chunkedInfo+2;
	if (chunkedInfoLength > 10) return -1;

	for(int i=0; i < chunkedInfoLength; i++)
	{
		if (exitFromCycle) break;

		ret *= 16;
		switch(chunkedInfo[i])
		{
			case '0':
				ret+= 0;
				break;
			case '1':
				ret += 1;
				break;
			case '2':
				ret+= 2;
				break;
			case '3':
				ret+= 3;
				break;
			case '4':
				ret+= 4;
				break;
			case '5':
				ret += 5;
				break;
			case '6':
				ret+= 6;
				break;
			case '7':
				ret+= 7;
				break;
			case '8':
				ret+= 8;
				break;
			case '9':
				ret += 9;
				break;
			case 'a':
			case 'A':
				ret+= 10;
				break;
			case 'b':
			case 'B':
				ret+= 11;
				break;
			case 'c':
			case 'C':
				ret+= 12;
				break;
			case 'd':
			case 'D':
				ret += 13;
				break;
			case 'e':
			case 'E':
				ret+= 14;
				break;
			case 'f':
			case 'F':
				ret+= 15;
				break;
			default:
				ret /= 16;
				exitFromCycle = true;
				break;
		}
	}

	return ret;
}

bool HttpComm::getContentLengthData(char *buffer, int sizeBuf, int contentLength, FILE **fpOut)
{
	// Se mi viene passato un stream aperto lo chiudo comunque
	if (*fpOut) fclose(*fpOut);
	// Ricezione dei dati
	*fpOut = tmpfile();
	if (!*fpOut) return false;
	// Scrivo i dati in eccesso nel primo pacchetto
	fwrite(buffer, 1, sizeBuf, *fpOut);
	// Se devo ricevere altri dati (come e' solito) allora cerco di riceverli tutti
	if (contentLength > sizeBuf)
	{
		// So quanto dovro' ricevere ancora
		contentLength -= sizeBuf;
		// Creo un buffer temporaneo di ricezione di 16k
		char	tmpBuf[RECV_BUF_SIZE];

		// Spezzo la ricezione in pezzi da RECV_BUF_SIZE l'uno
		while(contentLength > RECV_BUF_SIZE)
		{
			if (recvAll(tmpBuf, RECV_BUF_SIZE))
			{
				fwrite(tmpBuf, 1, RECV_BUF_SIZE, *fpOut);
				contentLength -= RECV_BUF_SIZE;
			}
			else
			{
				fclose(*fpOut);
				return false;
			}
		}
		if (recvAll(tmpBuf, contentLength))
		{
			fwrite(tmpBuf, 1, contentLength, *fpOut);
			contentLength = 0;
		}
		else
		{
			fclose(*fpOut);
			return false;
		}
	}

	return true;
}

bool HttpComm::getChunkedData(char *buffer, int sizeBuf, FILE **fpOut)
{
	// Se mi viene passato un stream aperto lo chiudo comunque
	if (*fpOut) fclose(*fpOut);
	// Ricezione dei dati
	*fpOut = tmpfile();
	if (!*fpOut) return false;
	// Creo un buffer di RECV_BUF_SIZE byte
	char	chunkBuf[RECV_BUF_SIZE];
	int		recvBytes = 0;
	int		infoLenght = 0;
	// Variabile booleana che indica se e' la prima volta che si riceve dati
	bool	firstTime = true;

	while(true)
	{
		int rb = 0;

		if (firstTime && sizeBuf)
		{
			// imposto il size ricevuto
			rb = sizeBuf;
			// Copio i caratteri temporanei
			memcpy(chunkBuf, buffer, rb);
			firstTime = false;
		}
		else
		{
			rb = recv(_sd, chunkBuf, RECV_BUF_SIZE, 0);
			if (rb < 0)
			{
				fclose(*fpOut);
				return false;
			}
			else if (rb == 0)
			{
				break;
			}
		}
		// trovo il valore del primo chunk
		recvBytes = getChunkedSize(chunkBuf, infoLenght);
		// Se e' 0 esci dal ciclo while
		if (0 == recvBytes) break;
		else if (-1 == recvBytes)
		{
			fclose(*fpOut);
			return false;
		}
		// Se i bytes che devo ricevere li ho gia' nel buffer
		// Prendo ogni chunk e lo scrivo nel file temporaneo
		while(recvBytes && (recvBytes+2 < rb-infoLenght))
		{
			// scrivo il chunk
			fwrite(chunkBuf+infoLenght, 1, recvBytes, *fpOut);
			// Incremento infoLenght
			infoLenght += (recvBytes+2);
			//for(int i = 0; i <  recvBytes; i++) printf("%c", *(chunkBuf+infoLenght+i));
			// prendo il prossimo chunk
			int tmpInfoLenght = 0;
			recvBytes = getChunkedSize(chunkBuf+infoLenght, tmpInfoLenght);
			if (-1 == recvBytes)
			{
				fclose(*fpOut);
				return false;
			}
			infoLenght += tmpInfoLenght;
		}
		// Se e' 0 esci dal ciclo while
		if (0 == recvBytes) break;
		// Li scrivo nel file
		fwrite(chunkBuf+infoLenght, 1, rb-infoLenght, *fpOut);
		// Sistemo il valore di recvBytes (tolgo i bytes ricevuti in piu' di informazioni nel chunk)
		recvBytes -= rb-infoLenght;
		// Sistemo il valore di recvBytes (aggiungo i caratteri "\r\n")
		recvBytes += 2;
		// Ricevo tutto il chunk
		while(recvBytes > RECV_BUF_SIZE)
		{
			if (recvAll(chunkBuf, RECV_BUF_SIZE))
			{
				fwrite(chunkBuf, 1, RECV_BUF_SIZE, *fpOut);
				recvBytes -= RECV_BUF_SIZE;
			}
			else
			{
				fclose(*fpOut);
				return false;
			}
		}
		if (recvAll(chunkBuf, recvBytes))
		{
			fwrite(chunkBuf, 1, recvBytes-2, *fpOut);
			recvBytes = 0;
		}
	}

	return true;
}

// Implementazione dei metodi pubblici

int HttpComm::Get(char *resource, FILE **fpOut, char *response, unsigned int responseSize)
{
	// Verifico la variabile iniziale
	if (!fpOut) return -1;

	if (!_connected) return -1;
	// Creazione stringa di richiesta HTTP
	string request;
	// Risorsa
	request.append("GET ");
	request.append(resource);
	request.append(" HTTP/1.1\r\n");
	// Host
	request.append("Host: ");
	request.append(_host);
	request.append("\r\n");
	// Keep-Alive
	request.append("Connection: Keep-Alive\r\n");
	// User-Agent
	request.append("User-Agent: AduCore/1.0\r\n");
	// Fine richiesta
	request.append("\r\n");
	// Mando la richiesta all'host
	if (!sendAll(request.data(), request.size())) return -1;
	// Ricevo l'header
	char	headerBuf[1024];
	int		sizeOut;
	int		headerSize = getHeader(headerBuf, 1024, sizeOut);
	if (headerSize == -1) return -1;
	// Elaborazione dell'header
	HttpComm::HTTPHeaderInfo	info;
	getHeaderInfos(headerBuf, headerSize, &info);
	// Se ho il campo Content-lenght
	if (info.contentLength > 0)
	{
		if (!getContentLengthData(headerBuf+headerSize+2, sizeOut-(headerSize+2), info.contentLength, fpOut)) return -1;
	}
	// Caso chunked
	else if (info.chunked)
	{
		if (!getChunkedData(headerBuf+headerSize+2, sizeOut-(headerSize+2), fpOut)) return -1;
	}
	// Caso HTTP standard (con connessione chiusa)
	else
	{
		// Se mi viene passato un stream aperto lo chiudo comunque
		if (*fpOut) fclose(*fpOut);
		// Ricezione dei dati
		*fpOut = tmpfile();
		if (!*fpOut) return -1;
		// Scrivo i dati in eccesso nel primo pacchetto
		fwrite(headerBuf+headerSize+2, 1, sizeOut-(headerSize+2), *fpOut);
		// dopodiche' finche' ricevo scrivo
		while(true)
		{
			char	tmpBuf[RECV_BUF_SIZE];
			int	rb = recv(_sd, tmpBuf, RECV_BUF_SIZE, 0);
			if (SOCKET_ERROR == rb)
			{
				fclose(*fpOut);
				return -1;
			}
			else if (0 == rb) break;
			else
			{
				fwrite(tmpBuf, 1, rb, *fpOut);
			}
		}
	}

	// Rimetto il puntatore del file all'inizio
	if (*fpOut) fseek(*fpOut, 0L, SEEK_SET);

	// Se ho un buffer per la risposta
	if (response && responseSize >= headerSize+1)
	{
		memcpy(response, headerBuf, headerSize);
		response[headerSize] = '\0';
	}

	return info.response;
}

int HttpComm::Post(char *resource, char *data, int dataLength, FILE **fpOut)
{
	char tmpStr[64];

	if (!_connected) return -1;
	// Creazione stringa di richiesta HTTP
	string request;
	// Risorsa
	request.append("POST ");
	request.append(resource);
	request.append(" HTTP/1.1\r\n");
	// Host
	request.append("Host: ");
	request.append(_host);
	request.append("\r\n");
	// Content-length
	sprintf(tmpStr, "Content-length: %i\r\n", dataLength);
	request.append(tmpStr);
	// Keep-Alive
	request.append("Connection: Keep-Alive\r\n");
	// User-Agent
	request.append("User-Agent: AduCore/1.0\r\n");
	// Content-Type
	request.append("Content-Type: Application/x-www-form-urlencoded\r\n");
	// Fine richiesta
	request.append("\r\n");
	// Mando la richiesta all'host
	if (!sendAll(request.data(), request.size())) return -1;
	// Mando i dati all'host
	if (!sendAll(data, dataLength)) return -1;
	// Ricevo l'header
	char	headerBuf[1024];
	int		sizeOut;
	int		headerSize = getHeader(headerBuf, 1024, sizeOut);
	if (headerSize == -1) return -1;
	// Elaborazione dell'header
	HttpComm::HTTPHeaderInfo	info;
	getHeaderInfos(headerBuf, headerSize, &info);
	// Anche con la POST molti host mi mandano comunque info nella risposta
	// Se ho il campo Content-lenght
	if (fpOut && info.contentLength > 0)
	{
		if (!getContentLengthData(headerBuf+headerSize+2, sizeOut-(headerSize+2), info.contentLength, fpOut)) return -1;
	}
	// Caso chunked
	else if (fpOut && info.chunked)
	{
		// Per ora non supporto il chunked
		if (!getChunkedData(headerBuf+headerSize+2, sizeOut-(headerSize+2), fpOut)) return -1;
	}
	// Caso HTTP standard (con connessione chiusa)
	else if (fpOut)
	{
		// Se mi viene passato un stream aperto lo chiudo comunque
		if (*fpOut) fclose(*fpOut);
		// Ricezione dei dati
		*fpOut = tmpfile();
		if (!*fpOut) return -1;
		// Scrivo i dati in eccesso nel primo pacchetto
		fwrite(headerBuf+headerSize+2, 1, sizeOut-(headerSize+2), *fpOut);
		// dopodiche' finche' ricevo scrivo
		while(true)
		{
			char	tmpBuf[RECV_BUF_SIZE];
			int	rb = recv(_sd, tmpBuf, RECV_BUF_SIZE, 0);
			if (SOCKET_ERROR == rb)
			{
				fclose(*fpOut);
				return -1;
			}
			else if (0 == rb) break;
			else
			{
				fwrite(tmpBuf, 1, rb, *fpOut);
			}
		}
	}

	// Rimetto il puntatore del file all'inizio
	if (fpOut && *fpOut) fseek(*fpOut, 0L, SEEK_SET);

	return info.response;
}

bool HttpComm::SaveTmpFile(FILE *fp, char *name)
{
	FILE *fpSave = fopen(name, "wb");
	if (!fpSave) return false;
	
	char	tmpBuf[2048];
	int		rb = 0;
	while(rb = fread(tmpBuf, 1, 2048, fp))
	{
		if (rb != fwrite(tmpBuf, 1, rb, fpSave))
		{
			fclose(fpSave);
			return false;
		}
	}

	fclose(fpSave);
	return true;
}

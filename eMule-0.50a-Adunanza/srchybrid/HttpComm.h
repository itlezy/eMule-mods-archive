class HttpComm
{
public:
	// Definizione tipo per le info dell'header
	typedef struct
	{
		int		response;	
		bool	chunked;
		int		contentLength;
	} HTTPHeaderInfo, *LPHTTPHeaderInfo;
private:
	bool			_connected;
	SOCKET			_sd;
	char			_host[MAX_PATH];
	unsigned short	_port;
	// Metodi privati
	// Comunicazione su Socket
	bool sendAll(const char *buf, int size);
	bool recvAll(char *buf, int size);
	// Gestione header HTTP
	int getHeader(char *bufHeader, int sizeIn, int &sizeOut);
	void getHeaderInfos(char *header, int headerSize, HttpComm::LPHTTPHeaderInfo info);
	// Elaborazione chunked
	int getChunkedSize(char *chunkedInfo, int &chunkedInfoLength);
	// Ricezione dati quando c'e' il campo Content-length
	bool getContentLengthData(char *buffer, int sizeBuf, int contentLength, FILE **fpOut);
	// Ricezione dati quando c'e' il campo chunked
	bool getChunkedData(char *buffer, int sizeBuf, FILE **fpOut);
public:
	// Metodi pubblici
	HttpComm(char *host, unsigned short port = 80);
	~HttpComm();
	int Get(char *resource, FILE **fpOut, char *response = NULL, unsigned int responseSize = 0);
	int Post(char *resource, char *data, int dataLength, FILE **fpOut = NULL);
	bool SaveTmpFile(FILE *fp, char *name);
};

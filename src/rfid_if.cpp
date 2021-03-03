#include "rfid_if.hpp"
#include "cpp_if.hpp"
#include <codecvt>
#include <functional>
#include <locale>
#include <string>

RfidInterface::RfidInterface() {
	PQParams pq_params = {
		.ip_type = IP_TYPE_IPV4, // IP_TYPE_IPV(4|6)
		.port = 1001,            // default 1001
		.loop = 100              // default 100
	};
	sprintf(pq_params.ip_addr, "192.168.88.91");
	PQInit(pq_params);
}

RfidInterface::~RfidInterface() {}

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================
bool RfidInterface::RegisterHookCallback(
	RFID_HOOK_PACKET_CALLBACK fnPacketHookCallback, uint64_t usUserData) {
	if (fnPacketHookCallback == NULL)
		return false;
	HOOK_PACKET_CALLBACK new_cb;
	new_cb.fnCallback = fnPacketHookCallback;
	new_cb.usUserData = usUserData;

	std::lock_guard<std::mutex> lock(m_muxHookLock);
	unique_add_replace<std::list<HOOK_PACKET_CALLBACK>>(
		m_lstHookCallback, new_cb,
		[&new_cb](HOOK_PACKET_CALLBACK item) { return new_cb == item; });
	return true;
}

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================
void RfidInterface::UnRegisterHookCallback(
	RFID_HOOK_PACKET_CALLBACK fnPacketHookCallback, uint64_t usUserData) {
	std::lock_guard<std::mutex> lock(m_muxHookLock);

	HOOK_PACKET_CALLBACK new_cb;
	new_cb.fnCallback = fnPacketHookCallback;
	new_cb.usUserData = usUserData;

	auto iterator =
		find_if(m_lstHookCallback.begin(), m_lstHookCallback.end(),
			[&new_cb](HOOK_PACKET_CALLBACK item) { return new_cb == item; });
	if (iterator != m_lstHookCallback.end()) {
		m_lstHookCallback.erase(iterator);
	}
}

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================
bool RfidInterface::OnHookCallback(const void *lpPacket, int nSize) {
	std::lock_guard<std::mutex> lock(m_muxHookLock);
	for (auto hook : m_lstHookCallback) {
		hook.fnCallback((void *)lpPacket, nSize, hook.usUserData);
	}
	return true;
}

//------------------------------------------------------------------------------
// Async Packet Callback
//------------------------------------------------------------------------------
//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================
bool RfidInterface::RegisterAsyncCallback(
	RFID_ASYNC_PACKET_CALLBACK fnAsyncPacketCallback, uint64_t usUserData,
	unsigned int uiType) {
	if (fnAsyncPacketCallback == NULL)
		return false;
	ASYNC_PACKET_CALLBACK new_cb;
	new_cb.uiType = uiType;
	new_cb.fnCallback = fnAsyncPacketCallback;
	new_cb.usUserData = usUserData;

	std::lock_guard<std::mutex> lock(m_muxHookLock);
	unique_add_replace<std::list<ASYNC_PACKET_CALLBACK>>(
		m_lstAsyncCallback, new_cb,
		[&new_cb](ASYNC_PACKET_CALLBACK item) { return new_cb == item; });
	return true;
}

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================
void RfidInterface::UnRegisterAsyncCallback(
	RFID_ASYNC_PACKET_CALLBACK fnAsyncPacketCallback, uint64_t usUserData) {
	std::lock_guard<std::mutex> lock(m_muxAsyncLock);

	ASYNC_PACKET_CALLBACK new_cb;
	new_cb.fnCallback = fnAsyncPacketCallback;
	new_cb.usUserData = usUserData;

	std::list<ASYNC_PACKET_CALLBACK>::iterator iterator =
		find_if(m_lstAsyncCallback.begin(), m_lstAsyncCallback.end(),
			[&new_cb](ASYNC_PACKET_CALLBACK cb) { return new_cb == cb; });

	if (iterator != m_lstAsyncCallback.end()) {
		m_lstAsyncCallback.erase(iterator);
	}
}

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================
bool RfidInterface::OnAsyncCallback(uint64_t uiID, unsigned int uiType,
                                    const void *lpPacket, int nSize) {
	std::lock_guard<std::mutex> lock(m_muxAsyncLock);

	for (auto cb : m_lstAsyncCallback) {
		if ((cb.uiType == 0) || (cb.uiType == uiType))
			cb.fnCallback(uiID, uiType, (void *)lpPacket, nSize, cb.usUserData);
	}
	return true;
}

//------------------------------------------------------------------------------
// Heartbeat Packet Callback
//------------------------------------------------------------------------------
//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================
bool RfidInterface::RegisterHeartbeatCallback(
	RFID_HEARTBEAT_PACKET_CALLBACK fnHeartbeatPacketCallback,
	uint64_t usUserData) {
	if (fnHeartbeatPacketCallback == NULL)
		return false;

	HEARTBEAT_PACKET_CALLBACK new_cb;
	new_cb.uiType = 0;
	new_cb.fnCallback = fnHeartbeatPacketCallback;
	new_cb.usUserData = usUserData;

	std::lock_guard<std::mutex> lock(m_muxHeartbeatLock);

	unique_add_replace(
		m_lstHeartbeatCallback, new_cb,
		[&new_cb](HEARTBEAT_PACKET_CALLBACK cb) { return cb == new_cb; });
	return true;
}

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================
void RfidInterface::UnRegisterHeartbeatCallback(
	RFID_HEARTBEAT_PACKET_CALLBACK fnHeartbeatPacketCallback,
	uint64_t usUserData) {
	std::lock_guard<std::mutex> lock(m_muxHeartbeatLock);

	HEARTBEAT_PACKET_CALLBACK new_cb;
	new_cb.fnCallback = fnHeartbeatPacketCallback;
	new_cb.usUserData = usUserData;

	auto iterator =
		find_if(m_lstHeartbeatCallback.begin(), m_lstHeartbeatCallback.end(),
			[&new_cb](HEARTBEAT_PACKET_CALLBACK cb) { return cb == new_cb; });
	if (iterator != m_lstHeartbeatCallback.end())
		m_lstHeartbeatCallback.erase(iterator);
}

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================
bool RfidInterface::OnHeartbeatCallback(uint64_t uiID, unsigned int uiType,
                                        const void *lpPacket, int nSize) {
	std::lock_guard<std::mutex> lock(m_muxHeartbeatLock);
	for (auto cb : m_lstHeartbeatCallback) {
		cb.fnCallback(uiID, uiType, (void *)lpPacket, nSize, cb.usUserData);
	}
	return true;
}

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-10
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       :
// Remarks      :
//==============================================================================
int RfidInterface::Receive(unsigned int &uiPacketType, void *lpBuf, int nBufLen,
                           int nFlags) {
	int nRecv = m_objSocket.Receive(lpBuf, nBufLen, nFlags);

	// uiPacketType = 0;

	char szBuffer[MAX_RECV_BUFFER * 2];
	int nGenerateSize =
		GeneratePacket((unsigned char *)szBuffer, _countof(szBuffer),
			       uiPacketType, (unsigned char *)lpBuf, nRecv, false);
	OnHookCallback(szBuffer, nGenerateSize);

	return nRecv;
}

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-10
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       :
// Remarks      :
//==============================================================================
int RfidInterface::Send(unsigned int uiPacketType, const void *lpBuf,
                        int nBufLen, int nFlags) {
	char szBuffer[MAX_SEND_BUFFER * 2];
	m_uiSendCommand = uiPacketType;
	int nGenerateSize =
		GeneratePacket((unsigned char *)szBuffer, _countof(szBuffer),
			       uiPacketType, (unsigned char *)lpBuf, nBufLen, true);
	OnHookCallback(szBuffer, nGenerateSize);

	int nSend = m_objSocket.Send(lpBuf, nBufLen, nFlags);
	return nSend;
}

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================
bool RfidInterface::InitWinsock(WORD wVersion) {
	WSADATA WSAData = {0};
	if (WSAStartup(wVersion, &WSAData) != 0) {
		// Tell the user that we could not find a usable WinSock DLL.
		if (LOBYTE(WSAData.wVersion) != LOBYTE(wVersion) ||
		    HIBYTE(WSAData.wVersion) != HIBYTE(wVersion))
			printf("Incorrect winsock version\n");

		WSACleanup();
		return false;
	}
	return true;
}

bool RfidInterface::Open(SOCKET &nSocket) {
	// Open non-blocking socket
	SOCKET nFd = socket(AF_INET, SOCK_STREAM, 0);
	if (nFd == INVALID_SOCKET) {
		printf("Error creating socket, ec: %d\n", WSAGetLastError());
		return false;
	}

	unsigned long nNonBlocking = 1;
	if (ioctlsocket(nFd, FIONBIO, &nNonBlocking) == SOCKET_ERROR) {
		printf("Unable to set nonblocking mode, ec: %d\n", WSAGetLastError());
		closesocket(nFd);
		return false;
	}
	printf("Creating socket: %d\n", nFd);
	nSocket = nFd;
	return true;
}

bool RfidInterface::SetLinger(int nSocket) {
	// linger 設定影響了 socket 在做 TCP handshaking 時的 behavior，Winsock 的
	// linger 設定方式與傳統 UNIX 的語意 (semantic) 是不同的，請參考
	// MSDN。這裡的設定是 graceful shutdown + handshaking timeout 3
	// seconds。另外這裡列的是 Winsock 2 的語法 (syntax)，與 Winsock 1.1
	// 有些出入。
	linger oLinger;
	oLinger.l_onoff = 1;
	oLinger.l_linger = 3; // wait 3 seconds for TCP handshake
	if (setsockopt(nSocket, SOL_SOCKET, SO_LINGER, (char *)&oLinger,
		       sizeof(oLinger)) == SOCKET_ERROR) {
		printf("error setsockopt, ec: %d\n", WSAGetLastError());
		return false;
	}
	return true;
}

bool RfidInterface::SetNameResolution(int nSocket, int nPort) {
	// Winsock 最大的缺點就在於它的 asynchronous name resolution 只有
	// WSAAsyncGetHostByName() 這一系列的可以用，WSAAsync 系的 API 是透過 Window
	// Message 來做 notification，也因此你的 程式裡必須要有 message loop
	// 來處理。但是，對於 NT Service program 而言，通常是沒有 message loop
	// 的！也因此這限制了 asynchronous name resolution 的使用。底下是 blocking
	// name resolution 的 best practice：
#if 0
	char szHost[256];

	sockaddr_in oAddr;
	hostent* poHost = 0;
	memset((void*)&oAddr, 0, sizeof(oAddr));
	oAddr.sin_family = AF_INET;
	//unsigned long uIP = inet_addr(szHost);
#ifdef _WINSOCK_DEPRECATED_NO_WARNINGS
	sockAddr.sin_addr.s_addr = inet_addr(lpszAscii);
	unsigned long uIP = inet_addr(szHost);
#else
	unsigned long uIP;
	int nResult = inet_pton(AF_INET, szHost, &uIP); // IPv4
#endif

	if (uIP == INADDR_NONE)
	{
		poHost = gethostbyname(szHost);
		if (poHost != 0)
		{
			struct in_addr** pptr = (struct in_addr**)poHost->h_addr_list;
			oAddr.sin_addr = **pptr; // memberwise clone
		}
		else
		{
			printf("Invalid host name %s\n", szHost);
			closesocket(nSocket);
			return false;
		}
	}
	else
	{
		oAddr.sin_addr.s_addr = uIP;
	}
	oAddr.sin_port = htons(nPort);
#endif
	return true;
}

//==============================================================================
// Function     : Create
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-02
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] : UINT nSocketPort, int nSocketType, LPCTSTR lpszSocketAddress
//              :  A particular port to be used with the socket, or 0 if you
//              want MFC to select a port.
//         [in] : int nSocketType
//              : SOCK_STREAM or SOCK_DGRAM.
//         [in] : LPCTSTR lpszSocketAddress
//              : A pointer to a string containing the network address of the
//              connected socket, a dotted number such as "128.56.22.8". Passing
//              the NULL string for this parameter indicates the CSocket
//              instance should listen for client activity on all network
//              interfaces.
// Return       : Nonzero if the function is successful; otherwise 0, and a
// specific error code can be retrieved by calling GetLastError. Remarks      :
//==============================================================================
bool RfidInterface::Create(UINT nSocketPort, int nSocketType,
                           LPCTSTR lpszSocketAddress) {
	// int nServerPort = 1001;
	// int nClientPort = 0; // A particular port to be used with the socket, or 0
	// if you want MFC to select a port. int nSocketType = SOCK_STREAM; //
	// SOCK_STREAM or SOCK_DGRAM.
	// A pointer to a string containing the network address of the connected
	// socket, a dotted number such as "128.56.22.8". Passing the NULL string for
	// this parameter indicates the CSocket instance should listen for client
	// activity on all network interfaces.
	// LPCTSTR lpszSocketAddress = NULL;
	// LPCTSTR lpszClientSocketAddress = _T("192.168.263.28");
	// LPCTSTR lpszServerSocketAddress = _T("192.168.263.91");

	if (m_objSocket.Create(nSocketPort, nSocketType, lpszSocketAddress)) {
		// printf("Create socket is successful.");
		return true;
	}
	return false;
}

bool RfidInterface::Connect(LPCTSTR lpszHostAddress, UINT nHostPort) {
	if (m_objSocket.Connect(lpszHostAddress, nHostPort)) {
		int nSockAddrLen = sizeof(SOCKADDR_IN);
		m_objSocket.GetSockName((PSOCKADDR)&m_stLocalAddress, &nSockAddrLen);
		m_strHostAddress = lpszHostAddress;
		m_nHostPort = nHostPort;
		m_fConnected = true;

		// Keep Socket Address (Host Byte Order)
#ifdef _UNICODE
		CT2A strTemp(lpszHostAddress); // uses LPCTSTR conversion operator for
		// CString and CT2A constructor
		LPSTR lpszAscii = strTemp;
#else
		LPSTR lpszAscii = lpszHostAddress;
#endif
		// Keep Host Address (Host Byte Order)
		m_stHostAddress.sin_family = AF_INET;
#ifdef _WINSOCK_DEPRECATED_NO_WARNINGS
		m_stHostAddress.sin_addr.s_addr = inet_addr(lpszAscii);
#else
		int nResult =
			inet_pton(AF_INET, lpszAscii, &m_stHostAddress.sin_addr.s_addr); // IPv4
#endif
		m_stHostAddress.sin_port = nHostPort;
		// m_stHostAddress.sin_port = htons((u_short)nHostPort);

		return true;
	}
	return false;
}

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-10
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       :
// Remarks      :
//==============================================================================
bool RfidInterface::Connect(const SOCKADDR *lpSockAddr, int nSockAddrLen) {
	if (lpSockAddr == NULL)
		return false;
	if (m_objSocket.Connect(lpSockAddr, nSockAddrLen)) {
		int nSockAddrLen = sizeof(SOCKADDR_IN);
		m_objSocket.GetSockName((PSOCKADDR)&m_stLocalAddress, &nSockAddrLen);
		void *lpAddr;
		char *ipver;
		// char szBuffer[INET_ADDRSTRLEN];
		char szAddr[INET6_ADDRSTRLEN];

		// Keep Host Address (Host Byte Order)
		m_stHostAddress = *(struct sockaddr_in *)lpSockAddr;
		if (lpSockAddr->sa_family == AF_INET) {
			// IPv4
			struct sockaddr_in *ipv4 = (struct sockaddr_in *)lpSockAddr;
			lpAddr = &(ipv4->sin_addr);
			ipver = "IPv4";

		} else if (lpSockAddr->sa_family == AF_INET6) {
			struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)lpSockAddr;
			lpAddr = &(ipv6->sin6_addr);
			ipver = "IPv6";
		}
		PSOCKADDR_IN pstSockAddr = (PSOCKADDR_IN)lpSockAddr;
		inet_ntop(lpSockAddr->sa_family, lpAddr, szAddr, sizeof szAddr);
		m_strHostAddress = CA2T(szAddr);
		m_nHostPort = pstSockAddr->sin_port;
		m_fConnected = true;
		return true;
	}
	return false;
}

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-10
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       :
// Remarks      :
//==============================================================================
bool RfidInterface::Disconnect() {
	bool fResult = false;
	if (m_objSocket.m_hSocket) {
		m_objSocket.Close();
		m_strHostAddress.clear();
		m_nHostPort = 0;
		m_fConnected = false;
		fResult = true;
	}
	return fResult;
}

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-10
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       :
// Remarks      :
//==============================================================================
bool RfidInterface::IsConnected() {
	if (m_objSocket.m_hSocket) {
		int nOptionName = 0; // The socket option for which the value is to be set.
		void *lpOptionValue = NULL; // A pointer to the buffer in which the value
		// for the requested option is supplied.
		int nOptionLen = 0; // The size of the lpOptionValue buffer in bytes.
		int nLevel =
			SOL_SOCKET; // The level at which the option is defined; the only
		// supported levels are SOL_SOCKET and IPPROTO_TCP.
		// return (SOCKET_ERROR != setsockopt(m_hSocket, nLevel, nOptionName,
		// (LPCSTR)lpOptionValue, nOptionLen));
		// if (m_objSocket.GetSockOpt(nOptionName, lpOptionValue, &nOptionLen,
		// nLevel))
		{

			// return true;
		}
		return m_fConnected;
	}
	return false;
}

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-10
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       :
// Remarks      :
//==============================================================================
bool RfidInterface::SockAddrToString(const SOCKADDR_IN stSockAddr,
                                     TString &strHostAddress, UINT &nHostPort) {
	void *lpAddr;
	char *ipver;
	// char szBuffer[INET_ADDRSTRLEN];
	char szAddr[INET6_ADDRSTRLEN];
	if (stSockAddr.sin_family == AF_INET) {
		// IPv4
		struct sockaddr_in *ipv4 = (struct sockaddr_in *)&stSockAddr;
		lpAddr = &(ipv4->sin_addr);
		ipver = "IPv4";
	} else if (stSockAddr.sin_family == AF_INET6) {
		struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)&stSockAddr;
		lpAddr = &(ipv6->sin6_addr);
		ipver = "IPv6";
	}
	PSOCKADDR_IN pstSockAddr = (PSOCKADDR_IN)&stSockAddr;
	inet_ntop(pstSockAddr->sin_family, lpAddr, szAddr, sizeof szAddr);
	strHostAddress = CA2T(szAddr);
	nHostPort = ntohs(pstSockAddr->sin_port);
	return true;
}

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-10
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       :
// Remarks      :
//==============================================================================
bool RfidInterface::StringToSockAddr(LPCTSTR lpszHostAddress, UINT nHostPort,
                                     SOCKADDR_IN &stSockAddr) {
	if (lpszHostAddress == NULL)
		return false;
#ifdef _UNICODE
	CT2A strTemp(lpszHostAddress); // uses LPCTSTR conversion operator for CString
	// and CT2A constructor
	LPSTR lpszAscii = strTemp;
#else
	LPSTR lpszAscii = lpszHostAddress;
#endif

	stSockAddr.sin_family = AF_INET;
#ifdef _WINSOCK_DEPRECATED_NO_WARNINGS
	sockAddr.sin_addr.s_addr = inet_addr(lpszAscii);
#else
	int nResult =
		inet_pton(AF_INET, lpszAscii, &stSockAddr.sin_addr.s_addr); // IPv4
#endif
	stSockAddr.sin_port = htons((u_short)nHostPort);
	return true;
}

bool RfidInterface::GetHostAddress(TString &strHostAddress, UINT &nHostPort) {
	strHostAddress = m_strHostAddress;
	nHostPort = m_nHostPort;
	return true;
}

//==============================================================================
// Function     : GetVersion
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================
bool RfidInterface::GetVersion(RFID_READER_VERSION &stVersion) {
	char szSend[MAX_SEND_BUFFER];
	char szReceive[MAX_RECV_BUFFER];
	unsigned int uiRecvCommand = 0; // The received command
	// sprintf(szSend, "<LF>V<CR>"); <= 0x0A 0x56 0x0D
	sprintf_s(szSend, _countof(szSend), "\n%s\r", "V"); // 0x0A 0x56 0x0D
	Send(RF_PT_REQ_GET_FIRMWARE_VERSION, szSend, strlen(szSend));
	int nRecv = Receive(uiRecvCommand, szReceive, _countof(szReceive));
	if (nRecv > 0) {
		szReceive[nRecv] = 0; // Set null-string
		return ParseVersion(szReceive, nRecv, stVersion);
	}
	return false;
}

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================
bool RfidInterface::GetReaderID(TString &strID) {
	char szSend[MAX_SEND_BUFFER];
	char szReceive[MAX_RECV_BUFFER];
	unsigned int uiRecvCommand = 0; // The received command
	// sprintf(szBuffer, "<LF>S<CR>");
	sprintf_s(szSend, _countof(szSend), "\n%s\r", "S");
	// int nSize = strlen(szSend);
	Send(RF_PT_REQ_GET_READER_ID, szSend, strlen(szSend));

	int nRecv = Receive(uiRecvCommand, szReceive, _countof(szReceive));
	if (nRecv > 0) {
		szReceive[nRecv] = 0; // Set null-string
		return ParseReaderID(szReceive, nRecv, strID);
	}
	return false;
}

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      : Setting regulation, reader no return message and will
// re-startup.
//==============================================================================
bool RfidInterface::SetRegulation(RFID_REGULATION emRegulation) {
	// Send: <LF>N5,02<CR>  <== 0x0A 0x40 0x4E 0x35 0x2C 0x30 0x32 0x0D
	// Send: <LF>@N5,02<CR>  <== 0x0A 0x40 0x4E 0x35 0x2C 0x30 0x32 0x0D
	// Recv: <LF> @2020/11/06 12:42:32.850-Antennea1-N0A<CR><LF>
	char szSend[MAX_SEND_BUFFER];
	char szReceive[MAX_RECV_BUFFER];
	unsigned int uiRecvCommand = 0; // The received command
	bool fResult = false;

	// Method 1
	// sprintf_s(szBuffer, _countof(szBuffer), "\n%s,%02X\r", "@N1", nPower); //
	// 0x0A [CMD] 0x0D
	// Method 2
	sprintf_s(szSend, _countof(szSend), "\n%s,%02d\r", "N5",
		  emRegulation); // 0x0A [CMD] 0x0D
	// int nSize = strlen(szSend);
	Send(RF_PT_REQ_SET_REGULATION, szSend, strlen(szSend));
	// Setting regulation, reader no return message and will re - startup.
#if 1
	int nRecv = Receive(uiRecvCommand, szReceive, _countof(szReceive));
	if (nRecv > 0) {
		szReceive[nRecv] = 0; // Set null-string
		int nResult = 0;
		if (ParseSetRegulation(szReceive, nRecv, &nResult)) {
			if (emRegulation == (RFID_REGULATION)nResult)
				fResult = true;
		}
	}
#else
	// Setting regulation, reader no return message and will re - startup.
	fResult = true;
#endif

	return fResult;
}

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================
bool RfidInterface::GetRegulation(RFID_REGULATION &emRegulation) {
	// Send: <LF>N4,00<CR>  <== 0x0A 0x40 0x4E 0x34 0x2C 0x30 0x30 0x0d
	// Send: <LF>@N4,00<CR>  <== 0x0A 0x40 0x4E 0x34 0x2C 0x30 0x30 0x0d
	// Recv: <LF> @2020/11/06 12:42:32.850-Antennea1-N0A<CR><LF>
	char szSend[MAX_SEND_BUFFER];
	char szReceive[MAX_RECV_BUFFER];
	unsigned int uiRecvCommand = 0; // The received command
	bool fResult = false;
	emRegulation = (RFID_REGULATION)REGULATION_UNKNOWN;

	// Method 1
	// sprintf_s(szBuffer, _countof(szBuffer), "\n%s,%02X\r", "@N4", nPower); //
	// 0x0A [CMD] 0x0D
	// Method 2
	sprintf_s(szSend, _countof(szSend), "\n%s,%02d\r", "N4",
		  emRegulation); // 0x0A [CMD] 0x0D
	// int nSize = strlen(szSend);
	Send(RF_PT_REQ_GET_REGULATION, szSend, strlen(szSend));

	int nRecv = Receive(uiRecvCommand, szReceive, _countof(szReceive));
	if (nRecv > 0) {
		szReceive[nRecv] = 0; // Set null-string
		if (ParseGetRegulation(szReceive, nRecv, emRegulation)) {
			fResult = true;
		}
	}
	return fResult;
}

//==============================================================================
// Function     : SetPower
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] : nPower(dbm)
//              :  Set the reader power to X.
//         [in] :
//              :
//         [in] : pnResult
//              :  0: 設定成功, -2: Power超過可設定範圍
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//              : Reader Version | Power(dbm)
//              : VC2	         | - 2~18 dbm
//              : VD2            | - 2~25 dbm
//              : VD3            | 0~27 dbm
//              : VD4            | 2~29 dbm
//              : V6(TBD)        |- 2~30 dbm
//==============================================================================
bool RfidInterface::SetPower(int nPower, int *pnResult) {
	// Send: <LF>N1,0A<CR>  <== 0x0A 0x40 0x4E 0x31 0x2C 0x30 0x41 0x0d
	// Send: <LF>@N1,0A<CR>  <== 0x0A 0x40 0x4E 0x31 0x2C 0x30 0x41 0x0d
	// Recv: <LF> @2020/11/06 12:42:32.850-Antennea1-N0A<CR><LF>
	char szSend[MAX_SEND_BUFFER];
	char szReceive[MAX_RECV_BUFFER];
	unsigned int uiRecvCommand = 0; // The received command
	bool fResult = false;

	// Method 1
	// sprintf_s(szBuffer, _countof(szBuffer), "\n%s,%02X\r", "@N1", nPower); //
	// 0x0A [CMD] 0x0D
	// Method 2
	sprintf_s(szSend, _countof(szSend), "\n%s,%02X\r", "N1",
		  nPower); // 0x0A [CMD] 0x0D
	// int nSize = strlen(szSend);
	Send(RF_PT_REQ_SET_POWER_LEVEL, szSend, strlen(szSend));

	int nRecv = Receive(uiRecvCommand, szReceive, _countof(szReceive));
	if (nRecv > 0) {
		szReceive[nRecv] = 0; // Set null-string
		if (ParseSetPower(szReceive, nRecv, pnResult)) {
			if (nPower == *pnResult)
				fResult = true;
		}
	}
	return fResult;
}

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================
bool RfidInterface::GetPower(int &nPower) {
	// Send: <LF>N0,00<CR>  <== 0x0A 0x40 0x4E 0x31 0x2C 0x30 0x30 0x0d
	// Send: <LF>@N0,00<CR>  <== 0x0A 0x40 0x4E 0x31 0x2C 0x30 0x30 0x0d
	// Recv: <LF> @2020/11/06 12:42:32.850-Antennea1-N0A<CR><LF>
	char szSend[MAX_SEND_BUFFER];
	char szReceive[MAX_RECV_BUFFER];
	unsigned int uiRecvCommand = 0; // The received command
	bool fResult = false;
	nPower = 0;

	// Method 1
	// sprintf_s(szBuffer, _countof(szBuffer), "\n%s,%02X\r", "@N0", nPower); //
	// 0x0A [CMD] 0x0D
	// Method 2
	sprintf_s(szSend, _countof(szSend), "\n%s,%02X\r", "N0",
		  nPower); // 0x0A [CMD] 0x0D
	// int nSize = strlen(szSend);
	Send(RF_PT_REQ_GET_POWER_LEVEL, szSend, strlen(szSend));

	int nRecv = Receive(uiRecvCommand, szReceive, _countof(szReceive));
	if (nRecv > 0) {
		szReceive[nRecv] = 0; // Set null-string
		if (ParseGetPower(szReceive, nRecv, nPower)) {
			fResult = true;
		}
	}
	return fResult;
}

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] : pnResult
//              :  0: 設定成功, -3: 為超過天線可設定Port號
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================
bool RfidInterface::SetSingleAntenna(unsigned int uiAntenna, bool fHub,
                                     int *pnResult) {
	// No Hub: 1~4
	// Hub: 01~32

	// No hub
	// Send: <LF>@Antenna[1~4]<CR>  <== 0x0A 0x40 0x4E 0x31 0x2C 0x30 0x41 0x0d
	// Recv: <LF> @Antennea[1~4]<CR><LF>
	//
	// Hub
	// Send: <LF>@Antenna[01~32]<CR>  <== 0x0A 0x40 0x4E 0x31 0x2C 0x30 0x41 0x0d
	// Recv: <LF> @Antennea[01~32]<CR><LF>
	char szSend[MAX_SEND_BUFFER];
	char szReceive[MAX_RECV_BUFFER];
	unsigned int uiRecvCommand = 0; // The received command
	bool fResult = false;

	if (fHub == true)
		// sprintf_s(szBuffer, _countof(szBuffer), "\n%s,%02d\r", "@Antenna",
		// nAntenna); // 0x0A [CMD] 0x0D
		sprintf_s(szSend, _countof(szSend), "\n%s%02d\r", "@Antenna",
			  uiAntenna); // 0x0A [CMD] 0x0D
	else
		// sprintf_s(szBuffer, _countof(szBuffer), "\n%s,%d\r", "@Antenna",
		// nAntenna); // 0x0A [CMD] 0x0D
		sprintf_s(szSend, _countof(szSend), "\n%s%d\r", "@Antenna",
			  uiAntenna); // 0x0A [CMD] 0x0D
	// int nSize = strlen(szSend);
	Send(RF_PT_REQ_SET_SIGNLE_ANTENNA, szSend, strlen(szSend));

	int nRecv = Receive(uiRecvCommand, szReceive, _countof(szReceive));
	if (nRecv > 0) {
		unsigned int uiReturnAntenna = 0;
		bool fReturnHub = false;
		szReceive[nRecv] = 0; // Set null-string
		if (ParseSetSingleAntenna(szReceive, nRecv, &uiReturnAntenna,
					  &fReturnHub)) {
			if (uiAntenna == *pnResult)
				fResult = true;
		}
	}
	return fResult;
}

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================
bool RfidInterface::GetSingleAntenna(unsigned int &uiAntenna, bool &fHub) {
	char szSend[MAX_SEND_BUFFER];
	char szReceive[MAX_RECV_BUFFER];
	unsigned int uiRecvCommand = 0; // The received command
	bool fResult = false;

	sprintf_s(szSend, _countof(szSend), "\n%s\r", "@Antenna"); // 0x0A [CMD] 0x0D
	// int nSize = strlen(szSend);
	Send(RF_PT_REQ_GET_SIGNLE_ANTENNA, szSend, strlen(szSend));

	int nRecv = Receive(uiRecvCommand, szReceive, _countof(szReceive));
	if (nRecv > 0) {
		szReceive[nRecv] = 0; // Set null-string
		if (ParseGetSingleAntenna(szReceive, nRecv, uiAntenna, fHub)) {
			fResult = true;
		}
	}
	return fResult;
}

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] : int uiAntennas
//              :     Antennas
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================
bool RfidInterface::SetLoopAntenna(unsigned int uiAntennas) {
	// Antenna|  1 |  2 |  3 |  4 |
	// -------+----+----+----+----+
	//       1| 01 | 09 | 17 | 25 |
	//       2| 02 | 10 | 18 | 26 |
	//       3| 03 | 11 | 19 | 27 |
	//       4| 04 | 12 | 20 | 28 |
	//       5| 05 | 13 | 21 | 29 |
	//       6| 06 | 14 | 22 | 30 |
	//       7| 07 | 15 | 23 | 31 |
	//       8| 08 | 16 | 24 | 32 |
	//
	// Ex. Set Loop Antenna: 1,2,9,17,18,19,31,32
	// int[] bAnt = new int[4];
	// bAnt[0] = 1 + 2;		// Antenna1 + Antenna2
	// bAnt[1] = 1;			// Antenna9
	// bAnt[2] = 1 + 2 + 4;	// Antenna17 + Antenna18 + Antenna19
	// bAnt[3] = 64 + 128;  // Antenna31 + Antenna32

	// Ex. Set Loop Antenna: 9,25
	//
	// Send: <LF>@LoopAntenna01000100<CR>  <== 0x0A 0x40 0x4C 0x6F 0x6F 0x70 0x41
	// 0x6e 0x74 0x65 0x6E 0x6E 0x61 0x30 0x31 0x30 0x30 0x30 0x31 0x30 0x30 0x30
	// 0x0d Recv: <LF>@LoopAntennea01000100<CR><LF> <== 0x0A 0x40 0x4C 0x6F 0x6F
	// 0x70 0x41 0x6e 0x74 0x65 0x6E 0x6E 0x61 0x30 0x31 0x30 0x30 0x30 0x31 0x30
	// 0x30 0x30 0x0d 0x0a

	// Ex.
	// nAntennas = RF_ANTENNA_01 + RF_ANTENNA_02 + RF_ANTENNA_11 + RF_ANTENNA_12 +
	// RF_ANTENNA_16 + 	RF_ANTENNA_17 + RF_ANTENNA_21 + RF_ANTENNA_22 +
	// RF_ANTENNA_25 + RF_ANTENNA_31 + RF_ANTENNA_32;

	char szSend[MAX_SEND_BUFFER];
	char szReceive[MAX_RECV_BUFFER];
	unsigned int uiRecvCommand = 0; // The received command
	bool fResult = false;
	sprintf_s(szSend, _countof(szSend), "\n%s%08X\r", "@LoopAntenna",
		  uiAntennas); // 0x0A [CMD] 0x0D

	// int nSize = strlen(szSend);
	Send(RF_PT_REQ_SET_LOOP_ANTENNA, szSend, strlen(szSend));

	int nRecv = Receive(uiRecvCommand, szReceive, _countof(szReceive));
	if (nRecv > 0) {
		unsigned int uiResult = 0;
		szReceive[nRecv] = 0; // Set null-string
		if (ParseSetLoopAntenna(szReceive, nRecv, &uiResult)) {
			if (uiAntennas == uiResult)
				fResult = true;
		}
	}
	return fResult;
}

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] : int[] GetLoopAntenna()
//              :  解析後得知使用中天線
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================
bool RfidInterface::GetLoopAntenna(unsigned int &uiAntennas) {
	// Get loop antenna
	// Antenna|  1 |  2 |  3 |  4 |
	// -------+----+----+----+----+
	//       1| 01 | 09 | 17 | 25 |
	//       2| 02 | 10 | 18 | 26 |
	//       3| 03 | 11 | 19 | 27 |
	//       4| 04 | 12 | 20 | 28 |
	//       5| 05 | 13 | 21 | 29 |
	//       6| 06 | 14 | 22 | 30 |
	//       7| 07 | 15 | 23 | 31 |
	//       8| 08 | 16 | 24 | 32 |
	//

	// Ex. Set Loop Antenna: 1,2,11,12,16,17,21,22,25,31,32
	// Antenna[0] = 1 + 2;			// 0x03: Antenna1 + Antenna2
	// bAnt[1] = 4 + 8 + 128;	// 0x8C: Antenna11 + Antenna12 + Antenna16
	// bAnt[2] = 1 + 32 + 64;	// 0x31: Antenna17 + Antenna21 + Antenna22
	// bAnt[3] = 1+ 64 + 128;	// 0xC1: Antenna25 + Antenna31 + Antenna32
	//
	// Send: <LF>@LoopAntenna<CR>  <== 0x0A 0x40 0x4C 0x6F 0x6F 0x70 0x41 0x6e
	// 0x74 0x65 0x6E 0x6E 0x61 0x0D Recv: <LF>@LoopAntennaC1318C03<CR><LF> <==
	// 0x0A 0x40 0x4C 0x6F 0x6F 0x70 0x41 0x6e 0x74 0x65 0x6E 0x6E 0x61
	//						 C1318C03             0x43 0x31
	//0x33 0x31 0x38 0x43 0x30 0x33 0x0D 0x0A
	// Recv: <LF>@LoopAntennaC1318C03<CR><LF> <== 0x0A 0x40 0x4C 0x6F 0x6F 0x70
	// 0x41 0x6e 0x74 0x65 0x6E 0x6E 0x61
	//                       c1318c03             0x63 0x31 0x33 0x31 0x38 0x63
	//                       0x30 0x33 0x0D
	// Ex.
	// nAntennas = RF_ANTENNA_01 + RF_ANTENNA_02 + RF_ANTENNA_11 + RF_ANTENNA_12 +
	// RF_ANTENNA_16 + 	RF_ANTENNA_17 + RF_ANTENNA_21 + RF_ANTENNA_22 +
	// RF_ANTENNA_25 + RF_ANTENNA_31 + RF_ANTENNA_32;

	char szSend[MAX_SEND_BUFFER];
	char szReceive[MAX_RECV_BUFFER];
	unsigned int uiRecvCommand = 0; // The received command
	bool fResult = false;
	sprintf_s(szSend, _countof(szSend), "\n%s\r",
		  "@LoopAntenna"); // 0x0A [CMD] 0x0D

	// int szSend = strlen(szSend);
	Send(RF_PT_REQ_GET_LOOP_ANTENNA, szSend, strlen(szSend));

	int nRecv = Receive(uiRecvCommand, szReceive, _countof(szReceive));
	if (nRecv > 0) {
		unsigned int uiResult = 0;
		szReceive[nRecv] = 0; // Set null-string
		if (ParseGetLoopAntenna(szReceive, nRecv, uiAntennas)) {
			fResult = true;
		}
	}
	return fResult;
}

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] : uiMilliseconds
//              :  設定天線讀取駐留時間。
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================
bool RfidInterface::SetLoopTime(unsigned int uiMilliseconds) {
	char szSend[MAX_SEND_BUFFER];
	char szReceive[MAX_RECV_BUFFER];
	unsigned int uiRecvCommand = 0; // The received command
	bool fResult = false;
	sprintf_s(szSend, _countof(szSend), "\n%s%d\r", "@LoopTime",
		  uiMilliseconds); // 0x0A [CMD] 0x0D

	// int nSize = strlen(szSend);
	Send(RF_PT_REQ_SET_LOOP_TIME, szSend, strlen(szSend));

	int nRecv = Receive(uiRecvCommand, szReceive, _countof(szReceive));
	if (nRecv > 0) {
		unsigned int uiResult = 0;
		szReceive[nRecv] = 0; // Set null-string
		if (ParseSetLoopTime(szReceive, nRecv, &uiResult)) {
			if (uiMilliseconds == uiResult)
				fResult = true;
		}
	}
	return fResult;
}

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :  取得目前Reader所設定的天線駐留時間。
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================
bool RfidInterface::GetLoopTime(unsigned int &uiMilliseconds) {
	char szSend[MAX_SEND_BUFFER];
	char szReceive[MAX_RECV_BUFFER];
	unsigned int uiRecvCommand = 0; // The received command
	bool fResult = false;
	sprintf_s(szSend, _countof(szSend), "\n%s\r", "@LoopTime"); // 0x0A [CMD] 0x0D

	// int nSize = strlen(szSend);
	Send(RF_PT_REQ_GET_LOOP_TIME, szSend, strlen(szSend));

	int nRecv = Receive(uiRecvCommand, szReceive, _countof(szReceive));
	if (nRecv > 0) {
		unsigned int uiResult = 0;
		szReceive[nRecv] = 0; // Set null-string
		if (ParseGetLoopTime(szReceive, nRecv, uiMilliseconds)) {
			fResult = true;
		}
	}
	return fResult;
}

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================
bool RfidInterface::SetSystemTime() {
	// Bug
	// Send: <LF>@SETDATE20011109202806<CR>  <== 0x0A 0x40 0x53 045 054 044 041
	// 054 045 0x32 0x30 0x30 0x31 0x31 0x31 0x31 0x30 0x39 0x32 0x30 0x32 0x38
	// 0x30 0x36 0x0D
	//                     ^^ Bug?
	// Send: <LF>@SETDATE20201109202806<CR>  <== 0x0A 0x40 0x53 045 054 044 041
	// 054 045 0x32 0x30 0x32 0x30 0x31 0x31 0x31 0x30 0x39 0x32 0x30 0x32 0x38
	// 0x30 0x36 0x0D Recv: <LF>@2020/11/09 20:28:06.000<CR><LF> <== 0x0A 0x40
	// 0x32 0x30 0x32 0x30 0x2F 0x31 0x31 0x2F
	//                                                0x30 0x39 0x20 0x32 0x30
	//                                                0x3A 0x32 0x38 0x3A 0x30
	//                                                0x36 0x2E 0x30 0x30 0x30
	//                                                0x0D 0x0A

	// reader.Connect("192.168.1.91");
	// Set Reader time
	// yyyyMMddHHmmss
	// 1902-03-12 09:35:00
	// int setResult = reader.SetTime("19020312093500");
	struct tm stTime;
	__time64_t long_time;
	_time64(&long_time); // Get time as 64-bit integer.
	// Convert to local time.
	_localtime64_s(&stTime, &long_time);
	stTime.tm_isdst = 0; // ** tm_isdst ** : Millisecond after second  (0 - 999).

#if 1
	return SetTime(stTime);
#else
	char szBuffer[MAX_SEND_BUFFER];
	char szReceive[MAX_RECV_BUFFER];
	bool fResult = false;
	sprintf_s(szBuffer, _countof(szBuffer),
		  "\n@SETDATE%04d%02d%02d%02d%02d%02d\r", stTime.tm_year + 1900,
		  stTime.tm_mon + 1, stTime.tm_mday, stTime.tm_hour, stTime.tm_min,
		  stTime.tm_sec); // 0x0A [CMD] 0x0D

	int nSize = strlen(szBuffer);

	Send(szBuffer, strlen(szBuffer));

	int nRecv = Receive(szReceive, _countof(szReceive));
	if (nRecv > 0) {
		// unsigned int uiResult = 0;
		struct tm stResponseTime;
		szReceive[nRecv] = 0; // Set null-string
		if (ParseSetTime(szReceive, nRecv, &stResponseTime)) {
			if ((stTime.tm_year == stResponseTime.tm_year) &&
			    (stTime.tm_mon == stResponseTime.tm_mon) &&
			    (stTime.tm_mday == stResponseTime.tm_mday) &&
			    (stTime.tm_hour == stResponseTime.tm_hour) &&
			    (stTime.tm_min == stResponseTime.tm_min) &&
			    (stTime.tm_sec == stResponseTime.tm_sec))
				fResult = true;
		}
	}
	return fResult;
#endif
}

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//              : tm_sec	Seconds after minute (0 - 59).
//              : tm_min	Minutes after hour(0 - 59).
//              : tm_hour	Hours since midnight(0 - 23).
//              : tm_mday	Day of month(1 - 31).
//              : tm_mon	Month(0 - 11; January = 0).
//              : tm_year	Year(current year minus 1900).
//              : tm_wday	Day of week(0 - 6; Sunday = 0).
//              : tm_yday	Day of year(0 - 365; January 1 = 0).
//              : tm_isdst	Positive value if daylight saving time is in
//              effect; 0 if daylight saving time is not in effect; negative
//              value if status of daylight saving time is unknown.
//
//              : int tm_sec;   // seconds after the minute - [0, 60] including
//              leap second : int tm_min;   // minutes after the hour - [0, 59]
//              : int tm_hour;  // hours since midnight - [0, 23]
//              : int tm_mday;  // day of the month - [1, 31]
//              : int tm_mon;   // months since January - [0, 11]
//              : int tm_year;  // years since 1900
//              : int tm_wday;  // days since Sunday - [0, 6]
//              : int tm_yday;  // days since January 1 - [0, 365]
//              : int tm_isdst; // daylight savings time flag
//==============================================================================
bool RfidInterface::SetTime(struct tm stTime) {
	// Bug
	// Send: <LF>@SETDATE20011109202806<CR>  <== 0x0A 0x40 0x53 0x45 0x54 0x44
	// 0x41 0x54 0x45 0x32 0x30 0x30 0x31 0x31 0x31 0x31 0x30 0x39 0x32 0x30 0x32
	// 0x38 0x30 0x36 0x0D
	//                     ^^ Bug?
	// Send: <LF>@SETDATE20201109202806<CR>  <== 0x0A 0x40 0x53 0x45 0x54 0x44
	// 0x41 0x54 0x45 0x32 0x30 0x32 0x30 0x31 0x31 0x31 0x30 0x39 0x32 0x30 0x32
	// 0x38 0x30 0x36 0x0D Recv: <LF>@2020/11/09 20:28:06.000<CR><LF> <== 0x0A
	// 0x40 0x32 0x30 0x32 0x30 0x2F 0x31 0x31 0x2F
	//                                                0x30 0x39 0x20 0x32 0x30
	//                                                0x3A 0x32 0x38 0x3A 0x30
	//                                                0x36 0x2E 0x30 0x30 0x30
	//                                                0x0D 0x0A

	// reader.Connect("192.168.1.91");
	// Set Reader time
	// yyyyMMddHHmmss
	// 1902-03-12 09:35:00
	// int setResult = reader.SetTime("19020312093500");
	char szSend[MAX_SEND_BUFFER];
	char szReceive[MAX_RECV_BUFFER];
	unsigned int uiRecvCommand = 0; // The received command
	bool fResult = false;
	sprintf_s(szSend, _countof(szSend), "\n@SETDATE%04d%02d%02d%02d%02d%02d\r",
		  stTime.tm_year + 1900, stTime.tm_mon + 1, stTime.tm_mday,
		  stTime.tm_hour, stTime.tm_min, stTime.tm_sec); // 0x0A [CMD] 0x0D

	// int nSize = strlen(szSend);
	Send(RF_PT_REQ_SET_DATE_TIME, szSend, strlen(szSend));

	int nRecv = Receive(uiRecvCommand, szReceive, _countof(szReceive));
	if (nRecv > 0) {
		// unsigned int uiResult = 0;
		struct tm stResponseTime;
		szReceive[nRecv] = 0; // Set null-string
		if (ParseSetTime(szReceive, nRecv, &stResponseTime)) {
			if ((stTime.tm_year == stResponseTime.tm_year) &&
			    (stTime.tm_mon == stResponseTime.tm_mon) &&
			    (stTime.tm_mday == stResponseTime.tm_mday) &&
			    (stTime.tm_hour == stResponseTime.tm_hour) &&
			    (stTime.tm_min == stResponseTime.tm_min) &&
			    (stTime.tm_sec == stResponseTime.tm_sec))
				fResult = true;
		}
	}
	return fResult;
}

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :  (yyyy/MM/dd HH:mm:ss:fff)
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//              : tm_sec	Seconds after minute (0 - 59).
//              : tm_min	Minutes after hour(0 - 59).
//              : tm_hour	Hours since midnight(0 - 23).
//              : tm_mday	Day of month(1 - 31).
//              : tm_mon	Month(0 - 11; January = 0).
//              : tm_year	Year(current year minus 1900).
//              : tm_wday	Day of week(0 - 6; Sunday = 0).
//              : tm_yday	Day of year(0 - 365; January 1 = 0).
//              : tm_isdst	Positive value if daylight saving time is in
//              effect; 0 if daylight saving time is not in effect; negative
//              value if status of daylight saving time is unknown.
//
//              : int tm_sec;   // seconds after the minute - [0, 60] including
//              leap second : int tm_min;   // minutes after the hour - [0, 59]
//              : int tm_hour;  // hours since midnight - [0, 23]
//              : int tm_mday;  // day of the month - [1, 31]
//              : int tm_mon;   // months since January - [0, 11]
//              : int tm_year;  // years since 1900
//              : int tm_wday;  // days since Sunday - [0, 6]
//              : int tm_yday;  // days since January 1 - [0, 365]
//              : int tm_isdst; // daylight savings time flag
//==============================================================================
bool RfidInterface::GetTime(struct tm &stTime) {
	// Send: <LF>@SETDATE<CR>  <== 0x0A 0x40 0x53 0x45 0x54 0x44 0x41 0x54 0x45
	// 0x0D Recv: <LF>@2020/11/09 20:46:57.374<CR><LF> <== 0x0A 0x40 0x32 0x30
	// 0x32 0x30 0x2F 0x31 0x31 0x2F
	//                                                0x30 0x39 0x20 0x32 0x30
	//                                                0x3A 0x34 0x36 0x3A 0x35
	//                                                0x37 0x2E 0x33 0x37 0x34
	//                                                0x0D 0x0A

	char szSend[MAX_SEND_BUFFER];
	char szReceive[MAX_RECV_BUFFER];
	unsigned int uiRecvCommand = 0; // The received command
	bool fResult = false;
	sprintf_s(szSend, _countof(szSend), "\n%s\r", "@SETDATE"); // 0x0A [CMD] 0x0D

	// int nSize = strlen(szSend);
	Send(RF_PT_REQ_GET_DATE_TIME, szSend, strlen(szSend));

	// @2020/11/09 20:46:57.374
	int nRecv = Receive(uiRecvCommand, szReceive, _countof(szReceive));
	if (nRecv > 0) {
		unsigned int uiResult = 0;
		szReceive[nRecv] = 0; // Set null-string
		if (ParseGetTime(szReceive, nRecv, stTime)) {
			fResult = true;
		}
	}
	return fResult;
}

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================
bool RfidInterface::ReadBank(RFID_BANK_TYPE emType, int nStart, int nLength) {
	return false;
}

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] : <bank> memory bank
//              :     0 : reserved
//              :     1 : EPC
//              :     2 : TID
//              :     3 : USER
//         [in] : <address> start address
//              :     0 ~ 3FFF
//         [in] : <length> read word length (1 Word = 2 Bytes)
//              :     1 ~ 20 (R300A) <= AL-510
//              :     1 ~ 1E (R300T)
//        [out] : puiErrorCode
//              :     Reference error code statement
//              : 0x00000000: ERROR_RFID_SUCCESS
//              :     ...
//              : 0x000000FF: ERROR_RFID_NON_SPECIFIC
// Return       : True if the function is successful; otherwise false.
// Remarks      :
// -----------------------------------------------------------------------------
// Reader
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] : <bank> memory bank
//              :     0 : reserved
//              :     1 : EPC
//              :     2 : TID
//              :     3 : USER
//         [in] : <address> start address
//              :     0 ~ 3FFF
//         [in] : <length> read word length (1 Word = 2 Bytes)
//              :     1 ~ 20 (R300A) <= AL-510
//              :     1 ~ 1E (R300T)
// Response     : <none or read data>
//              :     none: no tag in RF field
//              : <Error code>
//              :     0 : other error
//              :     3 : memory overrun
//              :     4 : memory locked
//              :     B : Insufficient power
//              :     F : Non - specific error
//==============================================================================
bool RfidInterface::ReadBank(RFID_MEMORY_BANK emBank,
                             unsigned int uiStartAddress,
                             unsigned int uiWordLength,
                             RFID_TAG_DATA &stTagData,
                             unsigned int *puiErrorCode) {
	// Send: <LF>R3,0,20<CR>  <== 0x0A 0x52 0x31 0x2C 0x32 0x2C 0x36 0x0D
	// Recv: <LF>@2020/11/10 16:26:39.338-Antenna2-R<CR><LF> <== 0x0A 0x40 0x32 30
	// 32 30 2f 31 31 2f
	//                                                   31 30 20 31 36 3a 32 36
	//                                                   3a 33 39 2e 33 33 38 2d
	//                                                   41 6e 74 65 6e 6e 61 32
	//                                                   2d 52 0d 0a

	char szSend[MAX_SEND_BUFFER];
	char szReceive[MAX_RECV_BUFFER];
	unsigned int uiRecvCommand = 0; // The received command
	bool fResult = false;
	if (emBank == RFID_MB_NONE)
		return fResult;

	if ((uiStartAddress <= 0) || (uiStartAddress > MAX_MEMORY_BANK_ADDRESS))
		return fResult;
	if ((uiWordLength <= 0) || (uiWordLength > MAX_MEMORY_BANK_LENGTH))
		return fResult;

	sprintf_s(szSend, _countof(szSend), "\n%c%d,%d,%d\r", CMD_RFID_READ_BANK,
		  emBank, uiStartAddress, uiWordLength); // 0x0A [CMD] 0x0D

	// int nSize = strlen(szSend);
	Send(RF_PT_REQ_GET_TAG_BANK, szSend, strlen(szSend));

	// @2020/11/09 20:46:57.374
	int nRecv = Receive(uiRecvCommand, szReceive, _countof(szReceive));
	if (nRecv > 0) {
		unsigned int uiResult = 0;
		szReceive[nRecv] = 0; // Set null-string
		if (ParseReadEPC(szReceive, nRecv, stTagData)) {
			fResult = true;
		}
	}
	return fResult;
}
//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] : <bank> memory bank
//              :     0 : reserved
//              :     1 : EPC
//              :     2 : TID
//              :     3 : USER
//         [in] : <address> start address
//              :     0 ~ 3FFF
//         [in] : <length> read word length (1 Word = 2 Bytes)
//              :     1 ~ 20 (R300A) <= AL-510
//              :     1 ~ 1E (R300T)
//        [out] : puiErrorCode
//              :     Reference error code statement
//              : 0x00000000: ERROR_RFID_SUCCESS
//              :     ...
//              : 0x000000FF: ERROR_RFID_NON_SPECIFIC
// Return       : True if the function is successful; otherwise false.
// Remarks      :
// -----------------------------------------------------------------------------
// Reader
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] : <bank> memory bank
//              :     0 : reserved
//              :     1 : EPC
//              :     2 : TID
//              :     3 : USER
//         [in] : <address> start address
//              :     0 ~ 3FFF
//         [in] : <length> read word length (1 Word = 2 Bytes)
//              :     1 ~ 20 (R300A) <= AL-510
//              :     1 ~ 1E (R300T)
// Response     : <none or read data>
//              :     none: no tag in RF field
//              : <Error code>
//              :     0 : other error
//              :     3 : memory overrun
//              :     4 : memory locked
//              :     B : Insufficient power
//              :     F : Non - specific error
//==============================================================================
#if 0
bool RfidInterface::ReadTag(RFID_MEMORY_BANK emBank, unsigned int uiStartAddress, unsigned int uiWordLength, RFID_TAG_DATA &stTagData, unsigned int * puiErrorCode)
{
	// Send: <LF>R1,2,6<CR>  <== 0x0A 0x52 0x31 0x2C 0x32 0x2C 0x36 0x0D
	// Recv: <LF>@2020/11/10 16:26:39.338-Antenna2-R<CR><LF> <== 0x0A 0x40 0x32 30 32 30 2f 31 31 2f
	//                                                   31 30 20 31 36 3a 32 36 3a 33 39 2e 33 33 38 2d
	//                                                   41 6e 74 65 6e 6e 61 32 2d 52 0d 0a

	char szBuffer[MAX_SEND_BUFFER];
	char szReceive[MAX_RECV_BUFFER];
	bool fResult = false;
	if (emBank == RFID_MB_NONE)
		return fResult;

	if ((uiStartAddress <= 0) || (uiStartAddress > MAX_MEMORY_BANK_ADDRESS))
		return fResult;
	if ((uiWordLength <= 0) || (uiWordLength > MAX_MEMORY_BANK_LENGTH))
		return fResult;

	sprintf_s(szBuffer, _countof(szBuffer), "\n%c%d,%d,%d\r", CMD_RFID_READ_BANK, emBank, uiStartAddress, uiWordLength); // 0x0A [CMD] 0x0D

	int nSize = strlen(szBuffer);
	Send(szBuffer, strlen(szBuffer));

	// @2020/11/09 20:46:57.374
	int nRecv = Receive(szReceive, _countof(szReceive));
	if (nRecv > 0)
	{
		unsigned int uiResult = 0;
		szReceive[nRecv] = 0; // Set null-string
		if (ParseReadEPC(szReceive, nRecv, stTagData))
		{
			fResult = true;
		}
	}
	return fResult;
}
#endif
//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] : <bank> memory bank
//              :     0 : reserved
//              :     1 : EPC
//              :     2 : TID
//              :     3 : USER
//         [in] : <address> start address
//              :     0 ~ 3FFF
//         [in] : <length> read word length (1 Word = 2 Bytes)
//              :     1 ~ 20 (R300A) <= AL-510
//              :     1 ~ 1E (R300T)
//        [out] : puiErrorCode
//              :     Reference error code statement
//              : 0x00000000: ERROR_RFID_SUCCESS
//              :     ...
//              : 0x000000FF: ERROR_RFID_NON_SPECIFIC
// Return       : True if the function is successful; otherwise false.
// Remarks      :
// -----------------------------------------------------------------------------
// Reader
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] : <bank> memory bank
//              :     0 : reserved
//              :     1 : EPC
//              :     2 : TID
//              :     3 : USER
//         [in] : <address> start address
//              :     0 ~ 3FFF
//         [in] : <length> read word length (1 Word = 2 Bytes)
//              :     1 ~ 20 (R300A) <= AL-510
//              :     1 ~ 1E (R300T)
// Response     : <none or read data>
//              :     none: no tag in RF field
//              : <Error code>
//              :     0 : other error
//              :     3 : memory overrun
//              :     4 : memory locked
//              :     B : Insufficient power
//              :     F : Non - specific error
//==============================================================================
bool RfidInterface::ReadEPC(unsigned int uiStartAddress,
                            unsigned int uiWordLength, RFID_TAG_DATA &stTagData,
                            unsigned int *puiErrorCode) {
	// Send: <LF>R1,2,6<CR>  <== 0x0A 0x52 0x31 0x2C 0x32 0x2C 0x36 0x0D
	// Recv: <LF>@2020/11/10 16:26:39.338-Antenna2-R<CR><LF> <== 0x0A 0x40 0x32 30
	// 32 30 2f 31 31 2f
	//                                                   31 30 20 31 36 3a 32 36
	//                                                   3a 33 39 2e 33 33 38 2d
	//                                                   41 6e 74 65 6e 6e 61 32
	//                                                   2d 52 0d 0a

	char szSend[MAX_SEND_BUFFER];
	char szReceive[MAX_RECV_BUFFER];
	unsigned int uiRecvCommand = 0; // The received command
	bool fResult = false;

	if ((uiStartAddress < 0) || (uiStartAddress > MAX_MEMORY_BANK_ADDRESS))
		return fResult;
	if ((uiWordLength <= 0) || (uiWordLength > MAX_MEMORY_BANK_LENGTH))
		return fResult;

	sprintf_s(szSend, _countof(szSend), "\n%c%d,%d,%d\r", CMD_RFID_READ_BANK,
		  RFID_MB_EPC, uiStartAddress, uiWordLength); // 0x0A [CMD] 0x0D

	// int nSize = strlen(szSend);
	Send(RF_PT_REQ_GET_TAG_EPC, szSend, strlen(szSend));

	// @2020/11/09 20:46:57.374
	int nRecv = Receive(uiRecvCommand, szReceive, _countof(szReceive));
	if (nRecv > 0) {
		unsigned int uiResult = 0;
		szReceive[nRecv] = 0; // Set null-string
		if (ParseReadEPC(szReceive, nRecv, stTagData)) {
			fResult = true;
		}
	}
	return fResult;
}

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] : <bank> memory bank
//              :     0 : reserved
//              :     1 : EPC
//              :     2 : TID
//              :     3 : USER
//         [in] : <address> start address
//              :     0 ~ 3FFF
//         [in] : <length> read word length (1 Word = 2 Bytes)
//              :     1 ~ 20 (R300A) <= AL-510
//              :     1 ~ 1E (R300T)
//        [out] : puiErrorCode
//              :     Reference error code statement
//              : 0x00000000: ERROR_RFID_SUCCESS
//              :     ...
//              : 0x000000FF: ERROR_RFID_NON_SPECIFIC
// Return       : True if the function is successful; otherwise false.
// Remarks      :
// -----------------------------------------------------------------------------
// Reader
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] : <bank> memory bank
//              :     0 : reserved
//              :     1 : EPC
//              :     2 : TID
//              :     3 : USER
//         [in] : <address> start address
//              :     0 ~ 3FFF
//         [in] : <length> read word length (1 Word = 2 Bytes)
//              :     1 ~ 20 (R300A) <= AL-510
//              :     1 ~ 1E (R300T)
// Response     : <none or read data>
//              :     none: no tag in RF field
//              : <Error code>
//              :     0 : other error
//              :     3 : memory overrun
//              :     4 : memory locked
//              :     B : Insufficient power
//              :     F : Non - specific error
//==============================================================================
bool RfidInterface::ReadTID(unsigned int uiStartAddress,
                            unsigned int uiWordLength, RFID_TAG_DATA &stTagData,
                            unsigned int *puiErrorCode) {
	// Send: <LF>R2,2,6<CR>  <== 0x0A 0x52 0x32 0x2C 0x32 0x2C 0x36 0x0D
	// Recv: <LF>@2020/11/10 16:26:39.338-Antenna2-R<CR><LF> <== 0x0A 0x40 0x32 30
	// 32 30 2f 31 31 2f
	//                                                   31 30 20 31 36 3a 32 36
	//                                                   3a 33 39 2e 33 33 38 2d
	//                                                   41 6e 74 65 6e 6e 61 32
	//                                                   2d 52 0d 0a

	char szSend[MAX_SEND_BUFFER];
	char szReceive[MAX_RECV_BUFFER];
	unsigned int uiRecvCommand = 0; // The received command
	bool fResult = false;

	if ((uiStartAddress < 0) || (uiStartAddress > MAX_MEMORY_BANK_ADDRESS))
		return fResult;
	if ((uiWordLength <= 0) || (uiWordLength > MAX_MEMORY_BANK_LENGTH))
		return fResult;

	sprintf_s(szSend, _countof(szSend), "\n%c%d,%d,%d\r", CMD_RFID_READ_BANK,
		  RFID_MB_TID, uiStartAddress, uiWordLength); // 0x0A [CMD] 0x0D

	// int nSize = strlen(szSend);
	Send(RF_PT_REQ_GET_TAG_TID, szSend, strlen(szSend));

	// @2020/11/09 20:46:57.374
	int nRecv = Receive(uiRecvCommand, szReceive, _countof(szReceive));
	if (nRecv > 0) {
		unsigned int uiResult = 0;
		szReceive[nRecv] = 0; // Set null-string
		if (ParseReadTID(szReceive, nRecv, stTagData)) {
			fResult = true;
		}
	}
	return fResult;
}

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] : <bank> memory bank
//              :     0 : reserved
//              :     1 : EPC
//              :     2 : TID
//              :     3 : USER
//         [in] : <address> start address
//              :     0 ~ 3FFF
//         [in] : <length> read word length (1 Word = 2 Bytes)
//              :     1 ~ 20 (R300A) <= AL-510
//              :     1 ~ 1E (R300T)
//        [out] : puiErrorCode
//              :     Reference error code statement
//              : 0x00000000: ERROR_RFID_SUCCESS
//              :     ...
//              : 0x000000FF: ERROR_RFID_NON_SPECIFIC
// Return       : True if the function is successful; otherwise false.
// Remarks      :
// -----------------------------------------------------------------------------
// Reader
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] : <bank> memory bank
//              :     0 : reserved
//              :     1 : EPC
//              :     2 : TID
//              :     3 : USER
//         [in] : <address> start address
//              :     0 ~ 3FFF
//         [in] : <length> read word length (1 Word = 2 Bytes)
//              :     1 ~ 20 (R300A) <= AL-510
//              :     1 ~ 1E (R300T)
// Response     : <none or read data>
//              :     none: no tag in RF field
//              : <Error code>
//              :     0 : other error
//              :     3 : memory overrun
//              :     4 : memory locked
//              :     B : Insufficient power
//              :     F : Non - specific error
//==============================================================================
bool RfidInterface::ReadUserData(unsigned int uiStartAddress,
                                 unsigned int uiWordLength,
                                 RFID_TAG_DATA &stTagData,
                                 unsigned int *puiErrorCode) {
	// Send: <LF>R3,2,6<CR>  <== 0x0A 0x52 0x32 0x2C 0x32 0x2C 0x36 0x0D
	// Recv: <LF>@2020/11/10 16:26:39.338-Antenna2-R<CR><LF> <== 0x0A 0x40 0x32 30
	// 32 30 2f 31 31 2f
	//                                                   31 30 20 31 36 3a 32 36
	//                                                   3a 33 39 2e 33 33 38 2d
	//                                                   41 6e 74 65 6e 6e 61 32
	//                                                   2d 52 0d 0a

	char szSend[MAX_SEND_BUFFER];
	char szReceive[MAX_RECV_BUFFER];
	unsigned int uiRecvCommand = 0; // The received command
	bool fResult = false;

	if ((uiStartAddress <= 0) || (uiStartAddress > MAX_MEMORY_BANK_ADDRESS))
		return fResult;
	if ((uiWordLength <= 0) || (uiWordLength > MAX_MEMORY_BANK_LENGTH))
		return fResult;

	sprintf_s(szSend, _countof(szSend), "\n%c%d,%d,%d\r", CMD_RFID_READ_BANK,
		  RFID_MB_USER, uiStartAddress, uiWordLength); // 0x0A [CMD] 0x0D

	// int nSize = strlen(szSend);
	Send(RF_PT_REQ_GET_TAG_TID, szSend, strlen(szSend));

	// @2020/11/09 20:46:57.374
	int nRecv = Receive(uiRecvCommand, szReceive, _countof(szReceive));
	if (nRecv > 0) {
		unsigned int uiResult = 0;
		szReceive[nRecv] = 0; // Set null-string
		if (ParseReadUserData(szReceive, nRecv, stTagData)) {
			fResult = true;
		}
	}
	return fResult;
}

//==============================================================================
// Function     :
// Purpose      :
// Description	: Display tag EPC ID and read tag memory data
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] : <bank> memory bank
//              :     0 : reserved
//              :     1 : EPC
//              :     2 : TID
//              :     3 : USER
//         [in] : <address> start address
//              :     0 ~ 3FFF
//         [in] : <length> read word length (1 Word = 2 Bytes)
//              :     1 ~ 20 (R300A) <= AL-510
//              :     1 ~ 1E (R300T)
//        [out] : puiErrorCode
//              :     Reference error code statement
//              : 0x00000000: ERROR_RFID_SUCCESS
//              :     ...
//              : 0x000000FF: ERROR_RFID_NON_SPECIFIC
// Return       : True if the function is successful; otherwise false.
// Remarks      :
// -----------------------------------------------------------------------------
// Reader
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] : <bank> memory bank
//              :     0 : reserved
//              :     1 : EPC
//              :     2 : TID
//              :     3 : USER
//         [in] : <address> start address
//              :     0 ~ 3FFF
//         [in] : <length> read word length (1 Word = 2 Bytes)
//              :     1 ~ 20 (R300A) <= AL-510
//              :     1 ~ 1E (R300T)
// Response     : <none or read data>
//              :     none: no tag in RF field
//              : <Error code>
//              :     0 : other error
//              :     3 : memory overrun
//              :     4 : memory locked
//              :     B : Insufficient power
//              :     F : Non - specific error
//==============================================================================
bool RfidInterface::ReadEPCandTID(unsigned int uiStartAddress,
                                  unsigned int uiWordLength,
                                  RFID_TAG_DATA &stTagData,
                                  RFID_TAG_EPC *pstTagEPC,
                                  unsigned int *puiErrorCode) {
	// Send: <LF>Q,R2,0,6<CR>  <== 0x0A 0x51 0x2C 0x52 0x31 0x2C 0x32 0x2C 0x36
	// 0x0D Recv: <LF>@2020/11/10 16:26:39.338-Antenna2-Q[], R[]<CR><LF> <== 0x0A
	// [] 0D 0A

	// Display tag EPC ID and read tag memory data
	//---------------------------------------------------------------------------
	// 0000   00 24 77 56 4f bc 04 92 26 58 dc 63 08 00 45 00.$wVO...&X.c..E.
	// 0010   00 32 ea dd 00 00 80 06 00 00 c0 a8 ec 1c c0 a8   .2..............
	// 0020   ec 5b f0 db 03 e9 6b 3e 75 84 60 6b 0e 37 50 18   .[....k>u.`k.7P.
	// 0030   ff 70 43 55 00 00 0a 51 2c 52 32 2c 30 2c 36 0d.pCU...Q, R2, 0, 6.
	//
	// 0000   04 92 26 58 dc 63 00 24 77 56 4f bc 08 00 45 00   ..&X.c.$wVO...E.
	//	0010   00 88 09 23 40 00 ff 06 18 83 c0 a8 ec 5b c0 a8 ...#@........[..
	//	0020   ec 1c 03 e9 f0 db 60 6b 0e 37 6b 3e 75 8e 50 18 ......`k.7k>u.P.
	//	0030   00 ef a7 c6 00 00 0a 40 32 30 32 30 2f 31 31 2f   .......@2020/11
	/// 	0040   31 30 20 31 38 3a 33 31 3a 31 33 2e 31 38 39 2d   10 18:31
	/// : 13.189
	//- 	0050   41 6e 74 65 6e 6e 61 32 2d 51 33 30 30 30 45 32   Antenna2 -
	// Q3000E2 	0060   38 30 31 31 36 30 36 30 30 30 30 32 30 44 36 38
	// 8011606000020D68 	0070   34 32 43 41 42 46 42 46 41 46 2c 52 45 32 38 30
	// 42CABFBFAF, RE280 	0080   31 31 36 30 32 30 30 30 36 32 42 46 30 38 35 39
	// 1160200062BF0859 	0090   30 39 41 44 0d 0a 09AD..

	char szSend[MAX_SEND_BUFFER];
	char szReceive[MAX_RECV_BUFFER];
	unsigned int uiRecvCommand = 0; // The received command
	bool fResult = false;

	if ((uiStartAddress < 0) || (uiStartAddress > MAX_MEMORY_BANK_ADDRESS))
		return fResult;
	if ((uiWordLength <= 0) || (uiWordLength > MAX_MEMORY_BANK_LENGTH))
		return fResult;

	sprintf_s(szSend, _countof(szSend), "\n%c,%c%d,%d,%d\r", CMD_RFID_READ_EPC,
		  CMD_RFID_READ_BANK, RFID_MB_TID, uiStartAddress,
		  uiWordLength); // 0x0A [CMD] 0x0D

	// int nSize = strlen(szSend);
	Send(RF_PT_REQ_GET_TAG_EPC_TID, szSend, strlen(szSend));

	// @2020/11/09 20:46:57.374
	int nRecv = Receive(uiRecvCommand, szReceive, _countof(szReceive));
	if (nRecv > 0) {
		unsigned int uiResult = 0;
		szReceive[nRecv] = 0; // Set null-string
		if (ParseReadEPCandTID(szReceive, nRecv, stTagData, pstTagEPC)) {
			fResult = true;
		}
	}
	return fResult;
}

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] : <bank> memory bank
//              :     0 : reserved
//              :     1 : EPC
//              :     2 : TID
//              :     3 : USER
//         [in] : <address> start address
//              :     0 ~ 3FFF
//         [in] : <length> read word length (1 Word = 2 Bytes)
//              :     1 ~ 20 (R300A) <= AL-510
//              :     1 ~ 1E (R300T)
//        [out] : puiErrorCode
//              :     Reference error code statement
//              : 0x00000000: ERROR_RFID_SUCCESS
//              :     ...
//              : 0x000000FF: ERROR_RFID_NON_SPECIFIC
// Return       : True if the function is successful; otherwise false.
// Remarks      :
// -----------------------------------------------------------------------------
// Reader
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] : <bank> memory bank
//              :     0 : reserved
//              :     1 : EPC
//              :     2 : TID
//              :     3 : USER
//         [in] : <address> start address
//              :     0 ~ 3FFF
//         [in] : <length> read word length (1 Word = 2 Bytes)
//              :     1 ~ 20 (R300A) <= AL-510
//              :     1 ~ 1E (R300T)
// Response     : <none or read data>
//              :     none: no tag in RF field
//              : <Error code>
//              :     0 : other error
//              :     3 : memory overrun
//              :     4 : memory locked
//              :     B : Insufficient power
//              :     F : Non - specific error
//==============================================================================
bool RfidInterface::ReadEPCandUserData(unsigned int uiStartAddress,
                                       unsigned int uiWordLength,
                                       RFID_TAG_DATA &stTagData,
                                       RFID_TAG_EPC *pstTagEPC,
                                       unsigned int *puiErrorCode) {
	// Send: <LF>Q,R3,0,20<CR>  <== 0x0A [] 0x0D
	// Recv: <LF>@2020/11/10 16:26:39.338-Antenna2-Q, R<CR><LF> <==

	//---------------------------------------------------------------------------
	// Read EPC+User(Q,R3,0,20)
	//---------------------------------------------------------------------------
	// 0000   00 24 77 56 4f bc 04 92 26 58 dc 63 08 00 45 00.$wVO...&X.c..E.
	//	0010   00 33 ea df 00 00 80 06 00 00 c0 a8 ec 1c c0 a8 .3..............
	//	0020   ec 5b f0 db 03 e9 6b 3e 75 8e 60 6b 0e 97 50 18 .[....k>u.`k..P.
	//	0030   ff 10 39 27 00 00 0a 51 2c 52 33 2c 30 2c 32 30 ..9'...Q,R3,0,20
	//	0040   0d
	//
	// 0000   04 92 26 58 dc 63 00 24 77 56 4f bc 08 00 45 00   ..&X.c.$wVO...E.
	//	0010   00 50 09 25 40 00 ff 06 18 b9 c0 a8 ec 5b c0 a8.P.%@........[..
	//	0020   ec 1c 03 e9 f0 db 60 6b 0e 97 6b 3e 75 99 50 18 ......`k..k>u.P.
	//	0030   00 ef c7 9d 00 00 0a 40 32 30 32 30 2f 31 31 2f   .......@2020/11
	/// 	0040   31 30 20 31 38 3a 33 31 3a 32 35 2e 30 32 31 2d   10 18:31
	/// : 25.021
	//- 	0050   41 6e 74 65 6e 6e 61 32 2d 51 2c 52 0d 0a         Antenna2 - Q,
	// R..

	char szSend[MAX_SEND_BUFFER];
	char szReceive[MAX_RECV_BUFFER];
	unsigned int uiRecvCommand = 0; // The received command
	bool fResult = false;

	if ((uiStartAddress < 0) || (uiStartAddress > MAX_MEMORY_BANK_ADDRESS))
		return fResult;
	if ((uiWordLength <= 0) || (uiWordLength > MAX_MEMORY_BANK_LENGTH))
		return fResult;

	sprintf_s(szSend, _countof(szSend), "\n%c,%c%d,%d,%d\r", CMD_RFID_READ_EPC,
		  CMD_RFID_READ_BANK, RFID_MB_USER, uiStartAddress,
		  uiWordLength); // 0x0A [CMD] 0x0D

	// int nSize = strlen(szSend);
	Send(RF_PT_REQ_GET_TAG_EPC_USER, szSend, strlen(szSend));

	// @2020/11/09 20:46:57.374
	int nRecv = Receive(uiRecvCommand, szReceive, _countof(szReceive));
	if (nRecv > 0) {
		unsigned int uiResult = 0;
		szReceive[nRecv] = 0; // Set null-string
		if (ParseReadEPCandUserData(szReceive, nRecv, stTagData, pstTagEPC)) {
			fResult = true;
		}
	}
	return fResult;
}

//==============================================================================
// Function     :
// Purpose      : display tag EPC ID
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :  0:成功, 不為0:失敗
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================
bool RfidInterface::ReadSingleTagEPC(bool fLoop) {
	// Q   : display tag EPC ID
	// Send: Q
	// Recv: Q<none or EPC>
	//		  <none or EPC>
	// none: no tag in RF field
	// EPC : PC + EPC + CRC16
	//
	// Send: <LF>Q<CR>  <== 0x0A 0x40 0x51 0x0D
	// Recv: <LF>@2020/11/10 16:26:39.338-Antenna2-Q[]<CR><LF> <== 0x0A 0x40 0x32
	// 30 32 30 2f 31 31 2f
	//                                                   31 30 20 31 36 3a 32 36
	//                                                   3a 33 39 2e 33 33 38 2d
	//                                                   41 6e 74 65 6e 6e 61 32
	//                                                   2d 52 0d 0a

	//---------------------------------------------------------------------------
	// Read SingleTagEPC
	//---------------------------------------------------------------------------
	//	0000   00 24 77 56 4f bc 04 92 26 58 dc 63 08 00 45 00.$wVO...&X.c..E.
	//	0010   00 2c 55 80 00 00 80 06 00 00 c0 a8 ec 1c c0 a8   .,
	// U............. 	0020   ec 5b c8 92 03 e9 65 67 61 f4 f6 82 c0 01 50 18
	//.[....ega.....P. 	0030   ff 70 b0 e4 00 00 0a 40 51 0d .p.....@Q.

	//	0000   04 92 26 58 dc 63 00 24 77 56 4f bc 08 00 45 00 ..&X.c.$wVO...E.
	//	0010   00 6e 3e 52 40 00 ff 06 e3 6d c0 a8 ec 5b c0 a8 .n>R@....m...[..
	//	0020   ec 1c 03 e9 c8 92 f6 82 c0 01 65 67 61 f8 50 18 ..........ega.P.
	//	0030   00 ef 90 a1 00 00 0a 40 32 30 32 30 2f 31 31 2f   .......@2020/11
	/// 	0040   31 36 20 32 30 3a 33 36 3a 35 35 2e 36 31 37 2d   16 20:36
	/// : 55.617
	//- 	0050   41 6e 74 65 6e 6e 61 31 2d 51 33 30 30 30 45 32   Antenna1 -
	// Q3000E2 	0060   38 30 31 31 36 30 36 30 30 30 30 32 30 44 36 38
	// 8011606000020D68 	0070   34 33 30 30 36 46 42 41 37 44 0d 0a 43006FBA7D..

	// EPC:	E28011606000020D6843006F, Antenna : 1, Time : 2020 / 11 / 16 20
	// : 36 : 55.617, IP : 192.168.236.91 EPC : E28011606000020D6843006F, Antenna
	// : 1, Time : 2020 / 11 / 16 20 : 36 : 55.657, IP : 192.168.236.91 EPC :
	// E28011606000020D6843006F, Antenna : 1, Time : 2020 / 11 / 16 20 : 36
	// : 55.783, IP : 192.168.236.91 EPC : E28011606000020D6843006F, Antenna : 1,
	// Time : 2020 / 11 / 16 20 : 36 : 55.806, IP : 192.168.236.91 EPC :
	// E28011606000020D6843006F, Antenna : 1, Time : 2020 / 11 / 16 20 : 36
	// : 55.863, IP : 192.168.236.91 EPC : E28011606000020D6843006F, Antenna : 1,
	// Time : 2020 / 11 / 16 20 : 36 : 55.903, IP : 192.168.236.91 EPC :
	// E28011606000020D6843006F, Antenna : 1, Time : 2020 / 11 / 16 20 : 36
	// : 55.960, IP : 192.168.236.91 EPC : E28011606000020D6843006F, Antenna : 1,
	// Time : 2020 / 11 / 16 20 : 36 : 56.000, IP : 192.168.236.91 EPC :
	// E28011606000020D6843006F, Antenna : 1, Time : 2020 / 11 / 16 20 : 36
	// : 56.040, IP : 192.168.236.91 EPC : E28011606000020D6843006F, Antenna : 1,
	// Time : 2020 / 11 / 16 20 : 36 : 56.114, IP : 192.168.236.91 EPC :
	// E28011606000020D6843006F, Antenna : 1, Time : 2020 / 11 / 16 20 : 36
	// : 56.153, IP : 192.168.236.91 EPC : E28011606000020D6843006F, Antenna : 1,
	// Time : 2020 / 11 / 16 20 : 36 : 56.278, IP : 192.168.236.91 EPC :
	// E28011606000020D6843006F, Antenna : 1, Time : 2020 / 11 / 16 20 : 36
	// : 56.352, IP : 192.168.236.91 EPC : E28011606000020D6843006F, Antenna : 1,
	// Time : 2020 / 11 / 16 20 : 36 : 56.392, IP : 192.168.236.91 EPC :
	// E28011606000020D6843006F, Antenna : 1, Time : 2020 / 11 / 16 20 : 36
	// : 56.518, IP : 192.168.236.91 EPC : E28011606000020D6843006F, Antenna : 1,
	// Time : 2020 / 11 / 16 20 : 36 : 56.592, IP : 192.168.236.91

	char szSend[MAX_SEND_BUFFER];
	char szReceive[MAX_RECV_BUFFER * 10];
	unsigned int uiRecvCommand = 0; // The received command
	bool fResult = false;

	if (fLoop) {
		sprintf_s(szSend, _countof(szSend), "\n@%c\r",
			  CMD_RFID_READ_EPC); // 0x0A [CMD] 0x0D
		Send(RF_PT_REQ_GET_SIGNLE_TAG_EPC_LOOP, szSend, strlen(szSend));
	} else {
		sprintf_s(szSend, _countof(szSend), "\n%c\r",
			  CMD_RFID_READ_EPC); // 0x0A [CMD] 0x0D
		Send(RF_PT_REQ_GET_SIGNLE_TAG_EPC_ONCE, szSend, strlen(szSend));
	}

	// int nSize = strlen(szSend);

	// @2020/11/09 20:46:57.374
	if (fLoop == false) {
		int nRecv = Receive(uiRecvCommand, szReceive, _countof(szReceive));
		if (nRecv > 0) {
			unsigned int uiResult = 0;
			szReceive[nRecv] = 0; // Set null-string

			RFID_TAG_DATA stTagData;
			RFID_TAG_EPC stTagEPC;
			if (ParseReadSingleEPC(szReceive, nRecv, stTagData, &stTagEPC)) {
				fResult = true;
			}
		}
	} else {
		// Asynchronous mode
		fResult = true;
	}

	return fResult;
}

//==============================================================================
// Function     :
// Purpose      : Multi-TAG read EPC
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :  0:成功, 不為0:失敗
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================
bool RfidInterface::ReadMultiTagEPC(int nSlot, bool fLoop) {
	// Send: U or U<slot>
	// Recv: U<none or EPC>
	//		<none or EPC>
	// none: no tag in RF field
	// EPC : PC + EPC + CRC16

	//
	// Send: <LF>@U<CR>  <== 0x0A 0x52 0x32 0x2C 0x32 0x2C 0x36 0x0D
	// Recv: <LF>@2020/11/10 16:26:39.338-Antenna2-R<CR><LF> <== 0x0A 0x40 0x32 30
	// 32 30 2f 31 31 2f
	//                                                   31 30 20 31 36 3a 32 36
	//                                                   3a 33 39 2e 33 33 38 2d
	//                                                   41 6e 74 65 6e 6e 61 32
	//                                                   2d 52 0d 0a

	//  Request
	//	0000   00 24 77 56 4f bc 04 92 26 58 dc 63 08 00 45 00.$wVO...&X.c..E.
	//	0010   00 2c 55 bf 00 00 80 06 00 00 c0 a8 ec 1c c0 a8   .,
	// U............. 	0020   ec 5b cc 9c 03 e9 4e 83 cf c3 fc dc ba 01 50 18
	//.[....N.......P. 	0030   ff 70 51 95 00 00 0a 40 55 0d.pQ....@U.

	// Response 1
	//  0000   04 92 26 58 dc 63 00 24 77 56 4f bc 08 00 45 00   ..&X.c.$wVO...E.
	//	0010   00 6e 3e c3 40 00 ff 06 e2 fc c0 a8 ec 5b c0 a8.n>.@........[..
	//	0020   ec 1c 03 e9 cc 9c fc dc ba 01 4e 83 cf c7 50 18 ..........N...P.
	//	0030   00 ef 3e 53 00 00 0a 40 32 30 32 30 2f 31 31 2f   ..>S...@2020/11
	/// 	0040   31 36 20 32 30 3a 35 31 3a 30 34 2e 30 36 33 2d   16 20:51 :
	/// 04.063
	//- 	0050   41 6e 74 65 6e 6e 61 31 2d 55 33 30 30 30 45 32   Antenna1 -
	// U3000E2 	0060   38 30 31 31 36 30 36 30 30 30 30 32 30 44 36 38
	// 8011606000020D68 	0070   34 33 30 30 36 46 42 41 37 44 0d 0a 43006FBA7D..
	//
	// Response 2
	//  0000   04 92 26 58 dc 63 00 24 77 56 4f bc 08 00 45 00   ..&X.c.$wVO...E.
	//  0010   00 6e 3e c4 40 00 ff 06 e2 fb c0 a8 ec 5b c0 a8.n>.@........[..
	//  0020   ec 1c 03 e9 cc 9c fc dc ba 47 4e 83 cf c7 50 18   .........GN...P.
	//	0030   00 ef 19 f8 00 00 0a 40 32 30 32 30 2f 31 31 2f   .......@2020/11
	/// 	0040   31 36 20 32 30 3a 35 31 3a 30 34 2e 30 36 35 2d   16 20:51 :
	/// 04.065
	//- 	0050   41 6e 74 65 6e 6e 61 31 2d 55 33 30 30 30 45 32   Antenna1 -
	// U3000E2 	0060   38 30 31 31 36 30 36 30 30 30 30 32 30 44 36 38
	// 8011606000020D68 	0070   34 32 43 43 46 46 35 44 43 44 0d 0a 42CCFF5DCD..

	//  Response N
	//  0000   04 92 26 58 dc 63 00 24 77 56 4f bc 08 00 45 00   ..&X.c.$wVO...E.
	//	0010   00 4e 3e c6 40 00 ff 06 e3 19 c0 a8 ec 5b c0 a8.N>.@........[..
	//	0020   ec 1c 03 e9 cc 9c fc dc ba d3 4e 83 cf c7 50 18 ..........N...P.
	//	0030   00 ef 97 00 00 00 0a 40 32 30 32 30 2f 31 31 2f   .......@2020/11
	/// 	0040   31 36 20 32 30 3a 35 31 3a 30 34 2e 30 39 33 2d   16 20:51 :
	/// 04.093
	//- 	0050   41 6e 74 65 6e 6e 61 31 2d 55 0d 0a               Antenna1 - U..

	//	0000   04 92 26 58 dc 63 00 24 77 56 4f bc 08 00 45 00 ..&X.c.$wVO...E.
	//	0010   00 2f 3e f7 00 00 ff 06 23 08 c0 a8 ec 5b c0 a8   . /
	//>.....#....[.. 	0020   ec 1c 03 e9 cc 9c fc dc c6 99 4e 83 cf c7 50 18
	//..........N...P. 	0030   00 ef 05 2a 00 00 0a 40 45 4e 44 0d 0a
	//...*...@END..

	char szSend[MAX_SEND_BUFFER];
	char szReceive[MAX_RECV_BUFFER];
	unsigned int uiRecvCommand = 0; // The received command
	bool fResult = false;

	if (fLoop) {
		if ((nSlot > 0) && (nSlot <= 9))
			sprintf_s(szSend, _countof(szSend), "\n@%c%d\r", CMD_RFID_READ_MULTI_EPC,
				  nSlot); // 0x0A @[CMD] 0x0D
		else
			sprintf_s(szSend, _countof(szSend), "\n@%c\r",
				  CMD_RFID_READ_MULTI_EPC); // 0x0A @[CMD] 0x0D
		Send(RF_PT_REQ_GET_MULTI_TAG_EPC_LOOP, szSend, strlen(szSend));
	} else {
		if ((nSlot > 0) && (nSlot <= 9))
			sprintf_s(szSend, _countof(szSend), "\n%c%d\r", CMD_RFID_READ_MULTI_EPC,
				  nSlot); // 0x0A [CMD] 0x0D
		else
			sprintf_s(szSend, _countof(szSend), "\n%c\r",
				  CMD_RFID_READ_MULTI_EPC); // 0x0A [CMD] 0x0D
		Send(RF_PT_REQ_GET_MULTI_TAG_EPC_ONCE, szSend, strlen(szSend));
	}

	// int nSize = strlen(szSend);

	// @2020/11/09 20:46:57.374
	if (fLoop == false) {
		int nRecv = Receive(uiRecvCommand, szReceive, _countof(szReceive));
		if (nRecv > 0) {
			unsigned int uiResult = 0;
			szReceive[nRecv] = 0; // Set null-string
			RFID_TAG_DATA stTagData;
			RFID_TAG_EPC stTagEPC;
			if (ParseReadMultiEPC(szReceive, nRecv, stTagData, &stTagEPC)) {
				fResult = true;
			}
		}
	} else {
		// Asynchronous mode
		fResult = true;
	}

	return fResult;
}

//==============================================================================
// Function     :
// Purpose      : Multi-TAG read EPC and data
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] : emType
//              :
//         [in] : nStart
//              :
//         [in] : nLength
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================
bool RfidInterface::ReadMultiBank(RFID_BANK_TYPE emType, int nStart,
                                  int nLength, bool fLoop) {
	// Send: U,R<bank>,<address>,<length>
	//       or U<slot>, R<bank>, <address>, <length>
	// Recv: U<none or EPC>, R<none or read data>
	return false;
}

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] : nSlotQ
//              :
//         [in] : stTag
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================

// bool RfidInterface::Inventory(RFID_TAG_DATA &stTagData)
bool RfidInterface::SelectMatchingTag(RFID_TAG_DATA &stTagData) {
	// Send: <LF>T<CR>  <== 0x0A 0x54 0x0D
	// Recv: <LF>@2020/11/10 16:26:39.338-Antenna2-T<CR><LF> <== 0x0A 0x40 0x32 30
	// 32 30 2f 31 31 2f
	//                                                   31 30 20 31 36 3a 32 36
	//                                                   3a 33 39 2e 33 33 38 2d
	//                                                   41 6e 74 65 6e 6e 61 32
	//                                                   2d 54 0d 0a

	char szSend[MAX_SEND_BUFFER];
	char szReceive[MAX_RECV_BUFFER];
	unsigned int uiRecvCommand = 0; // The received command
	bool fResult = false;

	sprintf_s(szSend, _countof(szSend), "\n%c\r",
		  CMD_RFID_SELECT_MATCHING); // 0x0A [CMD] 0x0D

	// int nSize = strlen(szSend);
	Send(RF_PT_REQ_SELECT_MATCHING_TAG, szSend, strlen(szSend));

	// @2020/11/09 20:46:57.374
	int nRecv = Receive(uiRecvCommand, szReceive, _countof(szReceive));
	if (nRecv > 0) {
		unsigned int uiResult = 0;
		szReceive[nRecv] = 0; // Set null-string
		// RFID_TAG_DATA stTagData;
		if (ParseSelectMatching(szReceive, nRecv, stTagData)) {
			fResult = true;
		}
	}
	return fResult;
}

bool RfidInterface::SetSession(RFID_SESSION emSession, RFID_TARGET emTarget) {
	// Send: <LF>T<CR>  <== 0x0A 0x54 0x0D (Default)
	// Send: <LF>T000<CR>  <== 0x0A 0x54 0x0D (Session 0 + Target A)
	// Send: <LF>T010<CR>  <== 0x0A 0x54 0x0D (Session 1 + Target A)
	// Send: <LF>T020<CR>  <== 0x0A 0x54 0x0D (Session 2 + Target A)
	// Send: <LF>T030<CR>  <== 0x0A 0x54 0x0D (Session 3 + Target A)
	// Send: <LF>T001<CR>  <== 0x0A 0x54 0x0D (Session 0 + Target B)
	// Send: <LF>T011<CR>  <== 0x0A 0x54 0x0D (Session 1 + Target B)
	// Send: <LF>T021<CR>  <== 0x0A 0x54 0x0D (Session 2 + Target B)
	// Send: <LF>T031<CR>  <== 0x0A 0x54 0x0D (Session 3 + Target B)
	// Recv: <LF>@2020/11/10 16:26:39.338-Antenna2-T<CR><LF> <== 0x0A 0x40 0x32 30
	// 32 30 2f 31 31 2f
	//                                                   31 30 20 31 36 3a 32 36
	//                                                   3a 33 39 2e 33 33 38 2d
	//                                                   41 6e 74 65 6e 6e 61 32
	//                                                   2d 54 0d 0a

	char szSend[MAX_SEND_BUFFER];
	char szReceive[MAX_RECV_BUFFER];
	unsigned int uiRecvCommand = 0; // The received command
	bool fResult = false;
	if (emSession > 0)
		sprintf_s(szSend, _countof(szSend), "\n%c%02d%d\r", CMD_RFID_SET_SESSION,
			  emSession, emTarget); // 0x0A [CMD] 0x0D
	else
		sprintf_s(szSend, _countof(szSend), "\n%c\r",
			  CMD_RFID_SET_SESSION); // 0x0A [CMD] 0x0D

	// int nSize = strlen(szSend);
	Send(RF_PT_REQ_SET_SESSION, szSend, strlen(szSend));

	// @2020/11/09 20:46:57.374
	int nRecv = Receive(uiRecvCommand, szReceive, _countof(szReceive));
	if (nRecv > 0) {
		unsigned int uiResult = 0;
		szReceive[nRecv] = 0; // Set null-string
		RFID_TAG_DATA stTagData;
		if (ParseSession(szReceive, nRecv, stTagData)) {
			fResult = true;
		}
	}
	return fResult;
}

bool RfidInterface::InventoryEPC(int nSlotQ, RFID_TAG_DATA &stTagData) {
	// Send: <LF>T<CR>  <== 0x0A 0x54 0x0D
	// Recv: <LF>@2020/11/10 16:26:39.338-Antenna2-T<CR><LF> <== 0x0A 0x40 0x32 30
	// 32 30 2f 31 31 2f
	//                                                   31 30 20 31 36 3a 32 36
	//                                                   3a 33 39 2e 33 33 38 2d
	//                                                   41 6e 74 65 6e 6e 61 32
	//                                                   2d 54 0d 0a

	char szSend[MAX_SEND_BUFFER];
	char szReceive[MAX_RECV_BUFFER];
	unsigned int uiRecvCommand = 0; // The received command
	bool fResult = false;

	sprintf_s(szSend, _countof(szSend), "\n%c\r",
		  CMD_RFID_READ_MULTI_EPC); // 0x0A [CMD] 0x0D

	// int nSize = strlen(szSend);
	Send(RF_PT_REQ_INVENTORY_TAG_EPC_ONCE, szSend, strlen(szSend));

	// @2020/11/09 20:46:57.374
	int nRecv = Receive(uiRecvCommand, szReceive, _countof(szReceive));
	if (nRecv > 0) {
		unsigned int uiResult = 0;
		szReceive[nRecv] = 0; // Set null-string
		// RFID_TAG_DATA stTagData;
		// if (ParseSelectMatching(szReceive, nRecv, stTagData))
		//{
		//	fResult = true;
		//}
	}
	return fResult;
}

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] : nSlotQ
//              :
//         [in] : SessionType
//              :   會議模式
//         [in] : TargetType
//              :   搜尋模式
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================
#if 0
bool Inventory(int nSlotQ, SessionType session, TargetType target)
{
	return false;
}
#endif

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================
bool RfidInterface::InventoryTID(int nSlotQ, RFID_TAG_DATA &stTagData) {
	return false;
}

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================
#if 0
bool RfidInterface::InventoryTID(int nSlotQ, RFID_TAG &stTag, SessionType session, TargetType target)
{
	return false;
}
#endif
//==============================================================================
// Function     :
// Purpose      : Inventory Tag EPC與User Memory
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================
bool RfidInterface::InventoryUser(int nSlotQ, int nLength) { return false; }

//==============================================================================
// Function     :
// Purpose      : Inventory Tag EPC與User Memory
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================
#if 0
bool RfidInterface::InventoryUser(int nSlotQ, int nLength, SessionType session, TargetType target)
{
	return false;
}
#endif

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================
bool RfidInterface::WriteEPC() { return false; }

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================
bool RfidInterface::WriteBank() { return false; }

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================
bool RfidInterface::LockTag() { return false; }

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================
bool RfidInterface::KillTag() { return false; }

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================
bool RfidInterface::SetGPO(int nPort, bool fIsOn) { return false; }

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] : bool[] GetGPI()
//              :  True:On False:Off
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================
bool RfidInterface::GetGPO() { return false; }

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :  bool[] GetGPI()
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================
bool RfidInterface::GetGPI() { return false; }

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================
bool RfidInterface::GPI(RFID_GPI &stGPI) { return false; }

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================
bool RfidInterface::SetUSBHID() { return false; }

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================
bool RfidInterface::SetUSBKeyboard() { return false; }

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================
bool RfidInterface::SetWifiAP() { return false; }

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================
bool RfidInterface::GetWifiAP() { return false; }

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================
bool RfidInterface::SetWifiStaticIP() { return false; }

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================
bool RfidInterface::GetWifiStaticIP() { return false; }

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================
bool RfidInterface::SetDHCPEnable() { return false; }

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================
bool RfidInterface::SetDHCPDisable() { return false; }

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================
bool RfidInterface::GetDHCPStatus() { return false; }

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================
bool RfidInterface::GetWifiIP() { return false; }

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================
bool RfidInterface::GetWifiAPInfo() { return false; }

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================
bool RfidInterface::OpenHeartbeat(unsigned int uiMilliseconds) {
	char szSend[MAX_SEND_BUFFER];
	char szReceive[MAX_RECV_BUFFER];
	unsigned int uiRecvCommand = 0; // The received command
	bool fResult = false;
	sprintf_s(szSend, _countof(szSend), "\n%s%d\r", "@HeartbeatTime",
		  uiMilliseconds); // 0x0A [CMD] 0x0D

	// int nSize = strlen(szSend);
	Send(RF_PT_REQ_OPEN_HEARTBEAT, szSend, strlen(szSend));

	int nRecv = Receive(uiRecvCommand, szReceive, _countof(szReceive));
	if (nRecv > 0) {
		unsigned int uiResult = 0;
		szReceive[nRecv] = 0; // Set null-string
		if (ParseSetLoopTime(szReceive, nRecv, &uiResult)) {
			if (uiMilliseconds == uiResult)
				fResult = true;
		}
	}
	return fResult;
}

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================
bool RfidInterface::CloseHeartbeat() { return false; }

//==============================================================================
// Function     : GetModuleVersion
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================
bool RfidInterface::GetModuleVersion(TString &strVersion) {
	char szSend[MAX_SEND_BUFFER];
	char szReceive[MAX_RECV_BUFFER];
	unsigned int uiRecvCommand; // The received command
	// "<LF>@Version<CR>"); // 0x0a 0x40 0x56 0x65 0x72 0x73 0x69 0x6F 0x6E 0x0d
	// Response: <LF> @ Version:V5.5.201 90704.1 <CR><LF>
	sprintf_s(szSend, _countof(szSend), "\n%s\r", "@Version");
	// int nSize = strlen(szSend);

	Send(RF_PT_REQ_GET_MODULE_VERSION, szSend, strlen(szSend));

#if 0
	// For debug
	DWORD dwReceived = 0;
	//DWORD dw = 1000;
	do
	{
		Sleep(1000);
		if ((m_objSocket.IOCtl(FIONREAD, &dwReceived)) && (dwReceived > 0))
		{
			int nRecv = Receive(uiRecvCommand, szReceive, _countof(szReceive));
			if (nRecv > 0)
			{
				szReceive[nRecv] = 0; // Set null-string
				ParseModuleVersion(szReceive, nRecv, strVersion);
			}
		}
		//dw--;
	} while (dwReceived > 0);
	//} while (dw > 0);
#else

	int nRecv = Receive(uiRecvCommand, szReceive, _countof(szReceive));
	if (nRecv > 0) {
		szReceive[nRecv] = 0; // Set null-string
		return ParseModuleVersion(szReceive, nRecv, strVersion);
	}
#endif
	return false;
}

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================
bool RfidInterface::FlashEMTag() { return false; }

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================
bool RfidInterface::GetEMTemprature() { return false; }

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================
bool RfidInterface::GetEMBatteryVoltage() { return false; }

//==============================================================================
// Parse Functions
//==============================================================================

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] : fRemoveLeadingWord
//              :     Delete the leading word '@'.
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================
bool RfidInterface::GetContent(const void *lpBuffer, int nBufferLength,
                               TString &strContent, bool fRemoveLeadingWord) {
	// Ex. "\n@2020/11/04 11:18:01.271-Antenna1-XXXXXXXX, etc...\r\n"
	// fRemoveLeadingWord = true
	//		Content = "2020/11/04 11:18:01.271-Antenna1-XXXXXXXX, etc..."
	// fRemoveLeadingWord = false
	//		Content = "@2020/11/04 11:18:01.271-Antenna1-XXXXXXXX, etc..."
	TString strBuffer;
#ifdef _UNICODE
	std::string strBufferA = (char *)lpBuffer;
	strBuffer = m_objConvert.AnsiToUtf16(strBufferA);
#else
	strBuffer;
	= (char *)lpBuffer;
#endif
	if ((fRemoveLeadingWord == true) && (strBuffer.length() > 4) &&
	    (strBuffer.at(0) == _T('\n')) && (strBuffer.at(1) == _T('@')) &&
	    (strBuffer.at(nBufferLength - 2) == _T('\r')) &&
	    (strBuffer.at(nBufferLength - 1) == _T('\n'))) {
		strContent = strBuffer.substr(2, strBuffer.length() - 4);
		return true;
	} else if ((strBuffer.length() > 3) && (strBuffer.at(0) == _T('\n')) &&
		   (strBuffer.at(nBufferLength - 2) == _T('\r')) &&
		   (strBuffer.at(nBufferLength - 1) == _T('\n'))) {
		strContent = strBuffer.substr(1, strBuffer.length() - 3);
		return true;
	}
	return false;
}
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================
bool RfidInterface::ParseVersion(const void *lpBuffer, int nBufferLength,
                                 RFID_READER_VERSION &stVersion) {
	// Ex. "\n@2020/11/04 11:18:01.271-Antenna1-VD407,000015E1,CA,2\r\n"
	// Firmware version: VD407
	// Reader ID: 000015E1
	// Hardware version: CA
	// RF band regulation: 2
	//
	// SOFEWARE: VD407
	// READER ID : 000015E1
	// HARDWARE : CA
	// RFID BAND : TW
	// ModuleVersion :5.5.20190704.1

	bool fResult = false;
	TString strContent;
	GetContent(lpBuffer, nBufferLength, strContent);
	TStringArray aryVersion = m_objTokenizer.Token(strContent, _T(','));
	RFID_RESPONSE_INFO stResponseInfo;
	if (aryVersion.size() > 0) {
		if (aryVersion.at(0).at(0) != _T('V')) {
			TString strDate;
			TString strAntenna;
			TStringArray aryFirmware =
				m_objTokenizer.Token(aryVersion.at(0), _T('-'));
			if (aryFirmware.size() > 0) {
				if (aryFirmware.size() >= 3) {
					strDate = aryFirmware.at(0);
					strAntenna = aryFirmware.at(1);
					stResponseInfo.strDate = strDate;
					stResponseInfo.usAntenna = 0;
				}

				for (int nIndex = 0; nIndex < aryFirmware.size(); nIndex++) {
					if (aryFirmware.at(nIndex).at(0) == _T('V')) {
						// stVersion.strFirmware = aryFirmware.at(nIndex);
						stVersion.strFirmware = aryFirmware.at(nIndex).substr(
							1, aryFirmware.at(nIndex).length() - 1);
						break;
					}
				}
				stVersion.strMessage = aryVersion.at(0);
			} else {
				stVersion.strMessage = aryVersion.at(0);
			}
		} else
			stVersion.strFirmware = aryVersion.at(0);
		stVersion.strReaderId = aryVersion.at(1);
		stVersion.strHardware = aryVersion.at(2);
		stVersion.strRfBandRequalation = aryVersion.at(3);
		fResult = true;
	}
	return fResult;
}

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================
bool RfidInterface::ParseModuleVersion(const void *lpBuffer, int nBufferLength,
                                       TString &strVersion) {
	// Ex. "\n@Version:V5.5.201 90704.1\r\n"
	// "<LF>@Version<CR>"); // 0x0a 0x40 0x56 0x65 0x72 0x73 0x69 0x6F 0x6E 0x0d
	// Response: <LF>@Version:V5.5.201 90704.1<CR><LF>
	bool fResult = false;
	TString strContent;
	fResult = GetContent(lpBuffer, nBufferLength, strContent);
	if (strContent.at(0) == _T('V')) {
		TStringArray aryContent = m_objTokenizer.Token(strContent, _T(':'));
		// Version:V5.5.201 90704.1
		if (aryContent.size() >= 2) {
			// strName = aryContent.at(0);
			// strValue = aryContent.at(1);
			strVersion = aryContent.at(1);
			fResult = true;
		}
	}
	return fResult;
}

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================
bool RfidInterface::ParseReaderID(const void *lpBuffer, int nBufferLength,
                                  TString &strID) {
	// Ex. "\n@2020/11/04 11:18:01.271-Antenna1-S000015E1\r\n"

	// Ex. "\n@2020/11/04 11:18:01.271-Antenna1-S000015E1\r\n"
	bool fResult = false;
	TString strContent;
	GetContent(lpBuffer, nBufferLength, strContent);
	if (strContent.at(0) != _T('S')) {
		TStringArray aryContent = m_objTokenizer.Token(strContent, _T('-'));
		// 2020 / 11 / 04 11:18 : 01.271 - Antenna1
		TString strDate;
		TString strAntenna;
		if (aryContent.size() >= 3) {
			strDate = aryContent.at(0);
			strAntenna = aryContent.at(1);
		}

		if (aryContent.size() > 0) {
			for (int nIndex = 0; nIndex < aryContent.size(); nIndex++) {
				if (aryContent.at(nIndex).at(0) == _T('S')) {
					strID = aryContent.at(nIndex).substr(
						1, aryContent.at(nIndex).length() - 1);
					fResult = true;
					break;
				}
			}
		} else {
			strContent = aryContent.at(0);
		}
	} else {
		strID = strContent.substr(1, strContent.length() - 1);
		fResult = true;
	}
	return fResult;
}

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      : Setting regulation , reader no return message and will
// re-startup.
//==============================================================================
bool RfidInterface::ParseSetRegulation(const void *lpBuffer, int nBufferLength,
                                       int *pnResult) {
	bool fResult = false;
	TString strContent;
	GetContent(lpBuffer, nBufferLength, strContent);

	RFID_RESPONSE_INFO stResponseInfo;
	if (strContent.at(0) != _T('N')) {
		TStringArray aryContent = m_objTokenizer.Token(strContent, _T('-'));
		// 2020 / 11 / 04 11:18 : 01.271 - Antenna1
		TString strDate;
		TString strAntenna;
		if (aryContent.size() >= 3) {
			strDate = aryContent.at(0);
			strAntenna = aryContent.at(1);
			stResponseInfo.strDate = aryContent.at(0);
			stResponseInfo.usAntenna = 0;
		}

		if (aryContent.size() > 0) {
			for (int nIndex = 0; nIndex < aryContent.size(); nIndex++) {
				// if ((aryContent.at(nIndex).at(0) == _T('N')) &&
				// (aryContent.at(nIndex).at(0) == _T('5')))
				if (aryContent.at(nIndex).at(0) == _T('N')) {
					TString strValue = aryContent.at(nIndex).substr(
						1, aryContent.at(nIndex).length() - 1);
					// Decimal
					// int nRegulation = _tcstol(strValue.c_str(), NULL, 10);
					RFID_REGULATION emRegulation =
						(RFID_REGULATION)_tcstol(strValue.c_str(), NULL, 10);
					fResult = true;
					break;
				}
			}
		} else {
			strContent = aryContent.at(0);
		}
	} else if (strContent.at(0) == _T('N')) {
		TString strValue = strContent.substr(1, strContent.length() - 1);
		// Hexadecimal
		// nValue = _tcstol(strValue.c_str(), NULL, 16);
		// Decimal
		// int nRegulation = _tcstol(strValue.c_str(), NULL, 10);
		RFID_REGULATION emRegulation =
			(RFID_REGULATION)_tcstol(strValue.c_str(), NULL, 10);
		fResult = true;
	}
	return fResult;
}

bool RfidInterface::ParseGetRegulation(const void *lpBuffer, int nBufferLength,
                                       RFID_REGULATION &emRegulation) {
	bool fResult = false;
	TString strContent;
	GetContent(lpBuffer, nBufferLength, strContent);

	RFID_RESPONSE_INFO stResponseInfo;
	if (strContent.at(0) != _T('N')) {
		TStringArray aryContent = m_objTokenizer.Token(strContent, _T('-'));
		// 2020 / 11 / 04 11:18 : 01.271 - Antenna1
		TString strDate;
		TString strAntenna;
		if (aryContent.size() >= 3) {
			strDate = aryContent.at(0);
			strAntenna = aryContent.at(1);
			stResponseInfo.strDate = aryContent.at(0);
			stResponseInfo.usAntenna = 0;
		}

		if (aryContent.size() > 0) {
			for (int nIndex = 0; nIndex < aryContent.size(); nIndex++) {
				// if ((aryContent.at(nIndex).at(0) == _T('N')) &&
				// (aryContent.at(nIndex).at(0) == _T('5')))
				if (aryContent.at(nIndex).at(0) == _T('N')) {
					TString strValue = aryContent.at(nIndex).substr(
						1, aryContent.at(nIndex).length() - 1);
					// Decimal
					// int nRegulation = _tcstol(strValue.c_str(), NULL, 10);
					emRegulation = (RFID_REGULATION)_tcstol(strValue.c_str(), NULL, 10);
					fResult = true;
					break;
				}
			}
		} else {
			strContent = aryContent.at(0);
		}
	} else if (strContent.at(0) == _T('N')) {
		TString strValue = strContent.substr(1, strContent.length() - 1);
		// Hexadecimal
		// nValue = _tcstol(strValue.c_str(), NULL, 16);
		// Decimal
		// int nRegulation = _tcstol(strValue.c_str(), NULL, 10);
		emRegulation = (RFID_REGULATION)_tcstol(strValue.c_str(), NULL, 10);
		fResult = true;
	}
	return fResult;
}

// bool RfidInterface::ParseRegulation(const void* lpBuffer, int nBufferLength,
// RFID_REGULATION &emRegulation)
//{
//	return false;
//}

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================
bool RfidInterface::ParseGetPower(const void *lpBuffer, int nBufferLength,
                                  int &nValue) {
	bool fResult = false;
	TString strContent;
	GetContent(lpBuffer, nBufferLength, strContent);

	RFID_RESPONSE_INFO stResponseInfo;
	if (strContent.at(0) != _T('N')) {
		TStringArray aryContent = m_objTokenizer.Token(strContent, _T('-'));
		// 2020 / 11 / 04 11:18 : 01.271 - Antenna1
		TString strDate;
		TString strAntenna;
		if (aryContent.size() >= 3) {
			strDate = aryContent.at(0);
			strAntenna = aryContent.at(1);
			stResponseInfo.strDate = aryContent.at(0);
			stResponseInfo.usAntenna = 0;
		}

		if (aryContent.size() > 0) {
			for (int nIndex = 0; nIndex < aryContent.size(); nIndex++) {
				if (aryContent.at(nIndex).at(0) == _T('N')) {
					TString strValue = aryContent.at(nIndex).substr(
						1, aryContent.at(nIndex).length() - 1);
					// Hexadecimal
					nValue = _tcstol(strValue.c_str(), NULL, 16);
					fResult = true;
					break;
				}
			}
		} else {
			strContent = aryContent.at(0);
		}
	} else {
		TString strValue = strContent.substr(1, strContent.length() - 1);
		// Hexadecimal
		nValue = _tcstol(strValue.c_str(), NULL, 16);
		fResult = true;
	}
	return fResult;
}

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================
bool RfidInterface::ParseSetPower(const void *lpBuffer, int nBufferLength,
                                  int *pnResult) {
	bool fResult = false;
	TString strContent;
	GetContent(lpBuffer, nBufferLength, strContent);

	RFID_RESPONSE_INFO stResponseInfo;
	if (strContent.at(0) != _T('N')) {
		TStringArray aryContent = m_objTokenizer.Token(strContent, _T('-'));
		// 2020 / 11 / 04 11:18 : 01.271 - Antenna1
		TString strDate;
		TString strAntenna;
		if (aryContent.size() >= 3) {
			strDate = aryContent.at(0);
			strAntenna = aryContent.at(1);
			stResponseInfo.strDate = aryContent.at(0);
			stResponseInfo.usAntenna = 0;
		}

		if (aryContent.size() > 0) {
			for (int nIndex = 0; nIndex < aryContent.size(); nIndex++) {
				if (aryContent.at(nIndex).at(0) == _T('N')) {
					if (pnResult != NULL) {
						TString strValue = aryContent.at(nIndex).substr(
							1, aryContent.at(nIndex).length() - 1);
						// Hexadecimal
						*pnResult = _tcstol(strValue.c_str(), NULL, 16);
					}
					fResult = true;
					break;
				}
			}
		} else {
			strContent = aryContent.at(0);
		}
	} else {
		if (pnResult != NULL) {
			TString strValue = strContent.substr(1, strContent.length() - 1);
			// Hexadecimal
			*pnResult = _tcstol(strValue.c_str(), NULL, 16);
		}
		fResult = true;
	}
	return fResult;
}

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================
bool RfidInterface::ParseGetSingleAntenna(const void *lpBuffer,
                                          int nBufferLength,
                                          unsigned int &uiAntenna, bool &fHub) {
	bool fResult = false;
	TString strContent;
	GetContent(lpBuffer, nBufferLength, strContent);
	fResult = ParseAntenna(strContent.c_str(), uiAntenna, fHub);
	return fResult;
}

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================
bool RfidInterface::ParseSetSingleAntenna(const void *lpBuffer,
                                          int nBufferLength,
                                          unsigned int *puiAntenna,
                                          bool *pfHub) {
	bool fResult = false;
	unsigned int uiAntenna = 0;
	bool fHub = false;
	TString strContent;
	TString strKeyword = _T("Antenna");
	TString strSetError = _T("SetError");
	GetContent(lpBuffer, nBufferLength, strContent);

	if (strContent.compare(0, strKeyword.length(), strKeyword) == 0) {
		TString strValue = strContent.substr(
			strKeyword.length(), strContent.length() - strKeyword.length());
		if (strValue.compare(0, strSetError.length(), strSetError) != 0) {
			fResult = ParseAntenna(strContent.c_str(), uiAntenna, fHub);
			if (fResult == true) {
				if (puiAntenna)
					*puiAntenna = uiAntenna;
				if (pfHub)
					*pfHub = fHub;
			}
		}
	}
	return fResult;
}

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================
bool RfidInterface::ParseSetLoopAntenna(const void *lpBuffer, int nBufferLength,
                                        unsigned int *pnResult) {
	bool fResult = false;
	TString strContent;
	TString strKeyword = _T("LoopAntenna");
	TString strSetError = _T("SetError");
	GetContent(lpBuffer, nBufferLength, strContent);

	if (strContent.compare(0, strKeyword.length(), strKeyword) == 0) {
		TString strValue = strContent.substr(
			strKeyword.length(), strContent.length() - strKeyword.length());
		if (strValue.compare(0, strSetError.length(), strSetError) != 0) {
			// Hexadecimal
			unsigned int uiAntennas = _tcstoul(strValue.c_str(), NULL, 16);
			if (pnResult)
				*pnResult = uiAntennas;
			fResult = true;
		} else
			fResult = false;
	}
	return fResult;
}

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================
bool RfidInterface::ParseGetLoopAntenna(const void *lpBuffer, int nBufferLength,
                                        unsigned int &uiAntennas) {
	bool fResult = false;
	TString strContent;
	TString strKeyword = _T("LoopAntenna");
	TString strGetError = _T("GetError");
	GetContent(lpBuffer, nBufferLength, strContent);

	if (strContent.compare(0, strKeyword.length(), strKeyword) == 0) {
		TString strValue = strContent.substr(
			strKeyword.length(), strContent.length() - strKeyword.length());
		if (strValue.compare(0, strGetError.length(), strGetError) != 0) {
			// Hexadecimal
			unsigned int uiAntennas = _tcstoul(strValue.c_str(), NULL, 16);
			fResult = true;
		} else
			fResult = false;
	}
	return fResult;
}

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================
bool RfidInterface::ParseSetLoopTime(const void *lpBuffer, int nBufferLength,
                                     unsigned int *pnResult) {
	bool fResult = false;
	TString strContent;
	TString strKeyword = _T("LoopTime");
	TString strSetError = _T("SetError");
	GetContent(lpBuffer, nBufferLength, strContent);

	if (strContent.compare(0, strKeyword.length(), strKeyword) == 0) {
		TString strValue = strContent.substr(
			strKeyword.length(), strContent.length() - strKeyword.length());
		if (strValue.compare(0, strSetError.length(), strSetError) != 0) {
			// Decimal
			unsigned int uiLoopTime = _tcstoul(strValue.c_str(), NULL, 10);
			if (pnResult)
				*pnResult = uiLoopTime;
			fResult = true;
		} else
			fResult = false;
	}
	return fResult;
}

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================
bool RfidInterface::ParseGetLoopTime(const void *lpBuffer, int nBufferLength,
                                     unsigned int &uiLoopTime) {
	bool fResult = false;
	TString strContent;
	TString strKeyword = _T("LoopTime");
	TString strGetError = _T("GetError");
	GetContent(lpBuffer, nBufferLength, strContent);

	if (strContent.compare(0, strKeyword.length(), strKeyword) == 0) {
		TString strValue = strContent.substr(
			strKeyword.length(), strContent.length() - strKeyword.length());
		if (strValue.compare(0, strGetError.length(), strGetError) != 0) {
			// Decimal
			uiLoopTime = _tcstoul(strValue.c_str(), NULL, 10);
			fResult = true;
		} else
			fResult = false;
	}
	return fResult;
}

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      : ** tm_isdst ** : Millisecond after second  (0 - 999).
//==============================================================================
bool RfidInterface::ParseSetTime(const void *lpBuffer, int nBufferLength,
                                 struct tm *pstTime) {
	bool fResult = false;
	TString strContent;
	GetContent(lpBuffer, nBufferLength, strContent);
	// TString strKeyword = _T("SETDATE");
	// TString strSetError = _T("SetError");
	GetContent(lpBuffer, nBufferLength, strContent);
	struct tm stTime;
	fResult = ParseDateTime(strContent.c_str(), stTime);
	if (fResult && (pstTime != NULL))
		*pstTime = stTime;

	return fResult;
}

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      : ** tm_isdst ** : Millisecond after second  (0 - 999).
//              : tm_sec	Seconds after minute (0 - 59).
//              : tm_min	Minutes after hour(0 - 59).
//              : tm_hour	Hours since midnight(0 - 23).
//              : tm_mday	Day of month(1 - 31).
//              : tm_mon	Month(0 - 11; January = 0).
//              : tm_year	Year(current year minus 1900).
//              : tm_wday	Day of week(0 - 6; Sunday = 0).
//              : tm_yday	Day of year(0 - 365; January 1 = 0).
//              : tm_isdst	Positive value if daylight saving time is in
//              effect; 0 if daylight saving time is not in effect; negative
//              value if status of daylight saving time is unknown.
//
//              : int tm_sec;   // seconds after the minute - [0, 60] including
//              leap second : int tm_min;   // minutes after the hour - [0, 59]
//              : int tm_hour;  // hours since midnight - [0, 23]
//              : int tm_mday;  // day of the month - [1, 31]
//              : int tm_mon;   // months since January - [0, 11]
//              : int tm_year;  // years since 1900
//              : int tm_wday;  // days since Sunday - [0, 6]
//              : int tm_yday;  // days since January 1 - [0, 365]
//              : int tm_isdst; // daylight savings time flag
//==============================================================================
bool RfidInterface::ParseDateTime(LPCTSTR lpszDataTime, struct tm &stDateTime) {
	bool fResult = false;
	if (lpszDataTime == NULL)
		return fResult;
	stDateTime.tm_wday = 0;
	stDateTime.tm_yday = 0;
	stDateTime.tm_isdst = 0;
	TStringArray aryDateTime = m_objTokenizer.Token(lpszDataTime, _T(" "));
	if (aryDateTime.size() == 2) {
		TStringArray aryDate =
			m_objTokenizer.Token(aryDateTime.at(0).c_str(), _T("/"));
		if (aryDate.size() == 3) {
			stDateTime.tm_year = _tcstol(aryDate.at(0).c_str(), NULL, 10);
			stDateTime.tm_mon = _tcstol(aryDate.at(1).c_str(), NULL, 10);
			stDateTime.tm_mday = _tcstol(aryDate.at(2).c_str(), NULL, 10);
		}
		TStringArray aryTime =
			m_objTokenizer.Token(aryDateTime.at(1).c_str(), _T(":"));
		if (aryTime.size() == 3) {
			stDateTime.tm_hour = _tcstol(aryTime.at(0).c_str(), NULL, 10);
			stDateTime.tm_min = _tcstol(aryTime.at(1).c_str(), NULL, 10);
			TStringArray arySecond =
				m_objTokenizer.Token(aryTime.at(2).c_str(), _T("."));
			if (arySecond.size() == 2) {
				stDateTime.tm_sec = _tcstol(arySecond.at(0).c_str(), NULL, 10);
				unsigned int uiMillisecond = _tcstol(arySecond.at(1).c_str(), NULL, 10);
				stDateTime.tm_isdst = uiMillisecond;
			} else {
				stDateTime.tm_sec = _tcstol(aryTime.at(2).c_str(), NULL, 10);
			}
		}
		stDateTime.tm_year -= 1900; // years since 1900
		stDateTime.tm_mon -= 1;     // months since January - [0, 11]
		fResult = true;
	}
	return fResult;
}

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      : ** tm_isdst ** : Millisecond after second  (0 - 999).
//==============================================================================
bool RfidInterface::ParseGetTime(const void *lpBuffer, int nBufferLength,
                                 struct tm &stTime) {
	bool fResult = false;
	TString strContent;
	GetContent(lpBuffer, nBufferLength, strContent);
	fResult = ParseDateTime(strContent.c_str(), stTime);
	return fResult;
}

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================
bool RfidInterface::ParseAntenna(LPCTSTR lpszAntenna, unsigned int &uiAntenna) {
	bool fResult = false;

	TString strAntenna = lpszAntenna;
	TString strKeyword = TEXT_RFID_ANTENNA;
	if ((strAntenna.compare(0, strKeyword.length(), strKeyword) == 0) &&
	    (strAntenna.length() > strKeyword.length())) {
		TString strValue = strAntenna.substr(
			strKeyword.length(), strAntenna.length() - strKeyword.length());
		// Decimal
		uiAntenna = _tcstol(strValue.c_str(), NULL, 10);
		fResult = true;
	}
	return fResult;
}

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================
bool RfidInterface::ParseAntenna(LPCTSTR lpszAntenna, unsigned int &uiAntenna,
                                 bool &fHub) {
	bool fResult = false;

	TString strAntenna = lpszAntenna;
	TString strKeyword = TEXT_RFID_ANTENNA;
	if ((strAntenna.compare(0, strKeyword.length(), strKeyword) == 0) &&
	    (strAntenna.length() > strKeyword.length())) {
		TString strValue = strAntenna.substr(
			strKeyword.length(), strAntenna.length() - strKeyword.length());
		// Decimal
		uiAntenna = _tcstol(strValue.c_str(), NULL, 10);
		if ((uiAntenna > 4) || (strAntenna.at(strKeyword.length()) == _T('0')))
			fHub = true;
		fResult = true;
	}
	return fResult;
}

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================
unsigned int RfidInterface::ParseErrorCode(TCHAR szResponseCode) {
	unsigned int uiErrorCode = ERROR_RFID_SUCCESS;
	switch (szResponseCode) {
		case _T('0'):
			uiErrorCode = ERROR_RFID_UNKNOW;
			break;
		case _T('3'):
			uiErrorCode = ERROR_RFID_MEMORY_OVERRUN;
			break;
		case _T('4'):
			uiErrorCode = ERROR_RFID_MEMORY_LOCKED;
			break;
		case _T('B'):
			uiErrorCode = ERROR_RFID_INSUFFICIENT_POWER;
			break;
		case _T('F'):
			uiErrorCode = ERROR_RFID_NON_SPECIFIC;
			break;
		default:
			break;
	}
	return uiErrorCode;
}

//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================
bool RfidInterface::ParseReadBank(const void *lpBuffer, int nBufferLength,
                                  RFID_TAG_DATA &stTagData,
                                  unsigned int *puiErrorCode) {
	// Send: <LF>R[X],[X],[X]<CR>  <== 0x0A 0x40 0x4E 0x35 0x2C 0x30 0x32 0x0D
	// Send: <LF>@R[X],[X],[X]<CR>  <== 0x0A 0x40 0x4E 0x35 0x2C 0x30 0x32 0x0D
	// Recv: <LF>@2020/11/10 17:29:27.544-Antennea1-R<CR><LF>
	//     DateTime: 2020/11/10 17:29:27.544
	//     Antennea: 1
	//     Data: None
	// Recv: <LF>@2020/11/10
	// 17:29:27.544-Antennea1-RE28011606000020D6842CABF<CR><LF>
	//     DateTime: 2020/11/10 17:29:27.544
	//     Antennea: 1
	//     Data: E28011606000020D6842CABF

	bool fResult = false;
	TString strContent;
	GetContent(lpBuffer, nBufferLength, strContent);
	unsigned int uiErrorCode = ERROR_RFID_SUCCESS;
	stTagData.strData.c_str();
	stTagData.uiAntenna = 0;

	// Ex. 2020/11/04 11:18:01.271-Antenna1-R[DATA]
	// Ex. 2020/11/04 11:18:01.271-Antenna1-3	// 3: ERROR_CODE_MEMORY_OVERRUN
	TStringArray aryContent = m_objTokenizer.Token(strContent, _T('-'));

	if (aryContent.size() == 3) {
		// TString strDate;
		TString strAntenna;
		TString strData;
		stTagData.strTime = aryContent.at(0);
		strAntenna = aryContent.at(1);
		strData = aryContent.at(2);
		unsigned int uiAntenna;
		if (ParseAntenna(strAntenna.c_str(), uiAntenna))
			stTagData.uiAntenna = uiAntenna;

		if (strData.at(0) == _T(CMD_RFID_READ_BANK)) {
			stTagData.strData = strData.substr(1, strData.length() - 1);
			fResult = true;
		} else {
			uiErrorCode = ParseErrorCode(strData.at(1));
		}
	}

	if (puiErrorCode)
		*puiErrorCode = uiErrorCode;
	return fResult;
}
//==============================================================================
// Function     :
// Purpose      :
// Description	:
// Editor       : Richard Chung
// Update Date	: 2020-11-03
// -----------------------------------------------------------------------------
// Parameters   :
//         [in] :
//              :
//         [in] :
//              :
//         [in] :
//              :
// Return       : True if the function is successful; otherwise false.
// Remarks      :
//==============================================================================
#if 0
bool RfidInterface::ParseReadTag(const void* lpBuffer, int nBufferLength, RFID_TAG_DATA &stTagData, unsigned int * puiErrorCode)
{
	// Send: <LF>R[X],[X],[X]<CR>  <== 0x0A 0x40 0x4E 0x35 0x2C 0x30 0x32 0x0D
	// Send: <LF>@R[X],[X],[X]<CR>  <== 0x0A 0x40 0x4E 0x35 0x2C 0x30 0x32 0x0D
	// Recv: <LF>@2020/11/10 17:29:27.544-Antennea1-R<CR><LF>
	//     DateTime: 2020/11/10 17:29:27.544
	//     Antennea: 1
	//     Data: None
	// Recv: <LF>@2020/11/10 17:29:27.544-Antennea1-RE28011606000020D6842CABF<CR><LF>
	//     DateTime: 2020/11/10 17:29:27.544
	//     Antennea: 1
	//     Data: E28011606000020D6842CABF

#if 1
	bool fResult = false;
	TString strContent;
	GetContent(lpBuffer, nBufferLength, strContent);
	unsigned int uiErrorCode = ERROR_RFID_SUCCESS;
	stTagData.strData.c_str();
	stTagData.uiAntenna = 0;

	// Ex. 2020/11/04 11:18:01.271-Antenna1-R[DATA]
	// Ex. 2020/11/04 11:18:01.271-Antenna1-3	// 3: ERROR_CODE_MEMORY_OVERRUN
	TStringArray aryContent = m_objTokenizer.Token(strContent, _T('-'));

	if (aryContent.size() == 3)
	{
		//TString strDate;
		TString strData;
		TString strAntenna;
		stTagData.strTime = aryContent.at(0);
		strAntenna = aryContent.at(1);
		strData = aryContent.at(2);
		unsigned int uiAntenna;
		if (ParseAntenna(strAntenna.c_str(), uiAntenna))
			stTagData.uiAntenna = uiAntenna;

		if (strData.at(0) == _T(CMD_RFID_READ_TAG))
		{
			stTagData.strData = strData.substr(1, strData.length() - 1);
			fResult = true;
		}
		else
		{
			uiErrorCode = ParseErrorCode(strData.at(1));
		}
	}

	if (puiErrorCode)
		*puiErrorCode = uiErrorCode;
	return fResult;
#else

	bool fResult = false;
	TString strContent;
	GetContent(lpBuffer, nBufferLength, strContent);
	unsigned int uiErrorCode = ERROR_RFID_SUCCESS;
	stTagData.strData.c_str();

	if (strContent.at(0) != _T(CMD_RFID_READ_TAG))
	{
		TStringArray aryContent = m_objTokenizer.Token(strContent, _T('-'));
		//2020/11/04 11:18:01.271-Antenna1-R
		TString strDate;
		TString strAntenna;
		if (aryContent.size() == 3)
		{
			stTagData.strTime = aryContent.at(0);
			strAntenna = aryContent.at(1);
			strDate = aryContent.at(2);
			unsigned int uiAntenna;
			bool fHub;
			if (ParseAntenna(strAntenna.c_str(), uiAntenna, fHub))
				stTagData.uiAntenna = uiAntenna;
		}

		if (aryContent.size() > 0)
		{
			for (int nIndex = 0; nIndex < aryContent.size(); nIndex++)
			{
				if (aryContent.at(nIndex).at(0) == _T(CMD_RFID_READ_TAG))
				{
					if (aryContent.at(nIndex).length() == 2)
					{
						uiErrorCode = ParseErrorCode(aryContent.at(nIndex).at(1));
						fResult = false;
						break;
					}
					else if (aryContent.at(nIndex).length() > 1)
					{
						//TString strData = aryContent.at(nIndex).substr(1, aryContent.at(nIndex).length() - 1);
						stTagData.strData = aryContent.at(nIndex).substr(1, aryContent.at(nIndex).length() - 1);
					}
					fResult = true;
					break;
				}
			}
		}
		else
		{
			strContent = aryContent.at(0);
		}
	}
	else if (strContent.at(0) == _T(CMD_RFID_READ_TAG))
	{
		if (strContent.length() > 1)
		{
			stTagData.strData = strContent.substr(1, strContent.length() - 1);
		}
		fResult = true;
	}
	if (puiErrorCode)
		*puiErrorCode = ERROR_RFID_UNKNOW;
	return fResult;
#endif
}
#endif

// bool ParseReadSingleEPC(const void* lpBuffer, int nBufferLength,
// RFID_TAG_DATA &stTagData, RFID_TAG_EPC *pstTagEPC = NULL, unsigned int *
// puiErrorCode = NULL);
bool RfidInterface::ParseReadSingleEPC(const void *lpBuffer, int nBufferLength,
                                       RFID_TAG_DATA &stTagData,
                                       RFID_TAG_EPC *pstTagEPC,
                                       unsigned int *puiErrorCode) {
	// Q   : display tag EPC ID
	// Send: Q
	// Recv: Q<none or EPC>
	//		  <none or EPC>
	// none: no tag in RF field
	// EPC : PC + EPC + CRC16
	//
	// Send: <LF>Q<CR>  <== 0x0A 0x51 0x0D
	// Send: <LF>@Q<CR>  <== 0x0A 0x40 0x4E 0x0D
	// Recv: <LF>@2020/11/16 20:36:55.617-Antenna1-Q[]<CR><LF> <== 0a 40 32 30 32
	// ... 41 37 44 0d 0a
	//     DateTime: 2020/11/10 17:29:27.544
	//     Antennea: 1
	//     Data: None
	// Recv: <LF>@2020/11/16
	// 20:36:55.617-Antenna1-Q3000E28011606000020D6843006FBA7D<CR><LF>
	//     DateTime: 2020/11/10 17:29:27.544
	//     Antennea: 1
	//     Data: 3000E28011606000020D6843006FBA7D
	//         PC: 3000
	//         EPC: E28011606000020D6843006F
	//         CRC16: BA7D

	//---------------------------------------------------------------------------
	// Read SingleTagEPC
	//---------------------------------------------------------------------------
	//	0000   00 24 77 56 4f bc 04 92 26 58 dc 63 08 00 45 00.$wVO...&X.c..E.
	//	0010   00 2c 55 80 00 00 80 06 00 00 c0 a8 ec 1c c0 a8   .,
	// U............. 	0020   ec 5b c8 92 03 e9 65 67 61 f4 f6 82 c0 01 50 18
	//.[....ega.....P. 	0030   ff 70 b0 e4 00 00 0a 40 51 0d .p.....@Q.

	//	0000   04 92 26 58 dc 63 00 24 77 56 4f bc 08 00 45 00 ..&X.c.$wVO...E.
	//	0010   00 6e 3e 52 40 00 ff 06 e3 6d c0 a8 ec 5b c0 a8.n>R@....m...[..
	//	0020   ec 1c 03 e9 c8 92 f6 82 c0 01 65 67 61 f8 50 18 ..........ega.P.
	//	0030   00 ef 90 a1 00 00 0a 40 32 30 32 30 2f 31 31 2f   .......@2020/11
	/// 	0040   31 36 20 32 30 3a 33 36 3a 35 35 2e 36 31 37 2d   16 20:36
	/// : 55.617
	//- 	0050   41 6e 74 65 6e 6e 61 31 2d 51 33 30 30 30 45 32   Antenna1 -
	// Q3000E2 	0060   38 30 31 31 36 30 36 30 30 30 30 32 30 44 36 38
	// 8011606000020D68 	0070   34 33 30 30 36 46 42 41 37 44 0d 0a 43006FBA7D..

	bool fResult = false;
	TString strContent;
	GetContent(lpBuffer, nBufferLength, strContent);
	unsigned int uiErrorCode = ERROR_RFID_SUCCESS;
	stTagData.strData.c_str();
	stTagData.strEPC.c_str();
	stTagData.strTID.c_str();
	stTagData.uiAntenna = 0;

	// 2020/11/04 11:18:01.271-Antenna1-Q
	TStringArray aryContent = m_objTokenizer.Token(strContent, _T('-'));

	if (aryContent.size() == 3) {
		// TString strDate;
		TString strAntenna;
		TString strData;
		unsigned int uiAntenna;
		stTagData.strTime = aryContent.at(0);
		strAntenna = aryContent.at(1);
		strData = aryContent.at(2);
		if (ParseAntenna(strAntenna.c_str(), uiAntenna))
			stTagData.uiAntenna = uiAntenna;

		if ((strData.at(0) == _T(CMD_RFID_READ_SINGLE_EPC)) &&
		    (strData.length() > 1)) {
			// stTagData.strEPC = aryTag.at(nDataIndex);
			// Response EPC Content: PC(4 Bytes) + EPC + CRC16(4 Bytes)
			// stTagData.strData = aryTag.at(nDataIndex).substr(1,
			// aryTag.at(nDataIndex).length() - 1);
			stTagData.strEPC = strData.substr(
				1 + LEN_EPC_PEOTOCOL,
				strData.length() - (1 + LEN_EPC_PEOTOCOL + LEN_EPC_CRC16));
			if (pstTagEPC) {
				TString strPC = strData.substr(1, LEN_EPC_PEOTOCOL);
				TString strCRC =
					strData.substr(strData.length() - LEN_EPC_CRC16, LEN_EPC_CRC16);
				pstTagEPC->strRaw = strData.substr(1, strData.length() - 1);
				pstTagEPC->uiProtocolControl = _tcstol(strPC.c_str(), NULL, 16);
				pstTagEPC->strEPC = stTagData.strEPC;
				pstTagEPC->uiCRC16 = _tcstol(strCRC.c_str(), NULL, 16);
			}
			fResult |= true;
		} else {
			uiErrorCode = ParseErrorCode(strData.at(0));
		}
	}

	if (puiErrorCode)
		*puiErrorCode = uiErrorCode;
	return fResult;
}

bool RfidInterface::ParseReadEPC(const void *lpBuffer, int nBufferLength,
                                 RFID_TAG_DATA &stTagData,
                                 unsigned int *puiErrorCode) {
	// Send: <LF>R[X],[X],[X]<CR>  <== 0x0A 0x40 0x4E 0x35 0x2C 0x30 0x32 0x0D
	// Send: <LF>@R[X],[X],[X]<CR>  <== 0x0A 0x40 0x4E 0x35 0x2C 0x30 0x32 0x0D
	// Recv: <LF>@2020/11/10 17:29:27.544-Antennea1-R<CR><LF>
	//     DateTime: 2020/11/10 17:29:27.544
	//     Antennea: 1
	//     Data: None
	// Recv: <LF>@2020/11/10
	// 17:29:27.544-Antennea1-RE28011606000020D6842CABF<CR><LF>
	//     DateTime: 2020/11/10 17:29:27.544
	//     Antennea: 1
	//     Data: E28011606000020D6842CABF

#if 1
	bool fResult = false;
	TString strContent;
	GetContent(lpBuffer, nBufferLength, strContent);
	unsigned int uiErrorCode = ERROR_RFID_SUCCESS;
	stTagData.strData.c_str();
	stTagData.uiAntenna = 0;

	// Ex. 2020/11/04 11:18:01.271-Antenna1-R[DATA]
	// Ex. 2020/11/04 11:18:01.271-Antenna1-3	// 3: ERROR_CODE_MEMORY_OVERRUN
	TStringArray aryContent = m_objTokenizer.Token(strContent, _T('-'));

	if (aryContent.size() == 3) {
		// TString strDate;
		TString strAntenna;
		TString strData;
		stTagData.strTime = aryContent.at(0);
		strAntenna = aryContent.at(1);
		strData = aryContent.at(2);
		unsigned int uiAntenna;
		if (ParseAntenna(strAntenna.c_str(), uiAntenna))
			stTagData.uiAntenna = uiAntenna;

		if (strData.at(0) == _T(CMD_RFID_READ_BANK)) {
			stTagData.strData = strData.substr(1, strData.length() - 1);
			stTagData.strEPC = strData.substr(1, strData.length() - 1);
			fResult = true;
		} else {
			uiErrorCode = ParseErrorCode(strData.at(1));
		}
	}

	if (puiErrorCode)
		*puiErrorCode = uiErrorCode;
	return fResult;
#else
	bool fResult = false;
	TString strContent;
	GetContent(lpBuffer, nBufferLength, strContent);
	unsigned int uiErrorCode = ERROR_RFID_SUCCESS;
	stTagData.strData.c_str();

	if (strContent.at(0) != _T(CMD_RFID_READ_TAG)) {
		TStringArray aryContent = m_objTokenizer.Token(strContent, _T('-'));
		// 2020/11/04 11:18:01.271-Antenna1-R
		TString strDate;
		TString strAntenna;
		if (aryContent.size() == 3) {
			stTagData.strTime = aryContent.at(0);
			strAntenna = aryContent.at(1);
			strDate = aryContent.at(2);
			unsigned int uiAntenna;
			bool fHub;
			if (ParseAntenna(strAntenna.c_str(), uiAntenna, fHub))
				stTagData.uiAntenna = uiAntenna;
		}

		if (aryContent.size() > 0) {
			for (int nIndex = 0; nIndex < aryContent.size(); nIndex++) {
				if (aryContent.at(nIndex).at(0) == _T(CMD_RFID_READ_TAG)) {
					if (aryContent.at(nIndex).length() == 2) {
						uiErrorCode = ParseErrorCode(aryContent.at(nIndex).at(1));
						fResult = false;
						break;
						// return fResult;
					} else if (aryContent.at(nIndex).length() > 1) {
						// TString strData = aryContent.at(nIndex).substr(1,
						// aryContent.at(nIndex).length() - 1);
						stTagData.strData = aryContent.at(nIndex).substr(
							1, aryContent.at(nIndex).length() - 1);
					} else
						stTagData.strData.clear();
					fResult = true;
					break;
				}
			}
		} else {
			strContent = aryContent.at(0);
		}
	} else if (strContent.at(0) == _T('R')) {
		if (strContent.length() > 1) {
			stTagData.strData = strContent.substr(1, strContent.length() - 1);
		} else
			stTagData.strData.clear();
		fResult = true;
	}
	return fResult;
#endif
}

bool RfidInterface::ParseReadTID(const void *lpBuffer, int nBufferLength,
                                 RFID_TAG_DATA &stTagData,
                                 unsigned int *puiErrorCode) {
	// Send: <LF>R[X],[X],[X]<CR>  <== 0x0A 0x40 0x4E 0x35 0x2C 0x30 0x32 0x0D
	// Send: <LF>@R[X],[X],[X]<CR>  <== 0x0A 0x40 0x4E 0x35 0x2C 0x30 0x32 0x0D
	// Recv: <LF>@2020/11/10 17:29:27.544-Antennea1-R<CR><LF>
	//     DateTime: 2020/11/10 17:29:27.544
	//     Antennea: 1
	//     Data: None
	// Recv: <LF>@2020/11/10
	// 17:29:27.544-Antennea1-RE28011606000020D6842CABF<CR><LF>
	//     DateTime: 2020/11/10 17:29:27.544
	//     Antennea: 1
	//     Data: E28011606000020D6842CABF

#if 1
	bool fResult = false;
	TString strContent;
	GetContent(lpBuffer, nBufferLength, strContent);
	unsigned int uiErrorCode = ERROR_RFID_SUCCESS;
	stTagData.strData.c_str();
	stTagData.uiAntenna = 0;

	// Ex. 2020/11/04 11:18:01.271-Antenna1-R[DATA]
	// Ex. 2020/11/04 11:18:01.271-Antenna1-3	// 3: ERROR_CODE_MEMORY_OVERRUN
	TStringArray aryContent = m_objTokenizer.Token(strContent, _T('-'));

	if (aryContent.size() == 3) {
		// TString strDate;
		TString strAntenna;
		TString strData;
		stTagData.strTime = aryContent.at(0);
		strAntenna = aryContent.at(1);
		strData = aryContent.at(2);
		unsigned int uiAntenna;
		if (ParseAntenna(strAntenna.c_str(), uiAntenna))
			stTagData.uiAntenna = uiAntenna;

		if (strData.at(0) == _T(CMD_RFID_READ_BANK)) {
			stTagData.strData = strData.substr(1, strData.length() - 1);
			stTagData.strTID = strData.substr(1, strData.length() - 1);
			// stTagData.strEPC = strData.substr(1, strData.length() - 1);
			fResult = true;
		} else {
			uiErrorCode = ParseErrorCode(strData.at(1));
		}
	}

	if (puiErrorCode)
		*puiErrorCode = uiErrorCode;
	return fResult;
#else
	bool fResult = false;
	TString strContent;
	GetContent(lpBuffer, nBufferLength, strContent);
	unsigned int uiErrorCode = ERROR_RFID_SUCCESS;
	stTagData.strData.c_str();

	if (strContent.at(0) != _T(CMD_RFID_READ_TAG)) {
		TStringArray aryContent = m_objTokenizer.Token(strContent, _T('-'));
		// 2020/11/04 11:18:01.271-Antenna1-R
		TString strDate;
		TString strAntenna;
		if (aryContent.size() == 3) {
			stTagData.strTime = aryContent.at(0);
			strAntenna = aryContent.at(1);
			strDate = aryContent.at(2);
			unsigned int uiAntenna;
			bool fHub;
			if (ParseAntenna(strAntenna.c_str(), uiAntenna, fHub))
				stTagData.uiAntenna = uiAntenna;
		}

		if (aryContent.size() > 0) {
			for (int nIndex = 0; nIndex < aryContent.size(); nIndex++) {
				if (aryContent.at(nIndex).at(0) == _T(CMD_RFID_READ_TAG)) {
					if (aryContent.at(nIndex).length() == 2) {
						uiErrorCode = ParseErrorCode(aryContent.at(nIndex).at(1));

						fResult = false;
						break;
						// return fResult;
					} else if (aryContent.at(nIndex).length() > 1) {
						// TString strData = aryContent.at(nIndex).substr(1,
						// aryContent.at(nIndex).length() - 1);
						stTagData.strData = aryContent.at(nIndex).substr(
							1, aryContent.at(nIndex).length() - 1);
					} else
						stTagData.strData.clear();
					fResult = true;
					break;
				}
			}
		} else {
			strContent = aryContent.at(0);
		}
	} else if (strContent.at(0) == _T(CMD_RFID_READ_TAG)) {
		if (strContent.length() > 1) {
			stTagData.strData = strContent.substr(1, strContent.length() - 1);
		} else
			stTagData.strData.clear();
		fResult = true;
	}
	return fResult;
#endif
}
bool RfidInterface::ParseReadUserData(const void *lpBuffer, int nBufferLength,
                                      RFID_TAG_DATA &stTagData,
                                      unsigned int *puiErrorCode) {
	// Send: <LF>R[X],[X],[X]<CR>  <== 0x0A 0x40 0x4E 0x35 0x2C 0x30 0x32 0x0D
	// Send: <LF>@R[X],[X],[X]<CR>  <== 0x0A 0x40 0x4E 0x35 0x2C 0x30 0x32 0x0D
	// Recv: <LF>@2020/11/10 17:29:27.544-Antennea1-R<CR><LF>
	//     DateTime: 2020/11/10 17:29:27.544
	//     Antennea: 1
	//     Data: None
	// Recv: <LF>@2020/11/10
	// 17:29:27.544-Antennea1-RE28011606000020D6842CABF<CR><LF>
	//     DateTime: 2020/11/10 17:29:27.544
	//     Antennea: 1
	//     Data: E28011606000020D6842CABF

#if 1
	bool fResult = false;
	TString strContent;
	GetContent(lpBuffer, nBufferLength, strContent);
	unsigned int uiErrorCode = ERROR_RFID_SUCCESS;
	stTagData.strData.c_str();
	stTagData.uiAntenna = 0;

	// Ex. 2020/11/04 11:18:01.271-Antenna1-R[DATA]
	// Ex. 2020/11/04 11:18:01.271-Antenna1-3	// 3: ERROR_CODE_MEMORY_OVERRUN
	TStringArray aryContent = m_objTokenizer.Token(strContent, _T('-'));

	if (aryContent.size() == 3) {
		// TString strDate;
		TString strAntenna;
		TString strData;
		stTagData.strTime = aryContent.at(0);
		strAntenna = aryContent.at(1);
		strData = aryContent.at(2);
		unsigned int uiAntenna;
		if (ParseAntenna(strAntenna.c_str(), uiAntenna))
			stTagData.uiAntenna = uiAntenna;

		if (strData.at(0) == _T(CMD_RFID_READ_BANK)) {
			stTagData.strData = strData.substr(1, strData.length() - 1);
			// stTagData.strEPC = strData.substr(1, strData.length() - 1);
			fResult = true;
		} else {
			uiErrorCode = ParseErrorCode(strData.at(1));
		}
	}

	if (puiErrorCode)
		*puiErrorCode = uiErrorCode;
	return fResult;
#else

	bool fResult = false;
	TString strContent;
	GetContent(lpBuffer, nBufferLength, strContent);

	unsigned int uiErrorCode = ERROR_RFID_SUCCESS;
	stTagData.strData.c_str();
	if (strContent.at(0) != _T(CMD_RFID_READ_TAG)) {
		TStringArray aryContent = m_objTokenizer.Token(strContent, _T('-'));
		// 2020/11/04 11:18:01.271-Antenna1-R
		TString strDate;
		TString strAntenna;
		if (aryContent.size() == 3) {
			stTagData.strTime = aryContent.at(0);
			strAntenna = aryContent.at(1);
			strDate = aryContent.at(2);
			unsigned int uiAntenna;
			bool fHub;
			if (ParseAntenna(strAntenna.c_str(), uiAntenna, fHub))
				stTagData.uiAntenna = uiAntenna;
		}

		if (aryContent.size() > 0) {
			for (int nIndex = 0; nIndex < aryContent.size(); nIndex++) {
				if (aryContent.at(nIndex).at(0) == _T(CMD_RFID_READ_TAG)) {
					if (aryContent.at(nIndex).length() == 2) {
						uiErrorCode = ParseErrorCode(aryContent.at(nIndex).at(1));
						fResult = false;
						break;
					} else if (aryContent.at(nIndex).length() > 1) {
						// TString strData = aryContent.at(nIndex).substr(1,
						// aryContent.at(nIndex).length() - 1);
						stTagData.strData = aryContent.at(nIndex).substr(
							1, aryContent.at(nIndex).length() - 1);
					} else
						stTagData.strData.clear();
					fResult = true;
					break;
				}
			}
		} else {
			strContent = aryContent.at(0);
		}
	} else if (strContent.at(0) == _T(CMD_RFID_READ_TAG)) {
		if (strContent.length() > 1) {
			stTagData.strData = strContent.substr(1, strContent.length() - 1);
		} else
			stTagData.strData.clear();
		fResult = true;
	}
	return fResult;
#endif
}

bool RfidInterface::ParseReadEPCandTID(const void *lpBuffer, int nBufferLength,
                                       RFID_TAG_DATA &stTagData,
                                       RFID_TAG_EPC *pstTagEPC,
                                       unsigned int *puiErrorCode) {
	// Send: <LF>Q,R[X],[X],[X]<CR>  <== 0x0A 0x40 0x4E 0x35 0x2C 0x30 0x32 0x0D
	// Send: <LF>@QR[X],[X],[X]<CR>  <== 0x0A 0x40 0x4E 0x35 0x2C 0x30 0x32 0x0D
	// ??? Recv: <LF>@2020/11/10 17:29:27.544-Antennea1-Q[EPC],R[TID]<CR><LF>
	//     DateTime: 2020/11/10 17:29:27.544
	//     Antennea: 1
	//     Data: None
	// Recv: <LF>@2020/11/10
	// 17:29:27.544-Antennea1-Q3000E28011606000020D6843006FBA7D,
	// RE28011602000606F086009AD<CR><LF>
	//     DateTime: 2020/11/10 17:29:27.544
	//     Antennea: 1
	//     EPC Data: 3000E28011606000020D6843006FBA7D
	//     EPC     : E28011606000020D6843006F
	//     TID Data: E28011602000606F086009AD

	//  0000   04 92 26 58 dc 63 00 24 77 56 4f bc 08 00 45 00   ..&X.c.$wVO...E.
	//	0010   00 88 0a 3a 40 00 ff 06 17 6c c0 a8 ec 5b c0 a8 ...:@....l...[..
	//	0020   ec 1c 03 e9 c2 ec 6f 11 26 54 7c c1 3b 73 50 18   ......o.&T | .;
	// sP. 	0030   00 ef 0f a8 00 00 0a 40 32 30 32 30 2f 31 31 2f
	// .......@2020/11
	/// 	0040   31 31 20 31 33 3a 33 35 3a 34 36 2e 31 36 37 2d   11 13:35
	/// : 46.167
	//- 	0050   41 6e 74 65 6e 6e 61 31 2d 51 33 30 30 30 45 32   Antenna1 -
	// Q3000E2 	0060   38 30 31 31 36 30 36 30 30 30 30 32 30 44 36 38
	// 8011606000020D68 	0070   34 33 30 30 36 46 42 41 37 44 2c 52 45 32 38 30
	// 43006FBA7D, RE280 	0080   31 31 36 30 32 30 30 30 36 30 36 46 30 38 36 30
	// 11602000606F0860 	0090   30 39 41 44 0d 0a 09AD..

#if 1
	bool fResult = false;
	TString strContent;
	GetContent(lpBuffer, nBufferLength, strContent);
	unsigned int uiErrorCode = ERROR_RFID_SUCCESS;
	stTagData.strData.c_str();
	stTagData.strEPC.c_str();
	stTagData.strTID.c_str();
	stTagData.uiAntenna = 0;

	// 2020/11/04 11:18:01.271-Antenna1-R
	TStringArray aryContent = m_objTokenizer.Token(strContent, _T('-'));

	if (aryContent.size() == 3) {
		// TString strDate;
		TString strAntenna;
		TString strData;
		unsigned int uiAntenna;
		stTagData.strTime = aryContent.at(0);
		strAntenna = aryContent.at(1);
		strData = aryContent.at(2);
		if (ParseAntenna(strAntenna.c_str(), uiAntenna))
			stTagData.uiAntenna = uiAntenna;

		if ((strData.at(0) == _T(CMD_RFID_READ_EPC_WITH_TID)) &&
		    (strData.length() > 1)) {
			TStringArray aryTag = m_objTokenizer.Token(strData, _T(','));
			for (int nDataIndex = 0; nDataIndex < aryTag.size(); nDataIndex++) {
				if (aryTag.at(nDataIndex).at(0) == _T(CMD_RFID_READ_EPC)) {
					// stTagData.strEPC = aryTag.at(nDataIndex);
					// Response EPC Content: PC(4 Bytes) + EPC + CRC16(4 Bytes)
					// stTagData.strData = aryTag.at(nDataIndex).substr(1,
					// aryTag.at(nDataIndex).length() - 1);
					stTagData.strEPC =
						aryTag.at(nDataIndex)
						.substr(1 + LEN_EPC_PEOTOCOL,
							aryTag.at(nDataIndex).length() -
							(1 + LEN_EPC_PEOTOCOL + LEN_EPC_CRC16));
					if (pstTagEPC) {
						TString strPC = aryTag.at(nDataIndex).substr(1, LEN_EPC_PEOTOCOL);
						TString strCRC =
							aryTag.at(nDataIndex)
							.substr(aryTag.at(nDataIndex).length() - LEN_EPC_CRC16,
								LEN_EPC_CRC16);
						pstTagEPC->strRaw =
							aryTag.at(nDataIndex)
							.substr(1, aryTag.at(nDataIndex).length() - 1);
						pstTagEPC->uiProtocolControl = _tcstol(strPC.c_str(), NULL, 16);
						pstTagEPC->strEPC = stTagData.strEPC;
						pstTagEPC->uiCRC16 = _tcstol(strCRC.c_str(), NULL, 16);
					}
					fResult |= true;
				} else if (aryTag.at(nDataIndex).at(0) == _T(CMD_RFID_READ_TID)) {
					// stTagData.strTID = aryTag.at(nDataIndex);
					stTagData.strTID = aryTag.at(nDataIndex)
						.substr(1, aryTag.at(nDataIndex).length() - 1);
					fResult |= true;
				} else {
					uiErrorCode = ParseErrorCode(aryTag.at(nDataIndex).at(0));
					fResult |= false;
				}
			}
		} else {
			uiErrorCode = ParseErrorCode(strData.at(0));
		}
	}

	if (puiErrorCode)
		*puiErrorCode = uiErrorCode;
	return fResult;
#else
	bool fResult = false;
	TString strContent;
	GetContent(lpBuffer, nBufferLength, strContent);
	unsigned int uiErrorCode = ERROR_RFID_SUCCESS;
	stTagData.strData.c_str();

	TStringArray aryContent = m_objTokenizer.Token(strContent, _T('-'));
	// 2020/11/04 11:18:01.271-Antenna1-R
	TString strDate;
	TString strAntenna;

	if (aryContent.size() > 0) {
		if (aryContent.size() == 3) {
			stTagData.strTime = aryContent.at(0);
			strAntenna = aryContent.at(1);
			strDate = aryContent.at(2);
			unsigned int uiAntenna;
			bool fHub;
			if (ParseAntenna(strAntenna.c_str(), uiAntenna, fHub))
				stTagData.uiAntenna = uiAntenna;
		}

		for (int nIndex = 0; nIndex < aryContent.size(); nIndex++) {
			if (aryContent.at(nIndex).at(0) == _T(CMD_RFID_READ_EPC_WITH_TID)) {
				if (aryContent.at(nIndex).length() == 2) {
					uiErrorCode = ParseErrorCode(aryContent.at(nIndex).at(1));
					fResult = false;
					break;
				} else if (aryContent.at(nIndex).length() > 1) {
					TString strData =
						aryContent.at(nIndex); // aryContent.at(nIndex).substr(1,
					// aryContent.at(nIndex).length() - 1);
					TStringArray aryTag = m_objTokenizer.Token(strData, _T(','));
					for (int nDataIndex = 0; nDataIndex < aryTag.size(); nDataIndex++) {
						if (aryTag.at(nDataIndex).at(0) == _T(CMD_RFID_READ_EPC)) {
							// stTagData.strEPC = aryTag.at(nDataIndex);
							stTagData.strEPC =
								aryTag.at(nDataIndex)
								.substr(1, aryTag.at(nDataIndex).length() - 1);
							fResult |= true;
						} else if (aryTag.at(nDataIndex).at(0) == _T(CMD_RFID_READ_TID)) {
							stTagData.strTID = aryTag.at(nDataIndex);
							fResult |= true;
						} else {
							uiErrorCode = ParseErrorCode(aryTag.at(nDataIndex).at(0));
							fResult |= false;
						}
					}
					// stTagData.strData = aryContent.at(nIndex).substr(1,
					// aryContent.at(nIndex).length() - 1);
				} else
					stTagData.strData.clear();
				break;
			}
		}
	}

	if (puiErrorCode)
		*puiErrorCode = uiErrorCode;
	return fResult;
#endif
}

bool RfidInterface::ParseReadEPCandUserData(const void *lpBuffer,
                                            int nBufferLength,
                                            RFID_TAG_DATA &stTagData,
                                            RFID_TAG_EPC *pstTagEPC,
                                            unsigned int *puiErrorCode) {
	// Send: <LF>Q,R3,[X],[X]<CR>  <== 0a 51 2c 52 33 2c 30 2c 32 30 0d <=
	// Q,R3,0,20D Send: <LF>@QR[X],[X],[X]<CR>  <== 0x0A 0x40 0x4E 0x35 0x2C 0x30
	// 0x32 0x0D ??? Recv: <LF>@2020/11/10
	// 17:29:27.544-Antennea1-Q[EPC],R[TID]<CR><LF>
	//     DateTime: 2020/11/10 17:29:27.544
	//     Antennea: 1
	//     Data: None
	// Recv: <LF>@2020/11/10
	// 17:29:27.544-Antennea1-Q3000E28011606000020D6843006FBA7D,
	// RE28011602000606F086009AD<CR><LF>
	//     DateTime: 2020/11/10 17:29:27.544
	//     Antennea: 1
	//     EPC Data: 3000E28011606000020D6843006FBA7D
	//     UserData:

	// 0000   04 92 26 58 dc 63 00 24 77 56 4f bc 08 00 45 00   ..&X.c.$wVO...E.
	//	0010   00 70 0a 4f 40 00 ff 06 17 6f c0 a8 ec 5b c0 a8.p.O@....o...[..
	//	0020   ec 1c 03 e9 ee 74 b9 13 70 e4 40 79 e4 5f 50 18 .....t..p.@y._P.
	//	0030   00 ef 72 a2 00 00 0a 40 32 30 32 30 2f 31 31 2f   ..r....@2020/11
	/// 	0040   31 31 20 31 36 3a 31 39 3a 30 34 2e 30 34 35 2d   11 16:19 :
	/// 04.045
	//- 	0050   41 6e 74 65 6e 6e 61 31 2d 51 33 30 30 30 45 32   Antenna1 -
	// Q3000E2 	0060   38 30 31 31 36 30 36 30 30 30 30 32 30 44 36 38
	// 8011606000020D68 	0070   34 33 30 30 36 46 42 41 37 44 2c 33 0d 0a
	// 43006FBA7D, 3..

#if 1
	bool fResult = false;
	TString strContent;
	GetContent(lpBuffer, nBufferLength, strContent);
	unsigned int uiErrorCode = ERROR_RFID_SUCCESS;
	stTagData.strData.c_str();
	stTagData.strEPC.c_str();
	stTagData.strTID.c_str();
	stTagData.uiAntenna = 0;

	// 2020/11/04 11:18:01.271-Antenna1-R
	TStringArray aryContent = m_objTokenizer.Token(strContent, _T('-'));

	if (aryContent.size() == 3) {
		// TString strDate;
		TString strAntenna;
		TString strData;
		unsigned int uiAntenna;
		stTagData.strTime = aryContent.at(0);
		strAntenna = aryContent.at(1);
		strData = aryContent.at(2);
		if (ParseAntenna(strAntenna.c_str(), uiAntenna))
			stTagData.uiAntenna = uiAntenna;

		if ((strData.at(0) == _T(CMD_RFID_READ_EPC_WITH_TID)) &&
		    (strData.length() > 1)) {
			TStringArray aryTag = m_objTokenizer.Token(strData, _T(','));
			for (int nDataIndex = 0; nDataIndex < aryTag.size(); nDataIndex++) {
				if (aryTag.at(nDataIndex).at(0) == _T(CMD_RFID_READ_EPC)) {
					// stTagData.strData = aryTag.at(nDataIndex).substr(1,
					// aryTag.at(nDataIndex).length() - 1);
					// Response EPC Content: PC(4 Bytes) + EPC + CRC16(4 Bytes)
					stTagData.strEPC =
						aryTag.at(nDataIndex)
						.substr(1 + LEN_EPC_PEOTOCOL,
							aryTag.at(nDataIndex).length() -
							(1 + LEN_EPC_PEOTOCOL + LEN_EPC_CRC16));
					if (pstTagEPC) {
						TString strPC = aryTag.at(nDataIndex).substr(1, LEN_EPC_PEOTOCOL);
						TString strCRC =
							aryTag.at(nDataIndex)
							.substr(aryTag.at(nDataIndex).length() - LEN_EPC_CRC16,
								LEN_EPC_CRC16);
						pstTagEPC->strRaw =
							aryTag.at(nDataIndex)
							.substr(1, aryTag.at(nDataIndex).length() - 1);
						pstTagEPC->uiProtocolControl = _tcstol(strPC.c_str(), NULL, 16);
						pstTagEPC->strEPC = stTagData.strEPC;
						pstTagEPC->uiCRC16 = _tcstol(strCRC.c_str(), NULL, 16);
					}
					fResult |= true;
				} else if (aryTag.at(nDataIndex).at(0) == _T(CMD_RFID_READ_BANK)) {
					// stTagData.strUser = aryTag.at(nDataIndex);
					stTagData.strUser =
						aryTag.at(nDataIndex)
						.substr(1, aryTag.at(nDataIndex).length() - 1);
					fResult |= true;
				} else {
					uiErrorCode = ParseErrorCode(aryTag.at(nDataIndex).at(0));
					fResult |= false;
				}
			}
		} else {
			uiErrorCode = ParseErrorCode(strData.at(0));
		}
	}

	if (puiErrorCode)
		*puiErrorCode = uiErrorCode;
	return fResult;
#else

	bool fResult = false;
	TString strContent;
	GetContent(lpBuffer, nBufferLength, strContent);
	unsigned int uiErrorCode = ERROR_RFID_SUCCESS;
	stTagData.strData.c_str();

	TStringArray aryContent = m_objTokenizer.Token(strContent, _T('-'));
	// 2020/11/04 11:18:01.271-Antenna1-R
	TString strDate;
	TString strAntenna;

	if (aryContent.size() > 0) {
		if (aryContent.size() == 3) {
			stTagData.strTime = aryContent.at(0);
			strAntenna = aryContent.at(1);
			strDate = aryContent.at(2);
			unsigned int uiAntenna;
			bool fHub;
			if (ParseAntenna(strAntenna.c_str(), uiAntenna, fHub))
				stTagData.uiAntenna = uiAntenna;
		}

		for (int nIndex = 0; nIndex < aryContent.size(); nIndex++) {
			if (aryContent.at(nIndex).at(0) == _T(CMD_RFID_READ_EPC_WITH_TID)) {
				if (aryContent.at(nIndex).length() == 2) {
					uiErrorCode = ParseErrorCode(aryContent.at(nIndex).at(1));
					fResult = false;
					break;
					// return fResult;
				} else if (aryContent.at(nIndex).length() > 1) {
					TString strData =
						aryContent.at(nIndex); // aryContent.at(nIndex).substr(1,
					// aryContent.at(nIndex).length() - 1);
					TStringArray aryTag = m_objTokenizer.Token(strData, _T(','));
					for (int nDataIndex = 0; nDataIndex < aryTag.size(); nDataIndex++) {
						if (aryTag.at(nDataIndex).at(0) == _T(CMD_RFID_READ_EPC)) {
							stTagData.strEPC =
								aryTag.at(nDataIndex)
								.substr(1, aryTag.at(nDataIndex).length() - 1);
							fResult |= true;
						} else if (aryTag.at(nDataIndex).at(0) ==
							   _T(CMD_RFID_READ_USER_DATA)) {
							stTagData.strUser =
								aryTag.at(nDataIndex)
								.substr(1, aryTag.at(nDataIndex).length() - 1);
							fResult |= true;
						} else {
							uiErrorCode = ParseErrorCode(aryTag.at(nDataIndex).at(0));
							fResult |= false;
						}
					}
					// stTagData.strData = aryContent.at(nIndex).substr(1,
					// aryContent.at(nIndex).length() - 1);
				} else
					stTagData.strData.clear();
				fResult = true;
				break;
			}
		}
	}

	if (puiErrorCode)
		*puiErrorCode = uiErrorCode;
	return fResult;
#endif
}

bool RfidInterface::ParseReadMultiEPC(const void *lpBuffer, int nBufferLength,
                                      RFID_TAG_DATA &stTagData,
                                      RFID_TAG_EPC *pstTagEPC,
                                      unsigned int *puiErrorCode) {
	// Send: U or U<slot>
	// Recv: U<none or EPC>
	//		<none or EPC>
	// none: no tag in RF field
	// EPC : PC + EPC + CRC16

	//
	// Send: <LF>@U<CR>  <== 0x0A 0x52 0x32 0x2C 0x32 0x2C 0x36 0x0D
	// Recv: <LF>@2020/11/10 16:26:39.338-Antenna2-R<CR><LF> <== 0x0A 0x40 0x32 30
	// 32 30 2f 31 31 2f
	//                                                   31 30 20 31 36 3a 32 36
	//                                                   3a 33 39 2e 33 33 38 2d
	//                                                   41 6e 74 65 6e 6e 61 32
	//                                                   2d 52 0d 0a

	//  Request
	//	0000   00 24 77 56 4f bc 04 92 26 58 dc 63 08 00 45 00.$wVO...&X.c..E.
	//	0010   00 2c 55 bf 00 00 80 06 00 00 c0 a8 ec 1c c0 a8   .,
	// U............. 	0020   ec 5b cc 9c 03 e9 4e 83 cf c3 fc dc ba 01 50 18
	//.[....N.......P. 	0030   ff 70 51 95 00 00 0a 40 55 0d.pQ....@U.

	// Response 1
	//  0000   04 92 26 58 dc 63 00 24 77 56 4f bc 08 00 45 00   ..&X.c.$wVO...E.
	//	0010   00 6e 3e c3 40 00 ff 06 e2 fc c0 a8 ec 5b c0 a8.n>.@........[..
	//	0020   ec 1c 03 e9 cc 9c fc dc ba 01 4e 83 cf c7 50 18 ..........N...P.
	//	0030   00 ef 3e 53 00 00 0a 40 32 30 32 30 2f 31 31 2f   ..>S...@2020/11
	/// 	0040   31 36 20 32 30 3a 35 31 3a 30 34 2e 30 36 33 2d   16 20:51 :
	/// 04.063
	//- 	0050   41 6e 74 65 6e 6e 61 31 2d 55 33 30 30 30 45 32   Antenna1 -
	// U3000E2 	0060   38 30 31 31 36 30 36 30 30 30 30 32 30 44 36 38
	// 8011606000020D68 	0070   34 33 30 30 36 46 42 41 37 44 0d 0a 43006FBA7D..
	//
	// Response 2
	//  0000   04 92 26 58 dc 63 00 24 77 56 4f bc 08 00 45 00   ..&X.c.$wVO...E.
	//  0010   00 6e 3e c4 40 00 ff 06 e2 fb c0 a8 ec 5b c0 a8.n>.@........[..
	//  0020   ec 1c 03 e9 cc 9c fc dc ba 47 4e 83 cf c7 50 18   .........GN...P.
	//	0030   00 ef 19 f8 00 00 0a 40 32 30 32 30 2f 31 31 2f   .......@2020/11
	/// 	0040   31 36 20 32 30 3a 35 31 3a 30 34 2e 30 36 35 2d   16 20:51 :
	/// 04.065
	//- 	0050   41 6e 74 65 6e 6e 61 31 2d 55 33 30 30 30 45 32   Antenna1 -
	// U3000E2 	0060   38 30 31 31 36 30 36 30 30 30 30 32 30 44 36 38
	// 8011606000020D68 	0070   34 32 43 43 46 46 35 44 43 44 0d 0a 42CCFF5DCD..

	//  Response N
	//  0000   04 92 26 58 dc 63 00 24 77 56 4f bc 08 00 45 00   ..&X.c.$wVO...E.
	//	0010   00 4e 3e c6 40 00 ff 06 e3 19 c0 a8 ec 5b c0 a8.N>.@........[..
	//	0020   ec 1c 03 e9 cc 9c fc dc ba d3 4e 83 cf c7 50 18 ..........N...P.
	//	0030   00 ef 97 00 00 00 0a 40 32 30 32 30 2f 31 31 2f   .......@2020/11
	/// 	0040   31 36 20 32 30 3a 35 31 3a 30 34 2e 30 39 33 2d   16 20:51 :
	/// 04.093
	//- 	0050   41 6e 74 65 6e 6e 61 31 2d 55 0d 0a               Antenna1 - U..

	//	0000   04 92 26 58 dc 63 00 24 77 56 4f bc 08 00 45 00 ..&X.c.$wVO...E.
	//	0010   00 2f 3e f7 00 00 ff 06 23 08 c0 a8 ec 5b c0 a8   . /
	//>.....#....[.. 	0020   ec 1c 03 e9 cc 9c fc dc c6 99 4e 83 cf c7 50 18
	//..........N...P. 	0030   00 ef 05 2a 00 00 0a 40 45 4e 44 0d 0a
	//...*...@END..

	bool fResult = false;
	TString strContent;
	GetContent(lpBuffer, nBufferLength, strContent);
	unsigned int uiErrorCode = ERROR_RFID_SUCCESS;
	stTagData.strData.c_str();
	stTagData.strEPC.c_str();
	stTagData.strTID.c_str();
	stTagData.uiAntenna = 0;

	// 2020/11/04 11:18:01.271-Antenna1-U
	TStringArray aryContent = m_objTokenizer.Token(strContent, _T('-'));

	if (aryContent.size() == 3) {
		// TString strDate;
		TString strAntenna;
		TString strData;
		unsigned int uiAntenna;
		stTagData.strTime = aryContent.at(0);
		strAntenna = aryContent.at(1);
		strData = aryContent.at(2);
		if (ParseAntenna(strAntenna.c_str(), uiAntenna))
			stTagData.uiAntenna = uiAntenna;

		if ((strData.at(0) == _T(CMD_RFID_READ_MULTI_EPC)) &&
		    (strData.length() > 1)) {
			stTagData.strEPC = strData.substr(
				1 + LEN_EPC_PEOTOCOL,
				strData.length() - (1 + LEN_EPC_PEOTOCOL + LEN_EPC_CRC16));
			if (pstTagEPC) {
				TString strPC = strData.substr(1, LEN_EPC_PEOTOCOL);
				TString strCRC =
					strData.substr(strData.length() - LEN_EPC_CRC16, LEN_EPC_CRC16);
				pstTagEPC->strRaw = strData.substr(1, strData.length() - 1);
				pstTagEPC->uiProtocolControl = _tcstol(strPC.c_str(), NULL, 16);
				pstTagEPC->strEPC = stTagData.strEPC;
				pstTagEPC->uiCRC16 = _tcstol(strCRC.c_str(), NULL, 16);
			}
			fResult |= true;

		} else {
			uiErrorCode = ParseErrorCode(strData.at(0));
		}
	}

	if (puiErrorCode)
		*puiErrorCode = uiErrorCode;
	return fResult;
}

bool RfidInterface::ParseInventory(const void *lpBuffer, int nBufferLength,
                                   RFID_TAG_DATA &stTagData,
                                   RFID_TAG_EPC *pstTagEPC,
                                   unsigned int *puiErrorCode) {
	// Send: U or U<slot>
	// Recv: U<none or EPC>
	//		<none or EPC>
	// none: no tag in RF field
	// EPC : PC + EPC + CRC16

	//
	// Send: <LF>@U1,R2,0,6<CR>
	// Recv: <LF>@2020/11/10
	// 16:26:39.338-Antenna2-U3000E28011606000020D68425ACFD9D3,
	// RE2801160200072CF084B09AD<CR><LF>
	//

	// Request
	// 0000   00 24 77 56 4f bc 04 92 26 58 dc 63 08 00 45 00.$wVO...&X.c..E.
	// 0010   00 34 ca 53 00 00 80 06 00 00 c0 a8 ec 1c c0 a8   .4.S............
	// 0020   ec 5b ce 94 03 e9 c4 e9 06 0b b4 3c a0 7a 50 18   .[.........<.zP.
	// 0030   fe f7 40 ac 00 00 0a 40 55 31 2c 52 32 2c 30 2c   ..@....@U1,R2, 0,
	// 0040   36 0d                                             6.

	// Response 1
	// 0000   04 92 26 58 dc 63 00 24 77 56 4f bc 08 00 45 00   ..&X.c.$wVO...E.
	// 0010   00 88 02 53 40 00 ff 06 1f 53 c0 a8 ec 5b c0 a8   ...S@....S...[..
	// 0020   ec 1c 03 e9 ce 94 b4 3c a0 7a c4 e9 06 17 50 18   .......<.z....P.
	// 0030   00 ef 02 d4 00 00 0a 40 32 30 32 30 2f 31 32 2f   .......@2020/12 /
	// 0040   32 31 20 31 35 3a 35 34 3a 33 33 2e 32 37 35 2d   21 15:54 : 33.275
	// - 0050   41 6e 74 65 6e 6e 61 31 2d 55 33 30 30 30 45 32   Antenna1 -
	// U3000E2 0060   38 30 31 31 36 30 36 30 30 30 30 32 30 44 36 38
	// 8011606000020D68 0070   34 32 35 41 43 46 44 39 44 33 2c 52 45 32 38 30
	// 425ACFD9D3, RE280 0080   31 31 36 30 32 30 30 30 37 32 43 46 30 38 34 42
	// 1160200072CF084B 0090   30 39 41 44 0d 0a 09AD..

	//
	// Response 2
	// 0000   04 92 26 58 dc 63 00 24 77 56 4f bc 08 00 45 00   ..&X.c.$wVO...E.
	// 0010   00 4e 02 54 40 00 ff 06 1f 8c c0 a8 ec 5b c0 a8.N.T@........[..
	// 0020   ec 1c 03 e9 ce 94 b4 3c a0 da c4 e9 06 17 50 18   .......<......P.
	// 0030   00 ef 3c ed 00 00 0a 40 32 30 32 30 2f 31 32 2f   ..<....@2020/12 /
	// 0040   32 31 20 31 35 3a 35 34 3a 33 33 2e 32 38 39 2d   21 15:54 : 33.289
	// - 0050   41 6e 74 65 6e 6e 61 31 2d 55 0d 0a               Antenna1 - U..

	// 0000   04 92 26 58 dc 63 00 24 77 56 4f bc 08 00 45 00   ..&X.c.$wVO...E.
	// 0010   00 88 02 55 40 00 ff 06 1f 51 c0 a8 ec 5b c0 a8   ...U@....Q...[..
	// 0020   ec 1c 03 e9 ce 94 b4 3c a1 00 c4 e9 06 17 50 18   .......<......P.
	// 0030   00 ef 04 4f 00 00 0a 40 32 30 32 30 2f 31 32 2f   ...O...@2020/12 /
	// 0040   32 31 20 31 35 3a 35 34 3a 33 33 2e 33 36 32 2d   21 15:54 : 33.362
	// - 0050   41 6e 74 65 6e 6e 61 31 2d 55 33 30 30 30 45 32   Antenna1 -
	// U3000E2 0060   38 30 31 31 36 30 36 30 30 30 30 32 30 44 36 38
	// 8011606000020D68 0070   34 32 35 41 43 46 44 39 44 33 2c 52 45 32 38 30
	// 425ACFD9D3, RE280 0080   31 31 36 30 32 30 30 30 37 32 43 46 30 38 34 42
	// 1160200072CF084B 0090   30 39 41 44 0d 0a 09AD..

	// 0000   04 92 26 58 dc 63 00 24 77 56 4f bc 08 00 45 00   ..&X.c.$wVO...E.
	// 0010   00 88 02 56 40 00 ff 06 1f 50 c0 a8 ec 5b c0 a8   ...V@....P...[..
	// 0020   ec 1c 03 e9 ce 94 b4 3c a1 60 c4 e9 06 17 50 18   .......<.`....P.
	// 0030   00 ef 2d fa 00 00 0a 40 32 30 32 30 2f 31 32 2f   .. - ....@2020/12
	// / 0040   32 31 20 31 35 3a 35 34 3a 33 33 2e 33 36 35 2d   21 15:54
	// : 33.365 - 0050   41 6e 74 65 6e 6e 61 31 2d 55 33 30 30 30 45 32 Antenna1
	// - U3000E2 0060   38 30 31 31 36 30 36 30 30 30 30 32 30 44 36 38
	// 8011606000020D68 0070   34 33 30 30 36 46 42 41 37 44 2c 52 45 32 38 30
	// 43006FBA7D, RE280 0080   31 31 36 30 32 30 30 30 36 30 36 46 30 38 36 30
	// 11602000606F0860 0090   30 39 41 44 0d 0a 09AD..

	// Response N
	// 0000   04 92 26 58 dc 63 00 24 77 56 4f bc 08 00 45 00   ..&X.c.$wVO...E.
	// 0010   00 55 02 81 40 00 ff 06 1f 58 c0 a8 ec 5b c0 a8.U..@....X...[..
	// 0020   ec 1c 03 e9 ce 94 b4 3c a9 92 c4 e9 06 17 50 18   .......<......P.
	// 0030   00 ef 96 95 00 00 0a 40 32 30 32 30 2f 31 32 2f   .......@2020/12 /
	// 0040   32 31 20 31 35 3a 35 34 3a 33 34 2e 33 32 37 2d   21 15:54 : 34.327
	// - 0050   41 6e 74 65 6e 6e 61 34 2d 55 0d 0a 0a 40 45 4e   Antenna4 -
	// U...@EN 0060   44 0d 0a                                          D..

	bool fResult = false;
	TString strContent;
	GetContent(lpBuffer, nBufferLength, strContent);
	unsigned int uiErrorCode = ERROR_RFID_SUCCESS;
	stTagData.strData.c_str();
	stTagData.strEPC.c_str();
	stTagData.strTID.c_str();
	stTagData.uiAntenna = 0;

	// 2020/11/04 11:18:01.271-Antenna1-U
	TStringArray aryContent = m_objTokenizer.Token(strContent, _T('-'));

	if (aryContent.size() == 3) {
		// TString strDate;
		TString strAntenna;
		TString strData;
		unsigned int uiAntenna;
		stTagData.strTime = aryContent.at(0);
		strAntenna = aryContent.at(1);
		strData = aryContent.at(2);
		if (ParseAntenna(strAntenna.c_str(), uiAntenna))
			stTagData.uiAntenna = uiAntenna;

		if ((strData.at(0) == _T(CMD_RFID_READ_MULTI_EPC)) &&
		    (strData.length() > 1)) {
			TStringArray aryTag = m_objTokenizer.Token(strData, _T(','));
			for (int nDataIndex = 0; nDataIndex < aryTag.size(); nDataIndex++) {
				if (aryTag.at(nDataIndex).at(0) == _T(CMD_RFID_READ_MULTI_EPC)) {
					stTagData.strEPC =
						aryTag.at(nDataIndex)
						.substr(1 + LEN_EPC_PEOTOCOL,
							aryTag.at(nDataIndex).length() -
							(1 + LEN_EPC_PEOTOCOL + LEN_EPC_CRC16));
					if (pstTagEPC) {
						TString strPC = aryTag.at(nDataIndex).substr(1, LEN_EPC_PEOTOCOL);
						TString strCRC =
							aryTag.at(nDataIndex)
							.substr(aryTag.at(nDataIndex).length() - LEN_EPC_CRC16,
								LEN_EPC_CRC16);
						pstTagEPC->strRaw =
							aryTag.at(nDataIndex)
							.substr(1, aryTag.at(nDataIndex).length() - 1);
						pstTagEPC->uiProtocolControl = _tcstol(strPC.c_str(), NULL, 16);
						pstTagEPC->strEPC = stTagData.strEPC;
						pstTagEPC->uiCRC16 = _tcstol(strCRC.c_str(), NULL, 16);
					}
					fResult |= true;
				} else if (aryTag.at(nDataIndex).at(0) == _T(CMD_RFID_READ_TID)) {
					stTagData.strTID = aryTag.at(nDataIndex)
						.substr(1, aryTag.at(nDataIndex).length() - 1);
					fResult |= true;
				} else {
					uiErrorCode = ParseErrorCode(aryTag.at(nDataIndex).at(0));
					fResult |= false;
				}
			}

		} else {
			uiErrorCode = ParseErrorCode(strData.at(0));
		}
	}

	if (puiErrorCode)
		*puiErrorCode = uiErrorCode;
	return fResult;
}

bool RfidInterface::ParseSelectMatching(const void *lpBuffer, int nBufferLength,
                                        RFID_TAG_DATA &stTagData) {
	// Send: <LF>T<CR>  <== 0A 54 0D <= T
	// Recv: <LF>@2020/11/10 17:29:27.544-Antennea1-T<CR><LF>
	//     DateTime: 2020/11/10 17:29:27.544
	//     Antennea: 1
	//     Data: T

	//  Request T <= = 0a 54 0d
	//	0000   00 24 77 56 4f bc 04 92 26 58 dc 63 08 00 45 00.$wVO...&X.c..E.
	//	0010   00 2b 01 49 00 00 80 06 00 00 c0 a8 ec 1c c0 a8   . +
	//.I............ 	0020   ec 5b f9 e6 03 e9 83 f9 0f f1 29 48 79 0c 50 18
	//.[........)Hy.P. 	0030   fe fd 0b 9f 00 00 0a 54 0d .......T.
	//
	//	Response T
	//	0000   04 92 26 58 dc 63 00 24 77 56 4f bc 08 00 45 00 ..&X.c.$wVO...E.
	//	0010   00 4e 32 fd 40 00 ff 06 ee e2 c0 a8 ec 5b c0 a8.N2.@........[..
	//	0020   ec 1c 03 e9 f9 e6 29 48 79 0c 83 f9 0f f4 50 18 ......)Hy.....P.
	//	0030   00 ef fa 72 00 00 0a 40 32 30 32 30 2f 31 31 2f   ...r...@2020/11
	/// 	0040   31 32 20 31 35 3a 31 33 3a 32 34 2e 39 37 38 2d   12 15:13
	/// : 24.978
	//- 	0050   41 6e 74 65 6e 6e 61 32 2d 54 0d 0a               Antenna2 - T..

	bool fResult = false;
	TString strContent;
	GetContent(lpBuffer, nBufferLength, strContent);
	unsigned int uiErrorCode = ERROR_RFID_SUCCESS;

	stTagData.uiAntenna = 0;

	// 2020/11/04 11:18:01.271-Antenna1-T
	TStringArray aryContent = m_objTokenizer.Token(strContent, _T('-'));

	if (aryContent.size() == 3) {
		TString strAntenna;
		TString strData;
		unsigned int uiAntenna;
		stTagData.strTime = aryContent.at(0);
		strAntenna = aryContent.at(1);
		strData = aryContent.at(2);
		if (ParseAntenna(strAntenna.c_str(), uiAntenna))
			stTagData.uiAntenna = uiAntenna;

		if ((strData.at(0) == _T(CMD_RFID_SELECT_MATCHING)) &&
		    (strData.length() == 1)) {
			fResult = true;
		}
	}
	return fResult;
}

bool RfidInterface::ParseSession(const void *lpBuffer, int nBufferLength,
                                 RFID_TAG_DATA &stTagData) {
	// Send: <LF>T<CR>  <== 0A 54 0D <= T
	// Recv: <LF>@2020/11/10 17:29:27.544-Antennea1-T<CR><LF>
	//     DateTime: 2020/11/10 17:29:27.544
	//     Antennea: 1
	//     Data: T

	// Request T <= = 0a 54 0d
	// 0000   00 24 77 56 4f bc 04 92 26 58 dc 63 08 00 45 00.$wVO...&X.c..E.
	// 0010   00 2b 01 49 00 00 80 06 00 00 c0 a8 ec 1c c0 a8   . + .I............
	// 0020   ec 5b f9 e6 03 e9 83 f9 0f f1 29 48 79 0c 50 18   .[........)Hy.P.
	// 0030   fe fd 0b 9f 00 00 0a 54 0d                        .......T.
	//
	// Response T
	// 0000   04 92 26 58 dc 63 00 24 77 56 4f bc 08 00 45 00   ..&X.c.$wVO...E.
	// 0010   00 4e 32 fd 40 00 ff 06 ee e2 c0 a8 ec 5b c0 a8   .N2.@........[..
	// 0020   ec 1c 03 e9 f9 e6 29 48 79 0c 83 f9 0f f4 50 18   ......)Hy.....P.
	// 0030   00 ef fa 72 00 00 0a 40 32 30 32 30 2f 31 31 2f   ...r...@2020/11 /
	// 0040   31 32 20 31 35 3a 31 33 3a 32 34 2e 39 37 38 2d   12 15:13 : 24.978
	// - 0050   41 6e 74 65 6e 6e 61 32 2d 54 0d 0a               Antenna2 - T..

	// SessionType S0 + TargetA
	// 0000   00 24 77 56 4f bc 04 92 26 58 dc 63 08 00 45 00   .$wVO...&X.c..E.
	// 0010   00 2e 5b 0a 00 00 80 06 00 00 c0 a8 ec 1c c0 a8   ..[.............
	// 0020   ec 5b ca 5e 03 e9 aa b6 a0 19 0e 78 d8 52 50 18   .[.^.......x.RP.
	// 0030   fd a1 ed e6 00 00 0a 54 30 30 30 0d               .......T000.

	// SessionType S3 + TargetB
	// 0000   00 24 77 56 4f bc 04 92 26 58 dc 63 08 00 45 00   .$wVO...&X.c..E.
	// 0010   00 2e 6a 5a 00 00 80 06 00 00 c0 a8 ec 1c c0 a8   ..jZ............
	// 0020   ec 5b ca 5e 03 e9 aa b6 a1 74 0e 7e 11 a3 50 18   .[.^.....t.~..P.
	// 0030   fe d1 b1 02 00 00 0a 54 30 33 31 0d               .......T031.

	// Response T (Session)
	// 0000   04 92 26 58 dc 63 00 24 77 56 4f bc 08 00 45 00   ..&X.c.$wVO...E.
	// 0010   00 4e 25 47 40 00 ff 06 fc 98 c0 a8 ec 5b c0 a8.N%G@........[..
	// 0020   ec 1c 03 e9 ca 5e 0e 7e 11 a3 aa b6 a1 7a 50 18   .....^.~.....zP.
	// 0030   00 ef fc e3 00 00 0a 40 32 30 32 31 2f 30 31 2f   .......@2021/01 /
	// 0040   30 37 20 31 33 3a 33 37 3a 33 30 2e 36 35 37 2d   07 13:37 : 30.657
	// - 0050   41 6e 74 65 6e 6e 61 31 2d 54 0d 0a               Antenna1 - T..

	bool fResult = false;
	TString strContent;
	GetContent(lpBuffer, nBufferLength, strContent);
	unsigned int uiErrorCode = ERROR_RFID_SUCCESS;

	stTagData.uiAntenna = 0;

	// 2020/11/04 11:18:01.271-Antenna1-T
	TStringArray aryContent = m_objTokenizer.Token(strContent, _T('-'));

	if (aryContent.size() == 3) {
		TString strAntenna;
		TString strData;
		unsigned int uiAntenna;
		stTagData.strTime = aryContent.at(0);
		strAntenna = aryContent.at(1);
		strData = aryContent.at(2);
		if (ParseAntenna(strAntenna.c_str(), uiAntenna))
			stTagData.uiAntenna = uiAntenna;

		if ((strData.at(0) == _T(CMD_RFID_SET_SESSION)) &&
		    (strData.length() == 1)) {
			fResult = true;
		}
	}
	return fResult;
}

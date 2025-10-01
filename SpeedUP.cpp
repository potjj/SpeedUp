#include <stdio.h>
#include <Windows.h>
using namespace std;

typedef struct _data_t {
	char SP;
	double giatri;
	int SoCoreN;
	int SoLanTang;
	double S;
	double P;
	double Speedup;
	int SoCoreTang;
	double newSpeedup;

} data_t, *pdata_t;
typedef struct _param_t {
	pdata_t pdata;
	int Init;
	int Step;
	int Num;


} param_t, *pparam_t;




double TinhS(double P) {
	double S = 1.0 - P;
	return S;
}
double TinhP(double S) {
	double P = 1.0 - S;
	return S;
}
double TinhSpeedupSN(double S, int N) {
	double Speedup = 0.0;
	if ((S >= 0.0) && (S <= 1.0) && N>0) {
		Speedup = 1.0 / (S + (1.0 - S) / N);
	}
	return Speedup;
}

double TinhSpeedupPN(double P, int N) {
	double Speedup = 0.0;
	double S = TinhS(P);
	Speedup = TinhSpeedupSN(S, N);
	return Speedup;
}
double TinhSFromSpeedUp(double Speedup,int NumCore){

	double S = 0.0;
	int N = NumCore;
	S = (N - Speedup) / (Speedup*(N - 1.0));
	return S;
}
int TangSoCore(double CurrentSpeedup, int NumCore, int ntime) {
	if (ntime <= 0)
		return 0;
	double S = TinhSFromSpeedUp(CurrentSpeedup, NumCore); double LimitSpeedup = CurrentSpeedup * ntime;
	int i = NumCore;
	double Speedup = CurrentSpeedup;
	while (Speedup < LimitSpeedup)
	{
	}
	i++;
	Speedup = TinhSpeedupSN(S, i);
	return i, NumCore;
}
void TestSpeedUp() {
	double S = 0.25;
	int N = 2;
	double SpeedUp = TinhSpeedupSN(S, N);
	printf("S =%0.2f N=%d Speedup = %.2f\n", S, N, SpeedUp);
}
void TestSpeedupP() {
	double P = 0.75;
	int N = 2;
	double Speedup = TinhSpeedupPN(P, N);
	printf("S= %0.2f P=%0.2f N=%d Speedup = %0.2f\n", TinhS(P), P, N, Speedup);
}
void TestSpeedupS() {

}
void TestTinhSFromSpeedUp() {
	double Speedup = 1.6; int N = 2;
	double S = TinhSFromSpeedUp(Speedup, N); printf("S = %0.2f \n", S);
	int ntime = 2;
	int SoCoreTang= TangSoCore(Speedup, N, ntime);
	printf("So core tang them = %d\n", SoCoreTang);
	double NewSpeedup= TinhSpeedupSN(S, N + SoCoreTang);
	printf("OldSpeedup = %0.2f NewSpeedup = %0.2f\n", Speedup, NewSpeedup);
}

void TinhSpeedupData(pdata_t p) {
	if (p->SP == 'S') {
		p->S = p->giatri;
		p->P = TinhS(p->P);
	}
	else {
		p->P = p->giatri;
		p->S = TinhS(p->P);
	}
	p->Speedup = TinhSpeedupSN(p->S, p->SoCoreN);
	p->SoCoreTang = TangSoCore(p->Speedup, p->SoCoreN, p->SoLanTang);
	p->newSpeedup = TinhSpeedupSN(p->S, p->SoCoreN + p->SoCoreTang);
}
DWORD WINAPI TinhSpeedupThreaddProc(LPVOID lpParam) {
	printf("ThreadID= %d\n", GetCurrentThreadId());
	pparam_t p = (pparam_t)lpParam;
	for (int i = p->Init; i < p->Num; i += p->Step) {
		TinhSpeedupData(p->pdata + i);
	}
	return 0;
}
void TinhSpeedupMangData(pdata_t p, int n) {

	for (int i = 0; i < n; i++) {
		TinhSpeedupData(p + i);
	}
	
}
bool DocTapTin(char *filename, data_t p[],int n) {
	FILE *f = fopen(filename, "rt");
	if (!f)
		printf("Khong the doc tap tin %s", filename),exit(0);
	char header[_MAX_PATH];
	fgets(header, _MAX_PATH, f);
	int i = 0;
	printf(header);
	while ((!feof(f)) && (i < n))
	{
		fscanf(f,"%c%lf%d%d", &(p[i].SP), &(p[i].giatri), &(p[i].SoCoreN), &(p[i].SoCoreTang));
		fgets(header, _MAX_PATH, f);
		printf("%2d %c %0.2lf %3d %3d\n", i, p[i].SP, p[i].giatri, p[i].SoCoreN, p[i].SoCoreTang);
		i++;
	}
	fclose(f);
	return true;
}

void TinhSpeedupMangMT(pdata_t p, int n, int NumThread) {
	HANDLE *h = (HANDLE*)malloc(NumThread*sizeof(HANDLE));
	DWORD *dwID = (DWORD*)malloc(NumThread*sizeof(DWORD));
	pparam_t param = (pparam_t)malloc(NumThread*sizeof(param_t));

	for (int i = 0; i < NumThread; i++) {
		param[i].pdata = p;
		param[i].Init = i;
		param[i].Step = NumThread;
		param[i].Num = n;
		h[i] = CreateThread(NULL, 0, TinhSpeedupThreaddProc, (LPVOID)&param[i], 0, &dwID[i]);
	}
	WaitForMultipleObjects(NumThread, h, TRUE, INFINITE);
}
bool OpenExcel(char*filename) {
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));
	char cmdline[_MAX_PATH] = "D:/Speedup.csv";
	strcat(cmdline, filename);
	if (!CreateProcess(NULL,
		cmdline,
		NULL,
		NULL,
		FALSE,
		0,
		NULL,
		NULL,
		&si,
		&pi
		))
	{
		printf("CreateProcess Failed (%d).\n", GetLastError());
		return false;
	}
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	return true;
}


void main(int argc, char *argv[]) {
	if (argc < 5) {
		printf("%s filename sophantu NumThread\n", argv[0]), exit(0);
	}
	int SoPhanTuMang = atoi(argv[2]);
	if (SoPhanTuMang <= 0)
		printf("So Phan Tu la so am\n"), exit(0);
	pdata_t pdata = (pdata_t)malloc(SoPhanTuMang*sizeof(data_t));
	if (!pdata)
		printf("khong cap phat duoc bo nho\n"), exit(0);
	DocTapTin(argv[1], pdata, SoPhanTuMang);
	//TinhSpeedupMangData(pdata, SoPhanTuMang);
	int NumThread = atoi(argv[4]);
	if (NumThread <= 0 || NumThread >= 100)
		NumThread = 4;
	TinhSpeedupMangMT(pdata, SoPhanTuMang, NumThread);
	OpenExcel(argv[3]);

}
//------------------------------------------------SOCKET------------------------------------

/*
typedef struct _request_t {
	char SP;
	double giatri;
	int SoCoreN;
	int SolanTang;

}request_t, *prequest_t;

typedef struct _reply_t {
	double S;
	double P;
	double Speedup;
	int SoCoreTang;
	double newSpeedup;

} reply_t, *preply_t;

int InitializeWinsock() {
	WSADATA wsa;
	printf("\nInitializing Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		printf("Failed. ERROR CODE: %d", WSAGetLastError());
		exit(-1);
		return 0 ;

	}
	printf("Initialized.");
	return 1;
}
SOCKET SocketConnect(char*strServerIDAddress, u_short port) {
	SOCKET s;
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
		printf("Could not create socket:%d", WSAGetLastError());
		exit(-1);
	}
	printf("Socket created.\n");
	struct sockaddr_in server;
	server.sin_addr.s_addr = inet_addr(strServerIDAddress);
	server.sin_family = AF_INET;
	server.sin_port = htons(port);

	if (connect(s, (struct sockaddr *)&server, sizeof(server)) < 0) {
		puts("connect error");
		return 1;
	}
	puts("Connected");
	return s;
}

int main() {
	InitializeWinsock();
	request_t req;
	req.SP = 'S';
	req.giatri = 0.25;
	req.SoCoreN = 2;
	req.SolanTang = 2;

	char* SVRIPAddress = "172.17.14.54";
	u_short port = 54321;

	SOCKET svrsocket = SocketConnect(SVRIPAddress, port);
	send(svrsocket, (char*)&req, sizeof(req), 0);
	reply_t rep;
	recv(svrsocket, (char*)&rep, sizeof(req), 0);
	printf("Req:SP = %c, giatri=%0.2f,Socore = %3d , SoLanTang=%3d\n",req.SP,req.giatri,req.SoCoreN,req.SolanTang);
	printf("Reply: S= %0.2lf, P= %0.2lf, Speedup = %4.2lf, SoCoretang= %3d, NewSpeed= %3d\n", rep.S, rep.P, rep.Speedup, rep.SoCoreTang, rep.newSpeedup);
	closesocket(svrsocket);
	WSACleanup();
}



*/






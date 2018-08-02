#include <Windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>

#define HEX_BASE 16

int _tmain(int argc, TCHAR** argv)
{
	HANDLE hOriFile = NULL;
	HANDLE hNewFile = NULL;
	TCHAR arrCurPath[MAX_PATH] = { 0, };
	TCHAR arrNewFile[MAX_PATH] = { 0, };
	TCHAR arrTargetFileName[MAX_PATH] = { 0, };
	DWORD fileSize = 0;
	DWORD lpNumberOfBytesRead = 0;
	PBYTE lpBuffer = NULL;
	PBYTE lpNewBuffer = NULL;
	TCHAR *pFileName = NULL;
	TCHAR *endPtr = NULL;
	TCHAR *index = NULL;
	DWORD xorKey = 0;
	BOOL bOption = FALSE;
	int cnt = 0;
	int ch = '\\';



	if(argc != 3 && argc != 4)
	{
		printf(" \n [!] Usage: XORer.exe [Option] [Target File Path] [XOR key]\n");
		printf(" \n\t-d,\tCalculate XOR by 4 byte.\n");
		printf(" \t-w,\tCalculate XOR by 2 byte.\n");
		printf(" \t-b,\tCalculate XOR by 1 byte.\n");
		printf(" \n If not selected option, calculate 4 byte.\n");

		return -1;
	}

	if(argc == 4)
	{
		// selected option
		
		if(strncmp(argv[1], "-d", 0x02) != 0 && strncmp(argv[1], "-w", 0x02) != 0 && strncmp(argv[1], "-b", 0x02) != 0)
		{
			printf(" [!] ERROR: Worng Arguments\n");
			return -1;
		}

		// Read XOR_KEY (argv[3])
		xorKey = strtol((char *)argv[3], (char **)&endPtr, HEX_BASE);
		strncpy(arrTargetFileName, argv[2], strlen(argv[2]));
		index = (TCHAR*)strstr(argv[1], "-") + 1;
		bOption = TRUE;
	}
	else
	{
		// Read XOR_KEY (argv[2])
		xorKey = strtol((char *)argv[2], (char **)&endPtr, HEX_BASE);
		strncpy(arrTargetFileName, argv[1], strlen(argv[1]));
	}

	GetModuleFileName(NULL, arrCurPath, MAX_PATH);
	pFileName = strrchr(arrCurPath, ch);
	strncpy(pFileName, "\\outFile\0", 0x10);
	memcpy(arrNewFile, arrCurPath, MAX_PATH);

	hOriFile = CreateFile(arrTargetFileName, GENERIC_READ, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	hNewFile = CreateFile(arrNewFile, GENERIC_READ|GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if( (hOriFile == INVALID_HANDLE_VALUE)  || (hNewFile == INVALID_HANDLE_VALUE) )
	{
		printf(" [!] CreateFile- ERROR %08X\n", GetLastError());
		return -1;
	}

	fileSize = GetFileSize(hOriFile, NULL);
	lpBuffer = (PBYTE)VirtualAlloc(NULL, fileSize, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
	lpNewBuffer = (PBYTE)VirtualAlloc(NULL, fileSize, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);

	if(lpBuffer == NULL || lpNewBuffer == NULL)
	{
		printf(" [!] VirtualAlloc- ERROR %08X\n", GetLastError());
		return -1;
	}

	// Read Original file
	ReadFile(hOriFile, lpBuffer, fileSize, &lpNumberOfBytesRead, NULL);
	if(lpNumberOfBytesRead == 0)
	{
		printf(" [!] ReadFile- ERROR %08X\n", GetLastError());
		return -1;
	}
	
	if(bOption)
	{
		switch(*index)
		{
		case 'd':
			for(cnt = 0; cnt < fileSize; cnt + 4)
			{
				*(PDWORD)(lpNewBuffer + cnt) = *(PDWORD)(lpBuffer + cnt) ^ xorKey;
				cnt += 4;
			}
		break;

		case 'w':
			for(cnt = 0; cnt < fileSize; cnt + 2)
			{
				*(PWORD)(lpNewBuffer + cnt) = *(PWORD)(lpBuffer + cnt) ^ (WORD)xorKey;
				cnt += 2;
			}
		break;
		case 'b':
			for(cnt = 0; cnt < fileSize; cnt + 1)
			{
				*(PBYTE)(lpNewBuffer + cnt) = *(PBYTE)(lpBuffer + cnt) ^ (BYTE)xorKey;
				cnt += 1;
			}
		break;
		default:
			printf(" [!] ERROR: argument\n");
		break;
		}
	}
	else
	{
		for(cnt = 0; cnt < fileSize; cnt + 4)
		{
			*(PDWORD)(lpNewBuffer + cnt) = *(PDWORD)(lpBuffer + cnt) ^ xorKey;
			cnt += 4;
		}
	}
	
	// Write XORed data file
	WriteFile(hNewFile, lpNewBuffer, fileSize, &lpNumberOfBytesRead, NULL);
	if(lpNumberOfBytesRead <= 0)
	{
		printf(" [!] ERROR: %08X, %08X", GetLastError(), lpNumberOfBytesRead);
	}

	VirtualFree(lpBuffer, fileSize, MEM_RELEASE);
	VirtualFree(lpNewBuffer, fileSize, MEM_RELEASE);
	CloseHandle(hOriFile);
	CloseHandle(hNewFile);

	printf(" [*] XOR Completed %s. \n", arrNewFile);
	printf(" [*] Plz press any key. \n");

	return 0;
}
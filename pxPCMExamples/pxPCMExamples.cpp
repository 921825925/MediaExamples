#include "stdafx.h"
#include <fstream>
#include <iostream>
using namespace std;

int _tmain(int argc, _TCHAR* argv[])
{
	// Read file to buffer
	char szFileName[256] = {0};
	sprintf_s(szFileName, ".\\44100_stereo_s16le.pcm");

	ifstream inFile(szFileName, ios::in | ios::binary | ios::ate);
	long nFileSizeInBytes = inFile.tellg(); 

	char *pszPCMBuffer = new char[nFileSizeInBytes];  

	inFile.seekg(0, ios::beg);  
	inFile.read(pszPCMBuffer, nFileSizeInBytes);  
	inFile.close();  

	cout<<"FileName: " << szFileName <<", nFileSizeInBytes: " << nFileSizeInBytes <<endl;
	cout << "Read file to a buffer Done!!!\n\n";  

	// ���ļ��ж�ȡ�����㲢��ӡ
	short szItem[1024] = {0};
	memcpy(szItem, pszPCMBuffer, 256);

	for (int i = 0; i < 128; i++)
	{
		printf("%6d ", szItem[i]);

		if ((i + 1) % 8 == 0)
		{
			cout<<endl;
		}
	}

	// ��˫�����������������������
	FILE *fpLeft  = fopen("44100_mono_s16le_l.pcm","wb+");  
	FILE *fpRight = fopen("44100_mono_s16le_r.pcm","wb+");

	printf("\n Split stereo to left and right ...\n");

	for (int nPos = 0; nPos < nFileSizeInBytes; )
	{
		if ((nPos / 2) % 2 == 0) // ż��
		{
			if (fpLeft)
			{
				fwrite(pszPCMBuffer + nPos, 1, 2, fpLeft); 
			}
		}
		else
		{
			if (fpRight)
			{
				fwrite(pszPCMBuffer + nPos, 1, 2, fpRight); 
			}
		}

		nPos += 2;
	}

	if (fpLeft)
	{
		fclose(fpLeft);  
		fpLeft = NULL;
	}

    if (fpRight)
    {
		fclose(fpRight);
		fpRight = NULL;
    }
	  
	printf("\n Split stereo to left and right Done. \n");

	if (pszPCMBuffer)
	{
		delete [] pszPCMBuffer;
		pszPCMBuffer = NULL;
	}

	getchar();

	return 0;
}


// WindowsAudioSession.cpp
// ����������WAS�ɼ���Ƶ��demo
/*
References:
https://msdn.microsoft.com/en-us/library/dd370800(v=vs.85).aspx
http://blog.csdn.net/leave_rainbow/article/details/50917043
http://blog.csdn.net/lwsas1/article/details/46862195?locationNum=1
*/

#include "stdafx.h"

#include <MMDeviceAPI.h>
#include <AudioClient.h>
#include <iostream>
using namespace std;

// REFERENCE_TIME time units per second and per millisecond
#define REFTIMES_PER_SEC  10000000
#define REFTIMES_PER_MILLISEC  10000

#define EXIT_ON_ERROR(hres)  \
	if (FAILED(hres)) { goto Exit; }
#define SAFE_RELEASE(punk)  \
	if ((punk) != NULL)  \
				{ (punk)->Release(); (punk) = NULL; }

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient = __uuidof(IAudioClient);
const IID IID_IAudioCaptureClient = __uuidof(IAudioCaptureClient);

int _tmain(int argc, _TCHAR* argv[])
{
	HRESULT hr;
	REFERENCE_TIME hnsRequestedDuration = REFTIMES_PER_SEC; // ��λ����
	REFERENCE_TIME hnsActualDuration;
	UINT32 bufferFrameCount;
	UINT32 numFramesAvailable;
	IMMDeviceEnumerator *pEnumerator = NULL;
	IMMDevice *pDevice = NULL;
	IAudioClient *pAudioClient = NULL;
	IAudioCaptureClient *pCaptureClient = NULL;
	WAVEFORMATEX *pwfx = NULL;
	UINT32 packetLength = 0;
	BOOL bDone = FALSE;
	BYTE *pData;
	DWORD flags;

	hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (FAILED(hr))
	{
		printf("Unable to initialize COM in render thread: %x\n", hr);
		return hr;
	}

	// ����ö�������Ƶ�豸
	// ����������ʱ���ȡ������������п��õ��豸����ָ������Ҫ�õ����Ǹ�����
	hr = CoCreateInstance(
		CLSID_MMDeviceEnumerator, NULL,
		CLSCTX_ALL, IID_IMMDeviceEnumerator,
		(void**)&pEnumerator);
	EXIT_ON_ERROR(hr)

	hr = pEnumerator->GetDefaultAudioEndpoint(
		eCapture, eConsole, &pDevice);
	EXIT_ON_ERROR(hr)

	// ����һ���������ͨ�������Ի�ȡ������Ҫ��һ������
	hr = pDevice->Activate(
		IID_IAudioClient, CLSCTX_ALL,
		NULL, (void**)&pAudioClient);
	EXIT_ON_ERROR(hr)

	hr = pAudioClient->GetMixFormat(&pwfx);
	EXIT_ON_ERROR(hr)

	cout<<"hnsRequestedDuration:" <<hnsRequestedDuration<<" ns."<<endl;

	// ��ʼ�������������������ָ��������󻺳������ȣ��������Ҫ��
	// Ӧ�ó���������ݿ�Ĵ�С�Լ���ʱ���̶�������ĳ�ʼ�������������ҿ����ĵ�����
	// https://msdn.microsoft.com/en-us/library/dd370875(v=vs.85).aspx
	hr = pAudioClient->Initialize(
		AUDCLNT_SHAREMODE_SHARED,
		0,
		hnsRequestedDuration,
		0,
		pwfx,
		NULL);
	EXIT_ON_ERROR(hr)

	// Get the size of the allocated buffer.
	// ���buffersize��ָ���ǻ����������Դ�Ŷ���֡��������
	hr = pAudioClient->GetBufferSize(&bufferFrameCount);
	EXIT_ON_ERROR(hr)

	cout<<"bufferFrameCount:"<<bufferFrameCount<<endl;

	// �����ɼ�����ӿ�
	hr = pAudioClient->GetService(
		IID_IAudioCaptureClient,
		(void**)&pCaptureClient);
	EXIT_ON_ERROR(hr)

	// Calculate the actual duration of the allocated buffer.
	hnsActualDuration = (double)REFTIMES_PER_SEC * bufferFrameCount / pwfx->nSamplesPerSec;

	hr = pAudioClient->Start();  // Start recording.
	EXIT_ON_ERROR(hr)

	printf("\nAudio Capture begin...\n\n");

	// Each loop fills about half of the shared buffer.
	while (bDone == FALSE)
	{
		// Sleep for half the buffer duration.
		Sleep(hnsActualDuration/REFTIMES_PER_MILLISEC/2);

		hr = pCaptureClient->GetNextPacketSize(&packetLength);
		EXIT_ON_ERROR(hr)

		int nCnt = 0;

		while (packetLength != 0)
		{
			// Get the available data in the shared buffer.
			// ��������������ȡ����
			hr = pCaptureClient->GetBuffer(
				&pData,
				&numFramesAvailable,
				&flags, NULL, NULL);
			EXIT_ON_ERROR(hr)

			if (flags & AUDCLNT_BUFFERFLAGS_SILENT)
			{
				pData = NULL;  // Tell CopyData to write silence.
			}

			nCnt++;

			printf("%04d: GetBuffer Success!!! numFramesAvailable:%u\n", nCnt, numFramesAvailable);

			// Copy the available capture data to the audio sink.
			/*hr = pMySink->CopyData(
			pData, numFramesAvailable, &bDone);
			EXIT_ON_ERROR(hr)*/

			hr = pCaptureClient->ReleaseBuffer(numFramesAvailable);
			EXIT_ON_ERROR(hr)

			hr = pCaptureClient->GetNextPacketSize(&packetLength);
			EXIT_ON_ERROR(hr)

			// �ɼ�10��buffer���˳�
			if (nCnt == 10)
			{
				bDone = TRUE;
				break;
			}
		}
	}

	printf("\nAudio Capture Done.\n");

	hr = pAudioClient->Stop();  // Stop recording.
	EXIT_ON_ERROR(hr)

Exit:
	CoTaskMemFree(pwfx);
	SAFE_RELEASE(pEnumerator)
	SAFE_RELEASE(pDevice)
	SAFE_RELEASE(pAudioClient)
	SAFE_RELEASE(pCaptureClient)

	CoUninitialize();

	getchar();

	return 0;
}


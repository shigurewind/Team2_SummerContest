//=============================================================================
//
// サウンド処理 3D [sound3D.cpp]
//
//=============================================================================
#include "sound3D.h"
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <xaudio2.h>
#include <x3daudio.h>
#include <DirectXMath.h>

#include "camera.h"
#include "debugproc.h"

using namespace DirectX;

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT CheckChunk(HANDLE hFile, DWORD format, DWORD* pChunkSize, DWORD* pChunkDataPosition);
HRESULT ReadChunkData(HANDLE hFile, void* pBuffer, DWORD dwBuffersize, DWORD dwBufferoffset);

//*****************************************************************************
// グローバル変数
//*****************************************************************************

IXAudio2* g_pXAudio2_3D = nullptr;
IXAudio2MasteringVoice* g_pMasteringVoice_3D = nullptr;
IXAudio2SourceVoice* g_apSourceVoice_3D[SOUND3D_MAX] = {};
BYTE* g_apDataAudio_3D[SOUND3D_MAX] = {};
DWORD g_aSizeAudio_3D[SOUND3D_MAX] = {};
WAVEFORMATEXTENSIBLE g_wfx_3D[SOUND3D_MAX];

// X3DAudio
X3DAUDIO_HANDLE g_X3DInstance;
X3DAUDIO_LISTENER g_listener;
X3DAUDIO_EMITTER g_emitter[SOUND3D_MAX];
FLOAT32* g_pMatrixCoefficients[SOUND3D_MAX];

struct SOUNDPARAM3D { const char* filename; int loop; };
SOUNDPARAM3D g_aParam3D[SOUND3D_MAX] =
{
    { "data/SE/shot000.wav", 0 },
    { "data/SE/ghost_sound.wav", 0 }
};

//=============================================================================
// 初期化処理
//=============================================================================
BOOL InitSound3D(HWND hWnd)
{
    HRESULT hr;
    CoInitializeEx(NULL, COINIT_MULTITHREADED);

    hr = XAudio2Create(&g_pXAudio2_3D, 0);
    if (FAILED(hr))
    {
        PrintDebugProc("InitSound3D: Failed XAudio2Create\n");
        return FALSE;
    }

    hr = g_pXAudio2_3D->CreateMasteringVoice(&g_pMasteringVoice_3D);
    if (FAILED(hr))
    {
        PrintDebugProc("InitSound3D: Failed CreateMasteringVoice\n");
        return FALSE;
    }

    // アウトプット番号を取る
    DWORD channelMask = 0;
    g_pMasteringVoice_3D->GetChannelMask(&channelMask);
    if (channelMask == 0)
    {
        PrintDebugProc("InitSound3D: Invalid channel mask!\n");
        return FALSE;
    }

    X3DAudioInitialize(channelMask, X3DAUDIO_SPEED_OF_SOUND, g_X3DInstance);

    // Init listener
    ZeroMemory(&g_listener, sizeof(g_listener));
    g_listener.Position = XMFLOAT3(0, 0, 0);
    g_listener.OrientFront = XMFLOAT3(0, 0, 1);
    g_listener.OrientTop = XMFLOAT3(0, 1, 0);
    g_listener.Velocity = XMFLOAT3(0, 0, 0);

    // Load WAV files
    for (int i = 0; i < SOUND3D_MAX; i++)
    {
        char buf[256];
        sprintf_s(buf, "InitSound3D: Loading %s...\n", g_aParam3D[i].filename);
        PrintDebugProc(buf);

        HANDLE hFile = CreateFile(g_aParam3D[i].filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
        if (hFile == INVALID_HANDLE_VALUE)
        {
            sprintf_s(buf, "InitSound3D: Failed to open %s\n", g_aParam3D[i].filename);
            PrintDebugProc(buf);
            return FALSE;
        }

        DWORD chunkPos, chunkSize, fileType;

        if (FAILED(CheckChunk(hFile, 'FFIR', &chunkSize, &chunkPos))) return FALSE;
        if (FAILED(ReadChunkData(hFile, &fileType, sizeof(DWORD), chunkPos))) return FALSE;

        if (FAILED(CheckChunk(hFile, ' tmf', &chunkSize, &chunkPos))) return FALSE;
        if (FAILED(ReadChunkData(hFile, &g_wfx_3D[i], chunkSize, chunkPos))) return FALSE;

        if (FAILED(CheckChunk(hFile, 'atad', &g_aSizeAudio_3D[i], &chunkPos))) return FALSE;
        g_apDataAudio_3D[i] = (BYTE*)malloc(g_aSizeAudio_3D[i]);
        if (FAILED(ReadChunkData(hFile, g_apDataAudio_3D[i], g_aSizeAudio_3D[i], chunkPos))) return FALSE;

        CloseHandle(hFile);

        sprintf_s(buf, "InitSound3D: Channels=%d, BitsPerSample=%d, SampleRate=%d\n",
            g_wfx_3D[i].Format.nChannels,
            g_wfx_3D[i].Format.wBitsPerSample,
            g_wfx_3D[i].Format.nSamplesPerSec);
        PrintDebugProc(buf);

        // Convert stereo -> mono
        if (g_wfx_3D[i].Format.nChannels == 2)
        {
            int sampleCount = g_aSizeAudio_3D[i] / (2 * sizeof(short));
            short* stereoSamples = (short*)g_apDataAudio_3D[i];
            short* monoSamples = new short[sampleCount];
            for (int s = 0; s < sampleCount; s++)
                monoSamples[s] = (stereoSamples[s * 2] / 2) + (stereoSamples[s * 2 + 1] / 2);

            free(g_apDataAudio_3D[i]);
            g_apDataAudio_3D[i] = (BYTE*)monoSamples;
            g_aSizeAudio_3D[i] = sampleCount * sizeof(short);

            g_wfx_3D[i].Format.nChannels = 1;
            g_wfx_3D[i].Format.nBlockAlign = g_wfx_3D[i].Format.nChannels * g_wfx_3D[i].Format.wBitsPerSample / 8;
            g_wfx_3D[i].Format.nAvgBytesPerSec = g_wfx_3D[i].Format.nBlockAlign * g_wfx_3D[i].Format.nSamplesPerSec;
        }

        // Create SourceVoice
        hr = g_pXAudio2_3D->CreateSourceVoice(&g_apSourceVoice_3D[i], &g_wfx_3D[i].Format);
        if (FAILED(hr))
        {
            sprintf_s(buf, "InitSound3D: Failed to create SourceVoice for %s\n", g_aParam3D[i].filename);
            PrintDebugProc(buf);
            return FALSE;
        }

        // Init emitter
        ZeroMemory(&g_emitter[i], sizeof(X3DAUDIO_EMITTER));
        g_emitter[i].ChannelCount = 1;
        g_emitter[i].CurveDistanceScaler = 1.0f;
        g_emitter[i].DopplerScaler = 1.0f;
        g_emitter[i].Position = XMFLOAT3(0, 0, 0);
        g_emitter[i].OrientFront = XMFLOAT3(0, 0, 1);
        g_emitter[i].OrientTop = XMFLOAT3(0, 1, 0);

        // Allocate matrix
        g_pMatrixCoefficients[i] = new FLOAT32[channelMask]; // max channels
    }

    return TRUE;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitSound3D()
{
    for (int i = 0; i < SOUND3D_MAX; i++)
    {
        if (g_apSourceVoice_3D[i])
        {
            g_apSourceVoice_3D[i]->Stop(0);
            g_apSourceVoice_3D[i]->DestroyVoice();
            g_apSourceVoice_3D[i] = nullptr;
        }
        if (g_apDataAudio_3D[i])
        {
            free(g_apDataAudio_3D[i]);
            g_apDataAudio_3D[i] = nullptr;
        }
        if (g_pMatrixCoefficients[i])
        {
            delete[] g_pMatrixCoefficients[i];
            g_pMatrixCoefficients[i] = nullptr;
        }
    }

    if (g_pMasteringVoice_3D) g_pMasteringVoice_3D->DestroyVoice();
    if (g_pXAudio2_3D) g_pXAudio2_3D->Release();

    g_pMasteringVoice_3D = nullptr;
    g_pXAudio2_3D = nullptr;
    CoUninitialize();
}

//=============================================================================
// Update listener
//=============================================================================
void UpdateListener(XMFLOAT3 listenerPos, XMFLOAT3 listenerFront)
{
    g_listener.Position = listenerPos;
    g_listener.OrientFront = listenerFront;
    g_listener.OrientTop = XMFLOAT3(0, 1, 0);
    g_listener.Velocity = XMFLOAT3(0, 0, 0);
}

//=============================================================================
// Update & calculate 3D sound
//=============================================================================
void UpdateSound3D()
{
    CAMERA* cam = GetCamera();
    XMFLOAT3 listenerPos = cam->pos;
    XMFLOAT3 listenerFront;
    listenerFront.x = sinf(cam->rot.y) * cosf(cam->rot.x);
    listenerFront.y = sinf(cam->rot.x);
    listenerFront.z = cosf(cam->rot.y) * cosf(cam->rot.x);

    UpdateListener(listenerPos, listenerFront);
    
    XMFLOAT3 emitterPos = listenerPos;
    emitterPos.x += 5.0f;
    g_emitter[0].Position = emitterPos;

    X3DAUDIO_DSP_SETTINGS dspSettings = {};
    dspSettings.pMatrixCoefficients = g_pMatrixCoefficients[0];
    dspSettings.SrcChannelCount = 1;
    dspSettings.DstChannelCount = 2;

    X3DAudioCalculate(g_X3DInstance, &g_listener, &g_emitter[0],
        X3DAUDIO_CALCULATE_MATRIX | X3DAUDIO_CALCULATE_LPF_REVERB,
        &dspSettings);

    // マスターリーグボイスに送る
    g_apSourceVoice_3D[0]->SetOutputMatrix(g_pMasteringVoice_3D, 1, 2, dspSettings.pMatrixCoefficients);
}

//=============================================================================
// 再生処理
//=============================================================================
void PlaySound3D(int label, XMFLOAT3 pos)
{
    if (!g_apSourceVoice_3D[label]) return;

    XAUDIO2_VOICE_STATE state;
    g_apSourceVoice_3D[label]->GetState(&state);
    if (state.BuffersQueued != 0)
    {
        g_apSourceVoice_3D[label]->Stop(0);
        g_apSourceVoice_3D[label]->FlushSourceBuffers();
    }

    g_emitter[label].Position = pos;

    X3DAUDIO_DSP_SETTINGS dspSettings = {};
    dspSettings.pMatrixCoefficients = g_pMatrixCoefficients[label];
    dspSettings.SrcChannelCount = 1;
    dspSettings.DstChannelCount = 2;

    X3DAudioCalculate(g_X3DInstance, &g_listener, &g_emitter[label],
        X3DAUDIO_CALCULATE_MATRIX, &dspSettings);

    g_apSourceVoice_3D[label]->SetOutputMatrix(g_pMasteringVoice_3D, 1, 2, dspSettings.pMatrixCoefficients);

    XAUDIO2_BUFFER buffer = {};
    buffer.AudioBytes = g_aSizeAudio_3D[label];
    buffer.pAudioData = g_apDataAudio_3D[label];
    buffer.Flags = XAUDIO2_END_OF_STREAM;
    buffer.LoopCount = g_aParam3D[label].loop;

    g_apSourceVoice_3D[label]->SubmitSourceBuffer(&buffer);
    g_apSourceVoice_3D[label]->Start(0);
}

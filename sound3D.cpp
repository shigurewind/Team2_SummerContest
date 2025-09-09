//=============================================================================
//
// サウンド処理 [sound3D.cpp]
//
//=============================================================================
#include "sound3D.h"
#include <malloc.h>
#include <string.h>
#include <stdio.h>

#include "player.h"
#include "debugproc.h"

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT CheckChunk(HANDLE hFile, DWORD format, DWORD* pChunkSize, DWORD* pChunkDataPosition);
HRESULT ReadChunkData(HANDLE hFile, void* pBuffer, DWORD dwBuffersize, DWORD dwBufferoffset);

//*****************************************************************************
// グローバル変数
//*****************************************************************************

// Global XAudio2
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
FLOAT32 g_DSPSettings[SOUND3D_MAX][2]; // stereo: left/right

struct SOUNDPARAM3D { const char* filename; int loop; };

SOUNDPARAM3D g_aParam3D[SOUND3D_MAX] =
{
    { "data/SE/shot000.wav", 0 },
    { "data/SE/insect.wav", 0 }
};

//*****************************************************************************
// 初期化処理 (Debug版)
//*****************************************************************************
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

    // Init listener
    ZeroMemory(&g_listener, sizeof(g_listener));
    g_listener.Position = XMFLOAT3(0, 0, 0);
    g_listener.OrientFront = XMFLOAT3(0, 0, 1);
    g_listener.OrientTop = XMFLOAT3(0, 1, 0);
    g_listener.Velocity = XMFLOAT3(0, 0, 0);

    // Create X3DAudio instance
    DWORD channelMask = 0;
    g_pMasteringVoice_3D->GetChannelMask(&channelMask);
    if (channelMask == 0) {
        PrintDebugProc("InitSound3D: Invalid channel mask!\n");
        return FALSE;
    }


    X3DAudioInitialize(channelMask, X3DAUDIO_SPEED_OF_SOUND, &g_X3DInstance);

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

        sprintf_s(buf, "InitSound3D: File info: Channels=%d, BitsPerSample=%d, SampleRate=%d\n",
            g_wfx_3D[i].Format.nChannels,
            g_wfx_3D[i].Format.wBitsPerSample,
            g_wfx_3D[i].Format.nSamplesPerSec);
        PrintDebugProc(buf);

        // --- stereo->mono ---
        if (g_wfx_3D[i].Format.nChannels == 2)
        {
            int sampleCount = g_aSizeAudio_3D[i] / (2 * sizeof(short));
            short* stereoSamples = (short*)g_apDataAudio_3D[i];
            short* monoSamples = new short[sampleCount];

            for (int s = 0; s < sampleCount; s++)
            {
                monoSamples[s] = (stereoSamples[s * 2] / 2) + (stereoSamples[s * 2 + 1] / 2);
            }

            free(g_apDataAudio_3D[i]);
            g_apDataAudio_3D[i] = (BYTE*)monoSamples;
            g_aSizeAudio_3D[i] = sampleCount * sizeof(short);

            g_wfx_3D[i].Format.nChannels = 1;
            g_wfx_3D[i].Format.nBlockAlign = g_wfx_3D[i].Format.nChannels * g_wfx_3D[i].Format.wBitsPerSample / 8;
            g_wfx_3D[i].Format.nAvgBytesPerSec = g_wfx_3D[i].Format.nBlockAlign * g_wfx_3D[i].Format.nSamplesPerSec;

            sprintf_s(buf, "InitSound3D: Converted %s to mono.\n", g_aParam3D[i].filename);
            PrintDebugProc(buf);
        }

        // SourceVoice
        hr = g_pXAudio2_3D->CreateSourceVoice(&g_apSourceVoice_3D[i], &g_wfx_3D[i].Format);
        if (FAILED(hr))
        {
            sprintf_s(buf, "InitSound3D: Failed to create SourceVoice for %s, hr=0x%08X\n", g_aParam3D[i].filename, hr);
            PrintDebugProc(buf);
            return FALSE;
        }
        else
        {
            sprintf_s(buf, "InitSound3D: Created SourceVoice for %s successfully.\n", g_aParam3D[i].filename);
            PrintDebugProc(buf);
        }

        ZeroMemory(&g_emitter[i], sizeof(X3DAUDIO_EMITTER));
        g_emitter[i].ChannelCount = 1;
        g_emitter[i].Position = XMFLOAT3(0, 0, 0);
        g_emitter[i].OrientFront = XMFLOAT3(0, 0, 1);
        g_emitter[i].OrientTop = XMFLOAT3(0, 1, 0);
        g_emitter[i].pVolumeCurve = nullptr;
        g_emitter[i].pLFECurve = nullptr;
        g_emitter[i].pChannelAzimuths = nullptr;
    }

    return TRUE;
}

//*****************************************************************************
// 終了処理
//*****************************************************************************
void UninitSound3D()
{
    for (int i = 0; i < SOUND3D_MAX; i++)
    {
        if (g_apSourceVoice_3D[i])
        {
            g_apSourceVoice_3D[i]->Stop(0);
            g_apSourceVoice_3D[i]->DestroyVoice();
            g_apSourceVoice_3D[i] = nullptr;
            free(g_apDataAudio_3D[i]);
            g_apDataAudio_3D[i] = nullptr;
        }
    }
    if (g_pMasteringVoice_3D) g_pMasteringVoice_3D->DestroyVoice();
    if (g_pXAudio2_3D) g_pXAudio2_3D->Release();
    g_pMasteringVoice_3D = nullptr;
    g_pXAudio2_3D = nullptr;
    CoUninitialize();
}

//*****************************************************************************
// 更新処理
//*****************************************************************************
void UpdateListener(XMFLOAT3 listenerPos, XMFLOAT3 listenerFront)
{
    g_listener.Position = listenerPos;
    g_listener.OrientFront = listenerFront;
}

void UpdateSound3D()
{
    PLAYER* player = GetPlayer();
    XMFLOAT3 listenerPos = player->GetPosition();

    XMFLOAT3 listenerFront;
    listenerFront.x = sinf(player->rot.y);
    listenerFront.y = 0.0f;
    listenerFront.z = cosf(player->rot.y);

    g_listener.Position = listenerPos;
    g_listener.OrientFront = listenerFront;
    g_listener.OrientTop = XMFLOAT3(0, 1, 0);
    g_listener.Velocity = XMFLOAT3(0, 0, 0);

    XMFLOAT3 testEmitterPos = listenerPos;
    testEmitterPos.x += 5.0f;  
    g_emitter[0].Position = testEmitterPos; 

    char buf[256];
    sprintf_s(buf, "Listener Pos: %.2f, %.2f, %.2f | Emitter Pos: %.2f, %.2f, %.2f\n",
        g_listener.Position.x, g_listener.Position.y, g_listener.Position.z,
        g_emitter[0].Position.x, g_emitter[0].Position.y, g_emitter[0].Position.z);
    PrintDebugProc(buf);

    X3DAUDIO_DSP_SETTINGS dspSettings = {};
    FLOAT32 matrix[2]; // stereo
    dspSettings.pMatrixCoefficients = matrix;
    dspSettings.SrcChannelCount = 1;
    dspSettings.DstChannelCount = 2;

    X3DAudioCalculate(g_X3DInstance, &g_listener, &g_emitter[0],
        X3DAUDIO_CALCULATE_MATRIX | X3DAUDIO_CALCULATE_LPF_REVERB,
        &dspSettings);

    sprintf_s(buf, "Matrix: %.6f, %.6f\n", matrix[0], matrix[1]);
    PrintDebugProc(buf);

    if (g_apSourceVoice_3D[0])
        g_apSourceVoice_3D[0]->SetOutputMatrix(nullptr, 1, 2, dspSettings.pMatrixCoefficients);
}

//*****************************************************************************
// 再生処理
//*****************************************************************************
void PlaySound3D(int label, XMFLOAT3 pos)
{
    if (!g_apSourceVoice_3D[label])
    {
        char buf[128];
        sprintf_s(buf, "PlaySound3D: SourceVoice for label %d is nullptr!\n", label);
        PrintDebugProc(buf);
        return;
    }

    XAUDIO2_VOICE_STATE state;
    g_apSourceVoice_3D[label]->GetState(&state);

    if (state.BuffersQueued != 0)
    {
        g_apSourceVoice_3D[label]->Stop(0);
        g_apSourceVoice_3D[label]->FlushSourceBuffers();
    }

    g_emitter[label].Position = pos;

    X3DAUDIO_DSP_SETTINGS dspSettings = {};
    FLOAT32 matrix[2]; // stereo
    dspSettings.pMatrixCoefficients = matrix;
    dspSettings.SrcChannelCount = 1;
    dspSettings.DstChannelCount = 2;

    X3DAudioCalculate(g_X3DInstance, &g_listener, &g_emitter[label],
        X3DAUDIO_CALCULATE_MATRIX, &dspSettings);

    char buf[256];
    sprintf_s(buf, "PlaySound3D: matrix[0]=%.3f, matrix[1]=%.3f\n", matrix[0], matrix[1]);
    PrintDebugProc(buf);

    g_apSourceVoice_3D[label]->SetOutputMatrix(nullptr, 1, 2, dspSettings.pMatrixCoefficients);

    XAUDIO2_BUFFER buffer = {};
    buffer.AudioBytes = g_aSizeAudio_3D[label];
    buffer.pAudioData = g_apDataAudio_3D[label];
    buffer.Flags = XAUDIO2_END_OF_STREAM;
    buffer.LoopCount = g_aParam3D[label].loop;

    g_apSourceVoice_3D[label]->SubmitSourceBuffer(&buffer);
    g_apSourceVoice_3D[label]->Start(0);
}

void PlaySound3D_Test(int label)
{
    if (!g_apSourceVoice_3D[label]) return;

    g_apSourceVoice_3D[label]->Stop(0);
    g_apSourceVoice_3D[label]->FlushSourceBuffers();

    XAUDIO2_BUFFER buffer = {};
    buffer.AudioBytes = g_aSizeAudio_3D[label];
    buffer.pAudioData = g_apDataAudio_3D[label];
    buffer.Flags = XAUDIO2_END_OF_STREAM;
    buffer.LoopCount = 0;

    g_apSourceVoice_3D[label]->SubmitSourceBuffer(&buffer);
    g_apSourceVoice_3D[label]->SetVolume(1.0f);
    g_apSourceVoice_3D[label]->Start(0);
}

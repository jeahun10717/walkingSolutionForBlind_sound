#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <alsa/asoundlib.h>

using namespace std;

// WAV 파일 헤더 구조체
struct WAVHeader {
    char riff[4];           // "RIFF"
    unsigned int fileSize;  // 파일 크기 - 8
    char wave[4];           // "WAVE"
    char fmt[4];            // "fmt "
    unsigned int fmtSize;   // 포맷 청크 크기
    // 새로운 필드 추가
    unsigned short audioFormat;  // 오디오 포맷 (1은 PCM)
    unsigned short numChannels;  // 채널 수
    unsigned int sampleRate;     // 샘플 레이트
    unsigned int byteRate;       // 바이트 레이트
    unsigned short blockAlign;   // 블록 얼라인
    unsigned short bitsPerSample;  // 샘플 당 비트 수
};

// 오디오 데이터 읽기 및 재생 함수
void playAudio(const string& filePath, double volumeBalance) {
    cout << "소리 재생 시작!" << '\n';

    // WAV 파일 열기
    ifstream file(filePath, ios::binary);
    if (!file) {
        cout << "파일을 열 수 없습니다." << endl;
        return;
    }

    // WAV 헤더 읽기
    WAVHeader header;
    file.read(reinterpret_cast<char*>(&header), sizeof(header));

    // 오디오 파라미터 설정
    unsigned short numChannels = header.numChannels;
    unsigned short blockAlign = header.blockAlign;

    // ALSA 오디오 장치 열기
    snd_pcm_t* handle;
    int err;
    if ((err = snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0)) < 0) 
    {
        cout << "오디오 장치를 열 수 없습니다: " << snd_strerror(err) << endl;
        return;
    }

    // 오디오 파라미터 설정
    snd_pcm_hw_params_t* params;
    snd_pcm_hw_params_alloca(&params);
    snd_pcm_hw_params_any(handle, params);
    snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);
    snd_pcm_hw_params_set_channels(handle, params, numChannels);
    snd_pcm_hw_params_set_rate_near(handle, params, &header.sampleRate, 0);
    snd_pcm_hw_params(handle, params);

    // 버퍼 할당 및 초기화
    const int bufferSize = 1024;
    short buffer[bufferSize];
    memset(buffer, 0, sizeof(buffer));

    // 좌우 소리 균형 설정
    int leftVolume = static_cast<int>(32767 * volumeBalance);
    int rightVolume = 32767 - leftVolume;

    // 오디오 데이터 읽기 및 재생
    while (file.read(reinterpret_cast<char*>(buffer), sizeof(buffer))) {
        for (int i = 0; i < bufferSize; i += numChannels) {
            buffer[i] = static_cast<short>(buffer[i] * leftVolume / 32767);
            if (numChannels == 2) {
                buffer[i + 1] = static_cast<short>(buffer[i + 1] * rightVolume / 32767);
            }
        }
        if ((err = snd_pcm_writei(handle, buffer, bufferSize / blockAlign)) < 0) {
            cout << "오디오 재생 중 오류 발생: " << snd_strerror(err) << endl;
            break;
        }
        memset(buffer, 0, sizeof(buffer));
    }
    
    // 파일 및 장치 닫기
    file.close();
    snd_pcm_close(handle);

    cout << "소리 재생 종료!" << '\n';
}

int main() {
    string filePath = "/home/jetson4/sound_cpp/warning_sound.wav";
    double volumeBalance = 0.5;  // 좌우 소리 균형 값 (0.0은 완전 왼쪽, 1.0은 완전 오른쪽)

    for (int i = 0; i < 10; i++) {
        std::chrono::seconds duration(1);
        std::this_thread::sleep_for(duration);
        playAudio(filePath, volumeBalance);
    }
    return 0;
}


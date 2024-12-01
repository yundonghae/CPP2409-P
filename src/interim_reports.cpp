#include <iostream>
#include <vector>
#include <algorithm>          // shuffle
#include <random>             // random_device, mt19937
#include <thread>             // thread
#include <condition_variable> // condition_variable
#include <chrono>             // chrono

using namespace std;

void shuffleWord(string &word)
{
    random_device rd;                             // 난수 생성기
    mt19937 generator(rd());                      // Mersenne Twister 엔진
    shuffle(word.begin(), word.end(), generator); // std::shuffle 사용
}

vector<string> chooseTheme()
{
    int choice;
    cout << "주제를 선택하세요:\n1. 과일\n2. 동물\n3. 직업\n선택: ";
    cin >> choice;

    switch (choice)
    {
    case 1:
        return {"apple", "banana", "mango", "grape", "melon", "orange"};
    case 2:
        return {"lion", "tiger", "elephant", "monkey", "rabbit", "mouse"};
    case 3:
        return {"doctor", "engineer", "teacher", "nurse", "lawyer"};
    default:
        cout << "잘못된 입력입니다. 기본 주제(과일)로 진행합니다.\n";
        return {"apple", "banana", "cherry", "grape", "melon", "orange"};
    }
}

// 단어 길이에 따라 난이도 조절
void adjustDifficulty(vector<string> &words)
{
    sort(words.begin(), words.end(), [](const string &a, const string &b)
         { return a.size() < b.size(); });
}

// 제한 시간 기능 추가
bool userInputReceived = false;
condition_variable cv;
mutex mtx;

void waitForInput(string &playerGuess)
{
    cin >> playerGuess; // 사용자 입력 대기
    lock_guard<mutex> lock(mtx);
    userInputReceived = true; // 입력 완료 표시
    cv.notify_one();          // 입력 완료 알림 (제한 시간을 기다리는 스레드에 입력이 완료되었음을 알림)
}

bool startTimer(int timeLimit, string &playerGuess)
{
    unique_lock<mutex> lock(mtx);
    if (cv.wait_for(lock, chrono::seconds(timeLimit), []
                    { return userInputReceived; }))
    {
        return true; // 시간 내 입력 완료
    }
    else
    {
        return false; // 시간 초과
    }
}

int main()
{
    int score = 0;        // 초기 점수 설정
    char playAgain = 'y'; // 재시작 여부

    while (playAgain == 'y' || playAgain == 'Y') // 대소문자
    {
        // 주제 선택 및 단어 로드
        vector<string> words = chooseTheme();

        // 단어 난이도 조절
        adjustDifficulty(words);

        // 게임 시작
        for (const string &originalWord : words)
        {
            // 단어 섞기
            string scrambledWord = originalWord;
            shuffleWord(scrambledWord);

            cout << "\n섞인 단어: " << scrambledWord << "\n";
            cout << "원래 단어를 맞춰보세요 (제한 시간: 10초): ";

            string playerGuess;
            userInputReceived = false;                          // 초기화
            thread inputThread(waitForInput, ref(playerGuess)); // 입력 대기 스레드 시작

            if (startTimer(10, playerGuess))
            {
                inputThread.join(); // 입력이 완료되면 스레드 종료
                if (playerGuess == originalWord)
                {
                    cout << "정답입니다!\n";
                    score += 10; // 정답 시 점수 증가
                }
                else
                {
                    cout << "틀렸습니다. 정답은 '" << originalWord << "' 입니다.\n";
                    score -= 5; // 오답 시 점수 감소
                }
            }
            else
            {
                inputThread.detach(); // 시간 초과 시 스레드 분리
                cout << "\n시간 초과! 정답은 '" << originalWord << "' 입니다.\n";
                score -= 5; // 시간 초과 시 점수 감소
            }

            cout << "현재 점수: " << score << "\n";

            // 플레이어가 중간에 종료를 원하면 루프를 빠져나갑니다.
            cout << "다음 단어로 진행하시겠습니까? (y/n): ";
            char next;
            cin >> next;
            if (next != 'y' && next != 'Y')
                break;
        }

        // 재시작 여부 확인
        cout << "다시 하시겠습니까? (y/n): ";
        cin >> playAgain;
    }
    cout << "\n최종 점수는 " << score << "점 입니다!\n";
    return 0;
}
#include <iostream>
#include <vector>
#include <algorithm>          // shuffle
#include <random>             // random_device, mt19937
#include <thread>             // thread
#include <condition_variable> // condition_variable
#include <chrono>             // chrono
#include <fstream>            // ifstream, ofstream (파일 입출력)

using namespace std;

// 최고 점수
int highScore = 0;                              // 프로그램 실행 시 저장된 기록을 불러올 변수
const string HIGH_SCORE_FILE = "highscore.txt"; // 최고 점수 저장 파일 이름

// 최고 점수 불러오기
void loadHighScore()
{
    ifstream file(HIGH_SCORE_FILE);
    if (file.is_open())
    {
        file >> highScore;
        file.close();
    }
    else
    {
        highScore = 0; // 파일이 없거나 열 수 없으면 기본값 0
    }
}

// 최고 점수 저장하기
void saveHighScore(int currentScore)
{
    // 현재 점수가 기존의 highScore보다 크면 갱신
    if (currentScore > highScore)
    {
        highScore = currentScore;
        ofstream file(HIGH_SCORE_FILE);
        if (file.is_open())
        {
            file << highScore;
            file.close();
        }
    }
}

// 단어 섞기 함수
void shuffleWord(string &word)
{
    random_device rd;                             // 난수 생성기
    mt19937 generator(rd());                      // Mersenne Twister 엔진
    shuffle(word.begin(), word.end(), generator); // std::shuffle 사용
}

// 주제 선택 함수
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

// 제한 시간 기능과 관련된 전역 변수, 조건 변수, 뮤텍스
bool userInputReceived = false;
condition_variable cv;
mutex mtx;

// 사용자 입력 대기 함수 (별도 스레드에서 실행)
void waitForInput(string &playerGuess)
{
    cin >> playerGuess; // 사용자 입력
    lock_guard<mutex> lock(mtx);
    userInputReceived = true;
    cv.notify_one(); // 입력 완료 알림
}

// 타이머 함수
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
    // 프로그램 시작 시 최고 점수 로드
    loadHighScore();
    cout << "현재 최고 점수: " << highScore << "점\n";

    int score = 0;        // 현재 세션의 점수
    char playAgain = 'y'; // 재시작 여부

    while (playAgain == 'y' || playAgain == 'Y')
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
            userInputReceived = false;
            thread inputThread(waitForInput, ref(playerGuess));

            if (startTimer(10, playerGuess))
            {
                inputThread.join(); // 입력 완료
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
                inputThread.detach(); // 시간 초과
                cout << "\n시간 초과! 정답은 '" << originalWord << "' 입니다.\n";
                score -= 5; // 시간 초과 시 점수 감소
            }

            cout << "현재 점수: " << score << "\n";

            // 중단 여부
            cout << "다음 단어로 진행하시겠습니까? (y/n): ";
            char next;
            cin >> next;
            if (next != 'y' && next != 'Y')
                break;
        }

        // 다시 시작 여부
        cout << "다시 하시겠습니까? (y/n): ";
        cin >> playAgain;
    }

    // 게임 종료 후 최종 점수와 최고 점수 갱신
    cout << "\n최종 점수: " << score << "점 입니다!\n";
    saveHighScore(score); // 최고 점수 갱신 시도
    cout << "현재 최고 점수: " << highScore << "점\n";
    return 0;
}

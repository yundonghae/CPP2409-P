#include <iostream>
#include <vector>
#include <algorithm> // shuffle
#include <random>    // random_device, mt19937

using namespace std;

void shuffleWord(string &word) {
    random_device rd;              // 난수 생성기
    mt19937 generator(rd());       // Mersenne Twister 엔진
    shuffle(word.begin(), word.end(), generator); // std::shuffle 사용
}

vector<string> chooseTheme() {
    int choice;
    cout << "주제를 선택하세요:\n1. 과일\n2. 동물\n3. 직업\n선택: ";
    cin >> choice;

    switch (choice) {
        case 1: return {"apple", "banana", "mango", "grape", "melon", "orange"};
        case 2: return {"lion", "tiger", "elephant", "monkey", "rabbit", "mouse"};
        case 3: return {"doctor", "engineer", "teacher", "nurse", "lawyer"};
        default:
            cout << "잘못된 입력입니다. 기본 주제(과일)로 진행합니다.\n";
            return {"apple", "banana", "cherry", "grape", "melon", "orange"};
    }
}

int main() {
    // 주제 선택 및 단어 로드
    vector<string> words = chooseTheme();

    // 랜덤 단어 선택
    random_device rd;              
    mt19937 generator(rd());       
    uniform_int_distribution<int> distribution(0, words.size() - 1);
    string originalWord = words[distribution(generator)];

    // 단어 섞기
    string scrambledWord = originalWord;
    shuffleWord(scrambledWord);

    cout << "섞인 단어: " << scrambledWord << "\n";
    cout << "원래 단어를 맞춰보세요: ";

    string playerGuess;
    cin >> playerGuess;

    if (playerGuess == originalWord) {
        cout << "정답입니다!\n";
    } else {
        cout << "틀렸습니다. 정답은 '" << originalWord << "' 입니다.\n";
    }

    return 0;
}



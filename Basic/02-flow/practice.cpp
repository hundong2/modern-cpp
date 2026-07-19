#include <iostream>
#include <string>
#include <vector>

std::string grade_of(int score)
{
    if (score >= 90) {
        return "A";
    }
    if (score >= 80) {
        return "B";
    }
    if (score >= 70) {
        return "C";
    }
    if (score >= 60) {
        return "D";
    }
    return "F";
}

int sum_even_numbers(const std::vector<int>& values)
{
    int sum = 0;
    for (const int value : values) {
        if (value % 2 == 0) {
            sum += value;
        }
    }
    return sum;
}

void normalize_score(int& score)
{
    if (score < 0) {
        score = 0;
    } else if (score > 100) {
        score = 100;
    }
}

int main()
{
    int score = 107;
    normalize_score(score);

    const std::vector<int> data = {1, 2, 3, 4, 5, 6};

    std::cout << "score: " << score << ", grade: " << grade_of(score) << '\n';
    std::cout << "sum of evens: " << sum_even_numbers(data) << '\n';

    for (int i = 1; i <= 15; ++i) {
        if (i % 15 == 0) {
            std::cout << "FizzBuzz ";
        } else if (i % 3 == 0) {
            std::cout << "Fizz ";
        } else if (i % 5 == 0) {
            std::cout << "Buzz ";
        } else {
            std::cout << i << ' ';
        }
    }
    std::cout << '\n';

    return 0;
}

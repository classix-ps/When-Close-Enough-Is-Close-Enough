// Shunting-yard Algorithm
// https://en.wikipedia.org/wiki/Shunting-yard_algorithm
//
// https://ideone.com/DYX5CW
//
// License:
//  See https://gist.github.com/t-mat/b9f681b7591cdae712f6#gistcomment-3638162
//
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <deque>
#include <stdio.h>
#include <tuple>

#include "mpreal.h"
//#include <mpfr.h> // wrapped by mpreal for C++

class Fraction {
private:
    // Calculates the greates common divisor with
    // Euclid's algorithm
    // both arguments have to be positive
    long long gcd(long long a, long long b) {
        while (a != b) {
            if (a > b) {
                a -= b;
            }
            else {
                b -= a;
            }
        }
        return a;
    }

public:
    long long numerator, denominator;

    Fraction() {
        numerator = 1;
        denominator = 1;
    }

    Fraction(int n) {
        numerator = n;
        denominator = 1;
    }

    Fraction(long long n, long long d) {
        if (d == 0) {
            std::cout << "Denominator may not be 0." << std::endl;
            exit(0);
        }
        else if (n == 0) {
            numerator = 0;
            denominator = 1;
        }
        else {
            int sign = 1;
            if (n < 0) {
                sign *= -1;
                n *= -1;
            }
            if (d < 0) {
                sign *= -1;
                d *= -1;
            }

            long long tmp = gcd(n, d);
            numerator = n / tmp * sign;
            denominator = d / tmp;
        }
    }

    operator int() { return numerator / denominator; }
    operator float() { return ((float)numerator) / denominator; }
    operator double() { return ((double)numerator) / denominator; }
};

Fraction operator+(Fraction lhs, Fraction rhs) {
    Fraction tmp(lhs.numerator * rhs.denominator
        + rhs.numerator * lhs.denominator,
        lhs.denominator * rhs.denominator);
    return tmp;
}

Fraction operator-(Fraction lhs, Fraction rhs) {
    Fraction tmp(lhs.numerator * rhs.denominator
        - rhs.numerator * lhs.denominator,
        lhs.denominator * rhs.denominator);
    return tmp;
}

Fraction operator*(Fraction lhs, Fraction rhs) {
    Fraction tmp(lhs.numerator * rhs.numerator,
        lhs.denominator * rhs.denominator);
    return tmp;
}
Fraction operator*(int lhs, Fraction rhs) {
    Fraction tmp(lhs * rhs.numerator, rhs.denominator);
    return tmp;
}
Fraction operator*(Fraction rhs, int lhs) {
    Fraction tmp(lhs * rhs.numerator, rhs.denominator);
    return tmp;
}

Fraction operator/(Fraction lhs, Fraction rhs) {
    Fraction tmp(lhs.numerator * rhs.denominator,
        lhs.denominator * rhs.numerator);
    return tmp;
}


Fraction operator^(Fraction lhs, Fraction rhs) {
    if (rhs.denominator == 1) {
        Fraction tmp(static_cast<long long>(pow(lhs.numerator, rhs.numerator)), static_cast<long long>(pow(lhs.denominator, rhs.numerator)));
        return tmp;
    }
    else {
        double val = abs(pow(double(abs(lhs.numerator)) / lhs.denominator, double(rhs.numerator) / rhs.denominator));
        if (val < 1)
            val = 1 / val;
        Fraction tmp(static_cast<int>(ceil(val)));
        return tmp;
    }
}


class Lambda {
public:
	unsigned long long n, b;

	Lambda() {
		n = 0;
		b = 0;
	}
    Lambda(unsigned long long dim, unsigned long long max) {
        n = dim;
        b = max;
    }

    // 1., 2. irrelevant

	// 4.
	Lambda(int k) {
		n = 1;
		b = abs(k);
	}
};

// 5.
Lambda operator*(Lambda lhs, Lambda rhs) {
    Lambda tmp(lhs.n * rhs.n, lhs.b * rhs.b);
    return tmp;
}
Lambda operator/(Lambda lhs, Lambda rhs) {
    return lhs * rhs; // This works because of Rule 7
}

// 6.
Lambda operator+(Lambda lhs, Lambda rhs) {
    Lambda tmp(lhs.n * rhs.n, lhs.b + rhs.b);
    return tmp;
}
Lambda operator-(Lambda lhs, Lambda rhs) {
    return lhs + rhs; // This works because of Rule 3
}

// 7.
Lambda operator^(Lambda base, Fraction exponent) {
    Lambda tmp(base);
    for (size_t mult = 1; mult < exponent.numerator; mult++) {
        tmp = tmp * base;
    }
    tmp.n *= exponent.denominator;
    return tmp;
}

static const char* reportFmt = "|%-5s|%-37s|%17s| %s\n";

class Token {
public:
    enum class Type {
        Unknown,
        Number,
        Operator,
        LeftParen,
        RightParen,
    };

    Token(Type t, const std::string& s, int prec = -1, bool ra = false)
        : type{ t }, str(s), precedence{ prec }, rightAssociative{ ra }
    {}

    const Type type;
    const std::string str;
    const int precedence;
    const bool rightAssociative;
};

std::ostream& operator<<(std::ostream& os, const Token& token) {
    os << token.str;
    return os;
}

std::deque<Token> exprToTokens(const std::string& expr) {
    std::deque<Token> tokens;

    for (const auto* p = expr.c_str(); *p; ++p) {
        if (isblank(*p)) {
            // do nothing
        }
        else if (isdigit(*p)) {
            const auto* b = p;
            while (isdigit(*p)) {
                ++p;
            }
            const auto s = std::string(b, p);
            tokens.push_back(Token{ Token::Type::Number, s });
            --p;
        }
        else {
            Token::Type t = Token::Type::Unknown;
            int pr = -1;            // precedence
            bool ra = false;        // rightAssociative
            switch (*p) {
            default:                                    break;
            case '(':   t = Token::Type::LeftParen;     break;
            case ')':   t = Token::Type::RightParen;    break;
            case '^':   t = Token::Type::Operator;      pr = 4; ra = true;  break;
            case '*':   t = Token::Type::Operator;      pr = 3; break;
            case '/':   t = Token::Type::Operator;      pr = 3; break;
            case '+':   t = Token::Type::Operator;      pr = 2; break;
            case '-':   t = Token::Type::Operator;      pr = 2; break;
            }
            const auto s = std::string(1, *p);
            tokens.push_back(Token{ t, s, pr, ra });
        }
    }

    return tokens;
}


std::deque<Token> shuntingYard(const std::deque<Token>& tokens) {
    std::deque<Token> queue;
    std::vector<Token> stack;

    // While there are tokens to be read:
    for (auto token : tokens) {
        // Read a token
        switch (token.type) {
        case Token::Type::Number:
            // If the token is a number, then add it to the output queue
            queue.push_back(token);
            break;

        case Token::Type::Operator:
        {
            // If the token is operator, o1, then:
            const auto o1 = token;

            // while there is an operator token,
            while (!stack.empty()) {
                // o2, at the top of stack, and
                const auto o2 = stack.back();

                // either o1 is left-associative and its precedence is
                // *less than or equal* to that of o2,
                // or o1 if right associative, and has precedence
                // *less than* that of o2,
                if (
                    (!o1.rightAssociative && o1.precedence <= o2.precedence)
                    || (o1.rightAssociative && o1.precedence < o2.precedence)
                    ) {
                    // then pop o2 off the stack,
                    stack.pop_back();
                    // onto the output queue;
                    queue.push_back(o2);

                    continue;
                }

                // @@ otherwise, exit.
                break;
            }

            // push o1 onto the stack.
            stack.push_back(o1);
        }
        break;

        case Token::Type::LeftParen:
            // If token is left parenthesis, then push it onto the stack
            stack.push_back(token);
            break;

        case Token::Type::RightParen:
            // If token is right parenthesis:
        {
            bool match = false;

            // Until the token at the top of the stack
            // is a left parenthesis,
            while (!stack.empty() && stack.back().type != Token::Type::LeftParen) {
                // pop operators off the stack
                // onto the output queue.
                queue.push_back(stack.back());
                stack.pop_back();
                match = true;
            }

            // Pop the left parenthesis from the stack,
            // but not onto the output queue.
            stack.pop_back();

            if (!match && stack.empty()) {
                // If the stack runs out without finding a left parenthesis,
                // then there are mismatched parentheses.
                std::cout << "RightParen error: " << token.str << std::endl;
                return {};
            }
        }
        break;

        default:
            std::cout << "error: " << token.str << std::endl;
            return {};
        }
    }

    // When there are no more tokens to read:
    //   While there are still operator tokens in the stack:
    while (!stack.empty()) {
        // If the operator token on the top of the stack is a parenthesis,
        // then there are mismatched parentheses.
        if (stack.back().type == Token::Type::LeftParen) {
            std::cout << "Mismatched parentheses error" << std::endl;
            return {};
        }

        // Pop the operator onto the output queue.
        queue.push_back(std::move(stack.back()));
        stack.pop_back();
    }

    //Exit.
    return queue;
}

bool isEqual(std::string val1, std::string val2) {
    std::string equation = val1 + '-' + val2;
    std::deque<Token> tokens = exprToTokens(equation);
    std::deque<Token> queue = shuntingYard(tokens);
    std::deque<Token> calcQueue(queue);

    std::vector<std::pair<Fraction, Lambda>> stack;
    while (!queue.empty()) {
        Token token = queue.front();
        queue.pop_front();
        switch (token.type) {
        case Token::Type::Number:
            stack.push_back(std::make_pair(Fraction(std::stoi(token.str)), Lambda(std::stoi(token.str))));
            break;

        case Token::Type::Operator:
        {
            std::pair<Fraction, Lambda> rhs = stack.back();
            stack.pop_back();
            std::pair<Fraction, Lambda> lhs = stack.back();
            stack.pop_back();

            switch (token.str[0]) {
            case '^':
                stack.push_back(std::make_pair(lhs.first ^ rhs.first, lhs.second ^ rhs.first));
                break;
            case '*':
                stack.push_back(std::make_pair(lhs.first * rhs.first, lhs.second * rhs.second));
                break;
            case '/':
                stack.push_back(std::make_pair(lhs.first / rhs.first, lhs.second / rhs.second));
                break;
            case '+':
                stack.push_back(std::make_pair(lhs.first + rhs.first, lhs.second + rhs.second));
                break;
            case '-':
                stack.push_back(std::make_pair(lhs.first - rhs.first, lhs.second - rhs.second));
                break;
            default:
                std::cout << "Operator error: " << token.str << std::endl;
                exit(0);
                break;
            }
            break;
        }

        default:
            std::cout << "Token error." << std::endl;
            exit(0);
        }
    }

    Lambda lam = stack.back().second;
    std::cout << "Final Lambda: " << lam.n << ", " << lam.b << std::endl;
    mpfr::mpreal prod = lam.n * lam.b;
    size_t digits = prod.toString().length() * lam.n;
    mpfr::mpreal::set_default_prec(mpfr::digits2bits(digits));
    mpfr::mpreal epsilon = mpfr::mpreal(1) / pow(lam.n * lam.b, lam.n - 1);
    std::cout << "Corresponding epsilon bound: " << epsilon << std::endl;

    std::vector<mpfr::mpreal> calcStack;
    while (!calcQueue.empty()) {
        Token token = calcQueue.front();
        calcQueue.pop_front();
        switch (token.type) {
        case Token::Type::Number:
            calcStack.push_back(mpfr::mpreal(token.str));
            break;

        case Token::Type::Operator:
        {
            mpfr::mpreal rhs = calcStack.back();
            calcStack.pop_back();
            mpfr::mpreal lhs = calcStack.back();
            calcStack.pop_back();

            switch (token.str[0]) {
            case '^':
                calcStack.push_back(pow(lhs, rhs));
                break;
            case '*':
                calcStack.push_back(lhs * rhs);
                break;
            case '/':
                calcStack.push_back(lhs / rhs);
                break;
            case '+':
                calcStack.push_back(lhs + rhs);
                break;
            case '-':
                calcStack.push_back(lhs - rhs);
                break;
            }
            break;
        }
        }
    }

    mpfr::mpreal result = calcStack.back();

    return result < epsilon;
}

int main() {
    // Must be algebraic equations
    
    bool equal = isEqual("2^(1/2)+(5-2*(6)^(1/2))^(1/2)", "3^(1/2)");
    std::cout << "Algebraic integers equal? " << equal << std::endl;

    return 0;
}
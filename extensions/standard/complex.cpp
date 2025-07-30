#include "complex.hpp"
#include <sstream>

namespace lamina {

Complex::Complex(double real, double imag)
    : real(real), imag(imag) {}

Complex Complex::operator+(const Complex& other) const {
    return Complex(real + other.real, imag + other.imag);
}

Complex Complex::operator-(const Complex& other) const {
    return Complex(real - other.real, imag - other.imag);
}

Complex Complex::operator*(const Complex& other) const {
    return Complex(
        real * other.real - imag * other.imag,
        real * other.imag + imag * other.real
    );
}

Complex Complex::operator/(const Complex& other) const {
    double denominator = other.real * other.real + other.imag * other.imag;
    if (denominator == 0) {
        throw std::runtime_error("Division by zero");
    }
    return Complex(
        (real * other.real + imag * other.imag) / denominator,
        (imag * other.real - real * other.imag) / denominator
    );
}

bool Complex::operator==(const Complex& other) const {
    return real == other.real && imag == other.imag;
}

bool Complex::operator!=(const Complex& other) const {
    return !(*this == other);
}

Complex Complex::conjugate() const {
    return Complex(real, -imag);
}

double Complex::magnitude() const {
    return std::sqrt(real * real + imag * imag);
}

double Complex::phase() const {
    return std::atan2(imag, real);
}

std::string Complex::toString() const {
    std::ostringstream oss;
    if (imag == 0) {
        oss << real;
    } else if (real == 0) {
        oss << imag << "i";
    } else {
        oss << real;
        if (imag > 0) {
            oss << "+" << imag << "i";
        } else {
            oss << "-" << -imag << "i";
        }
    }
    return oss.str();
}

} // namespace lamina

#ifndef COMPLEX_HPP
#define COMPLEX_HPP

#include <cmath>
#include <complex>

namespace lamina {
    class Complex {
    public:
        double real;
        double imag;

        Complex(double real = 0.0, double imag = 0.0);

        // Basic arithmetic operations
        Complex operator+(const Complex& other) const;
        Complex operator-(const Complex& other) const;
        Complex operator*(const Complex& other) const;
        Complex operator/(const Complex& other) const;

        // Comparison operators
        bool operator==(const Complex& other) const;
        bool operator!=(const Complex& other) const;

        // Conjugate
        Complex conjugate() const;

        // Magnitude
        double magnitude() const;

        // Phase angle
        double phase() const;

        // String representation
        std::string toString() const;
    };
}

#endif // COMPLEX_HPP

#include <iostream>
#include <complex>
#include "Display.h"

using C = std::complex<double>;

static int N = 1;
static C JULIA_C = C{ 0.4, -0.325 };

static char mandelbrot(double x, double y) {
	C c;
	c.real(x);
	c.imag(y);

	C z = c;
	for (int n = 0; n < N; n++) {
		z = z * z + c;
		if (abs(z) > 2) {
			return ' ';
		}
	}

	return '+';
}

static char mandelbrot_emulating_complex_numbers(double x, double y) {
    glm::vec2 c{x, y};
    glm::vec2 z{c};

    auto mul = [](const glm::vec2 &a, const glm::vec2 &b) {
        double real = (a.x * b.x) - (a.y * b.y);
        double imag = (a.x * b.y) + (a.y * b.x);
        return glm::vec2{real, imag};
    };

    auto length = [](const glm::vec2 &v) {
        return std::sqrt(v.x * v.x + v.y * v.y);
    };

    for(int n = 0; n < N; n++) {
        z = mul(z, z) + c;
        if(length(z) > 2) {
            return ' ';
        }
    }

    return '+';
}

static char julia(double x, double y) {

	C z{ x, y };
	for (int n = 0; n < N; n++) {
		z = z * z + JULIA_C;
		if (abs(z) > 2) {
			return ' ';
		}
	}

	return '+';
}

int main()
{
	Display d;
	d.setViewportSize({ 100, 50 });
	d.setViewportOrigin(Display::Origin::CENTER);
	d.setShader(&mandelbrot_emulating_complex_numbers);

	while (true) {
		d.draw();
		N++;
		std::cin.ignore();
	}

}

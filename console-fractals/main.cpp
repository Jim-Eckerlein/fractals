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
	d.setViewportSize({ 300, 200 });
	d.setViewportOrigin(Display::Origin::CENTER);
	d.setShader(&mandelbrot);

	while (true) {
		d.draw();
		N++;
		std::cin.ignore();
	}

}

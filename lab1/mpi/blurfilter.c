#include <stdio.h>
#include <stdlib.h>
#include "blurfilter.h"
#include "../ppmio.h"

pixel *pix(pixel *image, const int xx, const int yy, const int xsize)
{
	int off = xsize * yy + xx;
	return (image + off);
}

void compute_row(int y, int xsize, int radius, const double *weights, pixel* buf, pixel* dst)
{
	for (int x = 0; x < xsize; ++x)
	{
		double r = 0, g = 0, b = 0, n = 0;
		for (int wi = -radius; wi <= radius; wi++)
		{
			double wc = weights[abs(wi)];
			int x2 = x + wi;
			if (x2 >= 0 && x2 < xsize)
			{
				r += wc * pix(buf, x2, y, xsize)->r;
				g += wc * pix(buf, x2, y, xsize)->g;
				b += wc * pix(buf, x2, y, xsize)->b;
				n += wc;
			}
		}

		pix(dst, x, y, xsize)->r = r / n;
		pix(dst, x, y, xsize)->g = g / n;
		pix(dst, x, y, xsize)->b = b / n;
	}
}

void compute_col(int x, int xsize, int ysize, int radius, const double *weights, pixel* buf, pixel* dst)
{
	for (int y = 0; y < args->ysize; ++y)
	{

		double r = 0, g = 0, b = 0, n = 0;
		for (int wi = -radius; wi <= radius; wi++)
		{
			double wc = weights[abs(wi)];
			int y2 = y + wi;
			if (y2 >= 0 && y2 < ysize)
			{
				r += wc * pix(buf, x, y2, xsize)->r;
				g += wc * pix(buf, x, y2, xsize)->g;
				b += wc * pix(buf, x, y2, xsize)->b;
				n += wc;
			}
		}

		pix(dst, x, y, xsize)->r = r / n;
		pix(dst, x, y, xsize)->g = g / n;
		pix(dst, x, y, xsize)->b = b / n;
	}
}
/*======================================================================
    Maratis Tiny C Library
    version 1.0
------------------------------------------------------------------------
    Copyright (c) 2015 Anael Seghezzi <www.maratis3d.com>

    This software is provided 'as-is', without any express or implied
    warranty. In no event will the authors be held liable for any damages
    arising from the use of this software.

    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
    claim that you wrote the original software. If you use this software
    in a product, an acknowledgment in the product documentation would
    be appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not
    be misrepresented as being the original software.

    3. This notice may not be removed or altered from any source
    distribution.

========================================================================*/

/* voronoi test */

#define M_MATH_IMPLEMENTATION
#define M_IMAGE_IMPLEMENTATION
#define M_DIST_IMPLEMENTATION
#include <m_math.h>
#include <m_image.h>
#include <m_dist.h>

#include "../test.h"

#define POINT_COUNT 64
#define RANDF ((float)rand() / (float)RAND_MAX)

struct test_point
{
	float2 pos;
	float2 dir;
	float3 col;
};

static struct m_image test_buffer = M_IMAGE_IDENTITY();
static struct test_point points[POINT_COUNT];
static struct m_image tmp_buffer = M_IMAGE_IDENTITY();
static struct m_image tmpi = M_IMAGE_IDENTITY();


static void init(void)
{
	int i;
	for (i = 0; i < POINT_COUNT; i++) {
		points[i].pos.x = RANDF;
		points[i].pos.y = RANDF;
		points[i].dir.x = RANDF - 0.5f;
		points[i].dir.y = RANDF - 0.5f;
		points[i].col.x = RANDF;
		points[i].col.y = RANDF;
		points[i].col.z = RANDF;
	}
}

static void clear(void)
{
	m_image_destroy(&tmp_buffer);
	m_image_destroy(&tmpi);
}

static void animate(void)
{
    int i;
    for (i = 0; i < POINT_COUNT; i++) {

		/* move point */
		points[i].pos.x += points[i].dir.x * 0.01f;
		points[i].pos.y += points[i].dir.y * 0.01f;

		/* bounce x */
		if (points[i].pos.x > 1) {
			points[i].pos.x = 1;
			points[i].dir.x = -points[i].dir.x;
		}
		else if (points[i].pos.x < 0) {
			points[i].pos.x = 0;
			points[i].dir.x = -points[i].dir.x;
		}
		
		/* bounce y */
		if (points[i].pos.y > 1) {
			points[i].pos.y = 1;
			points[i].dir.y = -points[i].dir.y;
		}
		else if (points[i].pos.y < 0) {
			points[i].pos.y = 0;
			points[i].dir.y = -points[i].dir.y;
		}
    }  
}

static void draw(void)
{
	float *tmp_data;
	float *test_data;
	int w = test_buffer.width;
	int h = test_buffer.height;
	int size = w * h;
	int i;

	m_image_create(&tmp_buffer, M_FLOAT, w, h, 1);
	tmp_data = (float *)tmp_buffer.data;

	for (i = 0; i < size; i++)
		tmp_data[i] = M_DIST_MAX;

	for (i = 0; i < POINT_COUNT; i++) {

		/* draw point */
		int x = (int)(points[i].pos.x * (w - 1));
		int y = (int)(points[i].pos.y * (h - 1));

		/* mask */
		tmp_data[y * w + x] = 0;

		/* color */
		test_data = (float *)test_buffer.data + (y * w + x) * test_buffer.comp;
		test_data[0] = points[i].col.x;
		test_data[1] = points[i].col.y;
		test_data[2] = points[i].col.z;
	}

	m_image_voronoi_transform(&tmp_buffer, &tmpi, &tmp_buffer);
	m_image_voronoi_fill(&test_buffer, &test_buffer, &tmpi);
}

void main_loop(void)
{	
    animate();
	draw();
	test_swap_buffer(&test_buffer);
	test_update();
}

int main(int argc, char **argv)
{	
	if (! test_create("M - VoronoiTest", 256, 256))
		return EXIT_FAILURE;

	m_image_create(&test_buffer, M_FLOAT, 256, 256, 3);
	init();

	#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop(main_loop, 0, 1);
	#else
	while (test_state) {
		main_loop();
		thrd_yield();
	}
	#endif

	m_image_destroy(&test_buffer);
	clear();
	test_destroy();
	return EXIT_SUCCESS;
}

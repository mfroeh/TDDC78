#ifndef _coordinate_h
#define _coordinate_h

struct cord
{
    float x0;
    float x1;
    float y0;
    float y1;
};

struct part_cord
{
    float x;
    float y;
    float vx;
    float vy;
    float has_collided;
};

typedef struct cord cord_t;
typedef struct part_cord pcord_t;

#endif

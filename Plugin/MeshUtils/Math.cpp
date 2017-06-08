#include "pch.h"
#include "Math.h"

#ifdef muMath_AddNamespace
namespace mu {
#endif

const float PI = 3.14159265358979323846264338327950288419716939937510f;
const float Deg2Rad = PI / 180.0f;
const float Rad2Deg = 1.0f / (PI / 180.0f);

bool polygon_inside(const float2 points[], int num_points, const float2 pos)
{
    float miny, maxy;
    maxy = miny = points[0].y;
    for (int i = 1; i < num_points; i++)
    {
        miny = std::min<float>(miny, points[i].y);
        maxy = std::max<float>(maxy, points[i].y);
    }

    const int MaxXCoords = 64;
    float xcoords[MaxXCoords];
    int c = 0;
    int t, u;
    for (int i = 0; i < num_points; i++) {
        if (i == 0) {
            t = num_points - 1;
            u = 0;
        }
        else {
            t = i - 1;
            u = i;
        }

        float2 p1 = points[t];
        float2 p2 = points[u];
        if (p1.y == p2.y) { continue; }
        else if (p1.y > p2.y) { std::swap(p1, p2); }

        if ((pos.y >= p1.y && pos.y < p2.y) ||
            (pos.y == maxy && pos.y > p1.y && pos.y <= p2.y))
        {
            xcoords[c++] = (pos.y - p1.y) * (p2.x - p1.x) / (p2.y - p1.y) + p1.x;
            if (c == MaxXCoords - 1) break;
        }
    }
    std::sort(xcoords, xcoords + c);

    for (int i = 0; i < c; i += 2) {
        if (pos.x >= xcoords[i] && pos.x < xcoords[i + 1]) {
            return true;
        }
    }
    return false;
}

#ifdef muMath_AddNamespace
}
#endif

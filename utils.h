#include <vector>
#include <cmath>
#include <algorithm>
#include "tgaimage.cpp" //tga画图库
#include "model.cpp"    //模型类，主要实现模型的读取
#include "geometry.cpp" //几何库，主要定义了Vec2和Vec3类型

bool tri_compare(const Vec3i &a, const Vec3i &b)
{
    if (a.y != b.y)
        return a.y < b.y;
    return a.x < b.x;
}

struct ScreenTriangle
{
    std::vector<Vec3f> ts;
    ScreenTriangle(std::vector<Vec3f> &_ts) : ts(_ts)
    {
        std::sort(ts.begin(), ts.end(), tri_compare);
    }
};

struct Ray
{
    Vec3f o, d;
    Ray(Vec3f _o, Vec3f _d) : o(_o), d(_d) {}
};

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
Model *model = NULL;
// 定义宽度高度
const int width = 800;
const int height = 800;
const float K_d = 0.8;
const int l = -1, b = -1, n = 1, r = 1, t = 1, f = -1;
Ray light(Vec3f{0.0, 0.0, 10.0}, Vec3f{0.0, 0.0, 0.0});
float zbuffer[width][height];

void init_zbuffer()
{
    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            zbuffer[i][j] = std::numeric_limits<float>::min();
        }
    }
}

Vec3f barycentric(ScreenTriangle &t, Vec2f P)
{
    Vec3f AB = t.ts[1] - t.ts[0];
    Vec3f AC = t.ts[2] - t.ts[0];
    Vec3f PA = t.ts[0] - Vec3f{P.x, P.y, 0.f};

    Vec3f uv = cross(Vec3f{AB.x, AC.x, PA.x}, Vec3f{AB.y, AC.y, PA.y});

    if (std::abs(uv[2]) > 1e-2)
    {
        return Vec3f(1.f - (uv.x + uv.y) / uv.z, uv.y / uv.z, uv.x / uv.z);
    }
    return Vec3f{-1, 1, 1};
}

void draw_line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color)
{
    bool lean = false;
    if (std::abs((float)(y1 - y0) / (float)(x1 - x0)) > 1)
    {
        lean = true;
        std::swap(x0, y0);
        std::swap(x1, y1);
    }
    if (x0 > x1)
    {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }
    for (int x = x0; x < x1; x++)
    {
        float t = (float)(x - x0) / (float)(x1 - x0);
        int y = y0 * (1 - t) + y1 * t;
        image.set(lean ? y : x, lean ? x : y, color);
    }
}

void draw_triangle(ScreenTriangle &t, TGAImage &image, TGAColor color)
{
    for (float y = t.ts[0].y; y < t.ts[2].y; y++)
    {
        float x0 = (y - t.ts[2].y) / (t.ts[0].y - t.ts[2].y) * (t.ts[0].x - t.ts[2].x) + t.ts[2].x;
        float x1 = 0.f;
        if (y < t.ts[1].y)
        {
            x1 = (y - t.ts[1].y) / (t.ts[0].y - t.ts[1].y) * (t.ts[0].x - t.ts[1].x) + t.ts[1].x;
        }
        else
        {
            x1 = (y - t.ts[1].y) / (t.ts[2].y - t.ts[1].y) * (t.ts[2].x - t.ts[1].x) + t.ts[1].x;
        }
        if (x0 > x1)
        {
            std::swap(x0, x1);
        }
        // 直到最后作图再转成int
        for (float x = x0; x < x1; x++)
        {
            Vec3f uv = barycentric(t, Vec2f{x, y});
            float z = Vec3f{t.ts[0].z, t.ts[1].z, t.ts[2].z} * uv;
            if (z > zbuffer[(int)x][(int)y])
            {
                zbuffer[(int)x][(int)y] = z;
                image.set(x, (int)y, color);
            }
        }
    }
}
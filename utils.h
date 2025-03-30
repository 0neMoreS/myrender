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
const float K_d = 0.8f;
const float l = -1.f, b = -1.f, n = 1.f, r = 1.f, t = 1.f, f = -10.f;
Ray light(Vec3f{0.0, 0.0, 10.0}, Vec3f{0.0, 0.0, 0.0});
float zbuffer[width][height];
Matrix perspective;
Matrix view_port;

void init_zbuffer()
{
    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            zbuffer[i][j] = std::numeric_limits<float>::lowest();
        }
    }
}

void init_matrix()
{
    view_port[0][0] = width / 2.f;
    view_port[0][3] = width / 2.f;
    view_port[1][1] = height / 2.f;
    view_port[1][3] = height / 2.f;
    view_port[2][2] = 1.f;
    view_port[3][3] = 1.f;

    Matrix move;
    move[0][0] = 1.f;
    move[0][3] = -(r + l) / 2;
    move[1][1] = 1.f;
    move[1][3] = -(t + b) / 2;
    move[2][2] = 1.f;
    move[2][3] = -(n + f) / 2;
    move[3][3] = 1.f;

    Matrix scale;
    scale[0][0] = 2 / (r - l);
    scale[1][1] = 2 / (t - b);
    scale[2][2] = 2 / (n - f);
    scale[3][3] = 1;

    Matrix persp_to_ortho;
    persp_to_ortho[0][0] = n;
    persp_to_ortho[1][1] = n;
    persp_to_ortho[2][2] = n + f;
    persp_to_ortho[2][3] = -n * f;
    persp_to_ortho[3][2] = 1;
    perspective = scale * move * persp_to_ortho;
    // perspective[0][0] = 2 * n / (r - l);
    // perspective[0][2] = (l + r) / (l - r);
    // perspective[1][1] = 2 * n / (t - b);
    // perspective[1][2] = (b + t) / (b - t);
    // perspective[2][2] = (f + n) / (n - f);
    // perspective[2][3] = (2 * f * n) / (f - n);
    // perspective[3][2] = 1;
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
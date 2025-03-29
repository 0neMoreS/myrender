#include <vector>
#include <cmath>
#include <algorithm>
#include "tgaimage.cpp" //tga画图库
#include "model.cpp"    //模型类，主要实现模型的读取
#include "geometry.cpp" //几何库，主要定义了Vec2和Vec3类型

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
Model *model = NULL;
// 定义宽度高度
const int width = 800;
const int height = 800;
const int l = -1, b = -1, n = 1, r = 1, t = 1, f = -1;

// 4d-->3d
// 除以最后一个分量。（当最后一个分量为0，表示向量）
// 不为0，表示坐标
Vec3f m2v(Matrix m)
{
    return Vec3f(m[0][0] / m[3][0], m[1][0] / m[3][0], m[2][0] / m[3][0]);
}

// 3d-->4d
// 添加一个1表示坐标
Matrix v2m(Vec3f v)
{
    Matrix m(4, 1);
    m[0][0] = v.x;
    m[1][0] = v.y;
    m[2][0] = v.z;
    m[3][0] = 1.f;
    return m;
}

bool tri_compare(const Vec2i &a, const Vec2i &b)
{
    if (a.y != b.y)
        return a.y < b.y;
    return a.x < b.x;
}

struct ScreenTriangle
{
    std::vector<Vec2i> ts;
    ScreenTriangle(Vec2i t0, Vec2i t1, Vec2i t2)
    {
        ts.push_back(t0);
        ts.push_back(t1);
        ts.push_back(t2);
        std::sort(ts.begin(), ts.end(), tri_compare);
    }
};

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
    for (int y = t.ts[0].y; y < t.ts[2].y; y++)
    {
        int x0 = (float)(y - t.ts[2].y) / (float)(t.ts[0].y - t.ts[2].y) * (t.ts[0].x - t.ts[2].x) + t.ts[2].x;
        int x1 = 0;
        if (y < t.ts[1].y)
        {
            x1 = (float)(y - t.ts[1].y) / (float)(t.ts[0].y - t.ts[1].y) * (t.ts[0].x - t.ts[1].x) + t.ts[1].x;
        }
        else
        {
            x1 = (float)(y - t.ts[1].y) / (float)(t.ts[2].y - t.ts[1].y) * (t.ts[2].x - t.ts[1].x) + t.ts[1].x;
        }
        if (x0 > x1)
        {
            std::swap(x0, x1);
        }
        for (int x = x0; x < x1; x++)
        {
            image.set(x, y, color);
        }
    }
}

int main(int argc, char **argv)
{
    TGAImage image(width, height, TGAImage::RGB);
    model = new Model("./obj/man.obj ");
    for (int i = 0; i < model->nfaces(); i++)
    {
        std::vector<int> face = model->face(i);
        Vec3f v0 = model->vert(face[0]);
        Vec3f v1 = model->vert(face[1]);
        Vec3f v2 = model->vert(face[2]);
        Vec2i t0((v0.x + 1.0) * width / 2.0, (v0.y + 1.0) * height / 2.0);
        Vec2i t1((v1.x + 1.0) * width / 2.0, (v1.y + 1.0) * height / 2.0);
        Vec2i t2((v2.x + 1.0) * width / 2.0, (v2.y + 1.0) * height / 2.0);
        draw_line(t0.x, t0.y, t1.x, t1.y, image, white);
        draw_line(t0.x, t0.y, t2.x, t2.y, image, white);
        draw_line(t1.x, t1.y, t2.x, t2.y, image, white);
        ScreenTriangle tri{t0, t1, t2};
        draw_triangle(tri, image, white);
    }
    image.flip_vertically();
    image.write_tga_file("output.tga");
    return 0;
}

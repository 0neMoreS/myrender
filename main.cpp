#include <vector>
#include <cmath>
#include <algorithm>
#include "tgaimage.cpp" //tga画图库
#include "model.cpp"    //模型类，主要实现模型的读取
#include "geometry.cpp" //几何库，主要定义了Vec2和Vec3类型
#include "utils.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
Model *model = NULL;
// 定义宽度高度
const int width = 800;
const int height = 800;
const int l = -1, b = -1, n = 1, r = 1, t = 1, f = -1;

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
    Ray light(Vec3f{0.0, 0.0, 0.0}, Vec3f{0.0, 0.0, 0.0});
    for (int i = 0; i < model->nfaces(); i++)
    {
        std::vector<Vec3f> verts{3};
        std::vector<Vec3f> normals{3};
        std::vector<Vec2i> screen_tris{3};

        for (int j = 0; j < 3; j++)
        {
            verts[j] = model->vert(i, j);
            normals[j] = model->normal(i, j);
            int x = (verts[j].x + 1.0) * width / 2.0, y = (verts[j].y + 1.0) * height / 2.0;
            screen_tris[j] = {x, y};
        }

        // for (int j = 0; j < 3; j++)
        // {
        //     draw_line(screen_tris[j].x, screen_tris[j].y, screen_tris[(j + 1) % 3].x, screen_tris[(j + 1) % 3].y, image, red);
        // }

        ScreenTriangle tri{screen_tris};
        draw_triangle(tri, image, white);
    }
    image.flip_vertically();
    image.write_tga_file("output.tga");
    return 0;
}

#include <vector>
#include <cmath>
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

void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color)
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

int main(int argc, char **argv)
{
    TGAImage image(width, height, TGAImage::RGB);
    model = new Model("./obj/african_head.obj ");
    for (int i = 0; i < model->nfaces(); i++)
    {
        std::vector<int> face = model->face(i);
        for (int k = 0; k < 3; k++)
        {
            Vec3f v0 = model->vert(face[k]);
            Vec3f v1 = model->vert(face[(k + 1) % 3]);

            Matrix project(4, 4);
            project[0][0] = 2 * n / (r - l);
            project[0][2] = (l + r) / (l - r);
            project[1][1] = 2 * n / (t - b);
            project[1][2] = (b + t) / (b - t);
            project[2][2] = (f + n) / (n - f);
            project[2][3] = 2 * f * n / (f - n);
            project[3][2] = 1;

            Vec3f v0_pro = m2v(project * v2m(v0));
            Vec3f v1_pro = m2v(project * v2m(v1));

            int x0 = (v0.x + 1.) * width / 2.;
            int y0 = (v0.y + 1.) * height / 2.;
            int x1 = (v1.x + 1.) * width / 2.;
            int y1 = (v1.y + 1.) * height / 2.;
            line(v0_pro[0], v0_pro[1], v1_pro[0], v1_pro[1], image, white);
        }
    }
    image.flip_vertically();
    image.write_tga_file("output.tga");
    return 0;
}

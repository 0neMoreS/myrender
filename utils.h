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

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
Model *model = NULL;
// 定义宽度高度
const int width = 800;
const int height = 800;
const float K_d = 0.8f;
const float fov = 90.f / 180.f * M_PI, aspect_ratio = 1.f, z_near = -0.1f, z_far = -2.f;
Vec3f light{0.f, 0.f, 10.f};
Vec3f camera{0.5f, 0.2f, 1.0f}, look_at{0.f, 0.f, 0.f}, up{0.f, 1.f, 0.f};
// Vec3f camera{0.f, 0.f, 1.5f}, look_at{0.f, 0.f, -1.f}, up{0.f, 1.f, 0.f};
float zbuffer[width][height];
Matrix mvp;
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
    // 假设模型坐标就是世界坐标
    Matrix model = Matrix::identity();

    // 从世界坐标转移到以相机为原点的坐标
    Vec3f camera_z = (camera - look_at).normalize();
    Vec3f camera_x = cross(up, camera_z).normalize();
    Vec3f camera_y = cross(camera_z, camera_x).normalize();

    Matrix coordinate = Matrix::identity();
    for (int i = 0; i < 3; i++)
    {
        coordinate[0][i] = camera_x[i];
        coordinate[1][i] = camera_y[i];
        coordinate[2][i] = camera_z[i];
    }

    Matrix move = Matrix::identity();
    move[0][3] = -camera.x;
    move[1][3] = -camera.y;
    move[2][3] = -camera.z;

    Matrix view = coordinate * move;

    // 从以相机为原点的无限空间转换到以相机为原点的[-1, 1] ^ 3空间

    float n = z_near, f = z_far;
    float t = abs(n) * tan(fov / 2);
    float b = -t;
    float r = t * aspect_ratio;
    float l = -r;

    Matrix translate = Matrix::identity();
    translate[0][3] = -(r + l) / 2;
    translate[1][3] = -(t + b) / 2;
    translate[2][3] = -(n + f) / 2;

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

    Matrix perspective = scale * translate * persp_to_ortho;

    mvp = perspective * view * model;

    view_port[0][0] = width / 2.f;
    view_port[0][3] = width / 2.f;
    view_port[1][1] = height / 2.f;
    view_port[1][3] = height / 2.f;
    view_port[2][2] = 1.f;
    view_port[3][3] = 1.f;
}

void init_light()
{
    Vec4f l4 = embed<4>(light);
    l4 = mvp * l4;
    l4 = l4 / l4[3];
    light.x = l4[0];
    light.y = l4[1];
    light.z = l4[2];
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
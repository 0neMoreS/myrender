#define _USE_MATH_DEFINES
#include <vector>
#include <cmath>
#include <algorithm>
#include "tgaimage.cpp" //tga画图库
#include "model.cpp"    //模型类，主要实现模型的读取
#include "geometry.cpp" //几何库，主要定义了Vec2和Vec3类型

struct IShader
{
    virtual ~IShader() = default;
    virtual Vec3f vertex(int iface, int nthvert) = 0;
    virtual bool fragment(Vec3f bary, TGAColor &color) = 0;
};

struct ScreenTriangleVert
{
    Vec3f screen_vert;
    Vec3f screen_vert_normal;
    float screen_vert_intensity;
    Vec2f texture_uv;

    ScreenTriangleVert() = default;
    ScreenTriangleVert(const Vec3f &_screen_vert, const Vec3f &_screen_vert_normal, const float &_screen_vert_intensity, const Vec2f &_texture_uv) : screen_vert(_screen_vert), screen_vert_normal(_screen_vert_normal), screen_vert_intensity(_screen_vert_intensity), texture_uv(_texture_uv)
    {
    }
    bool operator<(const ScreenTriangleVert &other) const
    {
        if (screen_vert.y != other.screen_vert.y)
            return screen_vert.y < other.screen_vert.y;
        return screen_vert.x < other.screen_vert.x;
    }
};

struct ScreenTriangle
{
    std::vector<ScreenTriangleVert> ts;
    ScreenTriangle(std::vector<ScreenTriangleVert> &_ts) : ts(_ts)
    {
        std::sort(ts.begin(), ts.end());
    }
};

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
Model *model = NULL;
// 定义宽度高度
const int width = 800;
const int height = 800;
const float K_a = 0.1f;
const float K_d = 0.8f;
const float K_s = 0.5f;
const float fov = 103.f / 180.f * M_PI, aspect_ratio = 1.f, z_near = 0.1f, z_far = 2.f;
Vec3f light{0.f, 0.f, 10.f};
// Vec3f camera{-1.f, -1.f, -2.5f}, look_at{0.f, 0.f, 0.f}, up{0.f, 1.f, 0.f};
Vec3f camera{0.f, 0.f, -2.5f}, look_at{0.f, 0.f, 0.f}, up{0.f, 1.f, 0.f};
const float depth = 2048.f;
float zbuffer[width][height];
float shaowbuffer[width][height];
Matrix mvp;
Matrix view_port;

void init_buffer()
{
    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            zbuffer[i][j] = shaowbuffer[i][j] = std::numeric_limits<float>::lowest();
        }
    }
}

Matrix get_model_matrix()
{
    // 假设模型坐标就是世界坐标
    Matrix model = Matrix::identity();
    return model;
}

Matrix get_view_matrix(Vec3f camera, Vec3f look_at, Vec3f up)
{
    // 从世界坐标转移到以相机为原点的坐标
    Vec3f camera_z = (camera - look_at).normalize();
    Vec3f camera_x = cross(up, camera_z).normalize();
    Vec3f camera_y = cross(camera_z, camera_x).normalize();

    Matrix view = Matrix::identity();
    for (int i = 0; i < 3; i++)
    {
        view[0][i] = camera_x[i];
        view[1][i] = camera_y[i];
        view[2][i] = camera_z[i];
    }
    view[0][3] = -(camera * camera_x);
    view[1][3] = -(camera * camera_y);
    view[2][3] = -(camera * camera_z);

    return view;
}

Matrix get_perspective_matrix(float fov, float aspect_ratio, float n, float f)
{
    // 从以相机为原点的透视投影规定空间转换到以相机为原点的[-1 , 1] ^ 3空间
    float t = abs(n) * tan(fov / 2);
    float b = -t;
    float r = t * aspect_ratio;
    float l = -r;

    Matrix perspective;

    perspective[0][0] = 2 * n / (r - l);
    perspective[1][1] = 2 * n / (t - b);
    perspective[2][0] = (r + l) / (r - l);
    perspective[2][1] = (t + b) / (t - b);
    perspective[2][2] = -(f + n) / (f - n);
    perspective[2][3] = -1;
    perspective[3][2] = -2 * f * n / (f - n);

    return perspective;
}

Matrix get_orthographic_matrix(float r, float t, float n, float f)
{
    // see https://www.songho.ca/opengl/gl_projectionmatrix.html
    Matrix orthographic = Matrix::identity();
    orthographic[0][0] = 1 / r;
    orthographic[1][1] = 1 / t;
    orthographic[2][2] = 2 / (n - f);
    orthographic[2][3] = -(f + n) / (f - n);
    return orthographic;
}

Matrix get_viewport_matrix(float width, float height)
{
    // 从[-1, 1] ^ 3转到屏幕二维坐标
    Matrix view_port;
    view_port[0][0] = width / 2.f;
    view_port[0][3] = width / 2.f;
    view_port[1][1] = height / 2.f;
    view_port[1][3] = height / 2.f;
    view_port[2][2] = 1.f;
    view_port[3][3] = 1.f;
    return view_port;
}

inline Vec3f barycentric(Vec3f pts[], Vec2f P)
{
    Vec3f AB = pts[1] - pts[0];
    Vec3f AC = pts[2] - pts[0];
    Vec3f PA = pts[0] - Vec3f{P.x, P.y, 0.f};

    Vec3f uv = cross(Vec3f{AB.x, AC.x, PA.x}, Vec3f{AB.y, AC.y, PA.y});

    if (std::abs(uv[2]) > 1e-2)
    {
        return Vec3f(1.f - (uv.x + uv.y) / uv.z, uv.x / uv.z, uv.y / uv.z);
    }
    return Vec3f{-1, 1, 1};
}

template <typename T>
inline T bary_attribute(Vec3f bary, T attribute[])
{
    return attribute[0] * bary[0] + attribute[1] * bary[1] + attribute[2] * bary[2];
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

// void draw_triangle(ScreenTriangle &t, TGAImage &image, TGAColor color)
// {
//     std::vector<Vec3f> verts{t.ts[0].screen_vert, t.ts[1].screen_vert, t.ts[2].screen_vert};
//     std::vector<Vec3f> normals{t.ts[0].screen_vert_normal, t.ts[1].screen_vert_normal, t.ts[2].screen_vert_normal};
//     std::vector<float> intensities{t.ts[0].screen_vert_intensity, t.ts[1].screen_vert_intensity, t.ts[2].screen_vert_intensity};
//     std::vector<Vec2f> texture_uvs{t.ts[0].texture_uv, t.ts[1].texture_uv, t.ts[2].texture_uv};

//     for (float y = t.ts[0].screen_vert.y; y < t.ts[2].screen_vert.y; y++)
//     {
//         float x0 = (y - t.ts[2].screen_vert.y) / (t.ts[0].screen_vert.y - t.ts[2].screen_vert.y) * (t.ts[0].screen_vert.x - t.ts[2].screen_vert.x) + t.ts[2].screen_vert.x;
//         float x1 = 0.f;
//         if (y < t.ts[1].screen_vert.y)
//         {
//             x1 = (y - t.ts[1].screen_vert.y) / (t.ts[0].screen_vert.y - t.ts[1].screen_vert.y) * (t.ts[0].screen_vert.x - t.ts[1].screen_vert.x) + t.ts[1].screen_vert.x;
//         }
//         else
//         {
//             x1 = (y - t.ts[1].screen_vert.y) / (t.ts[2].screen_vert.y - t.ts[1].screen_vert.y) * (t.ts[2].screen_vert.x - t.ts[1].screen_vert.x) + t.ts[1].screen_vert.x;
//         }
//         if (x0 > x1)
//         {
//             std::swap(x0, x1);
//         }
//         // 各种插值
//         for (float x = x0; x < x1; x++)
//         {
//             Vec3f uv = barycentric(t, Vec2f{x, y});
//             Vec3f vert = uv_attribute(uv, verts);
//             Vec3f normal = uv_attribute(uv, normals);
//             float intensity = uv_attribute(uv, intensities);
//             Vec2f texture_uv = uv_attribute(uv, texture_uvs);

//             if (vert.z > zbuffer[(int)x][(int)y])
//             {
//                 zbuffer[(int)x][(int)y] = vert.z;
//                 TGAColor pixel_color{model->diffuse(texture_uv) * K_d * intensity};
//                 image.set((int)x, (int)y, pixel_color);
//             }
//         }
//     }
// }

void draw_triangle(IShader &shader, Vec3f pts[], TGAImage &image, float buffer[width][height])
{
    Vec2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            bboxmin[j] = std::min(bboxmin[j], pts[i][j]);
            bboxmax[j] = std::max(bboxmax[j], pts[i][j]);
        }
    }
    Vec2f P;
    TGAColor color;
    for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++)
    {
        for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++)
        {
            Vec3f bary = barycentric(pts, P);
            if (bary.x < 0 || bary.y < 0 || bary.z < 0)
            {
                continue;
            }
            if (P.x < 0 || P.x > width || P.y < 0 || P.y > height)
            {
                continue;
            }
            float zs[3] = {pts[0][2], pts[1][2], pts[2][2]};
            float z = bary_attribute(bary, zs);
            if (z > buffer[(int)P.x][(int)P.y])
            {
                buffer[(int)P.x][(int)P.y] = z;
                shader.fragment(bary, color);
                image.set((int)P.x, (int)P.y, color);
            }
        }
    }
}
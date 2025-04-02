#include "utils.h"

int main(int argc, char **argv)
{
    TGAImage image(width, height, TGAImage::RGB);
    model = new Model("./obj/man.obj ");
    init_zbuffer();
    init_matrix();
    // init_light();

    for (int i = 0; i < model->nfaces(); i++)
    {
        std::vector<Vec3f> verts;
        std::vector<Vec3f> normals;
        std::vector<float> intensities;
        std::vector<Vec3f> screen_verts;
        std::vector<ScreenTriangleVert> screen_tris;

        for (int j = 0; j < 3; j++)
        {
            verts.push_back(model->vert(i, j));
            normals.push_back(model->normal(i, j));
            normals[j].z = -normals[j].z;

            intensities.push_back(normals[j] * (verts[j] - light).normalize());

            Vec4f v4 = embed<4>(verts[j]);
            v4 = mvp * v4;
            v4 = v4 / v4[3];
            v4 = view_port * v4;
            // 转换成屏幕坐标时就窄化到int避免精度误差导致的像素空白
            int x = (int)v4[0], y = (int)v4[1];
            screen_verts.push_back({(float)x, (float)y, v4[2]});
            screen_tris.push_back(ScreenTriangleVert{screen_verts[j], normals[j], intensities[j]});
        }

        ScreenTriangle tri{screen_tris};
        draw_triangle(tri, image, white);
        // triangle(screen_verts[0], screen_verts[1], screen_verts[2], intensities[0], intensities[1], intensities[2], image);
    }
    image.flip_vertically();
    image.write_tga_file("output.tga");
    return 0;
}

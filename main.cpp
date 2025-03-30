#include "utils.h"

int main(int argc, char **argv)
{
    TGAImage image(width, height, TGAImage::RGB);
    model = new Model("./obj/man.obj ");
    init_zbuffer();
    init_matrix();

    for (int i = 0; i < model->nfaces(); i++)
    {
        std::vector<Vec3f> verts{3};
        std::vector<Vec3f> normals{3};
        std::vector<Vec3f> screen_tris{3};

        for (int j = 0; j < 3; j++)
        {
            verts[j] = model->vert(i, j);
            // normals[j] = model->normal(i, j);

            Vec4f v4 = embed<4>(verts[j]);
            v4 = perspective * v4;
            // v4 = v4 / v4[3];
            v4 = view_port * v4;
            // 转换成屏幕坐标时就窄化到int避免精度误差导致的像素空白
            int x = (int)v4[0], y = (int)v4[1];
            screen_tris[j] = {(float)x, (float)y, v4[2]};
        }

        // for (int j = 0; j < 3; j++)
        // {
        //     draw_line(screen_tris[j].x, screen_tris[j].y, screen_tris[(j + 1) % 3].x, screen_tris[(j + 1) % 3].y, image, red);
        // }

        ScreenTriangle tri{screen_tris};
        // Vec3f normals_avg = (normals[0] + normals[1] + normals[2]).normalize();
        Vec3f normals_avg = cross((verts[2] - verts[0]), (verts[1] - verts[0])).normalize();
        Vec3f verts_avg = (verts[0] + verts[1] + verts[2]) / 3;
        float cos_theta = (verts_avg - light.o).normalize() * normals_avg;
        if (cos_theta > 0)
        {
            TGAColor color{white * (cos_theta * K_d)};
            draw_triangle(tri, image, color);
        }
    }
    image.flip_vertically();
    image.write_tga_file("output.tga");
    return 0;
}

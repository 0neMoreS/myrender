#include "utils.h"

int main(int argc, char **argv)
{
    TGAImage image(width, height, TGAImage::RGB);
    model = new Model("./obj/man.obj ");
    init_zbuffer();

    for (int i = 0; i < model->nfaces(); i++)
    {
        std::vector<Vec3f> verts{3};
        std::vector<Vec3f> normals{3};
        std::vector<Vec3f> screen_tris{3};

        for (int j = 0; j < 3; j++)
        {
            verts[j] = model->vert(i, j);
            normals[j] = model->normal(i, j);
            float x = (verts[j].x + 1.f) * width / 2.f, y = (verts[j].y + 1.f) * height / 2.f;
            screen_tris[j] = {x, y, verts[j].z};
        }

        // for (int j = 0; j < 3; j++)
        // {
        //     draw_line(screen_tris[j].x, screen_tris[j].y, screen_tris[(j + 1) % 3].x, screen_tris[(j + 1) % 3].y, image, red);
        // }
        ScreenTriangle tri{screen_tris};

        Vec3f normals_avg = (normals[0] + normals[1] + normals[2]).normalize();
        normals_avg = cross((verts[2] - verts[0]), (verts[1] - verts[0])).normalize();
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

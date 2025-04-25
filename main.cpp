#include "mygl.h"

struct GouraudShader : public IShader
{
    Vec3f intensities;
    Vec3f vertex(int iface, int nthvert)
    {
        Vec3f vert = model->vert(iface, nthvert);
        Vec3f norm = model->normal(iface, nthvert);
        norm.z = -norm.z;
        intensities[nthvert] = norm * (vert - light).normalize();
        Vec4f v4 = embed<4>(vert);
        v4 = mvp * v4;
        v4 = v4 / v4[3];
        v4 = view_port * v4;
        return {(float)((int)v4[0]), (float)((int)v4[1]), v4[2]};
    }

    bool fragment(Vec3f bary, TGAColor &color)
    {
        color = TGAColor(255, 255, 255) * (bary * intensities);
        return true;
    }

    ~GouraudShader() = default;
};

int main(int argc, char **argv)
{
    TGAImage image(width, height, TGAImage::RGB);
    model = new Model("./obj/man.obj ");
    init_zbuffer();
    init_matrix();
    GouraudShader shader;
    // init_light();

    for (int i = 0; i < model->nfaces(); i++)
    {
        Vec3f screen_coords[3];
        for (int j = 0; j < 3; j++)
        {
            screen_coords[j] = shader.vertex(i, j);
        }
        draw_triangle(shader, screen_coords, image);
    }
    image.flip_vertically();
    image.write_tga_file("output.tga");
    return 0;
}

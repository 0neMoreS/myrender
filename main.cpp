#include "shaders.h"

int main(int argc, char **argv)
{
    TGAImage image(width, height, TGAImage::RGB);
    model = new Model("./obj/african_head.obj ");
    init_zbuffer();
    init_matrix();
    SpecularMapShader shader;
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

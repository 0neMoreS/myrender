#include "shaders.h"

int main(int argc, char **argv)
{
    model = new Model("D:\\A1-Profession\\ComputerGraphic\\myrender\\obj\\african_head.obj");
    init_buffer();
    mvp = get_perspective_matrix(fov, aspect_ratio, z_near, z_far) * get_view_matrix(camera, look_at, up) * get_model_matrix();
    // mvp = get_orthographic_matrix(1.f, 1.f, z_near, z_far) * get_view_matrix(camera, look_at, up) * get_model_matrix();
    view_port = get_viewport_matrix(width, height);

    ShadowShader shadow_shader;
    TGAImage depth(width, height, TGAImage::RGB);
    for (int i = 0; i < model->nfaces(); i++)
    {
        Vec3f screen_coords[3];
        for (int j = 0; j < 3; j++)
        {
            screen_coords[j] = shadow_shader.vertex(i, j);
        }
        draw_triangle(shadow_shader, screen_coords, depth, shaowbuffer);
    }
    depth.flip_vertically();
    depth.write_tga_file("shadow.tga");

    DebugShader frame_shader;
    TGAImage frame(width, height, TGAImage::RGB);
    int end = model->nfaces();
    for (int i = 0; i < end; i++)
    {
        Vec3f screen_coords[3];
        for (int j = 0; j < 3; j++)
        {
            screen_coords[j] = frame_shader.vertex(i, j);
        }
        draw_triangle(frame_shader, screen_coords, frame, zbuffer);
    }
    frame.flip_vertically();
    frame.write_tga_file("output.tga");
    return 0;
}

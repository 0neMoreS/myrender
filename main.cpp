#include "tgaimage.cpp"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);

void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color)
{
    bool lean = false;
    if (std::abs((float)(x1 - x0) / (float)(y1 - y0)) > 1)
    {
        lean = true;
        std::swap(x0, x1);
        std::swap(y0, y1);
    }
    if (x0 > x1)
    {
        std::swap(x0, x1);
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
    TGAImage image(100, 100, TGAImage::RGB);
    line(13, 20, 80, 40, image, white); // 线段A
    line(20, 13, 40, 80, image, red);   // 线段B
    line(80, 40, 13, 20, image, red);   // 线段C
    image.flip_vertically();
    image.write_tga_file("output.tga");
    return 0;
}

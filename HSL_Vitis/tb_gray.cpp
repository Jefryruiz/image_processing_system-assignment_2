#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <vector>

extern "C" void gray_kernel(const unsigned char *in, unsigned char *out, int num_pixels);

int main(int argc,char**argv){
    const char *infile = "input.raw";
    const char *outfile = "output.raw";
    int num_pixels = 1920*1080;

    std::vector<unsigned char> inbuf(num_pixels*3);
    std::vector<unsigned char> outbuf(num_pixels);

    std::ifstream fin(infile, std::ios::binary);
    if(!fin) return -1;
    fin.read((char*)inbuf.data(), inbuf.size());
    fin.close();

    gray_kernel(inbuf.data(), outbuf.data(), num_pixels);

    std::ofstream fout(outfile, std::ios::binary);
    fout.write((char*)outbuf.data(), outbuf.size());
    fout.close();

    printf("Done\n");
    return 0;
}

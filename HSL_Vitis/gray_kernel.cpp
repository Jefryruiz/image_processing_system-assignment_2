#include <hls_stream.h>
#include <ap_int.h>
#include <stddef.h>

#define WIDTH 1920
#define HEIGHT 1080

typedef ap_uint<8>  u8;
typedef ap_uint<24> pix24;

struct Pixel3 { u8 r,g,b; };

void read_stage(const u8 *in, hls::stream<pix24> &stream_in, int num_pixels) {
#pragma HLS INLINE off
    for (int i=0;i<num_pixels;i++) {
#pragma HLS PIPELINE II=1
        pix24 v = (pix24)in[i*3] << 16 | (pix24)in[i*3+1] << 8 | (pix24)in[i*3+2];
        stream_in.write(v);
    }
}

void proc_stage(hls::stream<pix24> &stream_in, hls::stream<u8> &stream_out, int num_pixels) {
#pragma HLS INLINE off
    for (int i=0;i<num_pixels;i++) {
#pragma HLS PIPELINE II=1
        pix24 v = stream_in.read();
        u8 r = (u8)(v.range(23,16));
        u8 g = (u8)(v.range(15,8));
        u8 b = (u8)(v.range(7,0));
        // BT.601 approximation with integer math: gray = (0.299*r + 0.587*g + 0.114*b)
        unsigned int gray = (307u * r + 601u * g + 117u * b) >> 10; // scale factors sum ~1025
        stream_out.write((u8)gray);
    }
}

void write_stage(u8 *out, hls::stream<u8> &stream_out, int num_pixels) {
#pragma HLS INLINE off
    for (int i=0;i<num_pixels;i++) {
#pragma HLS PIPELINE II=1
        out[i] = stream_out.read();
    }
}

extern "C" {
void gray_kernel(const u8 *in, u8 *out, int num_pixels) {
#pragma HLS INTERFACE m_axi port=in  offset=slave bundle=gmem depth=WIDTH*HEIGHT*3
#pragma HLS INTERFACE m_axi port=out offset=slave bundle=gmem depth=WIDTH*HEIGHT
#pragma HLS INTERFACE s_axilite port=in  bundle=control
#pragma HLS INTERFACE s_axilite port=out bundle=control
#pragma HLS INTERFACE s_axilite port=num_pixels bundle=control
#pragma HLS INTERFACE s_axilite port=return bundle=control

#pragma HLS DATAFLOW
    hls::stream<pix24> stream_in;
    hls::stream<u8>    stream_out;
#pragma HLS STREAM variable=stream_in depth=256
#pragma HLS STREAM variable=stream_out depth=256

    read_stage(in, stream_in, num_pixels);
    proc_stage(stream_in, stream_out, num_pixels);
    write_stage(out, stream_out, num_pixels);
}
}
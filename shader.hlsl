Buffer<int> InputBuffer : register(t0);
RWBuffer<int> OutputBuffer : register(u0);

[numthreads(4, 1, 1)]
void main(uint dtid : SV_GroupIndex) {
    OutputBuffer[dtid] = InputBuffer[dtid] * 2;
}

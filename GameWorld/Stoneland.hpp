#ifndef VCraft_Chunk_stone
#define VCraft_Chunk_stone

class Stoneland : public Chunk {
public:
  Stoneland(int seed, int i, int j, int k) : Chunk(stoneland,seed, i, j, k) {}

  void Fill() {
    for (int i = 0; i < BLOCK_SIZE; i++) {
      for (int j = 0; j < BLOCK_SIZE; j++) {
        for (int k = 0; k < BLOCK_SIZE; k++) {
          double val = pn.noise(
              static_cast<double>(i) / static_cast<double>(BLOCK_SIZE),
              static_cast<double>(j) / static_cast<double>(BLOCK_SIZE),
              static_cast<double>(k) / static_cast<double>(BLOCK_SIZE));
          boost::array<block_index, 4> idx = {{i, j, k, 0}};
          val *= 18;
          if (val > 6 && val < 12 )
            block(idx) = 256;
          else if (val < 0 || val > 16)
            block(idx) = 1;
          else if (val < 3 || val > 12)
            block(idx) = 0;
          else if (val > 7 && val < 16)
            block(idx) = 2;
          else if (val > 4 && val < 14)
            block(idx) = 3;
          else
            block(idx) = 256;
        }
      }
    }
  }
};

#endif

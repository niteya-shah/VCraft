#ifndef VCraft_Chunk_grass
#define VCraft_Chunk_grass

class Grassland : public Chunk {
public:
  Grassland(int seed, int i, int j, int k) : Chunk(grassland,seed, i, j, k) {}

  void Fill() {
    for (int i = 0; i < BLOCK_SIZE; i++) {
      for (int j = 0; j < BLOCK_SIZE; j++) {
        for (int k = 0; k < BLOCK_SIZE; k++) {
          double val = pn.noise(
              static_cast<double>(i) / static_cast<double>(BLOCK_SIZE),
              static_cast<double>(j) / static_cast<double>(BLOCK_SIZE),
              static_cast<double>(k) / static_cast<double>(BLOCK_SIZE));
          boost::array<block_index, 4> idx = {{i, j, k, 0}};
          val *= 4;
          if (val < 1)
            block(idx) = 0;
          else if (val < 2)
            block(idx) = 1;
          else
            block(idx) = 2;
        }
      }
    }
  }
};

#endif

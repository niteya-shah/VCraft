#ifndef VCraft_chunk_impl
#define VCraft_chunk_impl

enum chunkType { grassland, mudland };

typedef boost::multi_array<size_t, 4> block_matrix;
typedef block_matrix::index block_index;

#include "PerlinNoise.hpp"

struct Chunk {

  Chunk(chunkType _chunk_type, int repeat = -1, int seed = 0)
      : chunk_type(_chunk_type), block(shape) {}

  void removeEmpty(const boost::array<block_index, 4> &idx) {
    boost::array<block_index, 4> idxTemp = {{0, 0, 0, 1}};
    for (int i = 0; i < 3; i++)
      if (idx[i] == 0 || idx[i] == BLOCK_SIZE - 1) {
        idxTemp[0] = idx[0];
        idxTemp[1] = idx[1];
        idxTemp[2] = idx[2];
        block(idxTemp) = 1;
        return;
      }
    idxTemp = {{0, 0, 0, 0}};
    for (int i = -1; i <= 1; i++) {
      for (int j = -1; j <= 1; j++) {
        for (int k = -1; k <= 1; k++) {
          idxTemp[0] = idx[0] + i;
          idxTemp[1] = idx[1] + j;
          idxTemp[2] = idx[2] + k;
          if (block(idxTemp) == 2) {
            idxTemp[0] = idx[0];
            idxTemp[1] = idx[1];
            idxTemp[2] = idx[2];
            idxTemp[3] = 1;
            block(idxTemp) = 1;
            return;
          }
        }
      }
    }
  }

  virtual void Fill() = 0;

  chunkType chunk_type;
  PerlinNoise pn;
  block_matrix block;
  constexpr static boost::array<block_index, 4> shape = {
      {BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE, 2}};
      int operator()(boost::array<block_index, 4> idx) { return block(idx); }

};

class Grassland : public Chunk {
public:
  Grassland() : Chunk(grassland, 3) {}

  void Fill() {
    for (int i = 0; i < BLOCK_SIZE; i++) {
      for (int j = 0; j < BLOCK_SIZE; j++) {
        for (int k = 0; k < BLOCK_SIZE; k++) {
          double val = pn.noise(
              static_cast<double>(i) / static_cast<double>(BLOCK_SIZE),
              static_cast<double>(j) / static_cast<double>(BLOCK_SIZE),
              static_cast<double>(k) / static_cast<double>(BLOCK_SIZE));
          boost::array<block_index, 4> idx = {{i, j, k, 0}};
          val *= 3;
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

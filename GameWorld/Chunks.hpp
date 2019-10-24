#ifndef VCraft_chunk_def
#define VCraft_chunk_def

enum chunkType { grassland, mudland };

typedef boost::multi_array<size_t, 4> block_matrix;
typedef block_matrix::index block_index;

#include "PerlinNoise.hpp"

#include "Cubes.hpp"

struct Chunk {

  Chunk(chunkType _chunk_type, int seed, int i, int j, int k)
      : chunk_type(_chunk_type), block(shape), pn(seed, i, j ,k) {}

  void removeEmpty(const boost::array<block_index, 4> &idx);

  virtual void Fill() = 0;

  static void loadCubes();

  void FillChunk(int blockIdX,int blockIdY,int blockIdZ,std::unordered_map<Vertex, uint32_t> &uniqueVertices,
                 std::vector<Vertex> &vertices, std::vector<uint32_t> &indices);

  void plotShape(tinyobj::shape_t &shape, tinyobj::attrib_t &attrib,
                 std::unordered_map<Vertex, uint32_t> &uniqueVertices,
                 std::vector<Vertex> &vertices, std::vector<uint32_t> &indices,
                 int i, int j, int k);



  chunkType chunk_type;

  PerlinNoise pn;

  block_matrix block;

  constexpr static boost::array<block_index, 4> shape = {
      {BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE, 2}};

  static inline std::vector<Cube> cubes;
};

#include "Chunks_impl.hpp"

#include "Grassland.hpp"

#endif

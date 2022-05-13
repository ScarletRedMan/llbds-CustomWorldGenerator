#pragma once
#include "pch.h"


#define C2G_COORD(chunkCoord) (chunkCoord << 4)
#define G2C_COORD(coord) (coord >> 4)
#define G2L_COORD(coord) (coord & 0xF)
#define L2G_COORD(chunkCoord, lCoord) (C2G_COORD(chunkCoord) + lCoord)

#define X 123456789
#define Y 362436069
#define Z 521288629
#define W 88675123

#define INT31_VALUE 0x7fffffff
#define INT32_VALUE 0xffffffff

#define M_SQRT3 1.7320508075689f
#define F2 (0.5f * (M_SQRT3 - 1.0f))
#define G2 ((3.0f - M_SQRT3) / 6.0f)
#define G22 (G2 * 2.0f - 1.0f)
#define F3 (1.0f / 3.0f)
#define G3 (1.0f / 6.0f)

#define WORLD_MIN_Y 0
#define WORLD_MAX_Y 383

#define CHUNK_SIZE 16
#define COORD_BIT_SIZE 4


namespace GEN_API {
    class ChunkManager {
    private:
        LevelChunk* levelChunk;
        ChunkPos* chunkPos;
        short** heightMap;

    public:
        ChunkManager(LevelChunk& levelChunk, ChunkPos const& chunkPos) {
            this->levelChunk = &levelChunk;
            this->chunkPos = new ChunkPos(chunkPos.x, chunkPos.z);

            heightMap = (short**) malloc(16 * sizeof(short*));
            for (char x = 0; x < 16; x++) {
                heightMap[x] = (short*) calloc(16, sizeof(short));
            }
        }

        ~ChunkManager() {
            delete chunkPos;
            for (char x = 0; x < 16; x++) free(heightMap[x]);
            free(heightMap);
        }

        void setBlockAt(int x, int y, int z, string const& stringId) {
            if(heightMap[x & 0xF][z & 0xF] < y) heightMap[x & 0xF][z & 0xF] = (short) y;
            levelChunk->setBlockSimple(ChunkBlockPos(G2L_COORD(x), (short) y, G2L_COORD(z)), *Block::create(stringId, 0));
        }

        void setBlockAt(int x, int y, int z, string const& stringId, unsigned short tileData) {
            if(heightMap[x & 0xF][z & 0xF] < y) heightMap[x & 0xF][z & 0xF] = (short) y;
            levelChunk->setBlockSimple(ChunkBlockPos(G2L_COORD(x), (short) y, G2L_COORD(z)), *Block::create(stringId, tileData));
        }

        void setBlockAt(int x, int y, int z, Block const* block) {
            if(heightMap[x & 0xF][z & 0xF] < y) heightMap[x & 0xF][z & 0xF] = (short) y;
            levelChunk->setBlockSimple(ChunkBlockPos(G2L_COORD(x), (short) y, G2L_COORD(z)), *block);
        }

        Block const& getBlockAt(int x, int y, int z) {
            return levelChunk->getBlock(ChunkBlockPos(G2L_COORD(x), (short) y, G2L_COORD(z)));
        }

        int getHighestBlockAt(int x, int z) {
            return heightMap[x & 0xF][z & 0xF];
        }

        void setBiomeAt(int x, int z, Biome* biome) {
            levelChunk->setBiome2d(*biome, ChunkBlockPos(G2L_COORD(x), 0, G2L_COORD(z)));
        }

        Biome const& getBiomeAt(int x, int z) {
            return levelChunk->getBiome(ChunkBlockPos(G2L_COORD(x), 0, G2L_COORD(z)));
        }

        Level& getLevel() {
            return levelChunk->getLevel();
        }

        LevelChunk* getLevelChunk() {
            return levelChunk;
        }

        ChunkPos* getChunkPos() {
            return chunkPos;
        }
    };

    class Random {
    private:
        int seed;
        long long x;
        long long y;
        long long z;
        long long w;

    public:
        Random(int seed) {
            setSeed(seed);
        }

        void setSeed(int seed);

        int getSeed();

        void next();

        int nextSignedInt();

        int nextInt();

        int nextInt(int bound);

        int nextInt(int min, int max);

        float nextFloat();

        float nextSignedFloat();

        bool nextBool();
    };

    const short SIMPLEX_GRAD3[12][3] = {
            {1, 1, 0}, {-1, 1, 0}, {1, -1, 0}, {-1, -1, 0},
            {1, 0, 1}, {-1, 0, 1}, {1, 0, -1}, {-1, 0, -1},
            {0, 1, 1}, {0, -1, 1}, {0, 1, -1}, {0, -1, -1}
    };

    class Noise {
    protected:
        float persistence;
        float expansion;
        int octaves;

    public:
        Noise(int octaves, float persistence, float expansion) {
            this->octaves = octaves;
            this->persistence = persistence;
            this->expansion = expansion;
        }

        virtual float getNoise2D(float x, float z) = 0;

        virtual float getNoise3D(float x, float y, float z) = 0;

        virtual float noise2D(float x, float z, bool normalized = false);

        virtual float noise3D(float x, float y, float z, bool normalized = false);
    };

    class Simplex: public Noise {
    protected:
        float offsetX;
        float offsetZ;
        float offsetY;
        int perm[512];

    public:
        Simplex(Random *random, int octaves, float persistence, float expansion) : Noise(octaves, persistence, expansion) {
            offsetX = random->nextFloat() * 256;
            offsetY = random->nextFloat() * 256;
            offsetZ = random->nextFloat() * 256;

            for (int & i : perm) i = 0;
            for (short i = 0; i < 256; ++i) perm[i] = random->nextInt(256);
            for (short i = 0; i < 245; ++i) {
                int pos = random->nextInt(256 - i) + i;
                int old = perm[i];

                perm[i] = perm[pos];
                perm[pos] = old;
                perm[i + 256] = perm[i];
            }

            random->next();
        }

        float getNoise2D(float x, float z) override;

        float getNoise3D(float x, float y, float z) override;
    };

    class WorldGenerator {
    protected:
        int seed;
        Random* random;

    public:
        WorldGenerator(int seed) {
            this->seed = seed;
            this->random = new Random(seed);
        }

        ~WorldGenerator() {
            delete random;
        }

        int getSeed() const {
            return seed;
        }

        GEN_API::Random* getRandom() {
            return random;
        }

        virtual void generateChunk(GEN_API::ChunkManager* world, int chunkX, int chunkZ) {

        }
    };

    class BlockTransactionElement {
    private:
        string blockId;
        char localX;
        short localY;
        char localZ;
        unsigned short tileData;
        bool placeIsNotFree;

    public:
        BlockTransactionElement(string encoded);

        BlockTransactionElement(int x, short y, int z, string const& blockId, unsigned short tileData, bool forcePlace) {
            localX = G2L_COORD(x);
            localY = y;
            localZ = G2L_COORD(z);
            this->blockId = "" + blockId;
            this->tileData = tileData;
            placeIsNotFree = forcePlace;
        }

        void tryPlace(LevelChunk* levelChunk);

        string encode();
    };

    void transactionPostProcessingGeneration(LevelChunk* levelChunk, ChunkPos const& chunkPos);

    void createTransactionCache(ChunkPos const& chunkPos, vector<BlockTransactionElement> elements);

    struct ChunkTransactionLink {
        int x;
        int z;
        vector<BlockTransactionElement> elements;
    };

    class BlockTransaction {
    private:
        vector<ChunkTransactionLink> chunks;
    public:
        BlockTransaction() {
            chunks = vector<ChunkTransactionLink>();
        }

        void addBlock(int x, short y, int z, Block const* block, bool force = true) {
            addBlock(x, y, z, block->getTypeName(), 0, force);
        }

        void addBlock(int x, short y, int z, string blockId, bool force = true) {
            addBlock(x, y, z, std::move(blockId), 0, force);
        }

        void addBlock(int x, short y, int z, string blockId, unsigned short tileData, bool force = true);

        void apply(ChunkManager* chunkManager) {
            apply(chunkManager->getLevelChunk(), chunkManager->getChunkPos());
        }

        void apply(LevelChunk* levelChunk, ChunkPos* chunkPos);
    };
}
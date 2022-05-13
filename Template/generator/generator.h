#pragma once
#include "pch.h"
#include "generator_tools.h"


#define WATER_LEVEL 60

class CustomGenerator: public GEN_API::WorldGenerator {
private:
    GEN_API::Simplex* simplex;

public:
    CustomGenerator(int seed) : WorldGenerator(seed) {
        simplex = new GEN_API::Simplex(random, 8, 1/32.0f, 1/64.0f);
    }

    void generateChunk(GEN_API::ChunkManager *world, int chunkX, int chunkZ) override {
        //TODO: Здесь ваш генератор мира

        for (int lx = 0; lx < CHUNK_SIZE; lx++) {
            int gx = (chunkX << COORD_BIT_SIZE) + lx;

            for (int lz = 0; lz < CHUNK_SIZE; lz++) {
                int gz = (chunkZ << COORD_BIT_SIZE) + lz;

                world->setBiomeAt(gx, gz, VanillaBiomes::mForest);

                float noise = simplex->noise2D((float) gx, (float) gz);
                int ty = (int) (noise * 8 + WATER_LEVEL);
                int yMax = ty < WATER_LEVEL? WATER_LEVEL : ty;

                for (int y = 0; y <= yMax; y++) {
                    Block const* block;

                    if (y <= 1) block = VanillaBlocks::mBedrock;
                    else if (y > ty) block = VanillaBlocks::mStillWater;
                    else {
                        if (y + 4 > ty && WATER_LEVEL <= ty) {
                            block = ty == y? VanillaBlocks::mGrass : VanillaBlocks::mDirt;
                        } else block = VanillaBlocks::mStone;
                    }

                    world->setBlockAt(gx, y, gz, block);
                }
            }
        }
    }
};

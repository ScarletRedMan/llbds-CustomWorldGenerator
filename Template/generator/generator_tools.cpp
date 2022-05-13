#include "generator_tools.h"

#define m_t4(val) (val * val * val * val)
#define STR_TO_INT(val) (std::atoi(val.c_str()))

using std::to_string;


void GEN_API::Random::setSeed(int newSeed) {
    seed = newSeed;
    x = X ^ seed;
    y = Y ^ (seed << 17) | ((seed >> 15) & INT31_VALUE) & INT32_VALUE;
    z = Z ^ (seed << 31) | ((seed >> 1) & INT31_VALUE) & INT32_VALUE;
    w = W ^ (seed << 18) | ((seed >> 14) & INT31_VALUE) & INT32_VALUE;
}

int GEN_API::Random::getSeed() {
    return seed;
}

void GEN_API::Random::next() {
    long long t = (x ^ (x << 11)) & INT32_VALUE;

    x = y;
    y = z;
    z = w;
    w = (w ^ ((w >> 19) & INT31_VALUE) ^ (t ^ ((t >> 8) & INT31_VALUE))) & INT32_VALUE;
}

int GEN_API::Random::nextSignedInt() {
    next();
    return (int) w;
}

int GEN_API::Random::nextInt() {
    return nextSignedInt() & INT31_VALUE;
}

int GEN_API::Random::nextInt(int bound) {
    return nextInt() % bound;
}

int GEN_API::Random::nextInt(int min, int max) {
    return min + (nextInt() % (max + 1 - min));
}

float GEN_API::Random::nextFloat() {
    return nextInt() / (float) INT31_VALUE;
}

float GEN_API::Random::nextSignedFloat() {
    return nextSignedInt() / (float) INT31_VALUE;
}

bool GEN_API::Random::nextBool() {
    return (nextInt() & 0x01) == 0;
}

float GEN_API::Noise::noise2D(float x, float z, bool normalized) {
    float result = 0;
    float amp = 1.0f;
    float freq = 1.0f;
    float max = 0.0f;

    x *= expansion;
    z *= expansion;

    for (int i = 0; i < octaves; ++i) {
        result += getNoise2D(x * freq, z * freq) * amp;
        max += amp;
        freq *= 2.0f;
        amp *= persistence;
    }

    return normalized? (result / max) : result;
}

float GEN_API::Noise::noise3D(float x, float y, float z, bool normalized) {
    float result = 0;
    float amp = 1.0f;
    float freq = 1.0f;
    float max = 0.0f;

    x *= expansion;
    z *= expansion;

    for (int i = 0; i < octaves; ++i) {
        result += getNoise3D(x * freq, y * freq, z * freq) * amp;
        max += amp;
        freq *= 2.0f;
        amp *= persistence;
    }

    return normalized? (result / max) : result;
}

float GEN_API::Simplex::getNoise2D(float x, float y) {
    x += offsetX;
    y += offsetY;

    float s = (x + y) * F2;
    int i = (int) (x + s);
    int j = (int) (y + s);
    float t = (i + j) * G2;

    float x0 = x - (i - t);
    float y0 = y - (j - t);

    int i1, j1;
    if (x0 > y0) {
        i1 = 1;
        j1 = 0;
    } else {
        i1 = 0;
        j1 = 1;
    }

    float x1 = x0 - i1 + G2;
    float y1 = y0 - j1 + G2;
    float x2 = x0 + G22;
    float y2 = y0 + G22;

    int ii = i & 255;
    int jj = j & 255;

    float n = 0;
    float ti;

    ti = 0.5f - x0 * x0 - y0 * y0;
    if (ti > 0) {
        const short index = perm[ii + perm[jj]] % 12;
        n += m_t4(ti) * (GEN_API::SIMPLEX_GRAD3[index][0] * x0 + GEN_API::SIMPLEX_GRAD3[index][1] * y0);
    }

    ti = 0.5f - x1 * x1 - y1 * y1;
    if (ti > 0) {
        const short index = perm[ii + i1 + perm[jj + j1]] % 12;
        n += m_t4(ti) * (GEN_API::SIMPLEX_GRAD3[index][0] * x1 + GEN_API::SIMPLEX_GRAD3[index][1] * y1);
    }

    ti = 0.5f - x2 * x2 - y2 * y2;
    if (ti > 0) {
        const short index = perm[ii + 1 + perm[jj + 1]] % 12;
        n += m_t4(ti) * (GEN_API::SIMPLEX_GRAD3[index][0] * x2 + GEN_API::SIMPLEX_GRAD3[index][1] * y2);
    }

    return 70.0f * n;
}

float GEN_API::Simplex::getNoise3D(float x, float y, float z) {
    x += offsetX;
    y += offsetY;
    z += offsetZ;

    float s = (x + y + z) * F3;
    int i = (int) (x + s);
    int j = (int) (y + s);
    int k = (int) (z + s);
    float t = (i + j + k) * G3;

    float x0 = x - (i - t);
    float y0 = y - (j - t);
    float z0 = z - (k - t);

    char i1, j1, k1, i2, j2, k2;
    if (x0 >= y0) {
        if (y0 >= z0) {
            i1 = 1;
            j1 = 0;
            k1 = 0;
            i2 = 1;
            j2 = 1;
            k2 = 0;
        } else if (x0 >= z0) {
            i1 = 1;
            j1 = 0;
            k1 = 0;
            i2 = 1;
            j2 = 0;
            k2 = 1;
        } else {
            i1 = 0;
            j1 = 0;
            k1 = 1;
            i2 = 1;
            j2 = 0;
            k2 = 1;
        }
    } else {
        if (y0 < z0) {
            i1 = 0;
            j1 = 0;
            k1 = 1;
            i2 = 0;
            j2 = 1;
            k2 = 1;
        } else if (x0 < z0) {
            i1 = 0;
            j1 = 1;
            k1 = 0;
            i2 = 0;
            j2 = 1;
            k2 = 1;
        } else {
            i1 = 0;
            j1 = 1;
            k1 = 0;
            i2 = 1;
            j2 = 1;
            k2 = 0;
        }
    }

    float x1 = x0 - i1 + G3;
    float y1 = y0 - j1 + G3;
    float z1 = z0 - k1 + G3;
    float x2 = x0 - i2 + 2.0f * G3;
    float y2 = y0 - j2 + 2.0f * G3;
    float z2 = z0 - k2 + 2.0f * G3;
    float x3 = x0 - 1.0f + 3.0f * G3;
    float y3 = y0 - 1.0f + 3.0f * G3;
    float z3 = z0 - 1.0f + 3.0f * G3;

    short ii = i & 255;
    short jj = j & 255;
    short kk = k & 255;

    float n = 0;
    float ti;

    ti = 0.6f - x0 * x0 - y0 * y0 - z0 * z0;
    if(ti > 0){
        auto gi0 = SIMPLEX_GRAD3[perm[ii + perm[jj + perm[kk]]] % 12];
        n += ti * ti * ti * ti * (gi0[0] * x0 + gi0[1] * y0 + gi0[2] * z0);
    }

    ti = 0.6f - x1 * x1 - y1 * y1 - z1 * z1;
    if(ti > 0){
        auto gi1 = SIMPLEX_GRAD3[perm[ii + i1 + perm[jj + j1 + perm[kk + k1]]] % 12];
        n += ti * ti * ti * ti * (gi1[0] * x1 + gi1[1] * y1 + gi1[2] * z1);
    }

    ti = 0.6f - x2 * x2 - y2 * y2 - z2 * z2;
    if(ti > 0){
        auto gi2 = SIMPLEX_GRAD3[perm[ii + i2 + perm[jj + j2 + perm[kk + k2]]] % 12];
        n += ti * ti * ti * ti * (gi2[0] * x2 + gi2[1] * y2 + gi2[2] * z2);
    }

    ti = 0.6f - x3 * x3 - y3 * y3 - z3 * z3;
    if(ti > 0){
        auto gi3 = SIMPLEX_GRAD3[perm[ii + 1 + perm[jj + 1 + perm[kk + 1]]] % 12];
        n += ti * ti * ti * ti * (gi3[0] * x3 + gi3[1] * y3 + gi3[2] * z3);
    }

    return 32.0f * n;
}

vector<string> split(const string &s, char delim) {
    vector<string> elems;
    std::stringstream ss;
    ss.str(s);
    string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

GEN_API::BlockTransactionElement::BlockTransactionElement(string encoded) {
    vector<string> tokens = split(encoded, '|');
    blockId = "" + tokens.at(0);
    localX = (char) STR_TO_INT(tokens.at(1));
    localY = (short) STR_TO_INT(tokens.at(2));
    localZ = (char) STR_TO_INT(tokens.at(3));
    tileData = (unsigned short) STR_TO_INT(tokens.at(4));
    placeIsNotFree = (bool) STR_TO_INT(tokens.at(5));
}

void GEN_API::BlockTransactionElement::tryPlace(LevelChunk* levelChunk) {
    ChunkBlockPos pos(localX, localY, localZ);
    if (!placeIsNotFree) {
        if (levelChunk->getBlock(pos).getId() != 0) return;
    }
    levelChunk->setBlockSimple(pos, *Block::create(blockId, tileData));
}

string GEN_API::BlockTransactionElement::encode() {
    return blockId + "|" +
           to_string((short) localX) + "|" +
           to_string(localY) + "|" +
           to_string((short) localZ) + "|" +
           to_string(tileData) + "|" +
           to_string((short) placeIsNotFree);
}

void GEN_API::transactionPostProcessingGeneration(LevelChunk* levelChunk, ChunkPos const& chunkPos) {
    vector <GEN_API::BlockTransactionElement> elements;
    std::string path = Level::getCurrentLevelPath() +
                       "/transactions/" + to_string(chunkPos.x) +
                       "." + to_string(chunkPos.z);

    std::ifstream transaction(path);
    if (transaction.is_open()) {
        std::string line;
        while (std::getline(transaction, line))
            if (line != "")
                elements.push_back(BlockTransactionElement(line));
    }
    transaction.close();
    std::remove(path.c_str());

    for (GEN_API::BlockTransactionElement element: elements){
        element.tryPlace(levelChunk);
    }
}

void GEN_API::createTransactionCache(ChunkPos const& chunkPos, vector<GEN_API::BlockTransactionElement> elements) {
    for (GEN_API::BlockTransactionElement element: elements) {
        std::string path = Level::getCurrentLevelPath() +
                           "/transactions/" + to_string(chunkPos.x) +
                           "." + to_string(chunkPos.z);
        std::ifstream itransaction(path);

        if (itransaction.is_open()) {
            std::string line;
            std::string content;

            while (std::getline(itransaction, line))
                content.insert(content.length(), line + "\n");

            content.insert(content.length(), element.encode());
            std::ofstream otransaction(path);
            otransaction << content;
        }
        else {
            std::ofstream otransaction(path);
            otransaction << element.encode();
        }
    }
}

void GEN_API::BlockTransaction::addBlock(int x, short y, int z, string blockId, unsigned short tileData, bool force) {
    GEN_API::BlockTransactionElement element(x, y, z, blockId, tileData,force);

    for (GEN_API::ChunkTransactionLink chunk: chunks) {
        if (chunk.x != G2C_COORD(x) || chunk.z != G2C_COORD(z)) continue;

        chunk.elements.push_back(element);
        return;
    }

    chunks.push_back({
                             G2C_COORD(x),
                             G2C_COORD(z),
                             {
                                     element,
                             }
                     });
}

void GEN_API::BlockTransaction::apply(LevelChunk* levelChunk, ChunkPos* chunkPos) {
    for (auto chunk: chunks) {
        if (chunk.x != chunkPos->x || chunk.z != chunkPos->z) {
            GEN_API::createTransactionCache(*chunkPos, chunk.elements);
            continue;
        }

        for (auto element: chunk.elements) {
            element.tryPlace(levelChunk);
        }
    }
}

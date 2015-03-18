#include "ExternalFile.h"
#include <fstream>
#include <filesystem>

ExternalFile::ExternalFile(std::wstring filename) : _filename(filename) {}

int ExternalFile::CheckValidity() const {
    std::tr2::sys::wpath filepath(_filename);
    if (!std::tr2::sys::exists(filepath)) {
        // New file, so we'll accept it!
        return 0;
    }

    std::ifstream infile(_filename, std::ios::in | std::ios::binary);
    if (infile.is_open()) {
        int header[4] = {0};
        infile.read((char*)header, sizeof(header));
    
        const int magic1 = header[0];
        const int magic2 = header[1];
        const int startFrame = header[2];
        const int size = header[3];

        if (magic1 == MAGIC_NUMBER_1 && magic2 == MAGIC_NUMBER_2 && size >= 0) {
            return size;
        }
    }

    return -1;
}

bool ExternalFile::Read(int* startFrameOut, std::vector<Grid*>* gridsOut) const {
    std::vector<Grid*> gridsTemp;

    std::ifstream infile(_filename, std::ios::in | std::ios::binary);
    if (!infile.is_open()) {
        goto fail;
    }
    
    // Read magic numbers, start frame, cache size.
    int header[4] = {0};
    infile.read((char*)header, sizeof(header));
    
    const int magic1 = header[0];
    const int magic2 = header[1];
    const int startFrame = header[2];
    const int size = header[3];

    if (magic1 != MAGIC_NUMBER_1 || magic2 != MAGIC_NUMBER_2 || size < 0) {
        goto fail;
    }

    // Read each grid.
    for (int i = 0; i < size; i++) {
        if (!infile.good()) {
            goto fail;
        }

        int blockHeader[1];
        float sizes[2];
        int segments[2];
        infile.read((char*)blockHeader, sizeof(blockHeader));
        infile.read((char*)sizes, sizeof(sizes));
        infile.read((char*)segments, sizeof(segments));
        
        if (!infile.good()) {
            goto fail;
        }

        const float width = sizes[0];
        const float length = sizes[1];
        const int widthSegs = segments[0];
        const int lengthSegs = segments[1];

        int numVertices = (widthSegs + 1) * (lengthSegs + 1);
        float* vertexHeights = new float[numVertices];
        infile.read((char*)vertexHeights, sizeof(float) * numVertices);
        
        if (!infile.good()) {
            delete [] vertexHeights;
            goto fail;
        }

        Grid* grid = new Grid(width, length, widthSegs, lengthSegs, vertexHeights);
        gridsTemp.push_back(grid);
    }
    
pass:
    for (Grid* g : *gridsOut) {
        delete g;
    }
    gridsOut->clear();
    *gridsOut = std::move(gridsTemp);
    *startFrameOut = startFrame;
    return true;

fail:
    for (Grid* g : gridsTemp) {
        delete g;
    }
    return false;
}

bool ExternalFile::Write(int startFrame, const std::vector<Grid*>& gridsIn) const {
    std::ofstream outfile(_filename, std::ios::out | std::ios::binary);
    if (!outfile.is_open()) {
        return false;
    }

    // Write magic numbers, start frame, cache size.
    int header[4] = {
        MAGIC_NUMBER_1,
        MAGIC_NUMBER_2,
        startFrame,
        gridsIn.size()
    };
    outfile.write((char*)header, sizeof(header));

    if (!outfile.good()) {
        return false;
    }

    for (Grid* grid : gridsIn) {
        float width = grid->GetWidth();
        float length = grid->GetLength();
        int widthSegs = grid->GetWidthSegs();
        int lengthSegs = grid->GetLengthSegs();
        float* vertexHeights = grid->GetVertexHeights();

        int blockHeader[1] = { BLOCK_HEADER };
        float sizes[2] = { width, length };
        int segments[2] = { widthSegs, lengthSegs };

        outfile.write((char*)blockHeader, sizeof(blockHeader));
        outfile.write((char*)sizes, sizeof(sizes));
        outfile.write((char*)segments, sizeof(segments));
        outfile.write((char*)vertexHeights, sizeof(float) * (widthSegs + 1) * (lengthSegs + 1));

        if (!outfile.good()) {
            return false;
        }
    }

    outfile.close();
    return true;
}

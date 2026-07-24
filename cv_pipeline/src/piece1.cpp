#include <iostream>

int main()
{
    unsigned char buffer[8] = {10, 20, 99, 99, // row 0: 2 real pixels + 2 padding bytes
                               30, 40, 99, 99};
    int cols = 2;
    int elemSize = 1;
    int nativeRowStride = cols * elemSize;
    int realRowStride = 4;

    std::cout << "--- Using the CORRECT given rowStride (4) ---\n";
    for (int r = 0; r < 2; r++)
    {
        for (int c = 0; c < cols; c++)
        {
            int address = r * realRowStride + c * elemSize;
            std::cout << (int)buffer[address] << " ";
        }
        std::cout << "\n";
    }
    std::cout << "--- Using the WRONG naive rowStride (cols*elemSize=2) ---\n";
    for (int r = 0; r < 2; r++)
    {
        for (int c = 0; c < cols; c++)
        {
            int address = r * nativeRowStride + c * elemSize;
            std::cout << (int)buffer[address] << " ";
        }
        std::cout << "\n";
    }
    return 0;
}
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* File read constants */

#define FILENAME_BUFFER_SIZE                1024
#define FILE_EXTENSION_SIZE                 5

/* Encoded block constant */

#define ENCODED_BLOCK_SIZE_IN_BYTES    512

/* Useful type constants */

#define UINT32_SIZE_IN_BITS                 32
#define UINT32_SIZE_IN_BYTES                4
#define UINT16_SIZE_IN_BYTES                2

/* WAVE header constants */

#define RIFF_ID_LENGTH                      4

/* WAVE format constants */

#define PCM_FORMAT                          1
#define NUMBER_OF_CHANNELS                  1
#define NUMBER_OF_BITS_IN_SAMPLE            16
#define NUMBER_OF_BYTES_IN_SAMPLE           2

/* Useful macro */

#define MIN(a, b)                           ((a) < (b) ? (a) : (b))

#define ASSERT(condition, message) { \
    if (!(condition)) { \
        printf("ERROR: %s\n", message); \
        return 1; \
    } \
}

/* File reading and writing macros */

#define READ(dest, size, file) { \
    uint32_t bytesRead = fread(dest, 1, size, file); \
    ASSERT(bytesRead == size, "Unexpectedly reached end of input file."); \
}

#define WRITE(source, size, file) { \
    uint32_t bytesWritten = fwrite(source, 1, size, file); \
    ASSERT(bytesWritten == size, "Could not write output file."); \
}

/* WAVE header structures */

#pragma pack(push, 1)

typedef struct {
    char id[RIFF_ID_LENGTH];
    uint32_t size;
} chunk_t;

typedef struct {
    uint16_t format;
    uint16_t numberOfChannels;
    uint32_t samplesPerSecond;
    uint32_t bytesPerSecond;
    uint16_t bytesPerCapture;
    uint16_t bitsPerSample;
} wavFormat_t;

#pragma pack(pop)

/* WAVE header components */

chunk_t chunk;

wavFormat_t wavFormat;

char id[RIFF_ID_LENGTH];

/* Buffers */

char outputFileName[FILENAME_BUFFER_SIZE];

char fileBuffer[ENCODED_BLOCK_SIZE_IN_BYTES];

char blankBuffer[ENCODED_BLOCK_SIZE_IN_BYTES];

/* Function to read the encoded block  */

uint32_t readEncodedBlock(char *buffer) {

    uint32_t numberOfBlocks = 0;

    for (uint32_t i = 0; i < UINT32_SIZE_IN_BITS; i += 1) {

        int16_t value = *((int16_t*)buffer + i);

        if (value == 1) {

            numberOfBlocks += (1 << i);

        } else if (value != -1) {

            return 0;

        }

    }

    for (uint32_t i = UINT32_SIZE_IN_BITS; i < ENCODED_BLOCK_SIZE_IN_BYTES / NUMBER_OF_BYTES_IN_SAMPLE; i += 1) {

        int16_t value = *((int16_t*)buffer + i);

        if (value != 0) return 0;

    }

    return numberOfBlocks;

}

/* Main function */

int main(int argc, char *argv[]) {

    /* Check arguments */

    ASSERT(argc == 2, "Incorrect arguments supplied.");

    char *inputFileName = argv[1];

    /* Check the input filename */

    uint32_t inputFileNameLength = strlen(inputFileName);

    ASSERT(strcmp(inputFileName + inputFileNameLength - FILE_EXTENSION_SIZE, "T.WAV") == 0, "Input file does not have a T.WAV extension.");

    /* Open the input files */

    FILE *fi = fopen(inputFileName, "rb");

    ASSERT(fi != NULL, "Could not open the input file.");

    /* Get the file size */

    fseek(fi, 0, SEEK_END);

    uint32_t inputFileSize = ftell(fi);

    fseek(fi, 0, SEEK_SET);

    /* Read the first chunk */

    READ(&chunk, sizeof(chunk_t), fi);
 
    ASSERT(memcmp(chunk.id, "RIFF", RIFF_ID_LENGTH) == 0, "Could not find RIFF chunk ID.");

    uint32_t riffSize = chunk.size;

    ASSERT(riffSize + sizeof(chunk_t) == inputFileSize, "RIFF chunk size does not match file size.");
   
    /* Read the WAVE id */

    READ(id, RIFF_ID_LENGTH, fi);
    
    ASSERT(memcmp (id, "WAVE", RIFF_ID_LENGTH) == 0, "Could not find WAVE ID.");

    /* Read the FMT chunk */

    READ(&chunk, sizeof(chunk_t), fi);

    ASSERT(memcmp (chunk.id, "fmt ", RIFF_ID_LENGTH) == 0, "Could not find FMT chunk ID.");

    ASSERT(chunk.size == sizeof(wavFormat_t), "FMT chunk size does not match expected size.");

    /* Read the WAVE format */

    READ(&wavFormat, sizeof(wavFormat_t), fi);

    ASSERT(wavFormat.format == PCM_FORMAT && wavFormat.numberOfChannels == NUMBER_OF_CHANNELS && wavFormat.bytesPerSecond == NUMBER_OF_BYTES_IN_SAMPLE * wavFormat.samplesPerSecond && wavFormat.bytesPerCapture == NUMBER_OF_BYTES_IN_SAMPLE && wavFormat.bitsPerSample == NUMBER_OF_BITS_IN_SAMPLE, "ERROR: Unexpected WAVE format."); 

    /* Skip the LIST chunk if it exists */

    READ(&chunk, sizeof(chunk_t), fi);

    if (memcmp (chunk.id, "LIST", RIFF_ID_LENGTH) == 0) { 
        
        fseek(fi, chunk.size, SEEK_CUR);

        READ(&chunk, sizeof(chunk_t), fi);

    }

    /* Read the DATA chunk */

    ASSERT(memcmp(chunk.id, "data", RIFF_ID_LENGTH) == 0, "Could not find DATA chunk ID.");

    uint32_t dataSize = chunk.size;

    uint32_t dataOffset = ftell(fi);

    ASSERT(dataSize + dataOffset == inputFileSize, "DATA chunk size does not match file size.");

    /* Generate the output filename */

    strncpy(outputFileName, inputFileName, inputFileNameLength - FILE_EXTENSION_SIZE);

    strncpy(outputFileName + inputFileNameLength - FILE_EXTENSION_SIZE, ".WAV", FILE_EXTENSION_SIZE);

    /* Open the output file */

    FILE *fo = fopen(outputFileName, "wb");

    ASSERT(fo != NULL, "Could not open the output file.");

    /* Rewind the input file */

    fseek(fi, 0, SEEK_SET);

    /* Start processing file sectors */

    uint32_t bytesRead = 0;

    uint32_t totalNumberOfExpandedBytes = 0;

    while (bytesRead < inputFileSize) {

        uint32_t bytes = MIN(inputFileSize - bytesRead, ENCODED_BLOCK_SIZE_IN_BYTES);

        READ(fileBuffer, bytes, fi);

        uint32_t numberOfBlocksToExpand = bytes == ENCODED_BLOCK_SIZE_IN_BYTES ? readEncodedBlock(fileBuffer) : 0;

        if (numberOfBlocksToExpand > 0) {

            totalNumberOfExpandedBytes += (numberOfBlocksToExpand - 1) * ENCODED_BLOCK_SIZE_IN_BYTES;

            for (uint32_t i = 0; i < numberOfBlocksToExpand; i += 1) {

                WRITE(blankBuffer, ENCODED_BLOCK_SIZE_IN_BYTES, fo);

            }

        } else {

            WRITE(fileBuffer, bytes, fo);

        }

        bytesRead += bytes;

    }

    /* Read the output file size */

    uint32_t outputFileSize = ftell(fo);

    /* Update header */
    
    if (totalNumberOfExpandedBytes > 0) {

        /* Update RIFF size in WAVE header */

        fseek(fo, RIFF_ID_LENGTH, SEEK_SET);

        riffSize += totalNumberOfExpandedBytes;

        WRITE(&riffSize, UINT32_SIZE_IN_BYTES, fo);

        /* Update DATA size in WAVE header */

        fseek(fo, dataOffset - UINT32_SIZE_IN_BYTES, SEEK_SET);

        dataSize += totalNumberOfExpandedBytes;

        WRITE(&dataSize, UINT32_SIZE_IN_BYTES, fo);

    }

    /* Check the output file size */

    ASSERT(dataSize + dataOffset == outputFileSize, "Output file size does not match DATA chunk size.");

    ASSERT(riffSize + sizeof(chunk_t) == outputFileSize, "Output file size does not match RIFF chunk size.");

    /* Close both files */

    ASSERT(fclose(fo) == 0, "Could not close the output file.");

    ASSERT(fclose(fi) == 0, "Could not close the input file.");

    /* Finished */

    puts("OKAY");

}

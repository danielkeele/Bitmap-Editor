/*
Written by Daniel Keele
*/

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "Pixel.h"

void ApplyThreshold(Pixel** rows, int height, int  width);
void ApplyGrayscale(Pixel** rows, int height, int  width);
Pixel** ScaleDown(Pixel** rows, int height, int width);
int HandleInputErrors(int argumentCount, char **arguments);
void PrintPixelArray(Pixel** rows, int height, int width);

int main(int argumentCount, char **arguments)
{
    //Handle input errors -------------------------------
    int errorResult = HandleInputErrors(argumentCount, arguments);

    if (errorResult != 0)
    {
        return errorResult;
    }

    //Read the requested bitmap file
    FILE* inputFile = fopen(arguments[1], "r");
    char* command = arguments[2];

    //Read the Bitmap Header
    char bm[3];
    int reserved;
    int headerSize;
    int numberOfColorPlanes;
    int bitsPerPixel;
    int compressionMethod;
    int imageSize;
    int bytePasser;
    int fileSize;
    int offset;
    signed int width;
    signed int height;
    int horizontalResolution;
    int verticalResolution;
    int numberColorsInPalette;
    int numberImportantColors;

    fread(bm, sizeof(bm) - 1, 1, inputFile);
    fread(&fileSize, 4, 1, inputFile);
    fread(&reserved, 4, 1, inputFile);
    fread(&offset, 4, 1, inputFile);

    //Read the Device-Independent Bitmap Header
    fread(&headerSize, 4, 1, inputFile);
    fread(&width, 4, 1, inputFile);
    fread(&height, 4, 1, inputFile);
    fread(&numberOfColorPlanes, 2, 1, inputFile);
    fread(&bitsPerPixel, 2, 1, inputFile);
    fread(&compressionMethod, 4, 1, inputFile);
    fread(&imageSize, 4, 1, inputFile);
    fread(&horizontalResolution, 4, 1, inputFile);
    fread(&verticalResolution, 4, 1, inputFile);
    fread(&numberColorsInPalette, 4, 1, inputFile);
    fread(&numberImportantColors, 4, 1, inputFile);

    //Create arrays to hold pixel values
    Pixel** rows = (Pixel**)malloc(sizeof(Pixel*) * height);
    
    for (int x = 0; x < height; x++)
    {
        Pixel* row = (Pixel*)malloc(sizeof(Pixel) * width);
        rows[x] = row;
    }

    //Read in and store pixels and their values
    for (int x = height - 1; x >= 0; x--)
    {
        for (int y = 0; y < width; y++)
        {
            fread(&rows[x][y].B, 1, 1, inputFile);
            fread(&rows[x][y].G, 1, 1, inputFile);
            fread(&rows[x][y].R, 1, 1, inputFile);
        }

        int paddingRemainder = width % 4;
        if (paddingRemainder != 0)
        {
            int holder;
            fread(&holder, paddingRemainder, 1, inputFile);
        }
    }

    fclose(inputFile);

    //Create new filename ---------------------------------------------
    char newFileName[strlen(arguments[1])];
    strcpy(newFileName, arguments[1]);
    newFileName[strlen(arguments[1]) - 4] = 0;

    //Perform Transformations ----------------------------------------
    if (strcmp(command, "T") == 0)
    {
        ApplyThreshold(rows, height, width);
        strcat(newFileName, "_Threshold.bmp");
    }
    else if (strcmp(command, "G") == 0)
    {
        ApplyGrayscale(rows, height, width);
        strcat(newFileName, "_Grayscale.bmp");
    }
    else if (strcmp(command, "S") == 0)
    {
        rows = ScaleDown(rows, height, width);

        int newHeight = height / 2;
        int newWidth = width / 2;

        if (height % 2 != 0)
        {
            newHeight += 1;
        }

        if (width % 2 != 0)
        {
            newWidth += 1;
        }

        height = newHeight;
        width = newWidth;
        fileSize = (newHeight * newWidth) + 14 + headerSize;
        strcat(newFileName, "_Scaled.bmp");
    }

    //Write and output new File ---------------------------------------------------
    FILE* outputFile;

    outputFile = fopen(newFileName, "w");

    fwrite(bm, sizeof(bm) - 1, 1, outputFile);
    fwrite(&fileSize, 4, 1, outputFile);
    fwrite(&reserved, 4, 1, outputFile);
    fwrite(&offset, 4, 1, outputFile);
    fwrite(&headerSize, 4, 1, outputFile);
    fwrite(&width, 4, 1, outputFile);
    fwrite(&height, 4, 1, outputFile);
    fwrite(&numberOfColorPlanes, 2, 1, outputFile);
    fwrite(&bitsPerPixel, 2, 1, outputFile);
    fwrite(&compressionMethod, 4, 1, outputFile);
    fwrite(&imageSize, 4, 1, outputFile);
    fwrite(&horizontalResolution, 4, 1, outputFile);
    fwrite(&verticalResolution, 4, 1, outputFile);
    fwrite(&numberColorsInPalette, 4, 1, outputFile);
    fwrite(&numberImportantColors, 4, 1, outputFile);

    for (int x = height - 1; x >= 0; x--)
    {
        for (int y = 0; y < width; y++)
        {
            fwrite(&rows[x][y].B, 1, 1, outputFile);
            fwrite(&rows[x][y].G, 1, 1, outputFile);
            fwrite(&rows[x][y].R, 1, 1, outputFile);
        }
        
        int paddingRemainder = width % 4;
        if (paddingRemainder != 0)
        {
            for (int y = 0; y < paddingRemainder; y++)
            {
                unsigned char holder = 0;
                fwrite(&holder, 1, 1, outputFile);
            }
        }
    }

    fclose(outputFile);

    return 0;
}

int HandleInputErrors(int argumentCount, char **arguments)
{
    if (argumentCount < 3)
    {
        printf("Please enter a filename and command.\nT - Threshold filter\nG - Grayscale filter\nS - Scale down the image\n");
        return 1;
    }

    FILE *inputFile = fopen(arguments[1], "r");
    char *command = arguments[2];

    if (inputFile == 0)
    {
        printf("Failed to open %s\n", arguments[1]);
        return 2;
    }

    if (strcmp(command, "T") != 0 &&
        strcmp(command, "G") != 0 &&
        strcmp(command, "S") != 0)
    {
        printf("Please enter T, G, or S as a command\n");
        return 3;
    }

    char* periodAddress = strrchr(arguments[1], '.');

    if (!periodAddress)
    {
        printf("File must be of type .bmp\n");
        return 4;
    }
    
    char* stringIndex = periodAddress;
    char fileType[5];
    int loopIndex = 0;
    while (loopIndex != 4)
    {
        fileType[loopIndex] = (char)*stringIndex;
        stringIndex++;
        loopIndex++;
    }
    fileType[loopIndex] = '\0';

    if ((strcmp(fileType, ".bmp") != 0 && strcmp(fileType, ".BMP") != 0))
    {
        printf("File must be of type .bmp\n");
        return 4;
    }

    return 0;
}

void ApplyThreshold(Pixel** rows, int height, int  width)
{
    for (int x = 0; x < height; x++)
    {
        for (int y = 0; y < width; y++)
        {
            int average = ((unsigned int)rows[x][y].R +
                            (unsigned int)rows[x][y].G +
                            (unsigned int)rows[x][y].B) / 3;

            //black
            int assignment = 0;

            if (average > 128)
            {
                //white
                assignment = 255;
            }

            rows[x][y].R = assignment;
            rows[x][y].G = assignment;
            rows[x][y].B = assignment;
        }
    }
}

void ApplyGrayscale(Pixel** rows, int height, int  width)
{
    for (int x = 0; x < height; x++)
    {
        for (int y = 0; y < width; y++)
        {
            int average = ((unsigned int)rows[x][y].R +
                            (unsigned int)rows[x][y].G +
                            (unsigned int)rows[x][y].B) / 3;

            rows[x][y].R = average;
            rows[x][y].G = average;
            rows[x][y].B = average;
        }
    }
}

Pixel** ScaleDown(Pixel** rows, int height, int width)
{
    int newHeight = height / 2;
    int newWidth = width / 2;

    if (height % 2 != 0)
    {
        newHeight += 1;
    }

    if (width % 2 != 0)
    {
        newWidth += 1;
    }

    Pixel** newRows = (Pixel**)malloc(sizeof(Pixel*) * newHeight);
    
    for (int x = 0; x < newHeight; x++)
    {
        Pixel* newRow = (Pixel*)malloc(sizeof(Pixel) * newWidth);
        newRows[x] = newRow;
    }

    int newColumnIndex = newHeight - 1;
    for (int x = height - 1; x >= 0; x -= 2)
    {
        int newRowIndex = 0;
        for (int y = 0; y < width; y += 2)
        {
            bool permitMoveUp = x != 0;
            bool permitMoveRight = y != width - 1;

            int totalRed = rows[x][y].R;
            int totalGreen = rows[x][y].G;
            int totalBlue = rows[x][y].B;

            //up
            if (permitMoveUp)
            {
                totalRed += rows[x - 1][y].R;
                totalGreen += rows[x - 1][y].G;
                totalBlue += rows[x - 1][y].B;
            }

            //up right
            if (permitMoveUp && permitMoveRight)
            {
                totalRed += rows[x - 1][y + 1].R;
                totalGreen += rows[x - 1][y + 1].G;
                totalBlue += rows[x - 1][y + 1].B;
            }

            //right
            if (permitMoveRight)
            {
                totalRed += rows[x][y + 1].R;
                totalGreen += rows[x][y + 1].G;
                totalBlue += rows[x][y + 1].B;
            }

            newRows[newColumnIndex][newRowIndex].R = totalRed / 4;
            newRows[newColumnIndex][newRowIndex].G = totalGreen / 4;
            newRows[newColumnIndex][newRowIndex].B = totalBlue / 4;
            newRowIndex++;
        }

        newColumnIndex--;
    }

    return newRows;
}

void PrintPixelArray(Pixel** rows, int height, int width)
{
    for (int x = 0; x < height; x++)
    {
        for (int y = 0; y < width; y++)
        {
            printf("rows[%i][%i]\n", x, y);
            printf("R: %i\n", (unsigned int)rows[x][y].R);
            printf("G: %i\n", (unsigned int)rows[x][y].G);
            printf("B: %i\n", (unsigned int)rows[x][y].B);
            printf("\n");
        }
    }
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct
{
    int width;
    int height;
    unsigned char *pixels;
    unsigned char header[54];
} Image;

Image *createImageFromFile(const char *filename)
{
    FILE *file = fopen(filename, "rb");
    if (!file)
    {
        printf("Error opening file: %s\n", filename);
        return NULL;
    }

    // Read and verify the bitmap header
    unsigned char header[54];
    if (fread(header, sizeof(unsigned char), 54, file) != 54 || header[0] != 'B' || header[1] != 'M')
    {
        printf("Invalid BMP file: %s\n", filename);
        fclose(file);
        return NULL;
    }

    // Extract image dimensions
    int width = *(int *)&header[18];
    int height = *(int *)&header[22];

    // Calculate image padding
    int padding = 0;
    while ((width * 3 + padding) % 4 != 0)
    {
        padding++;
    }

    // Allocate memory for image pixels and header
    unsigned char *pixels = (unsigned char *)malloc((width * 3 + padding) * height);
    unsigned char *imageHeader = (unsigned char *)malloc(54);
    if (!pixels || !imageHeader)
    {
        printf("Failed to allocate memory for image: %s\n", filename);
        fclose(file);
        free(pixels);
        free(imageHeader);
        return NULL;
    }

    // Store the header and read image pixel data
    memcpy(imageHeader, header, 54);
    if (fread(pixels, sizeof(unsigned char), (width * 3 + padding) * height, file) != (width * 3 + padding) * height)
    {
        printf("Error reading pixel data from file: %s\n", filename);
        fclose(file);
        free(pixels);
        free(imageHeader);
        return NULL;
    }

    fclose(file);

    // Create the Image structure
    Image *image = (Image *)malloc(sizeof(Image));
    if (!image)
    {
        printf("Failed to allocate memory for Image structure: %s\n", filename);
        free(pixels);
        free(imageHeader);
        return NULL;
    }

    image->width = width;
    image->height = height;
    image->pixels = pixels;
    memcpy(image->header, imageHeader, 54); // Copy the header data

    free(imageHeader); // Free the temporary header memory

    return image;
}

void invert(Image *image)
{
    // Modify the pixel data as desired
    // For example, let's invert the colors of the image
    int numPixels = image->width * image->height;
    unsigned char *pixel = image->pixels;
    for (int i = 0; i < numPixels; i++)
    {
        *pixel = 255 - *pixel; // Invert the pixel value
        *(pixel + 1) = 255 - *(pixel + 1);
        *(pixel + 2) = 255 - *(pixel + 2);
        pixel += 3;
    }
}

void grayscale(Image *image)
{
    int numPixels = image->width * image->height;
    unsigned char *pixel = image->pixels;
    for (int i = 0; i < numPixels; i++)
    {
        int avg_round = (*pixel + *(pixel + 1) + *(pixel + 2)) / 3;
        *pixel = *(pixel + 1) = *(pixel + 2) = avg_round;
        pixel += 3;
    }
}

void reflectY(Image *image)
{
    int rowSize = image->width * 3;
    unsigned char *tempRow = (unsigned char *)malloc(rowSize);
    if (!tempRow)
    {
        printf("Failed to allocate memory for temporary row\n");
        return;
    }

    int halfHeight = image->height / 2;
    int lastRowIndex = image->height - 1;

    for (int i = 0; i < halfHeight; i++)
    {
        int currentRowIndex = i * rowSize;
        int oppositeRowIndex = lastRowIndex * rowSize;

        // Copy the current row to a temporary row
        memcpy(tempRow, image->pixels + currentRowIndex, rowSize);

        // Copy the opposite row to the current row
        memcpy(image->pixels + currentRowIndex, image->pixels + oppositeRowIndex, rowSize);

        // Copy the temporary row to the opposite row
        memcpy(image->pixels + oppositeRowIndex, tempRow, rowSize);

        lastRowIndex--;
    }

    free(tempRow);
}

void reflectX(Image *image)
{
    int rowSize = image->width * 3;
    unsigned char *tempRow = (unsigned char *)malloc(rowSize);
    if (!tempRow)
    {
        printf("Failed to allocate memory for temporary row\n");
        return;
    }

    int halfWidth = image->width / 2;
    for (int i = 0; i < image->height; i++)
    {
        int rowIndex = i * rowSize;

        for (int j = 0; j < halfWidth; j++)
        {
            int currentPixelIndex = rowIndex + j * 3;
            int oppositePixelIndex = rowIndex + (image->width - j - 1) * 3;

            // Swap the pixels horizontally
            unsigned char temp = image->pixels[currentPixelIndex];
            image->pixels[currentPixelIndex] = image->pixels[oppositePixelIndex];
            image->pixels[oppositePixelIndex] = temp;

            temp = image->pixels[currentPixelIndex + 1];
            image->pixels[currentPixelIndex + 1] = image->pixels[oppositePixelIndex + 1];
            image->pixels[oppositePixelIndex + 1] = temp;

            temp = image->pixels[currentPixelIndex + 2];
            image->pixels[currentPixelIndex + 2] = image->pixels[oppositePixelIndex + 2];
            image->pixels[oppositePixelIndex + 2] = temp;
        }
    }

    free(tempRow);
}
// TO-Do Fix Repeat


int calculatePadding(int width)
{
    return (4 - (width * 3) % 4) % 4;
}

void saveImageToFile(const char *filename, Image *image)
{
    FILE *file = fopen(filename, "wb");
    if (!file)
    {
        printf("Error opening file for writing: %s\n", filename);
        return;
    }

    // Write the header back to the file
    fwrite(image->header, sizeof(unsigned char), 54, file);

    // Calculate padding
    int padding = calculatePadding(image->width);

    // Write the modified pixel data to the file
    int rowSize = image->width * 3 + padding;
    unsigned char *pixelRow = image->pixels;
    for (int i = 0; i < image->height; i++)
    {
        fwrite(pixelRow, sizeof(unsigned char), rowSize, file);
        pixelRow += rowSize;
    }

    fclose(file);
}

int main()
{
    const char *filename = "example.bmp";
    Image *image = createImageFromFile(filename);

    if (image)
    {
        printf("Image dimensions: %d x %d\n", image->width, image->height);

        // Update the image pixel data
        reflectX(image);

        // Save the modified image back to the file
        saveImageToFile("modified.bmp", image);

        // Free memory
        free(image->pixels);
        free(image);
    }

    return 0;
}

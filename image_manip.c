#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "image_manip.h"
#include "ppm_io.h"


////////////////////////////////////////
// Definitions of the functions       //
// declared in image_manip.h go here! //
////////////////////////////////////////
Pixel pixel_to_gray( Pixel p ) {
    unsigned char grayscale_intensity = (unsigned char)((.299*p.r)+(.587*p.g)+(.114*p.b));
    Pixel grayscale_pixel = {grayscale_intensity,grayscale_intensity,grayscale_intensity};
    return grayscale_pixel;
}

Image grayscale(const Image in) {

    Image out = make_image(in.rows, in.cols);

    for (int r = 0; r < in.rows; r++) {
        for (int c = 0; c < in.cols; c++) {

            int index = r * in.cols + c;
            out.data[index] = pixel_to_gray(in.data[index]);
        }
    }

    return out;
}

Pixel blend_pixel(const Pixel p1, const Pixel p2, double alpha) { 
    //Helper function to blend pixels that alpha can be applied to
    unsigned char new_r = (unsigned char)((p1.r)*(alpha) + (p2.r)*(1-alpha));
    unsigned char new_g = (unsigned char)((p1.g)*(alpha) + (p2.g)*(1-alpha));
    unsigned char new_b = (unsigned char)((p1.b)*(alpha) + (p2.b)*(1-alpha));
    Pixel blended_pixel = {new_r,new_g,new_b};
    return blended_pixel;
}

Image blend( const Image in1, const Image in2 , double alpha ) {
    if (alpha < 0 || alpha > 1) {
        printf("Alpha has to be between 0 and 1 inclusive");
        Image FAILURE = {NULL,0,0};
        return FAILURE;
    }
    int blend_rows = 0, blend_cols = 0;

    if (in1.rows > in2.rows) { //Obtain the new dimensions for the blended image
        blend_rows = in1.rows;
    } else {
        blend_rows = in2.rows; 
    }
    if (in1.cols > in2.cols) {
        blend_cols = in1.cols; 
    } else {
        blend_cols = in2.cols; 
    }
    Image blend_image = make_image(blend_rows,blend_cols); //Create new image that will be blended
    for (int r = 0; r < blend_rows; r++) {
        for (int c = 0; c < blend_cols; c++) {
            if (((r<in1.rows) && (r<in2.rows)) && ((c<in1.cols) && (c<in2.cols))) { //Overlap
                Pixel p1 = in1.data[(r*in1.cols)+c]; Pixel p2 = in2.data[(r*in2.cols)+c];
                blend_image.data[((r*blend_cols)+c)] = blend_pixel(p1,p2,alpha);
            }

            else if (((r<in1.rows) && (c<in1.cols)) && !((r<in2.rows) && (c<in2.cols))) { //Part of image that is covered by image 1 but not image 2
                blend_image.data[((r*blend_cols)+c)] = in1.data[(r*in1.cols)+c];
            }

            else if (!((r<in1.rows) && (c<in1.cols)) && ((r<in2.rows) && (c<in2.cols))) { //Part covered by image 2 but not image 1
                blend_image.data[((r*blend_cols)+c)] = in2.data[(r*in2.cols)+c]; 
            }
            else { //No no zone (no image covers this)
                Pixel black = {0,0,0};
                blend_image.data[((r*blend_cols)+c)] = black;
            }
        }
    }
    return blend_image;
}

Image crop( const Image in , Point top_left , Point bot_right ) {
    // Error management with top_left and bot_right)
    if (((top_left.x < 0) || (top_left.x >= in.cols)) || ((top_left.y < 0) || (top_left.y > in.rows))) {
        printf("Top left is not in the image, pick a different top_left");
        Image FAILURE = {NULL,0,0};
        return FAILURE;
    }
    if (((bot_right.x < 0) || (bot_right.x >= in.cols)) || ((bot_right.y < 0) || (bot_right.y > in.rows))) {
        printf("Bottom right is not in the image, pick a different bot_right");
        Image FAILURE = {NULL,0,0};
        return FAILURE;
    }
    if ((top_left.x >= bot_right.x) || (top_left.y >= bot_right.y)){ //Also includes error management for an x by 0 image
        //If top left dimension is greater, or equal (which would lead to 0) give error
        printf("Top left and bottom right are not valid");
        Image FAILURE = {NULL,0,0};
        return FAILURE;
    }
    int new_cols = 0; int new_rows = 0; //obtain size for new cropped image
    new_cols = bot_right.x - top_left.x; 
    new_rows = bot_right.y - top_left.y;
    Image cropped_image = make_image(new_rows,new_cols); //create size of new image
    for (int r = 0; r < new_rows; r++) {
        for (int c = 0; c < new_cols; c++) {
            cropped_image.data[(r*new_cols)+c] = in.data[(((r+top_left.y)*(in.cols))+(c+top_left.x))];
        }
    }
    return cropped_image;
}

Image rotate_ccw( const Image in ) { // rotate input image by 90 degrees counter-clockwise
    Image out = make_image(in.cols, in.rows); 
    for (int r = 0; r < in.rows; r++) { //allocate new image with reversed dimensions (cols & rows)
        for (int c= 0; c < in.cols; c++) {
            int in_index = r * in.cols + c;
            int new_row = in.cols - 1 - c; //formula to rotate image 90 degrees CCW
            int new_col = r; //new columns are just the old number of rows
            int out_index = new_row * out.cols + new_col;
            out.data[out_index] = in.data[in_index];
        }
    }
    return out;
}

Image pointilism(const Image in, unsigned int seed) {
    Image out = make_image(in.rows, in.cols);
    srand(seed);

    int total_pixels = in.rows * in.cols;
    int random_pixels = total_pixels * 3 / 100;

    // Step 1: Fill output with BLACK
    for (int i = 0; i < total_pixels; i++) {
        out.data[i].r = 0;
        out.data[i].g = 0;
        out.data[i].b = 0;
    }

    // Step 2: Add colored dots from original image
    for (int i = 0; i < random_pixels; i++) {

        int r = rand() % in.rows;
        int c = rand() % in.cols;
        int radius = rand() % 5 + 1;
  
                
        //original image color, very important
         Pixel center_color = in.data[r * in.cols + c];

        // Draw filled circle
        for (int rr = r - radius; rr <= r + radius; rr++) {
            for (int cc = c - radius; cc <= c + radius; cc++) {

                if (rr < 0 || rr >= in.rows || cc < 0 || cc >= in.cols)
                    continue;

                int dr = rr - r;
                int dc = cc - c;

                if (dr * dr + dc * dc <= radius * radius) {
                    out.data[rr * in.cols + cc] = center_color;
                }
            }
        }
    }

    return out;
}

double *gaussian(double sigma, int *N) { //2D array in row-major order, use pointer to return value of N
    *N = (int) (10 * sigma);
    if (*N % 2 == 0) { //Make N odd
        (*N)++;
    }
    double *distribution = malloc ((*N) * (*N) * sizeof(double)); //Dynamically allocate a 2D array as 1D
    if (!distribution) {
        return NULL;
    }

    int center = (*N)/2;
    for (int r = 0; r < (*N); r++) { //Implement Gaussian distribution
        for (int c = 0; c < (*N); c++) {
            int dx = c - center; int dy = r - center;
            double g = (1.0 / (2.0 * PI * sq(sigma))) * exp( -(sq(dx) + sq(dy)) / (2 * sq(sigma)));
            distribution[(r*(*N))+c]=g;
        }
    }
    return distribution;
}

Pixel blur_pixel(Image im, double *gaussian, int N, int r,int c) { //r,c is current Pixel in image
    int center = N/2;
    double r_total = 0,g_total = 0,b_total = 0,total_weight = 0; //initialize variables for eventual normalization
    for (int row = 0; row < N; row++) {  //row and col are for the Gaussian distribution
        for (int col = 0; col < N; col++) {

            int dy = row - center; int dx = col - center;
            int im_current_row = r + dy; //This will give the current row relative to the image
            int im_current_col = c + dx; //Current column relative to the image
            if (im_current_row < 0 || im_current_row >= im.rows ||im_current_col < 0||im_current_col >= im.cols) {
                continue; //If gaussian distribution goes out of bounds simply skip it
            }
            double gauss_weight = gaussian[row*N + col]; //access appropriate Gaussian weights
            Pixel cur_pixel = im.data[(im_current_row)*(im.cols)+(im_current_col)]; 
            r_total += gauss_weight *cur_pixel.r; //Apply weight to all pixels in an N by N box 
            g_total += gauss_weight *cur_pixel.g; 
            b_total += gauss_weight *cur_pixel.b;
            total_weight += gauss_weight;
        }
    } //now return normalized pixel with that Gaussian distribution
    Pixel output = {(double)(r_total/total_weight),(double)(g_total/total_weight),(double)(b_total/total_weight)};
    return output;
}

Image blur( const Image in , double sigma) {
    int N = 0;
    double *gauss_distribution = gaussian(sigma,&N); //Obtain Gaussian Distribution and value of N
    if (!gauss_distribution) {
        Image FAILURE = {NULL,0,0};
        return FAILURE; 
    }
    Image blurred_image = make_image(in.rows,in.cols);
    for (int r = 0; r < in.rows;r++) {
        for (int c = 0; c < in.cols; c++) {
            blurred_image.data[(r*in.cols)+c] = blur_pixel(in,gauss_distribution,N,r,c);
        }
    }
    free (gauss_distribution); //prevent memory leakage
    return blurred_image;
}




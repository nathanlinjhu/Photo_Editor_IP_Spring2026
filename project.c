//project.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ppm_io.h"
#include "image_manip.h"

// Return (exit) codes
#define RC_SUCCESS            0
#define RC_MISSING_FILENAME   1
#define RC_OPEN_FAILED        2
#define RC_INVALID_PPM        3
#define RC_INVALID_OPERATION  4
#define RC_INVALID_OP_ARGS    5
#define RC_OP_ARGS_RANGE_ERR  6
#define RC_WRITE_FAILED       7
#define RC_UNSPECIFIED_ERR    8

void print_usage(void);

int main (int argc, char *argv[]) {

  // Check that the correct number of command-line arguments were provided
  if (argc < 4) {
    fprintf(stderr,"Please input the correct # of arguments\n");
    print_usage();
    return RC_INVALID_OP_ARGS;
  }
  // Open the input image file in binary read mode
  FILE *input_file = fopen(argv[1], "rb");

  // Check if the file failed to open
  if (!input_file) {
    fprintf(stderr, "Error opening input file\n");
    return RC_OPEN_FAILED;
  }
  //Initialize Images to NULL,0,0
  Image input_image = {NULL,0,0};
  Image output_image = {NULL,0,0};

  // Read input image from the file
   input_image = read_ppm(input_file);
  fclose(input_file);
  if (!input_image.data) {
    fprintf(stderr,"Invalid PPM file\n");
    return RC_INVALID_PPM;
  }
  

  // Open the output file in binary write mode
  FILE *output_file;
  if (strcmp(argv[3],"blend")==0) {
  output_file = fopen(argv[4],"wb");
  } else {
  output_file = fopen(argv[2], "wb");
  } 
  // Check if the output file failed to open
  if (!output_file) {
    free(input_image.data);
    return RC_WRITE_FAILED;
  }
  // Determine which image operation the user requested
  // Applies the given request
  if (strcmp(argv[3], "pointilism") == 0) {
    if (argc != 4) {
      fprintf(stderr,"Pointilism only takes 4 CLI arguments\n");
      fclose(output_file);
      free(input_image.data);
      return RC_INVALID_OP_ARGS;
    }
    output_image = pointilism(input_image, 1);
  } else if (strcmp(argv[3], "grayscale") == 0) {
      if (argc != 4) {
      fprintf(stderr,"Grayscale only takes 4 CLI arguments\n");
      fclose(output_file);
      free(input_image.data);
      return RC_INVALID_OP_ARGS;
    }
    output_image = grayscale(input_image);
  } else if (strcmp(argv[3], "rotate-ccw") == 0) {
      if (argc != 4) {
      fprintf(stderr,"Rotate CCW only takes 4 CLI arguments\n");
      fclose(output_file);
      free(input_image.data);
      return RC_INVALID_OP_ARGS;
    }
    output_image = rotate_ccw(input_image);
  } else if (strcmp(argv[3], "blur") == 0) {
    if (argc != 5) {
      fprintf(stderr, "Blur requires a sigma value\n");
      fclose(output_file);
      free(input_image.data);
      return RC_INVALID_OP_ARGS;
    }
      double sigma = (double) strtod(argv[4],NULL); //Convert string to a double
      if (sigma <= 0) {
        fprintf(stderr,"Sigma must be a positive value\n");
        fclose(output_file);
        free(input_image.data);
        return RC_OP_ARGS_RANGE_ERR;
      }
      output_image = blur(input_image,sigma);
  } else if (strcmp(argv[3], "blend") == 0) {
    if (argc != 6) {
      fprintf(stderr,"You need a second image + an alpha value to blend\n");
      fclose(output_file);
      free(input_image.data); 
      return RC_INVALID_OP_ARGS;
    }
  
    FILE *in2 = fopen(argv[2], "rb");
    if (!in2) {
      fprintf(stderr,"Second file could not opened\n");
      fclose(output_file);
      free(input_image.data);
      return RC_OPEN_FAILED;
    }

    Image input_2 = read_ppm(in2);
    fclose(in2);
    if (!input_2.data) {
      fprintf(stderr,"Second file is not a valid PPM\n");
      fclose(output_file);
      free(input_image.data);
      return RC_INVALID_PPM;
    }


    double alpha = (double) strtod(argv[5],NULL);
    if (alpha < 0 || alpha > 1) {
      fprintf(stderr,"Alpha must be a value 0 through 1 inclusive\n");
      fclose(output_file);
      free(input_image.data);
      free(input_2.data);
      return RC_OP_ARGS_RANGE_ERR;
    }
    output_image = blend(input_image, input_2, alpha);
    free(input_2.data);

    } else if (strcmp(argv[3], "crop") == 0) {
      Point top_left = {(int) atoi(argv[4]),(int) atoi(argv[5])};
      Point bottom_right = {(int) atoi(argv[6]),(int) atoi(argv[7])};
      if (top_left.x < 0 || top_left.y < 0 || top_left.x > bottom_right.x || top_left.y > bottom_right.y) {
        fprintf(stderr,"Crop coordinates are not valid\n");
        fclose(output_file);
        free(input_image.data);
        return RC_OP_ARGS_RANGE_ERR;
      }
    output_image = crop(input_image,top_left,bottom_right);
    } else {
      fprintf(stderr,"Whatever operation you wanted is not supported, please try again\n");
      fclose(output_file);
      free(output_image.data);
      free(input_image.data);
      return RC_INVALID_OPERATION;
    }

    if (output_image.data == NULL && output_image.rows == 0 && output_image.cols == 0) {
      fprintf(stderr,"Output image was not able to be generated\n");
      fclose(output_file);
      free(input_image.data);
      return RC_UNSPECIFIED_ERR;
    }
    if (!write_ppm(output_file,output_image)) {
      fprintf(stderr,"Could not write output image to file\n");
      fclose(output_file);
      free(output_image.data);
      free(input_image.data);
      return RC_OPEN_FAILED;
    }
  // Program is completed successfully
  fclose(output_file);
  free(output_image.data);
  free(input_image.data);
  return RC_SUCCESS;
}

void print_usage(void) {
  printf("USAGE:\n");
  printf("   ./project <input-image> <output-image> grayscale\n");
  printf("   ./project <input-image> <output-image> rotate-ccw\n" );
  printf("   ./project <input-image> <output-image> pointilism\n" );
  printf("   ./project <input-image> <output-image> blur <sigma>\n" );
  printf("   ./project <input-image> <output-image> crop <top left (x,y)> <bottom right (x,y)>\n" );
  printf("   ./project <input-image1> <input-image2> blend <output-image> <alpha>\n" );
}

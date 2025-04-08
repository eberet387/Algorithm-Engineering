#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "implementation.hpp"

TEST_CASE("TestImage")
{
   int width = 5, height = 5, channels = 3;
   int total_size = width * height * channels;
   int pixel_amount = width * height;

   SECTION("Test loading a file that does not exist") {
      // file doesnt exist
      CHECK(load_ppm("tests/testfile.zip", width, height, channels) == nullptr);
   }

   SECTION("Test loading a ppm file with unsupported format") {
      // format : p6
      CHECK(load_ppm("tests/testfile_unsupported_format.ppm", width, height, channels) == nullptr);
   }

   SECTION("Test ppm file with unsupported max value") {
      // max val: 256
      CHECK(load_ppm("tests/testfile_unsupported_max_val.ppm", width, height, channels) == nullptr);
   }

   SECTION("Test ppm file with unsupported pixel data") {
      // -10 / 265
      CHECK(load_ppm("tests/testfile_unsupported_pixel_val.ppm", width, height, channels) == nullptr);
   }

   unsigned char* img = load_ppm("tests/testfile.ppm", width, height, channels);

   SECTION("Test correctly loading a ppm") {

      int img_values[] = {
        255, 0, 0, 200, 0, 0, 0, 0, 0, 100, 0, 0, 50, 0, 0,
        0, 255, 0, 0, 200, 0, 0, 0, 0, 0, 100, 0, 0, 50, 0,
        0, 0, 255, 0, 0, 200, 0, 0, 0, 0, 0, 100, 0, 0, 50,
        255, 255, 0, 200, 200, 0, 0, 0, 0, 100, 100, 0, 50, 50, 0,
        255, 0, 255, 200, 0, 200, 0, 0, 0, 100, 0, 100, 50, 0, 50
      };


      for (int i = 0; i < total_size; ++i) {
         CHECK(img[i] == img_values[i]);
      }
      
   }

   unsigned char* img_grayscale = toGrayScale(img, pixel_amount, channels);

   SECTION("Test grayscale conversion") {
      
      int gray_values[] = {
         85, 66, 0, 33, 16,
         85, 66, 0, 33, 16,
         85, 66, 0, 33, 16,
         170, 133, 0, 66, 33,
         170, 133, 0, 66, 33
      };

      for (int i = 0; i < pixel_amount; ++i) {
         CHECK(img_grayscale[i] == gray_values[i]);
      }
   
   }

   float mean = calculateMean(img_grayscale, pixel_amount);

   SECTION("Test mean calculation") {
      float mean_should = (16*3 + 33*5 + 66*5 + 85*3 + 133*2 + 170*2) / (float)25;

      CHECK(mean == mean_should);

      mean = mean_should; // for later computations if wrong
   }

   float* deviation = calculateDeviation(img_grayscale, pixel_amount, mean);

   SECTION("Test deviation calculation") {

      float deviation_should[] = {
         85-mean, 66-mean, mean-0, mean-33, mean-16,
         85-mean, 66-mean, mean-0, mean-33, mean-16,
         85-mean, 66-mean, mean-0, mean-33, mean-16,
         170-mean, 133-mean, mean-0, 66-mean, mean-33,
         170-mean, 133-mean, mean-0, 66-mean, mean-33
      };

      for (int i = 0; i < pixel_amount; ++i) {
         CHECK(deviation[i] == deviation_should[i]);
      }

      deviation = deviation_should; // for later computations if wrong
   }

   float standard_deviation = calculateStandardDeviation(img_grayscale, pixel_amount, mean, deviation);

   SECTION("Test standard deviation calculation") {

      long long sum_squared_should = 0.0;

      for (int i = 0; i < pixel_amount; ++i) {
        sum_squared_should += deviation[i] * deviation[i];
      }

      float standard_deviation_should = static_cast<float>(sum_squared_should) / pixel_amount;

      CHECK(standard_deviation > 2559.19);
      CHECK(standard_deviation < 2559.2);

      standard_deviation = standard_deviation_should; // for later computations if wrong
   }

   float min_deviation = checkMinDeviation(img_grayscale, pixel_amount, mean, deviation);
   float max_deviation = checkMaxDeviation(img_grayscale, pixel_amount, mean, deviation);

   SECTION("Test min and max deciation checking") {
      CHECK(min_deviation == deviation[1]);
      CHECK(max_deviation == deviation[20]);

      min_deviation = deviation[1];
      max_deviation = deviation[20]; // for later computations if wrong
   }

   int window_size = 3;
   int* window_border = calculateWindowBorder(width, height, window_size);

   SECTION("Test window border calculation") {
      // Top left pixel should only have a 2x2 window as it would otherise
      // access memory out of bounds.
      // The First 4 slots are always StartY, StartX, EndY, EndX positions,
      // while the last one is the amount of pixels within that window.

      // X -> Staring Pixel (also inside the window)
      // O -> All Pixels within the Pixel's window
      // # -> All Pixels outside the window

                                       //   0 1 2 3 4
      CHECK(window_border[0] == 0);  // 0 X O # # #
      CHECK(window_border[1] == 0);  // 1 O O # # #
      CHECK(window_border[2] == 1);  // 2 # # # # #
      CHECK(window_border[3] == 1);  // ...
      CHECK(window_border[4] == 4);  //
      // -> 2x2 pixels = 4

                                                             //   0 1 2 3 4
      CHECK(window_border[(2 * width + 2) * 5 + 0] == 1);  // 0 # # # # #
      CHECK(window_border[(2 * width + 2) * 5 + 1] == 1);  // 1 # O O O #
      CHECK(window_border[(2 * width + 2) * 5 + 2] == 3);  // 2 # O X O #
      CHECK(window_border[(2 * width + 2) * 5 + 3] == 3);  // 3 # O O O #
      CHECK(window_border[(2 * width + 2) * 5 + 4] == 9);  // 4 # # # # #
      // -> 3x3 pixels = 9

                                                             //   0 1 2 3 4
      CHECK(window_border[(4 * width + 4) * 5 + 0] == 3);  // 0 # # # # #
      CHECK(window_border[(4 * width + 4) * 5 + 1] == 3);  // 1 # # # # #
      CHECK(window_border[(4 * width + 4) * 5 + 2] == 4);  // 2 # # # # #
      CHECK(window_border[(4 * width + 4) * 5 + 3] == 4);  // 3 # # # O O
      CHECK(window_border[(4 * width + 4) * 5 + 4] == 4);  // 4 # # # O X
      // -> 2x2 pixels = 4
   }

   float* window_mean = calculateWindowMean(img_grayscale, width, height, window_size, window_border);

   SECTION("Test window mean calculation") {
      float top_left = (img_grayscale[0] + img_grayscale[1] + img_grayscale[5] + img_grayscale[6]) / (float)4;
      
      float middle = (img_grayscale[6] + img_grayscale[7] + img_grayscale[8] + 
                      img_grayscale[11] + img_grayscale[12] + img_grayscale[13] + 
                      img_grayscale[16] + img_grayscale[17] + img_grayscale[18]) / (float)9;

      float bottom_right = (img_grayscale[18] + img_grayscale[19] + img_grayscale[23] + img_grayscale[24]) / (float)4;
      
      
      CHECK(window_mean[0] == top_left);
      CHECK(window_mean[12] == middle);
      CHECK(window_mean[24] == bottom_right);
   }

   float* squared_diff = calculateWindowSquaredDiff(width, height, window_mean, deviation);
   float* window_standard_deviation = calculateWindowStandardDeviation(img_grayscale, width, height, window_size, squared_diff, window_border);

   SECTION("Test window standard deviation calculation") {

      // using pre-calculated values to assure it is correct for the corners and middle pixel.
      // corner top left
      CHECK(window_standard_deviation[0] > 43.6856);
      CHECK(window_standard_deviation[0] < 43.6857);

      //corner top right
      CHECK(window_standard_deviation[4] > 12.0797);
      CHECK(window_standard_deviation[4] < 12.0798);

      // middle
      CHECK(window_standard_deviation[12] > 25.8754);
      CHECK(window_standard_deviation[12] < 25.8756);

      // corner bottom left
      CHECK(window_standard_deviation[20] > 23.4882);
      CHECK(window_standard_deviation[20] < 23.4883);

      //corner bottom right
      CHECK(window_standard_deviation[24] > 21.5889);
      CHECK(window_standard_deviation[24] < 21.5890);
   }

   float* adaptive_deviation = calculateAdaptiveDeviation(window_standard_deviation, pixel_amount, min_deviation, max_deviation);

   SECTION("Test adaptive deviation calculation") {
      // using pre-calculated values to assure it is correct for the corners and middle pixel.
      // corner top left
      CHECK(adaptive_deviation[0] > 3519.9436);
      CHECK(adaptive_deviation[0] < 3519.9437);

      //corner top right
      CHECK(adaptive_deviation[4] > 232.9302);
      CHECK(adaptive_deviation[4] < 232.9303);

      // middle
      CHECK(adaptive_deviation[12] > 1667.6921);
      CHECK(adaptive_deviation[12] < 1667.6922);

      // corner bottom left
      CHECK(adaptive_deviation[20] > 1419.4213);
      CHECK(adaptive_deviation[20] < 1419.4214);

      //corner bottom right
      CHECK(adaptive_deviation[24] > 1221.8948);
      CHECK(adaptive_deviation[24] < 1221.8959);
   }

   float noise_multiplier = 0.45;
   float* threshold = calculateThreshold(mean, pixel_amount, window_mean, adaptive_deviation, window_standard_deviation, noise_multiplier);

   SECTION("Test threshold calculation") {
      // using pre-calculated values to assure it is correct for the corners and middle pixel.
      // corner top left
      CHECK(threshold[0] > 33.9678);
      CHECK(threshold[0] < 33.9679);

      //corner top right
      CHECK(threshold[4] > 11.0091);
      CHECK(threshold[4] < 11.0092);

      // middle
      CHECK(threshold[12] > 19.8437);
      CHECK(threshold[12] < 19.8438);

      // corner bottom left
      CHECK(threshold[20] > 68.0852);
      CHECK(threshold[20] < 68.0853);

      //corner bottom right
      CHECK(threshold[24] > 22.2636);
      CHECK(threshold[24] < 22.2637);
   }

   unsigned char* img_enhanced = toEnhanced(img_grayscale, pixel_amount, channels, threshold);

   SECTION("Test the new black/white pixels after enhancing") {
      for (int i = 0; i < pixel_amount; ++i) {
         // pixels should be all black in a straight line in the middle of the screen, from top to bottom.
         int pixel = (i % 5 == 2) ? 0 : 255;

         CHECK((int)img_enhanced[i] == pixel);
         
      }
   }

   SECTION("Test correct ppm writing") {
      const char* filename = "test_output.ppm";

      CHECK(writePPM(filename, width, height, img_enhanced) == 1);
   }

   SECTION("Test ppm writing with missing filename") {
      const char* filename = "";

      CHECK(writePPM(filename, width, height, img_enhanced) == 0);
   }
}

   // for (int i = 0; i < pixel_amount; ++i) {
   //       int data = img_grayscale[i];
   //       if (i % (width) == 0)
   //       std::cout << "\n";
   //       std::cout << data << ", ";
   //    }
   // }
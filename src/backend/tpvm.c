/***************************************************************************
 * File: tpvm.c
 *
 * Description:
 *  This file contains the functions needed to create a tpvm marker frame.
 *  For now it only has one function that is useable outside this file.
 *
 * Author: Tobias Zels
 * Date: August 10, 2024
 *
 * License:
 *
 * Functions:
 *  - marker_frame create_marker(char* data, 
 *                               marker_type type, 
 *                               int borderpxl,
 *                               int scale, 
 *                               bool repeated,
 *                               int width,
 *                               int height)
 *
 * Dependencies:
 *  - stdbool.h: To define bool.
 *  - stdio.h: For input/output operations.
 *  - stdint.h: For uint32_t.
 *  - stdlib.h: Standard lib.
 *  - string.h: For string functions.
 *  - math.h: For floor function.
 *  - fcntl.h: To open fifo file.
 *  - unistd.h: For standard symbolic constants and types.
 *  - zint.h: For Dot Code marker.
 *  - qrencode.h: For QR Code marker.
 *  - types.h: General types.
 *  - utils.h: General utils.
 *  - utash.h: For Hashmap.
 *  - picom.h: For global variables declared at the start.
 *  - tpvm.h: Headerfile for enum and struct declarations.
 *
 ****************************************************************************/

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <fcntl.h>
#include <unistd.h>


#include <zint.h>
#include <qrencode.h>

#include "uthash.h"
#include "picom.h"
#include "types.h"
#include "utils.h"
#include "backend/tpvm.h"
#define FIFO_PATH "datafifo"


/** 
 * @brief Creates a qr code marker.
 *
 * This function takes a char array and encodes it into a qr code marker.
 * The marker and its size will get returned in the other parameters.
 *
 * @param data A char array with the data that will be encoded into the marker.
 * @param marker A empty uint32_t pointer that will contain the marker data.
 * @param marker_width The width of the created marker.
 * @param marker_height The height of the created marker.
 *
 * @note This function shouldn't get called by itself, but through create_marker()
 *
 */
void create_qr_marker(
    char* data,
    uint32_t** marker,
    int* marker_width,
    int* marker_height
    ){

    // Create a new QR Code with the libqrencode library
    QRcode* qrCode = NULL;
    int dataLength = strlen(data);
	qrCode = QRcode_encodeData(dataLength, data, 0, QR_ECLEVEL_H);

    // Set the pointers to the values
    *marker_width = (int)qrCode->width;
    *marker_height = (int)qrCode->width;

    int image_size = (int)(*marker_width * *marker_height *  sizeof(uint32_t));
    *marker = malloc(image_size);
    
    auto marker_data = qrCode->data;

    // Convert the QR marker to uint32_t
    for(int x = 0; x < qrCode->width; x++){
        for(int y = 0; y < qrCode->width; y++){
            int index = y * qrCode->width + x;
            int module = marker_data[index];
            (*marker)[index] = (module & 1) ? 0xFFFFFF : 0x000000;
        }
    }

    // Free Memory
	QRcode_free(qrCode);
}

// OpenCV, the library for Aruco is no longer supported in C, so use C++ 
// or Premade rendered Markers for Aruco to implement this function
void create_aruco_marker(
    char* data,
    uint32_t** marker,
    int* marker_width,
    int* marker_height
    ){
    // TODO: For Support implement a way to generate Markers
    printf(stderr, "Error: The create_aruco_marker() function is not implemented yet\n"); 
    abort();
}

/** 
 * @brief Creates a dot code marker.
 *
 * This function takes a char array and encodes it into a dot code marker.
 * The marker and its size will get returned in the other parameters.
 *
 * @param data A char array with the data that will be encoded into the marker.
 * @param marker A empty uint32_t pointer that will contain the marker data.
 * @param marker_width The width of the created marker.
 * @param marker_height The height of the created marker.
 *
 * @note This function shouldn't get called by itself, but through create_marker()
 *
 */
void create_dot_code_marker(
    char* data,
    uint32_t** marker,
    int* marker_width,
    int* marker_height
    ){

    // Create DotCode Marker with Zint library
    struct zint_symbol *zint = ZBarcode_Create();
	zint->symbology = BARCODE_DOTCODE;
	zint->scale = 1; // Scale is 1 cause we rescale the marker manually later

	ZBarcode_Encode_and_Buffer(zint, (uint8_t *)(data), 0, 0);
    // Prints the Dotcode into out.png for debuging 
	// ZBarcode_Print(zint, 0);

    int image_size = zint->bitmap_width * zint->bitmap_height *  sizeof(uint32_t);
    *marker = malloc(image_size);

    // Convert the unsigned char format to uint32_t changing rgb values into hex
    int pixPos, arrayPos, r, g, b;
    pixPos = 0;
    arrayPos = 0;
   
    for(int row = 0; row < zint->bitmap_height; row++){
        for(int col = 0; col < zint->bitmap_width; col++){
            r = (int)zint->bitmap[pixPos];
            g = (int)zint->bitmap[pixPos + 1];
            b = (int)zint->bitmap[pixPos + 2];
            uint32_t rgb = ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
            (*marker)[arrayPos] = (rgb & 1) ? 0xFFFFFF : 0x000000;
            pixPos+= 3;
            arrayPos++;
        }
    }

    // Set the pointers to the values
    *marker_width = zint->bitmap_width;
    *marker_height = zint->bitmap_height;

    // Free Memory
    ZBarcode_Delete(zint);
}

/** 
 * @brief Repeats the marker depending on window size.
 *
 * This function scales the marker and adds a border around it.
 * The marker will get repeated depending on the size of the marker and the size
 * of the Window.
 *
 * @param data A char array with the data that will be encoded into the marker.
 * @param marker The marker as an uint32_t pointer that will contain the altered marker data.
 * @param marker_width The width of the altered marker as int pointer.
 * @param marker_height The height of the altered marker as int pointer.
 * @param borderpxl The ammount of pixles that should get added to the marker as border as int.
 * @param scale The factor of how much the marker should get scaled up as int.
 * @param repeat A bool value if the marker should get repeated.
 * @param height A int value of the height of the window.
 * @param width A int value of the width of the window.
 * @return The needed memory of the marker image
 *
 * @note This function shouldn't get called by itself, but through create_marker().
 *       Doesn't have an alternative to repeat build in yet, so the marker should appear
 *       at the top left corner of the window with repeat = false.
 *
 */
int create_repeated_marker(
    uint32_t** marker,
    int* marker_width,
    int* marker_height,
    int borderpxl,
    int scale,
    bool repeat,
    int height,
    int width){
    // Calculate the imagesize depending with borderpixel and scaling and
    // allocate the needed memory
    int border_width = *marker_width * scale + 2 * borderpxl;
    int border_height = *marker_height * scale + 2 * borderpxl;
    int image_size = border_width * border_height *  sizeof(uint32_t);
    uint32_t* scaledMarker;
    scaledMarker = malloc(image_size);

    // Fill the memory with black pixel
    for(int i = 0; i < border_height; i++){
        for(int j = 0; j < border_width; j++){
            scaledMarker[i* border_width + j] = 0x000000;
        }
    }

    // Make sure we start with x Pixel offset depending on the borderpixel width
    // to leave a black border around the Marker
    int currentRow = borderpxl; 

    // We Scale the Marker depending on scale parameter and save it into scaledMarker
    for(int y = 0; y < *marker_height; ++y){
        for(int row = 0; row < scale; ++row){
            int currentCol = borderpxl; // same as currentRow
            for(int x = 0; x < *marker_width; ++x){
                int index = y * *marker_width + x;
                int module = (*marker)[index];
                    for(int col = 0; col < scale; ++col){
                            (scaledMarker)[currentRow * border_width + currentCol] = (module & 1) ? 0xFFFFFF : 0x000000;
                        
                    currentCol++;
                }
            }
            currentRow++;
        }
    }
    
    // The Marker width now changed and we dont need the old ones anymore
    // so update the values
    *marker_width = border_width;
    *marker_height = border_height;

    // TODO: In case the Marker isnt getting repeated it will just appear in the
    // left upper corner maybe add a center function in the future
    if(repeat){
        // Calculate how often the Marker fits into the background image and
        // floor it to get integer numbers -> use floor to prevent overflow
        int heightMult = floor(height/ *marker_height);
        int widthMult = floor(width/ *marker_width);

        // Allocate memory for the repeated Marker, we override image_size for later
        image_size = *marker_height * *marker_width * sizeof(uint32_t) * heightMult * widthMult;
        uint32_t* pixel_data = malloc(image_size);

        // Make sure that the Marker fits into the background size
        if(heightMult*widthMult > 0){
            
            // Basicly same as the scale loop just that we repeat the pixel sequenze
            // instead of a single pixel
			for (int ry = 0; ry < heightMult; ry++){
				for (int y = 0; y < *marker_height; ++y){
					for (int rx = 0; rx < widthMult ;rx++){
						for(int x = 0; x < *marker_width; ++x){
							int index = y * *marker_width + x;
							int module = scaledMarker[index];

							pixel_data[(x + rx * *marker_width) + (y * *marker_width * widthMult) + ry * *marker_height * *marker_width * widthMult] = (module & 1) ? 0xFFFFFF : 0x000000;
						}
					}
				}
			}
        }
        // Update the marker size
        *marker_height = *marker_height * heightMult;
        *marker_width = *marker_width * widthMult;

        // We no longer need scaled Marker so we free the memory
        free(scaledMarker);
        scaledMarker = malloc(image_size); // and allocate new memory 

        // To copy the contents of pixel_data to scaledMarker
        memcpy(scaledMarker, pixel_data, image_size);

        free(pixel_data); // and free pixel_data for simplicity later on
    }

    // We repeat the same as above so we can return the finished marker image with the
    // received marker pointer, image_size should be the right value
    free(*marker);
    *marker = malloc(image_size);

    memcpy(*marker, scaledMarker, image_size);

    free(scaledMarker);

    // Last we return the image_size
    return image_size;
}

extern TPVM_Window* tpvm_windows;

TPVM_Window* add_window(const char* name){
	TPVM_Window* tpvm_window;
	tpvm_window = malloc(sizeof(TPVM_Window));
	tpvm_window->qr_code = NULL;
	strcpy(tpvm_window->name, name);
	tpvm_window->first_frame = true;
	HASH_ADD_STR(tpvm_windows, name, tpvm_window);
	return tpvm_window;
}

TPVM_Window* find_window_by_name(const char *name){
	TPVM_Window* tpvm_window;

	HASH_FIND_STR(tpvm_windows, name, tpvm_window);

	return tpvm_window;
}

/** 
 * @brief Function to create a marker frame.
 *
 * This function takes a char array and encodes it into the marker_type marker.
 *
 * @param data A char array with the data that will be encoded into the marker.
 * @param type A enum of marker_type describing what marker gets displayed.
 * @param borderpxl The ammount of pixles that should get added to the marker as border as int.
 * @param scale The factor of how much the marker should get scaled up as int.
 * @param repeated A bool value if the marker should get repeated.
 * @param height A int value of the height of the window.
 * @param width A int value of the width of the window.
 * @return A struct marker_frame that returns all the needed information for tpvm.
 *
 * @note The uint32_t* marker_frame.data needs to be freed after use.
 *
 */
marker_frame create_marker(
    char* data,
    marker_type type,
    int borderpxl,
    int scale,
    bool repeated,
    int height,
    int width){

    uint32_t* marker;
    int* marker_width = malloc(sizeof(int));
    int* marker_height = malloc(sizeof(int));
    int image_size = 0;

    // Runs a function depending on selected Marker type
    switch (type){
        case MARKER_QR:
            create_qr_marker(data, &marker, marker_width, marker_height);
            break;
        case MARKER_DOT_CODE:
            create_dot_code_marker(data, &marker, marker_width, marker_height);
            break;
        case MARKER_ARUCO:
            create_aruco_marker(data, &marker, marker_width, marker_height);
            break;
    }

    image_size = create_repeated_marker(&marker, 
                                        marker_width, 
                                        marker_height,
                                        borderpxl,
                                        scale,
                                        repeated,
                                        height,
                                        width);

    // Put everything into a struct
    marker_frame frame;
    frame.memorySize = image_size;
    frame.width = *marker_width;
    frame.height = *marker_height;
    frame.data = malloc(image_size);
    memcpy(frame.data, marker, frame.memorySize);

    // Free the pointer memory
    free(marker);
    free(marker_width);
    free(marker_height);

    return frame;
}


char* get_id(){
    // check if fifo exists and create one if not
    if(!access(FIFO_PATH, F_OK) == 0){
        if(mkfifo(FIFO_PATH, 0777) == -1){
            perror("mkfifo");
            return 1;
        }
    }

    // read the data from the fifo
    char buffer[1024];
    int fd;

    fd = open(FIFO_PATH, O_NONBLOCK);
    if(fd == -1){
        perror("open");
        return 1;
    }

    ssize_t read_bytes = read(fd, buffer, sizeof(buffer) - 1);
    if (read_bytes > 0){
        buffer[read_bytes] = '\0';
        close(fd);
        return strncat(buffer, " - ", sizeof(buffer) - strlen(buffer) - 3);
    }
    if (read_bytes == 0){
        close(fd);
        return "0 - ";
    }

    return "0 - ";
}


marker_frame create_marker_with_mapping(
    char* windowName,
    marker_type type,
    int borderpxl,
    int scale,
    bool repeated,
    int height,
    int width){

        marker_frame frame;
        // At the beginning check if the marker frame already exists and is part of the hashmap
        TPVM_Window* tpvm_window = find_window_by_name(windowName);

        // If no entry found add a new entry
        if(tpvm_window == NULL){
            // Create a Payload for the Marker combining the window Name and the Computer ID 
            size_t stringSize = strlen(windowName) +  strlen(get_id()) + 1;
            char* payload = malloc(stringSize * sizeof(char));
            strcpy(payload, get_id());
            strcat(payload, windowName);

            // Generate the Marker
            frame = create_marker(payload, type, borderpxl,scale,repeated,height,width);

            // Create an entry in the hashmap and find that entry
            add_window(windowName);
            tpvm_window = find_window_by_name(windowName);

            // Save the frame data into the hashmap
			tpvm_window->qr_code = malloc(frame.memorySize);
			memcpy(tpvm_window->qr_code, frame.data, frame.memorySize);
			tpvm_window->size = frame.memorySize;
			tpvm_window->width = frame.width;
			tpvm_window->height = frame.height;
        }
        else{
            // Copy the found values into the frame 
            frame.data = malloc(tpvm_window->size);
            frame.memorySize = tpvm_window->size;
			frame.width = tpvm_window->width;
			frame.height = tpvm_window->height;
			memcpy(frame.data, tpvm_window->qr_code, tpvm_window->size);
        }

        // Invert the first frame variable to invert the image each iteration 
        bool first_frame = tpvm_window->first_frame;
		tpvm_window->first_frame = !first_frame;
        frame.negativeImage = first_frame;

        return frame;
    }


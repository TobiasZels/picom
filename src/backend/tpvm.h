// Created by Tobias Zels
#pragma once

#include <stddef.h>
#include <stdio.h>

// All the possible MarkerTypes implemented in the create_marker() function
typedef enum{
    MARKER_QR, //QR Code
    MARKER_DOT_CODE, // Dot Code
    MARKER_ARUCO // Not implemented Aruco Marker
} marker_type;

// A struct with all the Marker Frame info
typedef struct{
    int width;
    int height;
    uint32_t* data;
    int memorySize;
    bool negativeImage;
} marker_frame;

marker_frame create_marker(
    char* data,
    marker_type type,
    int borderpxl,
    int scale,
    bool repeated,
    int width,
    int height
);

marker_frame create_marker_with_mapping(
    char* windowName,
    marker_type type,
    int borderpxl,
    int scale,
    bool repeated,
    int height,
    int width);
/*
	TPVM Hashmap
*/
typedef struct{
	char name[50];
	bool first_frame;
	uint32_t* qr_code;
	int size;
	int width;
	int height;
	UT_hash_handle hh;
} TPVM_Window;

extern TPVM_Window* tpvm_windows;

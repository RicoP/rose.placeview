#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <algorithm>
#include <iostream>

#define CUTE_PNG_IMPLEMENTATION
#include "cute_png.h"


struct Place {
    long long timestamp_s;
    int timestamp_ms;
    int x;
    int y;
    int color;
};

std::vector<Place> places;

char * mfile;
char * mhead;

//read a single line from a FILE object
char * getline() {
    char * line = mhead;
    for(char * p = mhead; *p; p++) {
        if(*p == '\n') {
            *p = 0;
            mhead = p + 1;
            return line;
        }
    }
    return nullptr;
}

/*
Ccolor table 
0	#FFFFFF
1	#E4E4E4
2	#888888
3	#222222
4	#FFA7D1
5	#E50000
6	#E59500
7	#A06A42
8	#E5D900
9	#94E044
10	#02BE01
11	#00E5F0
12	#0083C7
13	#0000EA
14	#E04AFF
15	#820080
*/

static cp_pixel_t make_color(int hex) {
    cp_pixel_t color ;
    color.r = (uint8_t)((hex & 0xFF0000) >> 16);
    color.g = (uint8_t)((hex & 0x00FF00) >> 8);
    color.b = (uint8_t)(hex & 0x0000FF);
    color.a = 255;
    return color;
}

cp_pixel_t colors2017[] = {
    make_color(0xFFFFFF),
    make_color(0xE4E4E4),
    make_color(0x888888),
    make_color(0x222222),
    make_color(0xFFA7D1),
    make_color(0xE50000),
    make_color(0xE59500),
    make_color(0xA06A42),
    make_color(0xE5D900),
    make_color(0x94E044),
    make_color(0x02BE01),
    make_color(0x00E5F0),
    make_color(0x0083C7),
    make_color(0x0000EA),
    make_color(0xE04AFF),
    make_color(0x820080),
};

int main() {
    //read entire file into memory        
    FILE *f = fopen("place_tiles.csv", "rb");
    if (f == NULL) {
        printf("Error opening file!\n");
        exit(1);
    }
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);  /* same as rewind(f); */

    char *string = (char*)malloc(fsize + 1);
    fread(string, fsize, 1, f);
    fclose(f);
    string[fsize] = 0;
    mfile = string;
    mhead = mfile;

    printf("File was read.\n");

    //LINE FORMAT: ts,user_hash,x_coordinate,y_coordinate,color
    const char * timestamp;
    const char * user_hash;
    const char * xcoord;
    const char * ycoord;
    const char * color;

    size_t read;

    int MAX_LINES = 20;
    int line_count = 0;

    char * line;

    int width = 0;
    int height = 0;

    for (; line = getline(); line_count++) {
        //printf("Line %d: %s\n", line_count, line);
        if (line_count == 0) {
            continue;
        }

        //printf("Retrieved line of length %zu:\n", read);
        //printf("%s\n", line);
        char * str = line;
        //parse line
        timestamp = strtok(str, ",");
        user_hash = strtok(NULL, ",");
        xcoord = strtok(NULL, ",");
        ycoord = strtok(NULL, ",");
        color = strtok(NULL, ",");

        if(timestamp == NULL || user_hash == NULL || xcoord == NULL || ycoord == NULL || color == NULL) {
            continue;
        }

        //printf("%s;%s;%s;%s;%s\n", timestamp, user_hash, xcoord, ycoord, color);

        //Timestamp
        //2017-04-03 04:02:02.577 UTC
        //parse timestamp in style YYYY-MM-DD HH:MM:SS.SSS UTC using scanf
        int yyyy, MM, dd, HH, mm, ss, SSS;
        SSS = 0;
        int n = sscanf(timestamp, "%d-%d-%d %d:%d:%d.%d UTC", &yyyy, &MM, &dd, &HH, &mm, &ss, &SSS);
        if (n != 7 && n != 6) {
            printf("Error parsing timestamp! (%d)\n", n);
            printf("%s\n", timestamp);
            exit(1);
        }
        //printf("%d-%d-%d %d:%d:%d.%d UTC\n", yyyy, MM, dd, HH, mm, ss, SSS);
        long long timestamp_s = 
            (long long) (yyyy * 60 * 60 * 24) 
            + (long long) (MM * 60 * 60) 
            + (long long) (dd * 60) 
            + (long long) (HH) 
            + (long long) (mm) 
            + (long long) (ss) ;
        int timestamp_ms = SSS;

        Place place = {
            timestamp_s,
            timestamp_ms,
            atoi(xcoord),
            atoi(ycoord),
            atoi(color)
        };

        width = std::max(width, place.x);
        height = std::max(height, place.y);
        
        places.push_back(place);
    }

    free(string);

    std::sort(std::begin(places), std::end(places), [](const Place & a, const Place & b) {
        if(a.timestamp_s == b.timestamp_s)
            return a.timestamp_ms < b.timestamp_ms;
        return a.timestamp_s < b.timestamp_s;
    });

    auto min_time = places.front().timestamp_s;
    auto max_time = places.back().timestamp_s;

    printf("Dimension  %d,%d\n", width, height);
    printf("Timerange in seconds %lld\n", max_time - min_time);

    //create image
    cp_image_t image = cp_load_blank(width, height);
    for(int i = 0; i != width * height; ++i) {
        //make it all white
        image.pix[i] = colors2017[0];
    }
    
    int max_frames = 1000;

    Place * current_place = &(*places.begin());
    Place * end_place = &(*places.end());

    for(int frame = 0; frame != max_frames; ++frame) {
        auto i = frame * (places.size() / max_frames);
        if(i > places.size()) break;

        Place * next_place = &places[ i ];

        for(;current_place != next_place; ++current_place) {
            auto & place = *current_place;

            if(place.color >= 16) {
                printf("Invalid color %d\n", place.color);
                exit(1);
            }

            int x = place.x;
            int y = place.y;

            if(x >= width) continue;
            if(y >= height) continue;

            image.pix[x + y * width] = colors2017[place.color];
        }

        char path [100];
        sprintf(path, "render/frame_%05d.png", frame);
        cp_save_png(path, &image);
        printf("Saved frame %d from %d\n", frame, max_frames);

        /*
        for(auto & place : places) {
            printf("%lld,%d,%d,%d\n", place.timestamp_s, place.x, place.y, place.color);
            if (MAX_LINES-- == 0) {
                break;
            }
        }
        */
    }

    cp_free_png(&image);
    return 0;
}
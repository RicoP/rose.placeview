#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <algorithm>

struct Place {
    long long timestamp_s;
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

        Place place = {
            timestamp_s,
            atoi(xcoord),
            atoi(ycoord),
            atoi(color)
        };
        
        places.push_back(place);
    }

    free(string);

    std::sort(std::begin(places), std::end(places), [](const Place & a, const Place & b) {
        return a.timestamp_s < b.timestamp_s;
    });

    for(auto & place : places) {
        printf("%lld,%d,%d,%d\n", place.timestamp_s, place.x, place.y, place.color);
        if (MAX_LINES-- == 0) {
            break;
        }
    }

    return 0;
}
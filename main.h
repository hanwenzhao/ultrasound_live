//
// Created by hanwen on 1/7/20.
//

#ifndef ULTRASOUND_LIVE_MAIN_H
#define ULTRASOUND_LIVE_MAIN_H

#include <cstdio>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <unistd.h>
#include <cstdlib>
#include <vector>
#include <queue>
#include <cmath>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

// DEFINES
// -------
#define TCP_BUFFER_SIZE 5021
#define ADC_BUFFER_SIZE 2500
#define SCAN_DATA_SIZE 100
#define Cos(th) cos(M_PI/180*(th))
#define Sin(th) sin(M_PI/180*(th))

// STRUCTURE
// ---------
struct scan_data_struct{
    unsigned long time_stamp;
    unsigned short encoder;
    short buffer[ADC_BUFFER_SIZE];
};

struct screen_data_struct{
    double X;
    double Y;
    double Z;
    double I;
};

// VARIABLES
// ---------
int global_sockfd;
unsigned char time_stamp_char[4];
unsigned long time_stamp;
unsigned char probe_type_char;
unsigned char encoder_char[2];
unsigned short encoder;
unsigned char adc_char[2*2500];
unsigned char adc_temp[2];
unsigned char crc_char[4];
short buffer[2500];
int16_t adc;
uint32_t crc_result;
unsigned char crc_result_char[4];
unsigned char crc_input[4+1+2+2*2500];
unsigned char tcp_buffer[TCP_BUFFER_SIZE];
std::deque<scan_data_struct> scan_data;
std::vector<screen_data_struct> screen_data;
const unsigned char marker[10] = {0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01};
int16_t adc_max = 0;
int16_t adc_min = 0;
double intensity;

// FUNCTIONS
// ---------
unsigned long changed_endian_4Bytes(unsigned long num);
int16_t changed_endian_2Bytes(int16_t value);
int compare_crc(unsigned char a[], unsigned char b[], size_t len);
uint32_t crc32c(uint32_t crc, const unsigned char *buf, size_t len);
void buffer_to_data(std::vector<unsigned char> buffer_vector);
double convert_angle_2d_probe(double angle);
double map_range_to_range(double input_start, double input_end, double output_start, double output_end, double input);
void scan_to_screen();

// OPENGL VARIABLES

#define LEN 8192  //  Maximum length of text string

#define Cos(th) cos(M_PI/180*(th))
#define Sin(th) sin(M_PI/180*(th))

int th=0;       // Azimuth of view angle
int ph=0;       // Elevation of view angle
double z=0;     // Z variable
double dim=2;   // Dimension of orthogonal box
int mode=3;

static GLint Frames = 0;
static GLfloat fps = -1;
static GLint T0 = 0;

GLfloat worldRotation[16] = {1,0,0,0,0,0,1,0,0,1,0,0,0,0,0,1};

void display();
void idle();
void Print(const char* format , ...);

#endif //ULTRASOUND_LIVE_MAIN_H

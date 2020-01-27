#include "main.h"

int main(int argc, char *argv[])
{
    // initialize socket
    // -----------------
    int sockfd, newsockfd, portno, clilen, n;
    struct sockaddr_in serv_addr, cli_addr;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) printf("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = 51717;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    // bind and accept socket connection
    // ---------------------------------
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) printf("ERROR on binding");
    listen(sockfd,5);
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, (socklen_t*) &clilen);
    if (newsockfd < 0) printf("ERROR on accept");
    global_sockfd = newsockfd;

    glutInit(&argc, argv);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(1300, 1000);
    glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
    glutCreateWindow("ultrasound");
    glutDisplayFunc(display);
    glutIdleFunc(idle);
    gluOrtho2D((GLdouble) -1250, (GLdouble) 1250, (GLdouble) -2500, (GLdouble) 100);
    glutMainLoop();
    return 0;
}

void display(){
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set background color to black and opaque
    glClear(GL_COLOR_BUFFER_BIT);         // Clear the color buffer (background)
    glPointSize(1);

    Frames++;
    GLint t = glutGet(GLUT_ELAPSED_TIME);
    if (t - T0 >= 5000) {
        GLfloat seconds = (t - T0) / 1000.0;
        fps = Frames / seconds;
        printf("%d frames in %6.3f seconds = %6.3f FPS\n", Frames, seconds, fps);
        T0 = t;
        Frames = 0;
    }
    glColor3f(1,1,1);
    glWindowPos2i(5,5);
    if (fps>0) Print("FPS %.3f", fps);
    glWindowPos2i(5,450);

    /* draw all */
    //printf("Number of Screen Data Points: %lu\n", screen_data.size());
    glBegin(GL_POINTS);
    for (int i = 0; i < (int)screen_data.size(); i++){
        double intensity = screen_data.at(i).I * 1.5;
        glColor3f(intensity, intensity, intensity);
        glVertex2d(screen_data.at(i).X,screen_data.at(i).Y);
        //printf("X: %f, Y: %f, I: %f\n", screen_data.at(i).X, screen_data.at(i).Y, screen_data.at(i).I);
    }
    glEnd();
    glFlush();
    glutSwapBuffers();
}

void scan_to_screen(){
    screen_data.clear();
    //printf("%d\n", (int)_scan_data.size());
    for (int i = 0; i < (int)scan_data.size(); i++){
        double angle = scan_data.at(i).encoder * 360.0 / 4096.0;
        angle = convert_angle_2d_probe(angle);
        /* find min and max */
        for (int j = 0; j < ADC_BUFFER_SIZE; j++){
            adc_max = std::max(adc_max, scan_data.at(i).buffer[j]);
            adc_min = std::min(adc_min, scan_data.at(i).buffer[j]);
        }
        /* normalize on the go */
        for (int j = 0; j < ADC_BUFFER_SIZE; j++){
            intensity = ((double)scan_data.at(i).buffer[j] - adc_min)/(adc_max-adc_min);
            screen_data_struct temp_data = {(j+1) * Cos(angle), (j+1) * Sin(angle), 0, intensity};
            screen_data.push_back(temp_data);
        }
        adc_max = 0; adc_min = 0;
    }
}

void idle(){
    // read socket into buffer
    // -----------------------
    bzero(tcp_buffer, TCP_BUFFER_SIZE); // reset buffer memory
    for (int received_bytes = 0; received_bytes < TCP_BUFFER_SIZE;){
        int result = recv(global_sockfd, tcp_buffer+received_bytes, TCP_BUFFER_SIZE-received_bytes, 0);
        if (result > 0){
            received_bytes += result;
            //printf("Bytes received: %d\n", result);
        } else if (result == 0){
            printf("Connection Closed\n");
            break;
        }
    }

    // convert char array into vector
    // ------------------------------
    std::vector<unsigned char> buffer_vector(std::begin(tcp_buffer), std::end(tcp_buffer));
    //printf("Converted Vector Size: %lu\n", buffer_vector.size());

    // print first 20 bytes in buffer vector
    // -------------------------------------
    /*
    printf("First 20 Bytes from Buffer Vector:\n");
    for (int i = 0; i < 20; i++) {
        printf("%02X ", buffer_vector.at(i));
    }
    printf("\n");
     */
    // convert buffer to scan data and compare crc checksum
    // ----------------------------------------------------
    buffer_to_data(buffer_vector);
    //printf("Scan Data Vector Size: %lu.\n", scan_data.size());

    // convert scan data to screen data
    scan_to_screen();

    glutPostRedisplay();
}

double map_range_to_range(double input_start, double input_end, double output_start, double output_end, double input){
    return output_start + ((output_end - output_start) / (input_end - input_start)) * (input - input_start);
}
double convert_angle_2d_probe(double angle){
    double left = 358; double right = 181; double top = 97; double bottom = 278;
    if ((angle >= right) && (angle < bottom)){
        return map_range_to_range(right, bottom, -110, -90, angle);
    }
    else if ((angle >= top) && (angle < right)){
        return map_range_to_range(top, right, -90, -110, angle);
    }
    else if ((angle >= left) || (angle < top)){
        if ((angle <= top)){angle = angle+360;}
        return map_range_to_range(left, top+360, -70, -90, angle);
    }
    else if ((angle >= bottom) && (angle <= left)){
        return map_range_to_range(left, bottom, -70, -90, angle);
    }
}
void Print(const char* format , ...){
    char    buf[LEN];
    char*   ch=buf;
    va_list args;
    //  Turn the parameters into a character string
    va_start(args,format);
    vsnprintf(buf,LEN,format,args);
    va_end(args);
    //  Display the characters one at a time at the current raster position
    while (*ch)
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24,*ch++);
}
unsigned long changed_endian_4Bytes(unsigned long num){
    int byte0, byte1, byte2, byte3;
    byte0 = (num & 0x000000FF) >> 0 ;
    byte1 = (num & 0x0000FF00) >> 8 ;
    byte2 = (num & 0x00FF0000) >> 16 ;
    byte3 = (num & 0xFF000000) >> 24 ;
    return((byte0 << 24) | (byte1 << 16) | (byte2 << 8) | (byte3 << 0));
}
int16_t changed_endian_2Bytes(int16_t value){
    return ((value >> 8) & 0x00ff) | ((value & 0x00ff) << 8);
}
int compare_crc(unsigned char a[], unsigned char b[], size_t len){
    int ii;
    for (ii = 0; ii < (int)len; ii++){
        if (a[ii] != b[ii]){
            return 0;
        }
    }
    return 1;
}
uint32_t crc32c(uint32_t crc, const unsigned char *buf, size_t len)
{
    int k;

    crc = ~crc;
    while (len--) {
        crc ^= *buf++;
        for (k = 0; k < 8; k++)
            crc = crc & 1 ? (crc >> 1) ^ 0xedb88320 : crc >> 1;
    }
    return ~crc;
}
void buffer_to_data(std::vector<unsigned char> buffer_vector){
    int start_marker = 0;
    int end_marker = TCP_BUFFER_SIZE;
    /* time stamp */
    for (int j = 0; j < (int)sizeof(time_stamp_char); j++){
        time_stamp_char[j] = buffer_vector.at(start_marker + sizeof(marker) + j);
    }
    memcpy(&time_stamp, time_stamp_char, sizeof(time_stamp));
    time_stamp = changed_endian_4Bytes(time_stamp);
    /* probe type char */
    probe_type_char = buffer_vector.at(start_marker + sizeof(marker) + sizeof(time_stamp_char));
    /* encoder */
    for (int j = 0; j < (int) sizeof(encoder_char); j++){
        encoder_char[j] = buffer_vector.at(start_marker + sizeof(marker) + sizeof(time_stamp_char) + sizeof(probe_type_char) + j);
    }
    memcpy(&encoder, encoder_char, sizeof(encoder));
    encoder = changed_endian_2Bytes(encoder);
    /* adc */
    /* determine the length of buffer */
    int buffer_length = (int)(end_marker - start_marker - sizeof(marker) - sizeof(time_stamp_char) -
                              sizeof(probe_type_char) - sizeof(encoder_char) - sizeof(crc_char))/2;
    for (int j = 0; j < buffer_length; j++){
        for (int k = 0; k < (int)sizeof(adc_temp); k++){
            adc_temp[k] = buffer_vector.at(start_marker + sizeof(marker) + sizeof(time_stamp_char) + sizeof(probe_type_char) + sizeof(encoder_char) + j * 2 + k);
            adc_char[2*j+k] = adc_temp[k];
        }
        memcpy(&adc, adc_temp, sizeof(adc));
        adc = changed_endian_2Bytes(adc);
        buffer[j] = adc;
    }
    /* checksum */
    for (int j = 0; j < (int)sizeof(crc_char); j++){
        crc_char[j] = buffer_vector.at(end_marker-(int)sizeof(crc_char)+j);
    }
    /* calculate crc locally */
    memcpy(crc_input, time_stamp_char, sizeof(time_stamp_char));
    memcpy(crc_input+sizeof(time_stamp_char), &probe_type_char, sizeof(probe_type_char));
    memcpy(crc_input+sizeof(time_stamp_char)+sizeof(probe_type_char), encoder_char, sizeof(encoder_char));
    memcpy(crc_input+sizeof(time_stamp_char)+sizeof(probe_type_char)+sizeof(encoder_char), adc_char, sizeof(adc_char));
    crc_result = crc32c(0, crc_input, sizeof(crc_input));
    crc_result = changed_endian_4Bytes(crc_result);
    memcpy(crc_result_char, (unsigned char *)&crc_result, sizeof (crc_result));
    /* if two crc matches */
    if (compare_crc(crc_char, crc_result_char, sizeof(crc_char))){
        scan_data_struct temp_struct;
        temp_struct.time_stamp = time_stamp;
        temp_struct.encoder = encoder;
        /* normalize on the go */
        for (int j = 0; j < buffer_length; j++) {
            temp_struct.buffer[j] = buffer[j];
        }
        if (scan_data.size() >= SCAN_DATA_SIZE){
            scan_data.pop_front();
        }
        scan_data.push_back(temp_struct);
    }

}
#include "TLM_management.h"
#include "hcc/api_fat.h"
#include "hcc/api_hcc_mem.h"
#include "hcc/api_mdriver_atmel_mcipdc.h"
#include <stdbool.h>

#define _SD_CARD 0 // used in demo, might not be correct


// helper functions//
void openAllFiles(){
    c_fileCreate(END_FILENAME_EPS_TLM);
    c_fileCreate(END_FILE_NAME_TX);
    c_fileCreate(END_FILE_NAME_ANTENNA);
    c_fileCreate(END_FILENAME_SOLAR_PANELS_TLM);
    c_fileCreate(END_FILENAME_WOD_TLM);
    c_fileCreate(END_FILE_NAME_RX);
    c_fileCreate(END_FILE_NAME_RX_FRAME);
    c_fileCreate(END_FILENAME_LOGS);

}
void delete_allTMFilesFromSD(){
    f_delete(END_FILENAME_EPS_TLM);
    f_delete(END_FILE_NAME_TX);
    f_delete(END_FILE_NAME_ANTENNA);
    f_delete(END_FILENAME_SOLAR_PANELS_TLM);
    f_delete(END_FILENAME_WOD_TLM);
    f_delete(END_FILE_NAME_RX);
    f_delete(END_FILE_NAME_RX_FRAME);
    f_delete(END_FILENAME_LOGS);
}

/*
* @brief an initialization of the file system. 
*/
FileSystemResult InitializeFS(){
    hcc_mem_init();
    unsigned int error = fs_init();
    if(error != F_NO_ERROR){
        printf("error %d found during file system init\n",error);
        return error;
    }
    error = f_enterFS();
    if(error){
        //printf("error %d found during file system enter\n",error);
        return error;
    }

    error = f_initvolume(0, atmel_mcipdc_initfunc , _SD_CARD);
    
    if(error == F_ERR_NOTFORMATTED){
       //printf("format error %d found while file system volume initialization\n",error);
        return error;
    }
    else if(error != F_NO_ERROR){
        return error;
    }
   //printf("error %d found while file system volume initialization\n",error);
    openAllFiles();
    return error;
}

void DeInitializeFS(int sd_card){
	return;
}

int getSize(void* data){
	//TODO:find size
	return 1;
}


int write2File(void* data, tlm_type_t tlmType){
	//TODO: add fail state returns.
    int dataSize = getSize(data);
    switch(tlmType){
        case tlm_eps:
            c_fileWrite(END_FILENAME_EPS_TLM, data,dataSize);
            break;
        case tlm_tx:
            c_fileWrite(END_FILE_NAME_TX, data,dataSize);
            break;
	    case tlm_antenna:
            c_fileWrite(END_FILE_NAME_ANTENNA, data,dataSize);
            break;
	    case tlm_solar:
            c_fileWrite(END_FILENAME_SOLAR_PANELS_TLM, data,dataSize);
            break;
	    case tlm_wod:
            c_fileWrite(END_FILENAME_WOD_TLM, data,dataSize);
            break;
        case tlm_rx:
            c_fileWrite(END_FILE_NAME_RX, data,dataSize);
            break;
        case tlm_rx_frame:
            c_fileWrite(END_FILE_NAME_RX_FRAME, data,dataSize);
            break;
        case tlm_log:
            c_fileWrite(END_FILENAME_LOGS, data,dataSize);
            break;
        default:
        	dataSize = 0;
        	//printf("used TLM not listed\n");
    }
    return 0;
}
/*
*@brief: simply open and close a file with name c_file_name
*/
FileSystemResult c_fileCreate(char* c_file_name){
    if(MAX_FILE_NAME_SIZE < strlen(c_file_name)){
        printf("file name %s too long\n", c_file_name);
        return FS_TOO_LONG_NAME;
    }
    F_FILE* file;
    file = f_open(c_file_name,"w");
    if(file == NULL){
        return FS_FAIL;
    }
    else{
        f_close(file);
        return FS_SUCCSESS;
    }
}
/*
*@brief: find c_file_name, go to end, and write data. include time and TLM information in each line.
*/
FileSystemResult c_fileWrite(char* c_file_name, void* element, int size_of_element){

    char BUf[100];
    long int time = xTaskGetTickCount() * (1/configTICK_RATE_HZ) * 1000; // time in ms

    //sprintf(BUf,"%ld|%s\n",time,(char *)element); // format:  time "\0" TLM "\n"
    F_FILE* file = f_open(c_file_name,"a");
    if(file != NULL){

        if(f_write(BUf,1,50,file)==50){ 
            f_close(file);
            return FS_SUCCSESS;
        }
        else{
            f_close(file);
            return FS_FAIL;
        }
    }
    return FS_FAIL;
}



time_unix getNextTime(FN_FILE* f_handle){
    char c;
    int i;
    char longInt[50];
    c= f_getc(f_handle);
    i=0;
    for(; c != '|' && !f_eof(f_handle); i++,c= f_getc(f_handle)){
        longInt[i] = c;
    }
    if(f_eof(f_handle)){
        return -1;
    }
    longInt[i] = '\0';
    return atol(longInt);
}

void GoToNextLine(FN_FILE* f_handle){
    while(!f_eof(f_handle) && f_getc(f_handle) != '\n'){
    }
}

FileSystemResult c_fileDeleteElements(char* c_file_name, time_unix from_time, time_unix to_time){
    time_unix time;

    F_FILE* f_handle = f_open(c_file_name,"r+");
    F_FILE* f_handle_end;

    unsigned long startPos , endPos;
    bool startFlag = true, endFlag = true; 
    if(f_handle == NULL){
        //printf("failed to open file\n");
        return -1;
    }
    while(!f_eof(f_handle)){
       
        time = getNextTime(f_handle);

        GoToNextLine(f_handle);

        if(from_time <= time && startFlag){
            startPos = f_tell(f_handle);
            startFlag = false;
        }
        if(to_time <= time   && endFlag){
            endPos = f_tell(f_handle);
            endFlag = false;
        }
        if(!endFlag && !startFlag){
            break;
        }

        
    }
    if(startFlag){
        return FS_SUCCSESS ;
    }
    else{
        f_seek(f_handle,startPos,FN_SEEK_SET);
    }

    if(endFlag){
        f_handle_end = NULL;
    }
    else{
        f_handle_end = f_open(c_file_name,"r");
        if(f_handle_end == NULL){
            //printf("failed to delete, file did not open\n");
        }

        f_seek(f_handle_end , endPos , FN_SEEK_SET);
    }

    while( f_handle_end != NULL && !f_eof(f_handle_end) ){
        f_putc(f_getc(f_handle_end),f_handle);
    }
    f_truncate(f_handle->reference,f_tell(f_handle));
    f_close(f_handle);
    f_close(f_handle_end);
    return FS_SUCCSESS;
}

int c_fileGetNumOfElements(char* c_file_name,time_unix from_time
		,time_unix to_time){

    time_unix time;
    int count = 0;
    F_FILE* f_handle = f_open(c_file_name,"r");
    if(f_handle == NULL){
        //printf("failed to open file\n");
        return -1;
    }
    while(!f_eof(f_handle)){
        
        time = getNextTime(f_handle);

        if(from_time <= time && time <= to_time){
            count++;
        }

        GoToNextLine(f_handle);

    }
    return count;
}

FileSystemResult c_fileRead(char* c_file_name, byte* buffer, int size_of_buffer,
		time_unix from_time, time_unix to_time, int* read, time_unix* last_read_time){

    F_FILE* f_handle = f_open(c_file_name,"r");
    int i = 0;
    time_unix time;
    do{
        time = getNextTime(f_handle);
        if(from_time <= time){
            for(; i <= size_of_buffer-1 && !f_eof(f_handle) ; i++){
                buffer[i] = f_getc(f_handle);
                if(buffer[i] == '\n'){
                    break;
                }
            }
            if(i == size_of_buffer){
                return FS_BUFFER_OVERFLOW;
            }
            buffer[i] = '|';
            i++;
        }
        else{
            GoToNextLine(f_handle);
        }
    }
    while(!f_eof(f_handle) && time < to_time);
    f_close (f_handle);
    buffer[i] = '\0';
    return 0;
}

void print_file(char* c_file_name){
    F_FILE* f_handle = f_open(c_file_name,"r");
    char buf[501]; // say all lines are shorter then 500 chars
    char c;
    int i ;
    while(!f_eof(f_handle)){
        i=0;
        for(c = f_getc(f_handle) ; c != '\n' && i < 500;  c = f_getc(f_handle)){
            buf[i++] = c;
        }
        buf[i] = '\0';
        //printf("%s\n",buf);
    }
}

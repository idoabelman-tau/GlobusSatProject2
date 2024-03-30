#include "TLM_management.h"
#include "hcc/api_fat.h"
#include "hcc/api_hcc_mem.h"
#include "hcc/api_mdriver_atmel_mcipdc.h"


#define _SD_CARD 0 // used in demo, might not be correct

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
        printf("error %d found during file system enter\n",error);
        return error;
    }

    error = f_initvolume(0, atmel_mcipdc_initfunc , _SD_CARD);
    
    if(error == F_ERR_NOTFORMATTED){
        printf("format error %d found while file system volume initialization\n",error);
        return error;
    }
    else if(error != F_NO_ERROR){
        return error;
    }
    printf("error %d found while file system volume initialization\n",error);
    return error;
}

void DeInitializeFS(int sd_card){

}
/*
*@brief: simply open and close a file with name c_file_name
*/
FileSystemResult c_fileCreate(char* c_file_name){
    if(MAX_FILE_NAME_SIZE < strlen(c_file_name)){
        printf("file name %s too long\n", c_file_name);
        return FS_TOO_LONG_NAME;
    }
    F_FILE file;
    file = f_open(c_file_name,'w');
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

    char* BUf[50];
    long int time = xTaskGetTickCount() * (1/configTICK_RATE_HZ) * 1000; // time in ms

    sprintf(BUf,"%llu\0%s\n",time,element); // format:  time "\0" TLM "\n"
    F_FILE file = f_open(c_file_name,'w');
    if(file != NULL){

        while(!f_eof(file)){ //find end of file
            f_getc(file);
        }

        if(
            f_write(buf,1,50,file)==50){ 
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

FileSystemResult c_fileDeleteElements(char* c_file_name, time_unix from_time, time_unix to_time){
    F_FILE file = f_open(c_file_name,'w');
    if(file != NULL){

        while(!f_eof(file)){ //find end of file
            f_getc(file);
        }
    

    }
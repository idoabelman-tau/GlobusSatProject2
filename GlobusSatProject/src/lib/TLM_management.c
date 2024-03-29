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

FileSystemResult c_fileCreate(char* c_file_name, int size_of_element){
    f_open(c_file_name,)
}
#include "TLM_management.h"
#include "hcc/api_fat.h"
#include "hcc/api_hcc_mem.h"
#include "hcc/api_mdriver_atmel_mcipdc.h"
#include "string.h"
#include "hal/Utility/Util.h"
#include <stdbool.h>

#define TEST

#define _SD_CARD 0 // used in demo, might not be correct

#define MAX_FILE_SIZE 100

#define NUMBER_OF_LOG_TYPES 14
#define LOG_FILE_LINE_SIZE sizeof(logElement)
//#define DATA_FILE_LINE_SIZE 58
#define DATA_TIME_STAMP_SIZE 5
#define DATA_SIZE 50

#define DIR_NAME_EPS_TLM "DI/EPS"
#define DIR_NAME_TX "DI/TX"
#define DIR_NAME_ANTENNA "DI/ATN"
#define DIR_NAME_SOLAR_PANELS_TLM "DI/SP"
#define DIR_NAME_WOD_TLM "DI/WOD"
#define DIR_NAME_RX "DI/RX"
#define DIR_NAME_RX_FRAME "DI/RX_FR"
#define DIR_NAME_LOGS "DI/LOGS"


char inDirPath[100];
char* dirNames[NUMBER_OF_LOG_TYPES] = {DIR_NAME_EPS_TLM,DIR_NAME_TX, DIR_NAME_ANTENNA,
		DIR_NAME_SOLAR_PANELS_TLM,DIR_NAME_WOD_TLM,"",DIR_NAME_RX,"",DIR_NAME_RX_FRAME,
		"","","","",DIR_NAME_LOGS};


tlm_type_t tlm_global = tlm_eps;
#define DATA_FILE_LINE_SIZE (sizeByTlm[tlm_global]+ sizeof(int))



typedef union _logElement{
	int fields[4];
	char raw[20];
} logElement;

const int sizeByTlm[NUMBER_OF_LOG_TYPES+1] = {22,0,0,0,0,0,0,0,0,0,0,0,0,0, sizeof(logElement)};

void hex_print(unsigned char* text , int length){
	for(int i = 0 ; i < length ; i++){
		printf("%02x ",text[i]);
	}
	printf("\n\r");
}
void WipeDirectory(char* folder);
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
   int year = 2024;
   int month = 7;
   int day = 13;
   sprintf(inDirPath,"%d/%d/%d",year,month,day);
   new_Day_Dir();
   return error;
}
void DeInitializeFS(int sd_card){
	return;
}

void WipeDirectory(char* folder);
void  new_Day_Dir(){
    char name[100];
    int i, err, j;
    char c = '0';
    for(i = 0 ; i < NUMBER_OF_LOG_TYPES ; i++){
        if(strcmp("",dirNames[i])){
            //c = '!';
            sprintf(name,"%s/%s",dirNames[i],inDirPath);

            for(j = 0 ;c != '\0'; j++){
                c = name[j];
                if(c == '/'){
                    name[j] = '\0';
                    err += f_mkdir(name);
                    name[j] = c;
                }
            }
            c = '0';
            err += f_mkdir(name);
            if(err){
            	printf("found error in new Dayy_Dir- error: %d",err);
            }
            WipeDirectory(name);
            sprintf(name,"%s/%s",name,"log");

            //Log(name);
            F_FILE* hold = f_open(name,"w");

            if(hold == NULL){
                //TODO:add to error log
                printf("failed to open default directory\n");
            }
            else{
            	logElement le;
            	for(int i = 0 ; i < 4 ; i++){
            		le.fields[i] = 0;
            	}
            	f_write(le.raw,sizeof(logElement),1,hold);
                f_close(hold);
            }
        }
    }
}

void getFileName(char* res , char* sys, int cur_time){
	logElement le;
    F_FILE* handle;
    char Log[120];

    sprintf(Log,"%s/%s/log",sys,inDirPath);
    handle = f_open(Log,"r+");
    if(handle == NULL){
        //TODO: LOG and Fail
    	printf("failed in finding file in function getFileName\n");
    }

    f_seek(handle,-sizeof(logElement),F_SEEK_END);

    if(f_read(le.raw,sizeof(logElement),1,handle)!=1){
        //fail to read size of current size of file
    	printf("failed in reading of log getFileName\n");
    }

    if(le.fields[0] == MAX_FILE_SIZE-1){
    	le.fields[0]=1;
    	le.fields[1]++;
    	le.fields[2] = cur_time;
    	le.fields[3] = cur_time;
        f_seek(handle,0,F_SEEK_END);
        f_write(le.raw,1,sizeof(logElement),handle);

    }
    else{

    	le.fields[0]++;
    	le.fields[2] = (le.fields[2] == 0 ? cur_time: le.fields[2]); // TODO: fix bug in this line
    	le.fields[3] = cur_time;
    	f_seek(handle,-sizeof(logElement),F_SEEK_CUR);
    	f_write(le.raw,1,sizeof(logElement),handle);
    }
    f_close(handle);
    sprintf(res,"%s/%s/%d",sys,inDirPath,le.fields[1]);
}

int day_seconds() {
	Time time;
	Time_get(&time);
    return (time.seconds+60*time.minutes+3600*time.hours)%86400;
}

void WriteData(tlm_type_t tlm  ,unsigned char* data){
    char filename[120];
    int time_seconds = day_seconds() ;

#ifdef TEST
    //printf("time stamp of entered data: %d\n", time_seconds);
#endif
    getFileName(filename,dirNames[tlm],time_seconds);
    F_FILE* file_handle = f_open(filename,"a");
    if(file_handle == NULL){
        printf("handle is NULL\n");
    }
    else{
        f_write(&time_seconds,sizeof(int),1 , file_handle);
        //printf("data being written:\r\n");
        //hex_print(data,sizeByTlm[tlm_global]);
        f_write(data,1,sizeByTlm[tlm_global] , file_handle);
        f_close(file_handle);
        }
}

void copyFile(F_FILE* copyFrom, F_FILE* copyTo, int numberOfEntries){
    char* buf= malloc(sizeByTlm[tlm_global]);
    if(buf == NULL){
    	printf("failed to allocate memory for file copy- TLM_mamagement -> copyFile\r\n");
    }
    long backup1 = f_tell(copyFrom);
    long backup2 = f_tell(copyTo);
    if(numberOfEntries != -1){
        for(int i = 0 ; i < numberOfEntries; i++){
        	if(f_read(buf,1,sizeByTlm[tlm_global],copyFrom) == sizeByTlm[tlm_global]){
        		f_write(buf,1,sizeByTlm[tlm_global],copyTo);
        	}
        	else{
        		printf("copy line failed in TLM_management -> copyFile\r\n");
        	}
        }
    }
    else{
        while(f_read(buf,1,sizeByTlm[tlm_global],copyFrom) == sizeByTlm[tlm_global]){
        	f_write(buf,1,sizeByTlm[tlm_global],copyTo);
        }
    }
    f_seek(copyFrom,backup1,F_SEEK_SET);
    f_seek(copyTo,backup2,F_SEEK_SET);
    f_flush(copyTo);
    free(buf);
}

// data file functions ////////////
// reads time stamp of data file, returns time if positive and -1 on fail.
int get_Time_From_Data(F_FILE* data, int line){
    long backup = f_tell(data);
    int res;
    f_seek(data,line*DATA_FILE_LINE_SIZE,F_SEEK_SET);
    if(f_read(&res,sizeof(int),1,data) != 1){
    	res = -1;
    }
    f_seek(data,backup,F_SEEK_SET);
    return res;
}
//returns number of entries still remaining
int copy_Data_Prefix(F_FILE* doc,F_FILE* copyTo, int upToBound){
     int cnt = 0;
     int timeStamp = 0;
     for(int i = 0 ; i < MAX_FILE_SIZE;i++){
    	 timeStamp = get_Time_From_Data(doc,i);
    	 if(timeStamp!=-1 && timeStamp<upToBound){
    		 cnt++;
    	 }
    	 else{
    		 break;
    	 }
     }

    if(cnt!=0){
        f_rewind(doc);
        copyFile(doc,copyTo,cnt);
    }
    return cnt;
}
//returns number of entries still remaining
int copy_Data_Suffix(F_FILE* doc, F_FILE* copyTo ,int fromBound){
    int cnt = 0, timeStamp = 0,i;
    for(i = 0 ; i < MAX_FILE_SIZE;i++){
        timeStamp = get_Time_From_Data(doc,i);
        if(timeStamp!=-1){
        	if(fromBound < timeStamp){
        		cnt++;
        		break;
        	}
        }
        else{
        	break;
        	 }
         }

    if(cnt){
        f_seek(copyTo,0,F_SEEK_END);
        f_seek(doc,i*DATA_FILE_LINE_SIZE,F_SEEK_SET);
        copyFile(doc,copyTo,-1);
    }
    f_seek(copyTo,0,F_SEEK_END);
    return f_tell(copyTo)/DATA_FILE_LINE_SIZE;
}


//////////////////////////////////
/// Log functions ////////////////////
void get_log_element(F_FILE* log,logElement* le_P, int line){
	long backup = f_tell(log);
	f_seek(log,line*sizeof(logElement),F_SEEK_SET);
	f_read(le_P,sizeof(logElement),1,log);
	f_seek(log,backup,F_SEEK_SET);
	return;
}
void update_log_element(F_FILE* log,logElement le, int line){
	long backup = f_tell(log);
	f_seek(log,line*sizeof(logElement),F_SEEK_SET);
	f_write(le.raw,sizeof(logElement),1,log);
	f_seek(log,backup,F_SEEK_SET);
	return;
}


int find_file_from_log(F_FILE* log, int numberOfEntries, int time, bool max, bool* inFile){
    int line = numberOfEntries/2;
    int lowerBound,upperBound;
    logElement le;
    *inFile = false;

    for(int i = 1 ; numberOfEntries>>(i-1); i++){
    	get_log_element(log,&le,line);
        lowerBound = le.fields[2];
        upperBound = le.fields[3];

        if(time<lowerBound){
            if(line == 0){
                return 0;
            }
            get_log_element(log,&le,line-1);
            upperBound = le.fields[3];
            if(upperBound<time){
                if(!max){
                    return line ;
                }
                else{
                    return line-1;
                }
            }
            line = line - (1 + (numberOfEntries-1)/(2<<i));
        }
        else if(upperBound <time){
            if(line == numberOfEntries-1){
                return numberOfEntries-1;
            }
            get_log_element(log,&le,line+1);
            lowerBound = le.fields[2];
            if(time < lowerBound){
                if(max){
                    return line;
                }
                else{
                    return line+1;
                }
            }
            line = line + (1 + (numberOfEntries-1)/(2<<i));

        }
        else{
            if(!max && time == lowerBound){
                while( 0 < line){
                	get_log_element(log,&le,line-1);
                	upperBound = le.fields[3];
                    if(time == upperBound){
                        line--;
                    }
                    else{
                        break;
                    }
                }

            }
            else if(max && time == upperBound){
                while( line<numberOfEntries-1){
                	get_log_element(log,&le,line+1);
                	lowerBound = le.fields[2];
                    if(time == lowerBound){
                        line++;
                    }
                    else{
                        break;
                    }
                }
            }
            break;
        }
       }

    *inFile = true;
    return line;
}



///////////////////////////////////////////////////////
void deleteData(tlm_type_t tlm,int timeBound[2]){
	tlm_global = tlm;
	char* sys = dirNames[tlm];
    int numOfLines;
    int lineBounds[2]; // holds the lines in log where the correct data file names are at.
    int  cnt = 0, log_update_collector[5];
    char log_name[125],log_copy_name[125],data_copy_name[125],data_name[125],path[125];
    bool startSkip = false, endSkip = false;
    logElement le;

    sprintf(path,"%s/%s",sys,inDirPath);
    sprintf(log_name ,"%s/log", path);

    //search in log:
    F_FILE* data_handle;
    F_FILE* data_temp;
    F_FILE* log_temp;
    F_FILE* log_handle = f_open(log_name,"r+");
    if(log_handle == NULL){
        //fail
        exit(0);
    }
    //size of file
    f_seek(log_handle, 0L, F_SEEK_END);
    numOfLines = f_tell(log_handle)/LOG_FILE_LINE_SIZE; // might include bug

    lineBounds[0] = find_file_from_log(log_handle,numOfLines,timeBound[0],false,&startSkip);
    lineBounds[1] = find_file_from_log(log_handle,numOfLines,timeBound[1],true,&endSkip);

    startSkip = !startSkip;
    endSkip = !endSkip;
    if(startSkip){
        lineBounds[0]--;
    }
    if(endSkip){
        lineBounds[1]++;
    }

    // start deleting

    if(lineBounds[0]<lineBounds[1]){
        // update start of log by copying into temp Log file (starting section where entries need to be deleted)
    	sprintf(data_copy_name,"%s/temp",path);
        if(!startSkip){
            //if we need to delete from multiple files.
            // copy start of first file(res[0]), section that is not being deleted .

        	get_log_element(log_handle,&le,lineBounds[0]);

            sprintf(data_name,"%s/%d",path,le.fields[1]);
            data_temp = f_open(data_copy_name,"w+");
            data_handle = f_open(data_name,"r");
            if(data_handle==NULL){
                printf("unable to open file 2\n");
            }
            if(data_temp==NULL){
                printf("unable to open file 3\n");
            }
            cnt = copy_Data_Prefix(data_handle,data_temp,timeBound[0]);

            log_update_collector[0] = cnt;
            log_update_collector[1] = get_Time_From_Data(data_handle,0);
            log_update_collector[4] = lineBounds[0];
            f_close(data_temp);
            f_close(data_handle);
            f_delete(data_name);
            if(cnt == 0){
                f_delete(data_copy_name);
            }
            else{
                f_rename(data_copy_name,data_name);
            }

        }
            // first file in deletion chain is done.



        // delete all files between the two edges- lineBounds[0] and lineBounds[1]
        lineBounds[0]++;
        while(lineBounds[0] < lineBounds[1]){
        	get_log_element(log_handle,&le,lineBounds[0]);
            sprintf(data_name,"%s/%d",path,le.fields[1]);
            f_delete(data_name);
            lineBounds[0]++;
        }



        // copy end of last file(lineBounds[1]), section that is not being deleted .
        if(!endSkip){

            get_log_element(log_handle,&le,lineBounds[1]);
            sprintf(data_name,"%s/%d",path,le.fields[1]);
            data_temp = f_open(data_copy_name,"w+");
            data_handle = f_open(data_name,"r");
            if(data_handle==NULL){
                printf("unable to open file\n");
            }
            if(data_temp==NULL){
                printf("unable to open file\n");
            }


            cnt = copy_Data_Suffix(data_handle,data_temp,timeBound[1]);

            log_update_collector[2] = cnt;
            log_update_collector[3] = get_Time_From_Data(data_handle,cnt);

            f_close(data_handle);
            f_close(data_temp);
            f_delete(data_name);
            if(cnt == 0){
            	f_delete(data_copy_name);
            }
            else{
                f_rename(data_copy_name,data_name);
            }
        }


        ///////// update log /////////////////
       tlm_type_t hold = tlm_global;
       tlm_global = NUMBER_OF_LOG_TYPES;
       sprintf(log_copy_name,"%s/tempLog",path);
       log_temp = f_open(log_copy_name,"w");
       if(log_temp == NULL){
    	   printf("failed to open temporary log file in deleteData\r\n");
       	   }




       if(!startSkip){
    	   f_rewind(log_handle);
    	   //TODO: fix bug, there exists no global_tlm with size "sizeof(logElement)"

    	   copyFile(log_handle,log_temp,log_update_collector[4]+1);


    	   get_log_element(log_handle,&le,log_update_collector[4]);
    	   le.fields[0] = log_update_collector[0];
    	   le.fields[2] = log_update_collector[1];
    	   f_write(le.raw,1,20,log_temp);
       }
       if(!endSkip){
    	   get_log_element(log_handle,&le,lineBounds[1]);
    	   le.fields[0] = log_update_collector[2];
    	   le.fields[2] = log_update_collector[3];
    	   f_write(le.raw,1,20,log_temp);

    	   f_seek(log_handle,lineBounds[1]*LOG_FILE_LINE_SIZE,F_SEEK_SET);

    	   copyFile(log_handle,log_temp,-1);
       }

       f_seek(log_temp,0,F_SEEK_END);
       numOfLines = f_tell(log_temp)/LOG_FILE_LINE_SIZE;

       f_close(log_handle);
       f_close(log_temp);

       if(numOfLines != 0){
    	   f_delete(log_name);
    	   f_rename(log_copy_name,log_name);
       }
       else{
    	   f_delete(log_copy_name);
    	   log_handle = f_open(log_name,"w");
    	   if(log_handle == NULL){
    		   printf("log wipe did not work\r\n");
    	   }
    	   for(int i = 0 ; i < 4 ; i++){
    		   le.fields[i] = 0;
    	   }
    	   if(f_write(le.raw,1,sizeof(logElement),log_handle) != sizeof(logElement)){
    		   printf("log default value was not written correctly during wipe\r\n");
    	   }
    	   f_close(log_handle);
       }
       tlm_global = hold;

   }
//TODO:stoped here


    else if(lineBounds[0] == lineBounds[1]){
        get_log_element(log_handle,&le,lineBounds[0]);
        sprintf(data_copy_name,"%s/temp",path);
        sprintf(data_name,"%s/%d",path,le.fields[1]);

        data_temp = f_open(data_copy_name,"w");
        data_handle = f_open(data_name,"r");
        if(data_handle==NULL){
            printf("unable to open file 2\n");
        }
        if(data_temp==NULL){
            printf("unable to open file 3\n");
        }
        cnt = copy_Data_Prefix(data_handle,data_temp,timeBound[0]);

        f_rewind(data_handle);
        f_rewind(data_temp);

        cnt = copy_Data_Suffix(data_handle,data_temp,timeBound[1]);


        if(cnt == 0){
            f_close(data_handle);
            f_close(data_temp);
            f_delete(data_copy_name);
            f_delete(data_name);
            sprintf(log_copy_name,"%s/tempLog",path);
            log_temp = f_open(log_copy_name,"w");
            if(log_temp == NULL){
                //TODO: asdfghj
                printf("(#2)\r\n");
            }
            copyFile(log_handle,log_temp,lineBounds[0]);
            f_seek(log_handle,LOG_FILE_LINE_SIZE,F_SEEK_CUR);
            copyFile(log_handle,log_temp,-1);
            if(!(f_tell(log_temp)/LOG_FILE_LINE_SIZE)){
                for(int i = 0 ; i < 4 ; i++){
    		        le.fields[i] = 0;
    	        }
                f_write(le.raw,1,sizeof(logElement),log_temp);
            }
            f_close(log_temp);
            f_close(log_handle);
            f_delete(log_name);
            f_rename(log_copy_name,log_name);
        }
        else{
            le.fields[2]=get_Time_From_Data(data_temp,0);
            le.fields[3]=get_Time_From_Data(data_temp,cnt);
            f_close(data_handle);
            f_close(data_temp);
            f_delete(data_name);
            f_rename(data_copy_name,data_name);
            update_log_element(log_handle,le,lineBounds[0]);
        }

    }
}




void findData(tlm_type_t tlm, int timeBound[2]){
	tlm_global = tlm;
	char* sys = dirNames[tlm];
    char path[120];
    char logFile[120];
    char fileName[125];
    int numOfLines;
    int res[2];
    bool inFile;
    sprintf(path,"%s/%s",sys,inDirPath);
    sprintf(logFile ,"%s/log", path);

    //search in log:
    F_FILE* fp = f_open(logFile,"r");
    if(fp == NULL){
        //fail
        exit(0);
    }
    //size of file
    f_seek(fp, 0L, F_SEEK_END);
    numOfLines = f_tell(fp)/LOG_FILE_LINE_SIZE; // might include bug



    res[0] = find_file_from_log(fp,numOfLines,timeBound[0],false,&inFile);
    if(!inFile){res[0]++;}
    res[1] = find_file_from_log(fp,numOfLines,timeBound[1],true,&inFile);
    if(!inFile){res[1]--;}
/*
    if(res[0] == -1 || res[1] == numOfLines+1){
        //log error
        printf("out of bound\n");
        return;
    }*/


    /////
    int hold;
    unsigned char data[100];
    logElement le;
    F_FILE* data_handle;
    get_log_element(fp,&le,res[0]);
    while(res[0]<=res[1]){
        sprintf(fileName,"%s/%d",path,le.fields[1]);
        data_handle = f_open(fileName,"r");

        while(f_read(&hold,1,sizeof(int) , data_handle) ==sizeof(int)){

            if(hold < timeBound[0]){
            	f_seek(data_handle,sizeByTlm[tlm_global],F_SEEK_CUR);
                continue;
            }
            else if(timeBound[1]< hold){
                break;
            }
            else{

                f_read(&data,1,sizeByTlm[tlm_global],data_handle);
                printf("data being found:\n\r");
                printf("%d: ",hold);
                hex_print(data,sizeByTlm[tlm_global]);
            }
        }
    f_close(data_handle);
    res[0]++;
    get_log_element(fp,&le,res[0]);
    }
    f_rewind(fp);
    f_close(fp);
    //writeDataToLog(path,lineBounds,timeBound,log_handle);
}

#ifdef TEST

int legalDirIndex[8] = {0,1,2,3,4,6,8,13};

static void rand_string(unsigned char *str, size_t size)
{
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    if (size) {
        --size;
        for (size_t n = 0; n < size; n++) {
            int key = rand() %  sizeof (charset) ;
            str[n] = charset[key];
        }
        str[size] = '\0';
    }
    return;
}
void  makeRandomFile(){
    int length = rand()%50+1;
    char unsigned data[55];
    rand_string(data,length);
    //WriteData(dirNames[legalDirIndex[rand()%9]],data);
    WriteData(legalDirIndex[rand()%8],data);
}
// taken from stack overflow

void print_sub_system_names() {
    for (int i = 0; i < NUMBER_OF_LOG_TYPES; i++) {
        if(strcmp(dirNames[i],"")){
            printf("Index %d: %s\r\n", i, dirNames[i]);
        }
    }
}
void print_file_contents(char *file_path) {
    F_FILE *file = f_open(file_path, "r");
    if (file == NULL) {
        printf("Error opening the file.\n");
        return;
    }

    int c = f_getc(file);
    while (c != -1) {
        printf("%c", c);
    }

    f_close(file);
}

void leaveLevel(char* path){ // take directory path and return father directory
    int length = strlen(path)-1;
    while(0 <= length){
        if(path[length] == '/'){
            path[length] = '\0';
            break;
        }
        length--;
    }
}
/**
#define LOG_NAME "log"

void Log(char* msg){

    F_FILE* file_handle = f_open(LOG_NAME,"a");
    f_printf(file_handle,"%s\n",msg);
    f_close(file_handle);
    return;
}*/

void WipeDirectory(char* folder){
	int err;
	F_FIND find;
	char path[100];
	sprintf(path, "%s/*.*", folder);
	if(!f_findfirst(path,&find)){
		if(!strncmp(find.name,".",1)){
			f_findnext(&find); // skip "."
			err =f_findnext(&find); // skip ".."
		}
		if(err == F_NO_ERROR){
			do{
				sprintf(path,"%s/%s",folder,find.name);
				f_delete(path);
			        }
			 while(!f_findnext(&find));
		}
	}
}


void DisplayDirectory(char* folder){
	int err= F_NO_ERROR;
	F_FIND find ;
	char path[100];
	sprintf(path, "%s/*.*", folder);
	if(!f_findfirst(path,&find)){
		if(!strncmp(find.name,".",1)){
			f_findnext(&find); // skip "."
			err =f_findnext(&find); // skip ".."
		}
		if(err == F_NO_ERROR){
			do{
				printf("%.8s\n",find.name); //printf("%s\n",find.name);
			        }
			 while(!f_findnext(&find));
		}
	}
}


void testFS(){
    unsigned char msg[50];
    int seq[] = {1,9,2,99};
    int seqI = 0;
    char fileName[150];
    int bounds[2];
    int command = 2, year, month, day, hold = 0, i;
    char* sys = dirNames[tlm_global];
    char show[200];
    srand(time(NULL));

    //seconds = time(NULL);
    //current_time=localtime(&seconds);
    //year = (current_time->tm_year+1900);
    //month = (current_time->tm_mon + 1);
    //day = (current_time->tm_mday);
    year = 2024;
    month = 7;
    day = 12;
    sprintf(inDirPath,"%d/%d/%d",year,month,day);

    printf("start Test\n");
    printf("operations:\n\n");
    printf("-1.  set sub system\n");
    printf("0.   set day\n");
    printf("1.   open new day directory\n");
    printf("2.   add manual message (up to length 50)\n");
    printf("3.   search for existing messages\n");
    printf("4.   delete elements\n");
    printf("5.   print log\n");
    printf("6.   print out file by name\n");
    printf("7.   add random file to system\n");
    printf("8.   manual explore\n");

    printf("99.  exit\n");
    while(1){
    printf("please choose operation:(98 to print menu)\n");
    UTIL_DbguGetIntegerMinMax(&command, 1, 16);
    //UTIL_DbguGetIntegerMinMax(&command, -2,100);
    command = seq[seqI++];
    switch(command){
        case(-1):
            printf("systems:\n");
            print_sub_system_names();
            UTIL_DbguGetIntegerMinMax(&command, 0, 15);
            tlm_global = command;
            sys = dirNames[tlm_global];
            break;

        case(0):
            printf("please enter year:\n");
        	//UTIL_DbguGetIntegerMinMax(&year, 0 , 3000);
            printf("please enter month:\n");
           // UTIL_DbguGetIntegerMinMax(&month, 1 , 12);
            printf("please enter day:\n");
            //UTIL_DbguGetIntegerMinMax(&day, 1 , 31);
            sprintf(inDirPath,"%d/%d/%d",year,month,day);
            break;
        case(1):
            new_Day_Dir();
            break;
        case(2):
            printf("please enter massage:\n");
        	//UTIL_DbguGetString(msg, 50);
            WriteData(tlm_global,msg);
            break;
        case(3):

            printf("from time: \n");
        	//UTIL_DbguGetIntegerMinMax( &bounds[0], 0,24*60*60);
            printf("to time: \n");
            //UTIL_DbguGetIntegerMinMax( &bounds[1], 0,24*60*60);
            findData(tlm_global,bounds);
            break;
        case(4):
            printf("from time: \n");
        	//UTIL_DbguGetIntegerMinMax( &bounds[0], 0,24*60*60);
            printf("to time: \n");
            //UTIL_DbguGetIntegerMinMax( &bounds[1], 0,24*60*60);
            deleteData(tlm_global,bounds);
            break;
        case(5):
            sprintf(fileName,"%s/%s/log",sys,inDirPath);
            print_file_contents(fileName);
            break;
        case(6):
            printf("enter file name\n");
        	UTIL_DbguGetInteger(&hold);
            sprintf(fileName,"%s/%s/%d",sys,inDirPath,hold);
            print_file_contents(fileName);
            break;
        case(7):
            printf("how many entries?\n");
        	//UTIL_DbguGetInteger(&hold);
            for(i=0;i<hold;i++){
                vTaskDelay(600);
                rand_string(msg,rand()%50);
                WriteData(tlm_global,msg);
            }
            break;
        case(8):

            printf("starting manual explore\nuse q! to exit\n");
            sprintf(show,"./%s",sys);
            printf("contents of dir(%s):\n",show);
            DisplayDirectory(show);
            while(1){
            	//UTIL_DbguGetString(msg, 50);
                printf("\n\n\n\n");
                if(!strcmp(msg,"q!")){
                    break;
                }
                else if(!strcmp(msg,"..")){
                    leaveLevel(show);
                }
                else{
                    sprintf(show,"%s/%s",show,msg);
                }
                printf("contents of dir(%s)\n",show);
                DisplayDirectory(show);

            }

            break;
        case(9):
				sprintf(show,"%s/%s",sys,inDirPath);
        		printf("contents of dir(%s)\n",show);
        		DisplayDirectory(show);
        		break;
        case(98):
            printf("-1.  set sub system\n");
            printf("0.   set day\n");
            printf("1.   open new day directory\n");
            printf("2.   add manual message (up to length 50)\n");
            printf("3.   search for existing messages\n");
            printf("4.   delete elements\n");
            printf("5.   print log\n");
            printf("6.   print out file by name\n");
            printf("7.   add random file to system\n");
            printf("8.   manual explore\n");
            printf("98.  print menu\n");
            printf("99.  exit\n");
            break;

        case(99):
            goto exit;
            break;
    }

    }
exit:;
}
#endif

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
#define LOG_FILE_LINE_SIZE 20
#define DATA_FILE_LINE_SIZE 58
#define DATA_TIME_STAMP_SIZE 5
#define DATA_SIZE 50

#define DIR_NAME_EPS_TLM "DI/EPS_TLM"
#define DIR_NAME_TX "DI/TX"
#define DIR_NAME_ANTENNA "DI/ATN"
#define DIR_NAME_SOLAR_PANELS_TLM "DI/SP_TLM"
#define DIR_NAME_WOD_TLM "DI/WOD_TLM"
#define DIR_NAME_RX "DI/E_RX"
#define DIR_NAME_RX_FRAME "DI/RX_FRM"
#define DIR_NAME_LOGS "DI/LOGS"


char inDirPath[100];
char* dirNames[NUMBER_OF_LOG_TYPES] = {DIR_NAME_EPS_TLM,DIR_NAME_TX, DIR_NAME_ANTENNA,
		DIR_NAME_SOLAR_PANELS_TLM,DIR_NAME_WOD_TLM,"",DIR_NAME_RX,"",DIR_NAME_RX_FRAME,
		"","","","",DIR_NAME_LOGS};


// compatibility section////////////////
void Log(char* msg);
void WipeDirectory(char* folder);

#include <stdarg.h>

int f_printf(F_FILE* file , char* format,... ){
	va_list list;
	char buf[200];
	char c = '0';
	int res,index = 0;
	va_start(list,format); // might be wrong argument- format
	res = vsprintf(buf,format,list);
	va_end(list);
	c = buf[index];
	while(c != '\0'){
		f_putc(c,file);
		index++;
		c = buf[index];
	}
	return res;


}
int f_scanf(F_FILE* file, const char *format, ...) {
    va_list list;
    int holdPos = f_tell(file);
    char buf[200];


    for(int i = 0 ; i < 200; i++){
    	buf[i] = f_getc(file);
    }
    buf[200] = '\0';


    va_start(list, format);

    int ret = vsscanf(buf, format, list);

    va_end(list);
    f_seek(file,holdPos, F_SEEK_SET);
    return ret;
}
/////////////////////////////////////////////////




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
    //openAllFiles();
    return error;
}
void DeInitializeFS(int sd_card){
	return;
}


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
                f_printf(hold," 0|  0|    0|    0\n");
                f_close(hold);
            }
        }
    }
}

void getFileName(char* res , char* sys, int cur_time){
    int size;
    int from;
    int to;
    int name;
    F_FILE* handle;
    char Log[120];

    sprintf(Log,"%s/%s/log",sys,inDirPath);
    handle = f_open(Log,"r+");
    if(handle == NULL){
        //TODO: LOG and Fail
    	printf("failed in finding file in function getFileName\n");
    }

    f_seek(handle,-20,F_SEEK_END);

    if(f_scanf(handle,"%2d|%3d|%5d|%5d\n",&size,&name,&from,&to) == 0){
        //fail to read size of current size of file
    	printf("failed in reading of log getFileName\n");
    }


    if(handle == NULL){
        //TODO: fail
    	return;
    }


    if(size == 99){
        name++;
        f_seek(handle,0,F_SEEK_END);
        f_printf(handle,"%2d|%3d|%5d|%5d\n",1,name,cur_time,cur_time);


    }
    else{
        f_seek(handle,-20,F_SEEK_END);
        f_printf(handle,"%2d|%3d|%5d|%5d\n",size+1,name,from == 0 ? cur_time:from,cur_time);
    }
    f_close(handle);
    sprintf(res,"%s/%s/%d",sys,inDirPath,name);
}

int day_seconds() {
    return TICKS_TO_SECONDS(xTaskGetTickCount())%86400;
}

void WriteData(char* dirName ,char* data){
    char filename[120];
    int time_seconds = day_seconds() ;

#ifdef TEST
    printf("time stamp of entered data: %d\n", time_seconds);
#endif
    getFileName(filename,dirName,time_seconds);
    F_FILE* file_handle = f_open(filename,"a");
    if(file_handle == NULL){
        printf("handle is NULL\n");
    }
    else{
        f_printf(file_handle,"%5d:%50s\n",time_seconds,data);
        f_close(file_handle);
        }
}

void copyFile(F_FILE* copyFrom, F_FILE* copyTo, int numberOfLines){
    char buf[200]={'0'};
    long backup1 = f_tell(copyFrom);
    long backup2 = f_tell(copyTo);
    if(numberOfLines != -1){
        for(int i = 0 ; i < numberOfLines; i++){
            f_scanf(copyFrom,"%[^\n]\n",buf);
            f_printf(copyTo,"%s\n",buf);
        }
    }
    else{
        while(f_scanf(copyFrom,"%[^\n]\n",buf) != EOF){
            f_printf(copyTo,"%s\n",buf);
        }
    }
    f_seek(copyFrom,backup1,F_SEEK_SET);
    f_seek(copyTo,backup2,F_SEEK_SET);
    f_flush(copyTo);
}

// data file functions ////////////
int get_Time_From_Data(F_FILE* data, int line){
    long backup = f_tell(data);
    int res;
    f_seek(data,line*DATA_FILE_LINE_SIZE,F_SEEK_SET);
    f_scanf(data,"%5d:",&res);
    f_seek(data,backup,F_SEEK_SET);
    return res;
}
int copy_Data_Prefix(F_FILE* doc,F_FILE* copyTo, int upToBound){
     int cnt = 0;
     int timeStamp;
     char dataBuf[200];
    while(f_scanf(doc,"%5d:%50[^\n]",&timeStamp,dataBuf)!=EOF && timeStamp < upToBound){
        cnt++;
    }

    if(cnt!=0){
        f_rewind(doc);
        copyFile(doc,copyTo,cnt);
    }
    return cnt;
}

int copy_Data_Suffix(F_FILE* doc, F_FILE* copyTo ,int fromBound){
    int cnt = 0, timeStamp;
    char dataBuf[200];
    while(f_scanf(doc,"%5d:%[^\n]\n",&timeStamp,dataBuf)!=EOF){
        if(fromBound < timeStamp){
            cnt = 1;
            break;
        }
    }


    if(cnt == 1){
        f_printf(copyTo,"%5d:%50s\n",timeStamp,dataBuf);
        f_seek(copyTo,0,F_SEEK_END);
        copyFile(doc,copyTo,-1);
    }
    f_seek(copyTo,0,F_SEEK_END);
    return f_tell(copyTo)/DATA_FILE_LINE_SIZE;
}


//////////////////////////////////
/// Log functions ////////////////////
int get_Name_from_Log(F_FILE* log, int line){
    long backup = f_tell(log);
    int res;
    f_seek(log,line*LOG_FILE_LINE_SIZE+3,F_SEEK_SET);
    f_scanf(log,"%d",&res);
    f_seek(log,backup,F_SEEK_SET);
    return res;
}
int get_from_from_Log(F_FILE* log,int line){
    long backup = f_tell(log);
    int res;
    f_seek(log,line*LOG_FILE_LINE_SIZE+7,F_SEEK_SET);
    f_scanf(log,"%d",&res);
    f_seek(log,backup,F_SEEK_SET);
    return res;
}
int get_To_from_Log(F_FILE* log, int line){
    long backup = f_tell(log);
    int res;
    f_seek(log,line*LOG_FILE_LINE_SIZE+13,F_SEEK_SET);
    f_scanf(log,"%d", &res);
    f_seek(log,backup,F_SEEK_SET);
    return res;
}

void getLineFromFile(F_FILE* fp, int line, char* buf){
    f_seek(fp, line*LOG_FILE_LINE_SIZE, F_SEEK_SET);
    f_scanf(fp,"%[^\n]\n",buf);
}
void update_Size_In_Log(F_FILE* log,int line,int newValue){
    long backup = f_tell(log);
    f_seek(log,line*LOG_FILE_LINE_SIZE,F_SEEK_SET);
    f_printf(log,"%2d",newValue);
    f_seek(log,backup,SEEK_SET);
}
void update_From_In_Log(F_FILE* log,int line,int newValue){
    long backup = f_tell(log);
    f_seek(log,line*LOG_FILE_LINE_SIZE+7,F_SEEK_SET);
    f_printf(log,"%5d",newValue);
    f_seek(log,backup,F_SEEK_SET);
}
void update_To_In_Log(F_FILE* log, int line ,int newValue){
    long backup = f_tell(log);
    f_seek(log,line*LOG_FILE_LINE_SIZE+13,F_SEEK_SET);
    f_printf(log,"%5d",newValue);
    f_seek(log,backup,F_SEEK_SET);
}


int find_file_from_log(F_FILE* fp, int numberOfEntries, int time, bool max, bool* inFile){
    int line = numberOfEntries/2;
    int lowerBound,upperBound;
    *inFile = false;

    for(int i = 1 ; numberOfEntries>>(i-1); i++){

        lowerBound = get_from_from_Log(fp,line);
        upperBound = get_To_from_Log(fp,line);

        if(time<lowerBound){
            if(line == 0){
                return 0;
            }
            upperBound = get_To_from_Log(fp,line-1);
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
            lowerBound = get_from_from_Log(fp,line+1);
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
                    upperBound = get_To_from_Log(fp,line-1);
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
                    lowerBound =get_from_from_Log(fp,line+1);
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
void deleteData(char* sys,int timeBound[2]){
    char path[120];
    char logFile[120];
    char fileName[125];
    int numOfLines;
    int res[2];
    bool startSkip = false, endSkip = false;
    sprintf(path,"%s/%s",sys,inDirPath);
    sprintf(logFile ,"%s/log", path);

    //search in log:
    F_FILE* fp = f_open(logFile,"r+");
    if(fp == NULL){
        //fail
        exit(0);
    }
    //size of file
    f_seek(fp, 0L, F_SEEK_END);
    numOfLines = f_tell(fp)/LOG_FILE_LINE_SIZE; // might include bug



    res[0] = find_file_from_log(fp,numOfLines,timeBound[0],false,&startSkip);
    res[1] = find_file_from_log(fp,numOfLines,timeBound[1],true,&endSkip);

    startSkip = !startSkip;
    endSkip = !endSkip;
    if(startSkip){
        res[0]--;
    }
    if(endSkip){
        res[1]++;
    }

    // start deleting
    int hold, cnt = 0;
    int timeStamp=0;
    F_FILE* Doc;
    F_FILE* Temp;
    F_FILE* TempLog;
    char dataBuf[100];
    char copyTo[125];
    char logCopy[125];





    if(res[0]<res[1]){
        // update start of log by copying into temp Log file (starting section where entries need to be deleted)
    	sprintf(logCopy,"%s/tempLog",path);
    	TempLog = f_open(logCopy,"w");
    	if(TempLog==NULL){
    		printf("unable to open file 1\n");
    	}


        if(!startSkip){
            f_seek(TempLog,0L,F_SEEK_END);
            f_seek(fp,0L,F_SEEK_SET);
            copyFile(fp,TempLog,res[0]);
            f_flush(TempLog);


            //if we need to delete from multiple files.
            // copy start of first file(res[0]), section that is not being deleted .


            sprintf(copyTo,"%s/temp",path);
            sprintf(fileName,"%s/%d",path,get_Name_from_Log(fp,res[0]));
            Temp = f_open(copyTo,"w+");
            Doc = f_open(fileName,"r");
            if(Doc==NULL){
                printf("unable to open file 2\n");
            }
            if(Temp==NULL){
                printf("unable to open file 3\n");
            }



            cnt = copy_Data_Prefix(Doc,Temp,timeBound[0]);
            if(cnt){
                f_seek(fp, res[0]*LOG_FILE_LINE_SIZE, F_SEEK_SET);
                f_scanf(fp,"%2d|%[^\n]\n",&hold,dataBuf);
                f_seek(TempLog, 0L, SEEK_END);
                f_printf(TempLog,"%2d|%s\n",cnt,dataBuf);
                update_To_In_Log(TempLog,res[0],get_Time_From_Data(Temp,cnt-1));
            }

            f_close(Temp);
            f_close(Doc);
            f_delete(fileName);
            if(cnt == 0){
                f_delete(copyTo);
            }
            else{
                f_rename(copyTo,fileName);
            }

        }
            // first file in deletion chain is done.



        // delete all files between the two edges- res[0] and res[1]
        res[0]++;
        while(res[0] < res[1]){
            sprintf(fileName,"%s/%d",path,get_Name_from_Log(fp,res[0]));
            f_delete(fileName);
            res[0]++;
        }



        // copy end of last file(res[1]), section that is not being deleted .
        if(!endSkip){
            sprintf(copyTo,"%s/temp",path);
            sprintf(fileName,"%s/%d",path,get_Name_from_Log(fp,res[1]));
            Temp = f_open(copyTo,"w+");
            Doc = f_open(fileName,"r");
            if(Doc==NULL){
                printf("unable to open file\n");
            }
            if(Temp==NULL){
                printf("unable to open file\n");
            }


            cnt = copy_Data_Suffix(Doc,Temp,timeBound[1]);
            if(cnt != 0){
                getLineFromFile(Temp,0,dataBuf);
                dataBuf[5] = '\0';
                hold = atoi(dataBuf);
            }


            f_close(Doc);
            f_close(Temp);
            f_delete(fileName);

            if(cnt == 0){
            	f_delete(copyTo);
            }
            else{
                f_rename(copyTo,fileName);
            }
        /////////
        f_seek(TempLog, 0L, F_SEEK_END);
        numOfLines = f_tell(TempLog)/LOG_FILE_LINE_SIZE;
        getLineFromFile(fp,res[1],dataBuf);
        f_printf(TempLog,"%s\n",dataBuf);
        update_Size_In_Log(TempLog,numOfLines,cnt);
        update_From_In_Log(TempLog,numOfLines,hold);
        f_seek(TempLog, 0, SEEK_END);
        f_flush(TempLog);
        ////////
        copyFile(fp,TempLog,-1);
        }
        f_seek(TempLog, 0L, F_SEEK_END);
        numOfLines = f_tell(TempLog)/LOG_FILE_LINE_SIZE;
        if(numOfLines == 0){
            f_printf(TempLog," 0|  0|    0|    0\n");
        }
        f_close(TempLog);
        f_close(fp);
        f_delete(logFile);
        f_rename(logCopy,logFile);
    }

    else if(res[0] == res[1]){
        f_seek(fp, res[0]*LOG_FILE_LINE_SIZE+3, F_SEEK_SET);
        f_scanf(fp,"%d",&hold);
        sprintf(copyTo,"%s/temp",path);
        sprintf(fileName,"%s/%d",path,hold);
        Temp = f_open(copyTo,"w");
        Doc = f_open(fileName,"r");
        if(Doc==NULL){
            printf("unable to open file 2\n");
        }
        if(Temp==NULL){
            printf("unable to open file 3\n");
        }
        int cnt = 0;
        while(f_scanf(Doc,"%5d:%50[^\n]",&timeStamp,dataBuf) != EOF){
            if(timeBound[0]<=timeStamp && timeStamp <= timeBound[1]){
                continue;
            }
            f_printf(Temp,"%5d:%50s\n",timeStamp,dataBuf);
            cnt++;
        }
        f_seek(fp,res[0]*LOG_FILE_LINE_SIZE,F_SEEK_SET);
        f_printf(fp,"%2d",cnt);

        if(numOfLines == 0){
            f_seek(TempLog,0L,SEEK_END);
            f_printf(TempLog," 0|  0|    0|    0\n");
        }

        if(cnt != 0){
            update_From_In_Log(fp,res[0], timeBound[0] + 1);
            update_To_In_Log(fp,res[0], timeBound[1] - 1);
        }
        else{
            copyFile(fp,TempLog,res[0]);
            f_seek(fp,(res[0]+1)*LOG_FILE_LINE_SIZE,F_SEEK_SET);
            copyFile(fp, TempLog,-1);
            f_close(TempLog);
            f_delete(logFile);
            f_rename(copyTo,logFile);
        }

        f_close(fp);
        f_close(Temp);
        f_close(Doc);
        f_delete(fileName);
        if(cnt == 0){
            f_delete(copyTo);
        }
        else{
            f_rename(copyTo,fileName);
        }
    }
}
void findData(char* sys, int timeBound[2]){
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
    res[1] = find_file_from_log(fp,numOfLines,timeBound[1],true,&inFile);

    if(res[0] == -1 || res[1] == numOfLines+1){
        //log error
        printf("out of bound\n");
        return;
    }


    /////
    int hold;
    char data[60];
    F_FILE* Doc;
    f_seek(fp, res[0]*LOG_FILE_LINE_SIZE+3, F_SEEK_SET);
    f_scanf(fp,"%d",&hold);
    while(res[0]<=res[1]){
    sprintf(fileName,"%s/%d",path,hold);
    Doc = f_open(fileName,"r");
    while(f_scanf(Doc,"%5d:%50[^\n]",&hold,data) != EOF){

        if(hold < timeBound[0]){
            continue;
        }
        else if(timeBound[1]< hold){
            break;
        }
        else{
            printf(data);
            printf("\n");
        }
    }
    f_close(Doc);
    res[0]++;
    f_seek(fp, res[0]*LOG_FILE_LINE_SIZE+3, F_SEEK_SET);
    f_scanf(fp,"%d",&hold);
    }
    f_rewind(fp);
    //writeDataToLog(path,res,timeBound,fp);

}

#ifdef TEST

int legalDirIndex[8] = {0,1,2,3,4,6,8,13};

static void rand_string(char *str, size_t size)
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
    char data[55];
    rand_string(data,length);
    //WriteData(dirNames[legalDirIndex[rand()%9]],data);
    WriteData(dirNames[legalDirIndex[rand()%8]],data);
}
// taken from stack overflow

void print_sub_system_names() {
    for (int i = 0; i < NUMBER_OF_LOG_TYPES; i++) {
        if(strcmp(dirNames[i],"")){
            printf("Index %d: %s\n", i, dirNames[i]);
        }
    }
}
void print_file_contents(char *file_path) {
    F_FILE *file = f_open(file_path, "r");
    if (file == NULL) {
        printf("Error opening the file.\n");
        return;
    }

    char c = f_getc(file);
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

#define LOG_NAME "log"

void Log(char* msg){

    F_FILE* file_handle = f_open(LOG_NAME,"a");
    f_printf(file_handle,"%s\n",msg);
    f_close(file_handle);
    return;
}

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
    char msg[50];
    int seq[] = {1,9,2,99};
    int seqI = 0;
    char fileName[150];
    f_delete(LOG_NAME);
    int bounds[2];
    int command = 2, year, month, day, hold, i;
    time_t seconds;
    struct tm* current_time;
    char* sys;
    char show[200];
    srand(time(NULL));
    sys = dirNames[0];
    //seconds = time(NULL);
    //current_time=localtime(&seconds);
    //year = (current_time->tm_year+1900);
    //month = (current_time->tm_mon + 1);
    //day = (current_time->tm_mday);
    year = 2024;
    month = 7;
    day = 5;
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
           // UTIL_DbguGetIntegerMinMax(&command, 0, 3000);
            sys = dirNames[command];
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
            WriteData(sys,msg);
            break;
        case(3):

            printf("from time: \n");
        	//UTIL_DbguGetIntegerMinMax( &bounds[0], 0,24*60*60);
            printf("to time: \n");
            //UTIL_DbguGetIntegerMinMax( &bounds[1], 0,24*60*60);
            findData(sys,bounds);
            break;
        case(4):
            printf("from time: \n");
        	//UTIL_DbguGetIntegerMinMax( &bounds[0], 0,24*60*60);
            printf("to time: \n");
            //UTIL_DbguGetIntegerMinMax( &bounds[1], 0,24*60*60);
            deleteData(sys,bounds);
            break;
        case(5):
            sprintf(fileName,"%s/%s/log",sys,inDirPath);
            print_file_contents(fileName);
            break;
        case(6):
            printf("enter file name\n");
        	//UTIL_DbguGetInteger(&hold);
            sprintf(fileName,"%s/%s/%d",sys,inDirPath,hold);
            print_file_contents(fileName);
            break;
        case(7):
            printf("how many entries?\n");
        	//UTIL_DbguGetInteger(&hold);
            for(i=0;i<hold;i++){
                vTaskDelay(600);
                rand_string(msg,rand()%50);
                WriteData(sys,msg);
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

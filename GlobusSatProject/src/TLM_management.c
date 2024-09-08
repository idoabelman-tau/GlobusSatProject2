#include "TLM_management.h"
#include "hcc/api_fat.h"
#include "hcc/api_hcc_mem.h"
#include "hcc/api_mdriver_atmel_mcipdc.h"



#define NUMBER_OF_LOG_TYPES 14

#define MAX_FILE_SIZE 100 //TODO: maybe change this into 8*512/(avrage entry size).(about 157 for eps might be optimal)

// used to write log files for easy search of data.
//fields[0] - number of entries in file
//fields[1] - name of file
//fields[2] - sample time first line in file
//fields[3] - sample time last line in file

typedef union _logElement{
	int fields[4];
	unsigned char raw[16];
} logElement;

#define _SD_CARD 0

// all prefixes of file path names depending on the system being used
const char* dirNames[NUMBER_OF_LOG_TYPES] = {"DI/EPS","DI/TX", "DI/ATN",
		"DI/SP","DI/WOD","","DI/RX","","DI/RX_FR",
		"","","","","DI/LOGS"};


// size of file element. for example: the size in bytes of eps telementry (first index in dir_Names) is of length 22.
const int sizeByTlm[NUMBER_OF_LOG_TYPES+1] = {22,0,0,0,0,0,0,0,0,0,0,0,0,0, sizeof(logElement)};

// each line written to file will include and time stamp (int) and the actual data. correct size of entry is sizeof(timeStamp)+ sizeof(tlementry).
#define DATA_FILE_LINE_SIZE(tlm) (sizeByTlm[tlm]+ sizeof(int))

///////////////////////////////illustrations///////////////////////////////////
//directory tree:

/* each sub system has a directory tree that look some what like this:

    sub-system name(directory):                                               DI/EPS
                                                                            /        \
                                                                           /          \
                                                                          /            \
    year(directory):                                                   2023            2024
                                                                       /|\             /|\
                                                                      / | \           / | \
    month(directory)                                                 1 ... 12        1 ...12
                                                                       /|\             /|\
                                                                      / | \           / | \
                                                                     /  |  \         /  |  \
    day(directory)                                                  1  ...  30      1  ...  30
                                                                       /|\             /|\
                                                                      / | \           / | \
                                                                     /  |  \         /  |  \
    files(file, some day has x and another has y files)             1  ...  x       1  ...  y

*/



///////////////////////////////utility functions///////////////////////////////

/*@param text: text to be printed out
**@param length: length of text.
**@effect: prints each charecter of text as two hex chars.
*/
void hex_print(unsigned char* text , int length){
	for(int i = 0 ; i < length ; i++){
		printf("%02x ",text[i]);
	}
	printf("\n\r");
}

/*
*@param[IN] copyFrom. a pointer to a F_FILE structure. elements from this file will be copyed from.
*@param[IN] copyTO. a pointer to a open F_FILE structure. elements will be copyed into this file.
*@param[IN] tlm. to calculate entry size.
*@param[IN] numberOfEntries. the number of elements to copy.
*/
void copyFile(F_FILE* copyFrom, F_FILE* copyTo,tlm_type_t tlm, int numberOfEntries){
    char* buf= malloc(sizeByTlm[tlm]);
    if(buf == NULL){
    	printf("failed to allocate memory for file copy- TLM_mamagement -> copyFile\r\n");
    }
    long backup1 = f_tell(copyFrom);
    long backup2 = f_tell(copyTo);
    if(numberOfEntries != -1){
        for(int i = 0 ; i < numberOfEntries; i++){
        	if(f_read(buf,1,sizeByTlm[tlm],copyFrom) == sizeByTlm[tlm]){
        		f_write(buf,1,sizeByTlm[tlm],copyTo);
        	}
        	else{
        		printf("copy line failed in TLM_management -> copyFile\r\n");
        	}
        }
    }
    else{
        while(f_read(buf,1,sizeByTlm[tlm],copyFrom) == sizeByTlm[tlm]){
        	f_write(buf,1,sizeByTlm[tlm],copyTo);
        }
    }
    f_seek(copyFrom,backup1,F_SEEK_SET);
    f_seek(copyTo,backup2,F_SEEK_SET);
    f_flush(copyTo);
    free(buf);
}
//@brief: create directory path to store data files. (today, as returned by time.h)
void  newDayDir(Time* time){

    char name[256];
    int i, err = 0, j;
    char c = '0';

    unsigned char dummy_atter;

    for(i = 0 ; i < NUMBER_OF_LOG_TYPES ; i++){ // for each sub-system
        if(strcmp("",dirNames[i])){
            sprintf(name,"%s/%d/%d/%d",dirNames[i],time->year,time->month,time->date); // build name path.
            if(f_getattr(name,&dummy_atter) == F_NO_ERROR){
            	continue;
            }

            for(j = 0 ;c != '\0'; j++){ // search for '\' char
                c = name[j];
                if(c == '/'){ // when '\' is found, replace it with null char and build path upto that point.
                    name[j] = '\0';
                    err = f_mkdir(name);
                    if(err != F_NO_ERROR && err !=F_ERR_DUPLICATED){
                    	printf("found error in new Day_Dir- error: %d\r\n",err);
                    }
                    name[j] = c;
                }
            }
            c = '0';
            err = f_mkdir(name); // build last directory.
            if(err != F_NO_ERROR && err !=F_ERR_DUPLICATED){
            	printf("found error in new Day_Dir- error: %d\r\n",err);
            }

            // open log file of this day.///
            sprintf(name,"%s/%s",name,"log");
            F_FILE* hold = f_open(name,"a");
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

// DI/EPS/2024/4/5
// DI/EPS/2024/4
// DI/EPS/2024/4/DI/EPS/2024/4/6
// F_FIND-> filename = DI/EPS
void WipeDirectory(char* folder){
	char cwd[F_MAXPATHNAME];
	int err = f_getcwd(cwd, F_MAXPATHNAME);
	if(err !=F_NO_ERROR){
		return;
	}
	err = f_chdir(folder);
	if(err !=F_NO_ERROR){
		return;
	}
	F_FIND find;
	if(!f_findfirst("*.*",&find)){
		if(!strncmp(find.name,".",1)){
			f_findnext(&find); // skip "."
			err =f_findnext(&find); // skip ".."
		}
		if(err == F_NO_ERROR){
			do{
				if(find.attr & F_ATTR_DIR){
					WipeDirectory(find.name);
					err = f_rmdir(find.filename);
				}
				else {
					err = f_delete(find.filename);
				}
				if(err !=F_NO_ERROR){
					printf("failed delete with error %d", err);
				}

			}while(!f_findnext(&find));
		}
	}
	f_chdir(cwd);
}
void readlogEntry(F_FILE* log , int index, logElement* le){
	f_seek(log,index* sizeof(logElement),F_SEEK_SET);
	if(f_read(le,1,sizeof(logElement),log)!=sizeof(logElement)){
		//TODO:FAIL
	}
}
void zeroize(){
	//char folderName[256];
	/*
	for(int i = 0 ; i<NUMBER_OF_LOG_TYPES ; i++){
		if(strcmp(dirNames[i],"") == 0){
			continue;
		}
		sprintf(folderName,"%s",dirNames[i]);
		WipeDirectory(folderName);
	}*/
	WipeDirectory("DI");
	Time time;
	Time_get_wrap(&time);
	newDayDir(&time);
}

// extract time stamp of entry in data file by line.
int getTimeFromData(F_FILE* data,tlm_type_t tlm, int line){
    long backup = f_tell(data);
    int res;
    f_seek(data,line*DATA_FILE_LINE_SIZE(tlm),F_SEEK_SET);
    if(f_read(&res,sizeof(int),1,data) != 1){
    	res = -1;
    }
    f_seek(data,backup,F_SEEK_SET);
    return res;
}



F_FILE* f_itruncate(F_FILE* data, char dataName[256],unsigned long cut){
    char copyName[256];
    int c;
    sprintf(copyName,"%sc",dataName);
    F_FILE* hold = f_open(copyName,"w");
    if(hold == NULL){
        //TODO: handle file not open
    }

    f_seek(data , cut , F_SEEK_SET);
    c = f_getc(data);
    while(c != EOF){
        if(f_putc(c,hold)){
        	//TODO: write failed.
        }
        c = f_getc(data);
    }

    f_seek(hold, 0 , F_SEEK_END);
    f_close(data);
    f_delete(dataName);

    if(f_tell(hold) == 0){
    	f_close(hold);
        f_delete(copyName);
        return NULL;
    }
    else{
        f_rename(copyName,dataName);
        f_seek(hold , 0 , F_SEEK_SET);
        return hold;
    }
}

int sendData(unsigned char * data, int length, time_unix timestamp) {
#ifdef TESTING
    printf("data being sent:\n\r");
    printf("%ld: ", timestamp);
    hex_print(data, length);
#endif
    unsigned char * dataWithTimestamp = malloc(sizeof(time_unix) + length);
    memcpy(dataWithTimestamp, &timestamp, sizeof(time_unix));
    memcpy(dataWithTimestamp + sizeof(time_unix), data, length);
    AddDataToSendBuffer(dataWithTimestamp, sizeof(time_unix) + length);
    free(dataWithTimestamp);
    return 0;
}
////////////////////////////////////////////////////////////////////////////////
/*@brief: initialize the file system. follows the demo and creates todays directory for each of the sub-systems.
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
       printf("format error %d found while file system volume initialization\n",error);
        return error;
    }
    else if(error != F_NO_ERROR){
    	printf(" error %d found while file system volume initialization\n",error);
        return error;
    }
   //printf("error %d found while file system volume initialization\n",error);
    //openAllFiles();

   Time time;
   Time_get_wrap(&time);
   newDayDir(&time);
   return error;
}


//@brief: deinitialize the file system. follows the demo.
void DeInitializeFS(int sd_card){

	return;
}

int day_seconds(Time *time) {
    return (time->seconds+60*time->minutes+3600*time->hours)%86400;
}


//@ log files are updated when writing into FS. open file, search for latest file (if full, open new) and increase size.
void giveLogNameForAdd(tlm_type_t tlm,char buf[256] ,Time* time){
	logElement le;
    F_FILE* handle;
    unsigned char dummy_attr;
    sprintf(buf,"%s/%d/%d/%d/LOG",dirNames[tlm],time->year,time->month,time->date);
    // check if dir exists.
    if(f_getattr(buf,&dummy_attr) != F_NO_ERROR){
		newDayDir(time);
	}


    handle = f_open(buf,"r+");
    if(handle == NULL){ // TODO: path does not exist error should can newDayDir() to create path.
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
    	le.fields[2] = day_seconds(time);
    	le.fields[3] = day_seconds(time);
        f_seek(handle,0,F_SEEK_END);
        f_write(le.raw,1,sizeof(logElement),handle);

    }
    else{ //le = {0,0,0,0}
    	le.fields[2] = (le.fields[0] == 0 ? day_seconds(time): le.fields[2]); // TODO: fix bug in this line
    	le.fields[0]++;
    	le.fields[3] = day_seconds(time);
    	f_seek(handle,-sizeof(logElement),F_SEEK_CUR);
    	f_write(le.raw,1,sizeof(logElement),handle);
    }

    f_close(handle);
    sprintf(buf,"%s/%d/%d/%d/%d",dirNames[tlm],time->year,time->month,time->date,le.fields[1]);
}




// writing is done by finding the currect log file, updating it and adding the entry to the end of a data file (dataname).
void WriteData(tlm_type_t tlm  ,unsigned char* data){
    char filename[256];
    Time time;
    Time_get_wrap(&time);
    int time_seconds = day_seconds(&time) ;
#ifdef TEST
    //printf("time stamp of entered data: %d\n", time_seconds);
#endif
    giveLogNameForAdd(tlm, filename,&time);
    F_FILE* file_handle = f_open(filename,"a");
    if(file_handle == NULL){
        printf("handle is NULL\n");
    }
    else{
        f_write(&time_seconds,sizeof(int),1 , file_handle);
        f_write(data,1,sizeByTlm[tlm] , file_handle);
    }
    f_close(file_handle);
}


// deletes *min bytes or the entire directory in *min is large enough. minimum delete is one day.
void deleteFilesBySize(char* path,long* min){
	int err;
	F_FIND find;
	if(!f_findfirst(path,&find)){
		if(!strncmp(find.name,".",1)){
			f_findnext(&find); // skip "."
			err =f_findnext(&find); // skip ".."
		}
		if(err == F_NO_ERROR){
			do{
				if(find.attr == F_ATTR_DIR){ // is dir, TODO: check assumption that first dir is oldest
					deleteFilesBySize(find.filename,min);
                    f_delete(find.filename); // this function fails if directory is not empty
					if(*min <= 0){
						break;
					}
				}
				else{ // is file
					*min -= find.filesize*sizeof(char);
					f_delete(find.filename);
				}
			}
			while(!f_findnext(&find));
		}
	}
}

void deleteBySize(tlm_type_t tlm,long* min){
	char root[256];
	sprintf(root,"%s/.",dirNames[tlm]);
	deleteFilesBySize(root,min);
}

void fs_zeroize(){

}


char getNameOfFirstDir(char* path){
    F_FIND find;
	if(!f_findfirst(path,&find)){
		if(!strncmp(find.name,".",1)){
			f_findnext(&find); // skip "."
			f_findnext(&find); // skip ".."
		}
        if(find.attr == F_ATTR_DIR){
            return atoi(find.name);
        }
    }
    return 0xff;
}

int calculateSecondsOfYear(Time* t) {
	int daysInMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	if (Time_isLeapYear(t->year)) {
		daysInMonth[1]++;
	}

	int seconds = 0;
	for (int i = 0; i < t->month - 1; i++) {
		seconds += daysInMonth[i] * 86400;
	}
	seconds += (t->date - 1) * 86400;
	seconds += t->hours * 3600;
	seconds += t->minutes * 60;
	seconds += t->seconds;
	return seconds;
}

int getTimeOldest(tlm_type_t tlm , Time* t){
    char path[256];
    sprintf(path,"%s/*.*",dirNames[tlm]);
    t->year = getNameOfFirstDir(path);
    sprintf(path,"%s/%d/*.*",dirNames[tlm],t->year);
    t->month = getNameOfFirstDir(path);
    sprintf(path,"%s/%d/%d/*.*",dirNames[tlm],t->year,t->month);
    t->date = getNameOfFirstDir(path);
    t->hours = 0;
    t->minutes = 0;
    t->seconds = 0;
    t->day = 1;
    t->secondsOfYear = calculateSecondsOfYear(t);
    return (t->year == 0xff || t->month == 0xff || t->date == 0xff); // returns 0 if name is reliable else 1.
}


void dumpTlmData(unsigned char* buf ,char dirpath[256] , Time* t, tlm_type_t tlm){
	int err = 0 , hold;
	F_FIND find;
	F_FILE* fp;
    char path[256];
    if(t != NULL){
        sprintf(path,"%s/%d/%d/%d",dirpath,t->year,t->month,t->date);
    }
    else{
        sprintf(path,"%s",dirpath);
    }

	if(!f_findfirst(path,&find)){
		if(!strncmp(find.name,".",1)){
			f_findnext(&find); // skip "."
			err =f_findnext(&find); // skip ".."
		}
        if(find.attr == F_ATTR_DIR){
            dumpTlmData(buf,find.filename,NULL,tlm);
        }
        else{
            do{
            if(!strncmp(find.name,"l",1)){
                    if(!f_findnext(&find)){
                        break;
                    }
                    fp = f_open(find.filename,"r");
                    if(fp == NULL){
                        //TODO: fail;
                    }
                    while(f_read(&hold,1,sizeof(int) , fp) == sizeof(int)){
                        f_read(buf,1,sizeByTlm[tlm],fp);
                        sendData(buf, sizeByTlm[tlm], Time_convertTimeToEpoch(t) + hold);
                    }
                }
            }while(!f_findnext(&find));
        }
    }
}


void dumpAllTlmData(){
	unsigned char buf[50] ;
	char name[256];
    for(int i = 0 ; i < NUMBER_OF_LOG_TYPES ; i++){
        sprintf(name,"%s", dirNames[i]);
        if(strncmp(name,"",1)){ ///TODO: check strncmp = 0 when equel.
            continue;
        }
        dumpTlmData(buf,name,NULL,i);
    }


}


// if timestamp falls between intervals say [2,6] and [9,14], [2,6] will be returned.
// if timestamp is further from max stamp in log the return numberofelement. also if stamp is prior to log return -1;
// return -2 if read failed;
int searchFileInLog(F_FILE* fp , int timestamp){
    f_seek(fp,0,F_SEEK_END);
    int h = f_tell(fp)/sizeof(logElement)-1;
    int l =0 , m = h;
    logElement le;

    f_seek(fp,m*(sizeof(logElement)),F_SEEK_SET);
    if(f_read(&le,1,sizeof(logElement),fp) != sizeof(logElement)){
        printf("failed to read log: searchFileInLog (0)");
        return -2;
        };
    if(le.fields[3] <= timestamp){
        return h;
    }

    m=0;
    f_seek(fp,m*sizeof(logElement),F_SEEK_SET);
    if(f_read(&le,1,sizeof(logElement),fp) !=sizeof(logElement)){
        printf("failed to read log: searchFileInLog (1)");
        return -2;
     }
    if(timestamp <= le.fields[2]){
        return -1;
    }
    while(l+1<h){
        m = (h+l)/2;
        f_seek(fp,m*sizeof(logElement),F_SEEK_SET);
        if(f_read(&le,1,sizeof(logElement),fp) != sizeof(logElement)){
            printf("failed to read log: searchFileInLog (2)");
            return -2;
        }
        if(le.fields[2] <= timestamp && timestamp <= le.fields[3]){
            return m;
        }
        else if(le.fields[3] < timestamp){
            l = m;
        }
        else{
            h = m;
        }
    }
    return m == l ? h:l;
}


unsigned int GetDataTimeStamp(F_FILE* data,unsigned int index , tlm_type_t tlm){
	int res;
	f_seek(data,index*sizeByTlm[tlm],F_SEEK_SET);
	if(f_read(&res,1,sizeof(unsigned int),data) != sizeof(unsigned int)){
		return -1;
	}
	return res;
}

// file must be non-empty
int searchDataTimeStamp(F_FILE* data,unsigned int search_stamp , tlm_type_t tlm){
    f_seek(data,0,F_SEEK_END);
    int h = f_tell(data)/DATA_FILE_LINE_SIZE(tlm)-1;
    int l =0 , m = h;
    unsigned int stamp;

    //edge cases...
    f_seek(data,m*DATA_FILE_LINE_SIZE(tlm),F_SEEK_SET);
    f_read(&stamp,1,sizeof(int),data);
    if(stamp <= search_stamp){
        return h;
    }

    m=0;
    f_seek(data,m*DATA_FILE_LINE_SIZE(tlm),F_SEEK_SET);
    f_read(&stamp,1,sizeof(int),data);
    if(search_stamp <= stamp){
        return 0;
    }
    ////////////////////////
    while(l+1 < h){
        m = (h+l)/2;
        f_seek(data,m*DATA_FILE_LINE_SIZE(tlm),F_SEEK_SET);
        f_read(&stamp,1,sizeof(int),data);
        if(stamp < search_stamp){
            l = m;
        }
        else if(search_stamp< stamp){
            h = m;
        }
        else{
            return m;
        }
    }
    if( m == l ){
        return h;
    }
    else{
        return l;
    }
}

F_FILE* cutDataByTimeStamp(char* dataname,unsigned int search_stamp , tlm_type_t tlm, Boolean upto){
	F_FILE* data = f_open(dataname,"r+");
	if(data == NULL){
		//TODO:fail correctly
	}
	int cutindex = searchDataTimeStamp(data,search_stamp,tlm);
	if(upto){
		f_close(data);
		data = f_truncate(dataname,(unsigned long)cutindex*DATA_FILE_LINE_SIZE(tlm));
	}
	else{
		f_close(data);
		data = f_itruncate(data,dataname,(unsigned long)cutindex*DATA_FILE_LINE_SIZE(tlm));
	}
	return data;
}


void giveLogOfDir(char* dirpath , F_FILE** fp , char* mode){
    char logname[256];
    sprintf(logname,"%s/log",dirpath);
    *fp = f_open(logname,mode);
}


void giveFileFromLog(char* dirpath , F_FILE* fp , int index, char* data){
    logElement le;
    readlogEntry(fp,index,&le);
    sprintf(data,"%s/%d",dirpath,le.fields[1]);
}

void updateTimeToNextDay(Time* t){
    int remainder;
    int now = Time_convertTimeToEpoch(t);
    if(t->seconds != 0 || t->hours != 0){
        remainder = 86400 -Time_convertTimeToEpoch(t)% 86400;
    }
    else{
        remainder = 86400;
    }
    Time_convertEpochToTime(now+remainder , t);
}


void openDataFileByLogIndex(F_FILE* log ,char* dirname , int index ,char* mode, F_FILE** data){
    char path[F_MAXPATHNAME];
    logElement le;
    f_seek(log , index*sizeof(logElement) , F_SEEK_SET);
    if(f_read(&le,1,sizeof(logElement),log)!= sizeof(logElement)){
        //TODO: handle
        printf("failed to read log: openDataFileByLogInex \n");
    }
    sprintf(path,"%s/%d",dirname,le.fields[1]);
    *data = f_open(path,mode);
}

Boolean sameDay(Time* t1, Time* t2){
    return (t1->month == t2->month && t1->date == t2->date && t1->year == t2->year) ? TRUE:FALSE;
}

unsigned int EpochTimeOfEndYear(int year){
    Time t;
    t.year = year+1;
    t.month = 1;
    t.date = 1;
    t.hours = 1;
    t.seconds = 0;
    t.secondsOfYear = 0;
    return Time_convertTimeToEpoch(&t)-1;
}

// returns a non zero when error occurs
int findInterval(unsigned int from , unsigned int to , Time* fromT , Time* toT , tlm_type_t tlm){
    Time t;
    getTimeOldest(tlm,&t);
    unsigned int hold_time;
    int err;
    // consider two intervals: [existing minimum time, existing maximum time ] and [from , to]. we would like to search the intersection.
    // this will ensure that we search only dates that are wanted and existing.

    hold_time = Time_convertTimeToEpoch(&t);
    if(from < hold_time){
        err=Time_convertEpochToTime(hold_time,fromT);
    }
    else{
        err = Time_convertEpochToTime(from,fromT);
    }

    Time_get_wrap(&t); //TODO: remove wrap
    hold_time = Time_convertTimeToEpoch(&t);
    if(hold_time<to){
        err+=Time_convertEpochToTime(hold_time,toT);
    }
    else{
        err+=Time_convertEpochToTime(to,toT);
    }
    return err;

}
void dumpFile(F_FILE* data, unsigned int from, unsigned int to , time_unix file_day, tlm_type_t tlm){
    int start_index = searchDataTimeStamp(data,from,tlm), end_index =  searchDataTimeStamp(data,to,tlm);
    unsigned int bufI;
    unsigned char *bufD =(unsigned char*) malloc(DATA_FILE_LINE_SIZE(tlm));
    f_seek(data,start_index*DATA_FILE_LINE_SIZE(tlm),F_SEEK_SET);
    for(int i = start_index ; i <= end_index ; i++){
        f_read(&bufI,1,sizeof(unsigned int) , data);
        if(f_read(bufD,1,sizeByTlm[tlm],data) != sizeByTlm[tlm]){
            //TODO: handle failed read.
        }
        sendData(bufD, sizeByTlm[tlm], file_day + bufI);
    }
    free(bufD);
}
void givePathOfTime(tlm_type_t tlm ,Time* t , char* path){
	sprintf(path,"%s/%d/%d/%d",dirNames[tlm],t->year,t->month,t->date);
}
unsigned int secondsInDay(Time* t){
	return t->seconds + 60*t->minutes +60*60*t->hours;
}

void findData(tlm_type_t tlm, unsigned int from , unsigned int to){

    Time fromT, toT; // from but if type Time and to but of type Time.
    F_FILE *fp;
    char path[256];
    F_FILE* data;
    unsigned char buf[50];
    if(findInterval(from ,to ,&fromT ,&toT,tlm)){
        //TODO: handle
        printf("failed to obtain interval: findData\n");
        return;
    }
	if(Time_diff(&toT ,&fromT) == TIMEDIFF_INVALID){
		// edge case of to<from. also if years don't match case is handled here.
		 //TODO: add this case. Timediff return TIMEDIFF_INVALID when years don't match or when fromT<toT.
		 if(toT.year <= fromT.year){
			  return; // toT is before fromT, this is an edge case.
		 }
	}
    //start search from Time fromT to time Time toT.
    givePathOfTime(tlm,&fromT,path);
    giveLogOfDir(path,&fp, "r");
    if(fp == NULL){
        //TODO: file did not open
    }
    f_seek(fp,0,F_SEEK_END);

    int loglength = f_tell(fp)/sizeByTlm[NUMBER_OF_LOG_TYPES];
    int start_index = searchFileInLog(fp,secondsInDay(&fromT));
    int end_index;
    if(sameDay(&toT,&fromT)){
        /*if toT and fromT represent the same day, the data is in one dirrectory.
        /1) log of this day is an open file in fp at this point.
        /2) call dumpFile(k,fromT.seconds,toT.seconds),for i <= k <= j.
        /where i, k ,j are files and  timeStamps(i) <= timeStamps(k) <= timeStamps(j);
        */
        end_index = searchFileInLog(fp,secondsInDay(&toT));
        while(start_index <= end_index){



        	//openDataFileByLogIndex(F_FILE* log ,char* dirname , int index ,char* mode, F_FILE** data)

        	openDataFileByLogIndex(fp,path,start_index,"r",&data);
            if(data == NULL){
                //TODO: file did not open
            }
            dumpFile(data, secondsInDay(&fromT),secondsInDay(&toT), Time_convertTimeToEpoch(&fromT) - secondsInDay(&fromT), tlm);
            f_close(data);
            start_index++;
        }
        f_close(fp);
        return;
    }

    // we cannot assume the start and end are at the same directory.
    // in this else cse we will print the first day from fromT.seconds onwards, continue to while loop that will dump all days between
    // fromT to toT and then print from the last directory.
    while(start_index <loglength){

    	   openDataFileByLogIndex(fp,path,start_index,"r",&data);
           if(data == NULL){
           //TODO: file did not open
           }
           dumpFile(data,secondsInDay(&fromT),90000, Time_convertTimeToEpoch(&fromT) - secondsInDay(&fromT),tlm); // all days have less then 90000 seconds meaning search without an upper bound.
           f_close(data);
           start_index++;
   }
   f_close(fp);
   updateTimeToNextDay(&fromT);


    while(!sameDay(&toT,&fromT)){
        dumpTlmData(buf,path,&fromT,tlm); // dump entire day.
        updateTimeToNextDay(&fromT);
    }


    // deal with last day.
    givePathOfTime(tlm,&fromT,path);
    giveLogOfDir(path,&fp, "r");
    if(fp == NULL){
        //TODO: file did not open
    }
    start_index = 0;
    end_index = searchFileInLog(fp,toT.seconds);
    while(start_index <= end_index){
    	openDataFileByLogIndex(fp,path,start_index,"r",&data);
        if(data == NULL){
            //TODO: file did not open
        }
        dumpFile(data, 0, toT.seconds, Time_convertTimeToEpoch(&fromT), tlm);
        f_close(data);
        start_index++;
    }
    f_close(fp);
    SendBuffer();
    return;

}



void updateLogByFiles(F_FILE* fp,F_FILE* data1,int sentry,F_FILE* data2,int eentry,tlm_type_t tlm, char* dirpath){
	char copyname[256];
	int size;
	int holdLocation;
	unsigned int stamp;
	logElement le;
	sprintf(copyname,"%s/logc",dirpath);
	F_FILE* copy = f_open(copyname,"w");
	f_rewind(fp);
	copyFile(fp,copy,14,sentry);

	f_seek(data1,0,F_SEEK_END);
	size = f_tell(data1)/DATA_FILE_LINE_SIZE(tlm);
	stamp = GetDataTimeStamp(data1,size-1,tlm);
	if(stamp == 0xffffffff){
		//TODO: ERROR
	}

	f_seek(copy,-sizeof(logElement),F_SEEK_CUR);
	if(f_read(&le , 1 , sizeof(logElement),copy) != sizeof(logElement)){
		//TODO: fail
	}
	le.fields[0] = size;
	le.fields[2] = stamp;

	f_seek(copy,-sizeof(logElement),F_SEEK_CUR);

	f_write(&le,1,sizeof(logElement),copy);




	f_seek(fp, eentry*sizeof(logElement),F_SEEK_SET);
	holdLocation = f_tell(copy)/sizeof(logElement);
	copyFile(fp,copy,14,-1);


	f_seek(copy,(holdLocation)*sizeof(logElement),F_SEEK_SET);

	f_seek(data2,0,F_SEEK_END);
	size = f_tell(data2)/DATA_FILE_LINE_SIZE(tlm);
	stamp = GetDataTimeStamp(data2,0,tlm);


	if(f_read(&le , 1 , sizeof(logElement),fp) != sizeof(logElement)){
		//TODO: fail
	}
	le.fields[0] = size;
	le.fields[2] = stamp;

	f_seek(copy,-sizeof(logElement),F_SEEK_CUR);
	f_write(&le,1,sizeof(logElement),copy);
}
void wipeInterval(char* dirpath,F_FILE* fp,int  sentry ,int eentry){
	logElement le;
	char deletePath[256];
	while(sentry <= eentry){
		readlogEntry(fp,sentry,&le);
		sprintf(deletePath,"%s/%d",dirpath,le.fields[1]);
		f_delete(deletePath);
	}
}

void deleteAndUpdateLog(unsigned int from , unsigned int to , tlm_type_t tlm , char* dirpath){
    F_FILE* fp;
    char dataname1[256] ;
    char dataname2[256];
    F_FILE *data1, *data2;
    giveLogOfDir(dirpath,&fp,"r+");
    if(fp == NULL){
        //TODO: handle file not opening
    }
    f_seek(fp,0,F_SEEK_END);
    int sizeOfLog = f_tell(fp)/sizeof(logElement);
    int sentry = searchFileInLog(fp,from); // start entry in log
    int eentry = searchFileInLog(fp,to); // end entry in log
    if(sentry == -1 && eentry == sizeOfLog){
        WipeDirectory(dirpath);
    }
    else{
        // we need to delete all files upto the eentry'th file in log. then we need to update eentry'th file (and its log entry).
        wipeInterval(dirpath,fp, sentry , eentry); // delete all files between these inecies.
        ///////

        if(sentry != -1){
			giveFileFromLog(dirpath,fp,sentry, dataname1); // give name of first file. needs to be freed after use
			if(dataname1 == NULL){
				//TODO: handle
				printf("did not open data file: deleteAndUpdateLog (0)\n");
			}
			data1 = cutDataByTimeStamp(dataname1,from,tlm,FALSE);
			// this variable holds the entry number in the data file where (to < entry[dataIndex+1] or entry[dataIndex+1] == eof).
        }
        if(eentry != sizeOfLog){
			giveFileFromLog(dirpath,fp,sentry,dataname2);
			if(dataname2 == NULL){
				//TODO: handle
				printf("did not open data file: deleteAndUpdateLog (1)\n");
			}
			data2 = cutDataByTimeStamp(dataname2,to,tlm,TRUE);
        }
        updateLogByFiles(fp,data1,sentry,data2,eentry,tlm,dirpath);
        f_close(data1);
        f_close(data2);
        // this variable holds the entry number in the data file where (to < entry[dataIndex+1] or entry[dataIndex+1] == eof).

    }




}


void deleteData(tlm_type_t tlm , unsigned int from , unsigned int to){
    // we start with the same algorithm as in findData to find optimal interval.

    Time fromT, toT;
    char path[256];
    if(findInterval(from ,to ,&fromT ,&toT,tlm)){
        //TODO: handle
        printf("failed to obtain interval: findData\n");
        return;
    }

    if(Time_diff(&toT ,&fromT) == TIMEDIFF_INVALID){ // same as before, different years will cuase Timediff to return an error. also toT < fromT.
    }

    //assuming fromT < toT and they are of the same year.

    if(sameDay(&toT,&fromT)){
    	givePathOfTime( tlm , &fromT , path);
        deleteAndUpdateLog(secondsInDay(&fromT),secondsInDay(&toT),tlm,path);
        return;
    }

    givePathOfTime( tlm , &fromT , path);
    deleteAndUpdateLog(secondsInDay(&fromT),90000,tlm,path); //delete from first day directory.
    updateTimeToNextDay(&fromT);
    while(Time_diff(&toT ,&fromT) != TIMEDIFF_INVALID){ // delete all directories corresponding to days between start to end.
    	givePathOfTime( tlm , &fromT , path);
    	WipeDirectory(path);
        updateTimeToNextDay(&fromT);
    }
    givePathOfTime( tlm , &fromT , path);
    deleteAndUpdateLog(secondsInDay(&fromT),secondsInDay(&toT),tlm,path); // delete from last day directory.

    return;
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


/**
#define LOG_NAME "log"

void Log(char* msg){

    F_FILE* file_handle = f_open(LOG_NAME,"a");
    f_printf(file_handle,"%s\n",msg);
    f_close(file_handle);
    return;
}*/




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

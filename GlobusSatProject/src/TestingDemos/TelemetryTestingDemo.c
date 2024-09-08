#include "TelemetryTestingDemo.h"
#include "hcc/api_fat.h"
#include "hal/Utility/util.h"

void print_file_contents(char *file_path) {
	int data_size, time;
	char buf[50];

	printf("enter size of data bytes\r\n");
	UTIL_DbguGetIntegerMinMax(&data_size, 0, 90000);

	F_FILE *file = f_open(file_path, "r");
    if (file == NULL) {
        printf("Error opening the file.\n");
        return;
    }
    printf("printing file:\r\n");

    while(f_read(&time,1,sizeof(int) , file) == sizeof(int)){
		f_read(buf,1,data_size,file);
		printf("\n\r");
		printf("%d: ",time);
		hex_print(buf,data_size);
	}

    f_close(file);
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
				printf("%.8s\r\n",find.name); //printf("%s\n",find.name);
			}
			while(!f_findnext(&find));
		}
	}
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


Boolean explore(){
	char show[256],msg[50];
	F_FIND find;
	unsigned char attr;
	msg[49] = '\0';
	sprintf(show,"DI");
	printf("starting manual explore\nuse q! to exit\r\n");
	printf("contents of dir(%s):\r\n",show);
	DisplayDirectory(show);
	while(1){
		//UTIL_DbguGetString(msg, 49);
		scanf("%s",msg);
		printf("\n\n\n\n");
		if(strcmp(msg,"q!") == 0){
			break;
		}
		else if(strcmp(msg,"..") == 0){
			leaveLevel(show);
		}
		else{
			sprintf(show,"%s/%s",show,msg);
		}
		printf("contents of dir(%s)\n",show);
		if(f_getattr(show,&attr) != F_NO_ERROR){

		}
		if(attr == F_ATTR_DIR){
			DisplayDirectory(show);
		}
		else{
			print_file_contents(show);
			leaveLevel(show);
			DisplayDirectory(show);
		}

	}
	return TRUE;
}

Boolean TestTelemetryCollection() {
	TelemetryCollectorLogic();
	return TRUE;
}

Boolean TestTelemetryRetrievel(){
	int selection;
	int bounds[2];
	printf("enter time stamp of element to be found\r\n");
	UTIL_DbguGetInteger(&selection);
	bounds[0] = selection;
	bounds[1] = selection;
	findData(tlm_eps,bounds[0],bounds[1]);
	return TRUE;
}

Boolean TestTelemetryInterval(){
		int bounds[2];
		printf("enter time stamp From\n\r");
		UTIL_DbguGetInteger(&bounds[0]);
		printf("enter time stamp To\n\r");
		UTIL_DbguGetInteger(&bounds[1]);
		findData(tlm_eps,bounds[0],bounds[1]);
		return TRUE;
}

Boolean testWriteRealTime(){
	xTimerHandle pxTimer;
	int numOfLoops;

	set_real_time(TRUE);
	zeroize();
	printf("please enter the number of entries to be written:\r\n");
	UTIL_DbguGetIntegerMinMax(&numOfLoops, 0, 90000);
	for(int i = 0 ; i < numOfLoops; i++){
		TelemetrySaveEPS(pxTimer);
		vTaskDelay(SECONDS_TO_TICKS(2));
	}
	return TRUE;
}

Boolean testWriteStubTime(){
	xTimerHandle pxTimer;
	int numOfLoops;

	set_real_time(FALSE);
	zeroize();
	printf("please enter the number of entries to be written:\r\n");
	UTIL_DbguGetIntegerMinMax(&numOfLoops, 0, 90000);
	for(int i = 0 ; i < numOfLoops; i++){
		TelemetrySaveEPS(pxTimer);
	}
	return TRUE;
}

Boolean selectAndExecuteTelemetryTest() {
	int selection = 0;

	printf( "\n\r Select the test to perform: \n\r");
	printf("\t 1) Test telemetry collection \n\r");
	printf("\t 2) find telemetry data \n\r");
	printf("\t 3) find telemetry data by interval \n\r");
	printf("\t 4) zeroize \n\r");
	printf("\t 5) explore\n\r");
	printf("\t 6) Test real-time writes to tlm (EPS)\n\r");
	printf("\t 7) Test stub-time writes to tlm (EPS)\n\r");
	printf("\t 8) Go back \n\r");

	while(UTIL_DbguGetIntegerMinMax(&selection, 1, 20) == 0);

	switch(selection)
	{
		case 1:
			if(!TestTelemetryCollection()) {
				printf("TestTelemetryCollection failed");
			}
			break;
		case 2:

			if(!TestTelemetryRetrievel()) {
					printf("TestTelemetryRetrievel failed");
			}
				break;
		case 3:
			if(!TestTelemetryInterval()) {
				printf("TestTelemetryRetrievel by interval failed");
			}
			break;
		case 4:
			zeroize();
			break;
		case 5:
			if(!explore()){
				printf("explore failed");
			}
			break;
		case 6:
			if(!testWriteRealTime()){
				printf("explore failed");
			}
			break;

		case 7:
			if(!testWriteStubTime()){
				printf("explore failed");
			}
			break;

		case 8:
			return FALSE;

		default:
			break;
	}

	return TRUE;
}

Boolean MainTelemetryTestBench() {
	Boolean offerMoreTests = FALSE;

	while(1)
	{
		offerMoreTests = selectAndExecuteTelemetryTest();

		if(offerMoreTests == FALSE)
		{
			break;
		}
	}
	return TRUE;
}

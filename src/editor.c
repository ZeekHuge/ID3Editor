//*************************************************************
//
//	This code is written by ZeekHuge, as an exercise to understand
//	and learn about the de facto mp3 file metadata standard i.e. ID3
//
//	This program have been throughly tested for 
//	ID3v3.0, though it is not able to read all 
//	the frames but only some important data frames.
//	
//	The program can also edit these frames.
//	
//	Limitaions: 
//				1.Files with Extended header are not supported
//				2.Frames with compressed or encrypted data will be skipped
//				3.Comments in english (eng) language are only supported
//				4.Only UTF-16 and ISO-8859-1 encodings are supported 
//
//	Supported frames are :
//					1. AENC - Audio Encryption 			2. UFID - Unique file Identifier
//					3. COMM - Comments					4. TIT2 - Title
//					5. TPE1 - Lead Singer				6. TPE2 - Band
//					7. TALB - Album-Movie 				8. TCOM - Composer
//					9. TIT3 - Subtitle-Description		10.TIME - Time
//					11.TIT1 - Content 					12.TLEN - Lenth
//					13.TOPE - Original artist 			14.TYER - Year
//					15.TSIZ - Size
//
//
//
//
//	To compile the program :
//							Use gcc compiler (recommended) to compile this source
//							code (source file - editor.c)
//
//	Example : gcc editor.c -o Editor
//
//
//
//
// 	To Start the program : run the Editor command n your terminal/CMD with the file 
//							parameter
//	
//	Example: Editor fileToEdit.mp3 (WINDOWS)
//	Example: ./Editor fileToEdit.mp3 (LINUX) 
//
//
//******************************************************************





#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define here printf("-----here-----");


char frameArray[15][5] = {0} ;
int frameNumber, frameOffset[15] = {0}, frameSize[15] ={0};

struct id3_header
{
	int version;
	int revision;
	int flag_unsynchronization;
	int flag_extendedHeader;
	int flag_experimentalIndicator;
	int flag_footerPresent;
	int tag_size;
};

struct id3_frameHeader
{
	char ID[5];
	int size;
	int flag_tagAlterPreservation;
	int flag_fileAltePreservation;
	int flag_readOnly;
	int flag_compression;
	int flag_encryption;
	int flag_groupIdentity;
};

struct editableFrameData
{
	int numberOfDataItems;
	int offset[3];
	char encoding;
}editableFrameDataObj;


void clearScreen(){
	
	#ifdef __linux__	
		system("clear");
	#else
		system("cls");
	#endif
}



int puts_modified(char str[], FILE* fl, char encoding, int offset){
	
	fseek(fl, offset, SEEK_SET);

	int i=0,len = strlen(str);

	if (encoding == 1){
		putc(0xFF, fl);
		putc(0xFE, fl);
		putc(str[0], fl);
		i = 1;
		while(i <  len){
			putc(0, fl);
			putc(str[i],fl);
			i++;
		}
		putc(0, fl);
	}
	else{
		while(i <  len){
			putc(str[i],fl);
			i++;
		}	
	}
	
	return ftell(fl);	
}

int isSupported(FILE *fl){
	
	if (fl == NULL){
		fprintf(stderr, "Err in func isSupported");
		return -1;
	}

	char str[5];

	fgets(str, 4, fl);
	fprintf(stderr, "isSupproted function found-%s\n",str);

	if (strcmp("ID3",str)==0)
	{	return 1 ;}
	else
	{ 	return 0 ;}
} 



struct id3_header readHeader (FILE* fl){

	rewind (fl);
	fseek (fl,	3, SEEK_SET);

	struct id3_header obj ;
	char ch;
	int temp ;

	obj.tag_size =0;

	ch = fgetc(fl);
	obj.version = (int) ch ;
	
	ch = fgetc(fl);
	obj.revision = (int) ch ;

	ch = fgetc(fl);
	
	if (0b10000000 & ch) 	obj.flag_unsynchronization = 1;
	else 					obj.flag_unsynchronization = 0;

	if (0b01000000 & ch) 	obj.flag_extendedHeader = 1;
	else 					obj.flag_extendedHeader = 0;

	if (0b00100000 & ch) 	obj.flag_experimentalIndicator = 1;
	else 					obj.flag_experimentalIndicator = 0;

	if (0b00010000 & ch) 	obj.flag_footerPresent = 1;
		else 					obj.flag_footerPresent = 0;

	temp = (int) fgetc(fl);
	temp = temp << 21;
	obj.tag_size = obj.tag_size | temp ;
	

	temp = (int) fgetc(fl);
	temp = temp << 14;
	obj.tag_size = obj.tag_size | temp ;


	temp = (int) fgetc(fl);
	temp = temp << 7;
	obj.tag_size = obj.tag_size | temp ;


	temp = (int) fgetc(fl);
	obj.tag_size = obj.tag_size | temp ;


	return obj;
}



void printfHeader (struct id3_header obj){

	printf("-----------------Header information -------------------------------\n\n");
	printf("ID3 Version= %d and revision= %d\n",obj.version,obj.revision);
	if (obj.version != 3 ){
		printf("Unsupported ID3 version\n");
	}
	
	if (obj.flag_unsynchronization) 		printf(">Unsynchronized\n");
	else									printf(">Synchronized\n");
	
	if (obj.flag_extendedHeader) 			printf(">Extended Header Pesent\n");
	else									printf(">Extended Header Not Pesent\n");
	
	if (obj.flag_experimentalIndicator) 	printf(">Experimental Tag\n");
	else									printf(">Not an Experimental Tag\n");
	
	if (obj.flag_footerPresent) 			printf(">Footer Present\n");
	else									printf(">Footer Not Present\n");

	printf(">Size of Tag = %d bytes", obj.flag_footerPresent? obj.tag_size+20:obj.tag_size+10);

	printf("\n------------------------------------------------------------------\n");
}



struct id3_frameHeader readFrameHeader (FILE* fl, int pos){

	fseek(fl, pos, SEEK_SET);

	struct id3_frameHeader obj;
	char ch ;
	int i ;

	for (i=0; i <= 3; i++){
		ch=fgetc(fl);
		obj.ID[i]=ch;
	}
	obj.ID[4]='\0';


	i=(int)fgetc(fl);
	obj.size = i << 24;

	i=(int)fgetc(fl);
	obj.size = obj.size | (i << 16);

	i=(int)fgetc(fl);
	obj.size = obj.size | (i << 8);

	i=(int)fgetc(fl);
	obj.size = obj.size | i ;

	ch = fgetc(fl);
	
	if (0b10000000 & ch)				obj.flag_tagAlterPreservation =1;
	else								obj.flag_tagAlterPreservation =0;

	if (0b01000000 & ch)				obj.flag_fileAltePreservation =1;
	else								obj.flag_fileAltePreservation =0;

	if (0b00100000 & ch)				obj.flag_readOnly =1;
	else								obj.flag_readOnly =0;
	
	ch=fgetc(fl);

	if (0b10000000 & ch)				obj.flag_compression =1;
	else								obj.flag_compression =0;

	if (0b01000000 & ch)				obj.flag_encryption =1;
	else 								obj.flag_encryption =0;

	if (0b00100000 & ch)				obj.flag_groupIdentity =1;
	else								obj.flag_groupIdentity =0;

	return obj;
}



char printfFrame( int tagSize, FILE* fl, int pos ){

	int currentFramePos = pos ;
	int count;
	char ch,encoding,str[5] ;
	struct id3_frameHeader frameHeaderObj;
	
	printf("-----------------------------Frame Details-----------------------\n\n");

	while(currentFramePos < tagSize){

		frameHeaderObj = readFrameHeader (fl, currentFramePos);
		fprintf(stderr,"frameID:%s\n",frameHeaderObj.ID);
		fprintf(stderr, "frameSize:%d\n",frameHeaderObj.size);

		if (frameHeaderObj.flag_compression && frameHeaderObj.flag_encryption){
			fprintf(stderr, "Encrypted %s\n",frameHeaderObj.ID );
			printf("**Sory, I do not support Encrypted or Compressed frames please try another file\nExiting...\n");
			exit (0);	
		}

		fseek(fl, currentFramePos + 10 ,SEEK_SET);

		if (strcmp(frameHeaderObj.ID,"AENC")==0){
			
			strcpy(frameArray[frameNumber],frameHeaderObj.ID);
			frameOffset[frameNumber] = currentFramePos;
			frameSize[frameNumber]= frameHeaderObj.size;
			editableFrameDataObj.numberOfDataItems = 1;
			editableFrameDataObj.encoding = 1;

			printf("%d. Audio Encryption Details : \n",frameNumber++);

			count = frameHeaderObj.size;
			ch = fgetc(fl);

			editableFrameDataObj.offset[0] = ftell(fl);

			printf(">Owner Identifier : ");
			while(ch != 0){
				printf("%c",ch);	count-- ;
			}
			printf("\n>Preview start [can't be edited]: %c%c\n",fgetc(fl),fgetc(fl));
			printf("\n>Preview lenght [can't be edited] : %c%c\n",fgetc(fl),fgetc(fl));
			count -= 4;

			printf("Encryption info in binary form [Can't be Edited]: \n");
			for( ; count > 0 ; count--) {
				ch=fgetc(fl);
				printf("%c\n", ch);
			} 
		}

		else if (strcmp(frameHeaderObj.ID,"UFID")==0){
			
			strcpy(frameArray[frameNumber],frameHeaderObj.ID);
			frameOffset[frameNumber] = currentFramePos;
			frameSize[frameNumber]= frameHeaderObj.size;
			editableFrameDataObj.numberOfDataItems=2;
			editableFrameDataObj.encoding = 1;

			count = frameHeaderObj.size ;
			printf("%d. Unique file identifier : ",frameNumber++);
			printf("Owner Identifier : ");
			ch = fgetc(fl);	count-- ;
			editableFrameDataObj.offset[0] = ftell(fl);
			while(ch != 0){
				printf("%c",ch );
				ch = fgetc(fl); count-- ;
			}
			editableFrameDataObj.offset[0] = ftell(fl);
			printf("\nIdentifier : ");
			for( ; count > 0 ; count--) {
				ch = fgetc(fl);
				printf("%c",ch );
			}
			printf("\n");
		}

		else if(strcmp(frameHeaderObj.ID,"COMM")==0){

			strcpy(frameArray[frameNumber],frameHeaderObj.ID);
			frameOffset[frameNumber] = currentFramePos;
			frameSize[frameNumber]= frameHeaderObj.size;
			editableFrameDataObj.numberOfDataItems = 1;
			printf("%d. Comment :",frameNumber++);
			
			encoding = fgetc(fl);
			
			for(count = 0; count < 3; count++)
			str[count] = fgetc(fl);	
			str[3]='\0';		
			
			if (encoding==1){
				count = (frameHeaderObj.size - 4)/2 - 1;
				ch = fgetc(fl);ch = fgetc(fl);
				editableFrameDataObj.encoding = 1;
			}
			else{
				count = frameHeaderObj.size - 4 ;
				editableFrameDataObj.encoding = 0;
				}

			if (strcmp(str,"eng")!=0){
				fprintf(stderr, "Language other than eng\n");
				printf("****Language other than eng *****\n");
				count = 0 ;
			}


			editableFrameDataObj.offset[0] = ftell(fl);
			for( ; count > 0 ; count--)  {
					
				ch = fgetc(fl);
				printf("%c",ch);
				if (encoding == 1)				ch = fgetc(fl);
				else if (encoding == 0 )		;
				else 						{fprintf(stderr, "unSupportedEncoding: %s\n", frameHeaderObj.ID); break;}
			}
			printf("\n\n");
		} 	

				
		else if(strcmp(frameHeaderObj.ID,"TIT2")==0){

			strcpy(frameArray[frameNumber],frameHeaderObj.ID);
			frameOffset[frameNumber] = currentFramePos;
			frameSize[frameNumber]= frameHeaderObj.size;
			editableFrameDataObj.numberOfDataItems=1;
			printf("%d. Title-SongName-Content description : ",frameNumber++);

			encoding = fgetc(fl);
			if (encoding==1){
				count = (frameHeaderObj.size - 1)/2 - 1;
				fgetc(fl);fgetc(fl); 
				editableFrameDataObj.encoding=1;
			}
			else{
				count = frameHeaderObj.size - 1 ;
				editableFrameDataObj.encoding=1;
				}

			editableFrameDataObj.offset[0]=ftell(fl);
			for( ; count > 0 ; count--)  {
					
				ch = fgetc(fl);
				printf("%c",ch);
				if (encoding == 1)				ch = fgetc(fl);
				else if (encoding == 0 )		;
				else 						{fprintf(stderr, "unSupportedEncoding: %s\n", frameHeaderObj.ID); break;}
				
			}
			printf("\n\n");		}

		else if(strcmp(frameHeaderObj.ID,"TPE1")==0){
			
			strcpy(frameArray[frameNumber],frameHeaderObj.ID);
			frameOffset[frameNumber] = currentFramePos;
			frameSize[frameNumber]= frameHeaderObj.size;
			editableFrameDataObj.numberOfDataItems=1;

			printf("%d. Lead artist-performer : ",frameNumber++);
		
			encoding = fgetc(fl);
			if (encoding==1){
				count = (frameHeaderObj.size - 1)/2 - 1;
				fgetc(fl);fgetc(fl); 
				editableFrameDataObj.encoding=1;
			}
			else{
				count = frameHeaderObj.size - 1 ;
				editableFrameDataObj.encoding=1;
				}

			editableFrameDataObj.offset[0]=ftell(fl);
			for( ; count > 0 ; count--)  {
					
				ch = fgetc(fl);
				printf("%c",ch);
				if (encoding == 1)				ch = fgetc(fl);
				else if (encoding == 0 )		;
				else 						{fprintf(stderr, "unSupportedEncoding: %s\n", frameHeaderObj.ID); break;}
				
			}
			printf("\n\n");
		}

		else if(strcmp(frameHeaderObj.ID,"TPE2")==0){
			
			strcpy(frameArray[frameNumber],frameHeaderObj.ID);
			frameOffset[frameNumber] = currentFramePos;
			frameSize[frameNumber]= frameHeaderObj.size;
			editableFrameDataObj.numberOfDataItems = 1;

			printf("%d. Band-Orchestra : ",frameNumber++);
			
			encoding = fgetc(fl);
			if (encoding==1){
				count = (frameHeaderObj.size - 1)/2 - 1;
				fgetc(fl);fgetc(fl); 
				editableFrameDataObj.encoding=1;
			}
			else{
				count = frameHeaderObj.size - 1 ;
				editableFrameDataObj.encoding=1;
				}

			editableFrameDataObj.offset[0]=ftell(fl);
			for( ; count > 0 ; count--)  {
					
				ch = fgetc(fl);
				printf("%c",ch);
				if (encoding == 1)				ch = fgetc(fl);
				else if (encoding == 0 )		;
				else 						{fprintf(stderr, "unSupportedEncoding: %s\n", frameHeaderObj.ID); break;}
				
			}
			printf("\n\n");
		}

		else if(strcmp(frameHeaderObj.ID,"TALB")==0){
			
			strcpy(frameArray[frameNumber],frameHeaderObj.ID);
			frameOffset[frameNumber] = currentFramePos;
			frameSize[frameNumber]= frameHeaderObj.size;
			editableFrameDataObj.numberOfDataItems = 1;

			printf("%d. Album-Movie : ",frameNumber++);
			
			encoding = fgetc(fl);
			if (encoding==1){
				count = (frameHeaderObj.size - 1)/2 - 1;
				fgetc(fl);fgetc(fl); 
				editableFrameDataObj.encoding=1;
			}
			else{
				count = frameHeaderObj.size - 1 ;
				editableFrameDataObj.encoding=1;
				}

			editableFrameDataObj.offset[0]=ftell(fl);
			for( ; count > 0 ; count--)  {
					
				ch = fgetc(fl);
				printf("%c",ch);
				if (encoding == 1)				ch = fgetc(fl);
				else if (encoding == 0 )		;
				else 						{fprintf(stderr, "unSupportedEncoding: %s\n", frameHeaderObj.ID); break;}
				
			}
			printf("\n\n");
		}

		else if(strcmp(frameHeaderObj.ID,"TCOM")==0){
			
			strcpy(frameArray[frameNumber],frameHeaderObj.ID);
			frameOffset[frameNumber] = currentFramePos;
			frameSize[frameNumber]= frameHeaderObj.size;
			editableFrameDataObj.numberOfDataItems = 1;

			printf("%d. Cmposers : ",frameNumber++);
			
			encoding = fgetc(fl);
			if (encoding==1){
				count = (frameHeaderObj.size - 1)/2 - 1;
				fgetc(fl);fgetc(fl); 
				editableFrameDataObj.encoding=1;
			}
			else{
				count = frameHeaderObj.size - 1 ;
				editableFrameDataObj.encoding=1;
				}

			editableFrameDataObj.offset[0]=ftell(fl);
			for( ; count > 0 ; count--)  {
					
				ch = fgetc(fl);
				printf("%c",ch);
				if (encoding == 1)				ch = fgetc(fl);
				else if (encoding == 0 )		;
				else 						{fprintf(stderr, "unSupportedEncoding: %s\n", frameHeaderObj.ID); break;}
				
			}
			printf("\n\n");
		}
		else if(strcmp(frameHeaderObj.ID,"TIT3")==0){
			
			strcpy(frameArray[frameNumber],frameHeaderObj.ID);
			frameOffset[frameNumber] = currentFramePos;
			frameSize[frameNumber]= frameHeaderObj.size;
			editableFrameDataObj.numberOfDataItems = 1;

			printf("%d. Subtitle-Description : ",frameNumber++);
			
			encoding = fgetc(fl);
			if (encoding==1){
				count = (frameHeaderObj.size - 1)/2 - 1;
				fgetc(fl);fgetc(fl); 
				editableFrameDataObj.encoding=1;
			}
			else{
				count = frameHeaderObj.size - 1 ;
				editableFrameDataObj.encoding=1;
				}

			editableFrameDataObj.offset[0]=ftell(fl);
			for( ; count > 0 ; count--)  {
					
				ch = fgetc(fl);
				printf("%c",ch);
				if (encoding == 1)				ch = fgetc(fl);
				else if (encoding == 0 )		;
				else 						{fprintf(stderr, "unSupportedEncoding: %s\n", frameHeaderObj.ID); break;}
				
			}
			printf("\n\n");
		}
		else if(strcmp(frameHeaderObj.ID,"TIME")==0){
			
			strcpy(frameArray[frameNumber],frameHeaderObj.ID);
			frameOffset[frameNumber] = currentFramePos;
			frameSize[frameNumber]= frameHeaderObj.size;
			editableFrameDataObj.numberOfDataItems = 1;

			printf("%d. Time for recording in format HHMM : ",frameNumber++);
			
			encoding = fgetc(fl);
			if (encoding==1){
				count = (frameHeaderObj.size - 1)/2 - 1;
				fgetc(fl);fgetc(fl); 
				editableFrameDataObj.encoding=1;
			}
			else{
				count = frameHeaderObj.size - 1 ;
				editableFrameDataObj.encoding=1;
				}

			editableFrameDataObj.offset[0]=ftell(fl);
			for( ; count > 0 ; count--)  {
					
				ch = fgetc(fl);
				printf("%c",ch);
				if (encoding == 1)				ch = fgetc(fl);
				else if (encoding == 0 )		;
				else 						{fprintf(stderr, "unSupportedEncoding: %s\n", frameHeaderObj.ID); break;}
				
			}
			printf("\n\n");
		}

		else if(strcmp(frameHeaderObj.ID,"TIT1")==0){
			
			strcpy(frameArray[frameNumber],frameHeaderObj.ID);
			frameOffset[frameNumber] = currentFramePos;
			frameSize[frameNumber]= frameHeaderObj.size;
			editableFrameDataObj.numberOfDataItems = 1;

			printf("%d. Content Group Description : ", frameNumber++);
			
			encoding = fgetc(fl);
			if (encoding==1){
				count = (frameHeaderObj.size - 1)/2 - 1;
				fgetc(fl);fgetc(fl); 
				editableFrameDataObj.encoding=1;
			}
			else{
				count = frameHeaderObj.size - 1 ;
				editableFrameDataObj.encoding=1;
				}

			editableFrameDataObj.offset[0]=ftell(fl);
			for( ; count > 0 ; count--)  {
					
				ch = fgetc(fl);
				printf("%c",ch);
				if (encoding == 1)				ch = fgetc(fl);
				else if (encoding == 0 )		;
				else 						{fprintf(stderr, "unSupportedEncoding: %s\n", frameHeaderObj.ID); break;}
				
			}
			printf("\n\n");
		}

		else if(strcmp(frameHeaderObj.ID,"TLEN")==0){
		
			strcpy(frameArray[frameNumber],frameHeaderObj.ID);
			frameOffset[frameNumber] = currentFramePos;
			frameSize[frameNumber]= frameHeaderObj.size;
			editableFrameDataObj.numberOfDataItems = 1;

			printf("%d. Lenth of Audio in millis : ",frameNumber++);
			
			encoding = fgetc(fl);
			if (encoding==1){
				count = (frameHeaderObj.size - 1)/2 - 1;
				fgetc(fl);fgetc(fl); 
				editableFrameDataObj.encoding=1;
			}
			else{
				count = frameHeaderObj.size - 1 ;
				editableFrameDataObj.encoding=1;
				}

			editableFrameDataObj.offset[0]=ftell(fl);
			for( ; count > 0 ; count--)  {
					
				ch = fgetc(fl);
				printf("%c",ch);
				if (encoding == 1)				ch = fgetc(fl);
				else if (encoding == 0 )		;
				else 						{fprintf(stderr, "unSupportedEncoding: %s\n", frameHeaderObj.ID); break;}
				
			}
			printf("\n\n");
			}

		else if(strcmp(frameHeaderObj.ID,"TOPE")==0){
			
			strcpy(frameArray[frameNumber],frameHeaderObj.ID);
			frameOffset[frameNumber] = currentFramePos;
			frameSize[frameNumber]= frameHeaderObj.size;
			editableFrameDataObj.numberOfDataItems = 1;

			printf("%d. Oiginal artist-performer : ",frameNumber++);
			
			encoding = fgetc(fl);
			if (encoding==1){
				count = (frameHeaderObj.size - 1)/2 - 1;
				fgetc(fl);fgetc(fl); 
				editableFrameDataObj.encoding=1;
			}
			else{
				count = frameHeaderObj.size - 1 ;
				editableFrameDataObj.encoding=1;
				}

			editableFrameDataObj.offset[0]=ftell(fl);
			for( ; count > 0 ; count--)  {
					
				ch = fgetc(fl);
				printf("%c",ch);
				if (encoding == 1)				ch = fgetc(fl);
				else if (encoding == 0 )		;
				else 						{fprintf(stderr, "unSupportedEncoding: %s\n", frameHeaderObj.ID); break;}
				
			}
			printf("\n\n");
		}


		else if(strcmp(frameHeaderObj.ID,"TYER")==0){
		
			strcpy(frameArray[frameNumber],frameHeaderObj.ID);
			frameOffset[frameNumber] = currentFramePos;
			frameSize[frameNumber]= frameHeaderObj.size;
			editableFrameDataObj.numberOfDataItems = 1;

			printf("%d. Year of release : ",frameNumber++);
			
			encoding = fgetc(fl);
			if (encoding==1){
				count = (frameHeaderObj.size - 1)/2 - 1;
				fgetc(fl);fgetc(fl); 
				editableFrameDataObj.encoding=1;
			}
			else{
				count = frameHeaderObj.size - 1 ;
				editableFrameDataObj.encoding=1;
				}

			editableFrameDataObj.offset[0]=ftell(fl);
			for( ; count > 0 ; count--)  {
					
				ch = fgetc(fl);
				printf("%c",ch);
				if (encoding == 1)				ch = fgetc(fl);
				else if (encoding == 0 )		;
				else 						{fprintf(stderr, "unSupportedEncoding: %s\n", frameHeaderObj.ID); break;}
				
			}
			printf("\n\n");
		}

		else if(strcmp(frameHeaderObj.ID,"TSIZ")==0){
			
			strcpy(frameArray[frameNumber],frameHeaderObj.ID);
			frameOffset[frameNumber] = currentFramePos;
			frameSize[frameNumber]= frameHeaderObj.size;
			editableFrameDataObj.numberOfDataItems = 1;
			
			printf("%d. Size of Audio file : ",frameNumber++);
			
			encoding = fgetc(fl);
			if (encoding==1){
				count = (frameHeaderObj.size - 1)/2 - 1;
				fgetc(fl);fgetc(fl); 
				editableFrameDataObj.encoding=1;
			}
			else{
				count = frameHeaderObj.size - 1 ;
				editableFrameDataObj.encoding=1;
				}

			editableFrameDataObj.offset[0]=ftell(fl);
			for( ; count > 0 ; count--)  {
					
				ch = fgetc(fl);
				printf("%c",ch);
				if (encoding == 1)				ch = fgetc(fl);
				else if (encoding == 0 )		;
				else 						{fprintf(stderr, "unSupportedEncoding: %s\n", frameHeaderObj.ID); break;}
				
			}
			printf("\n\n");
		}
		


		if(frameHeaderObj.size) 			currentFramePos+=10+frameHeaderObj.size ;
		else								break;
	}
	printf("----------------------------------------------------------------\n\n");
	return 0;
}


int writeToFrame(int choice_int, char outputFileName[], FILE* inputFile){

	int writeTill, size, positn, temp;
	char *ptr = (char*)&size ,ch, str[50];
	

	FILE *outputFile = fopen(outputFileName, "wb+");
	if(outputFile==NULL){
		printf("Couldnt Open file t write\n");
		exit(0);
	}

	if (strcmp(frameArray[choice_int],"TIME")==0)
	{
		fprintf(stderr, "Couldnt edit: %s\n" ,frameArray[choice_int] );
		printf("Unsupprted Format\nExiting\n");
		exit(0);
	}
	else if (strcmp(frameArray[choice_int],"UFID")==0)
	{
		fprintf(stderr, "Couldnt edit: %s\n" ,frameArray[choice_int] );
		printf("Unsupprted Format\nExiting\n");
		exit(0);
	}
	else if (strcmp(frameArray[choice_int],"COMM")==0)
	{
		fprintf(stderr, "Couldnt edit: %s\n" ,frameArray[choice_int] );
		printf("Unsupprted Format\nExiting\n");
		exit(0);
	}
	else if (strcmp(frameArray[choice_int],"TYER")==0)
	{
		fprintf(stderr, "Couldnt edit: %s\n" ,frameArray[choice_int] );
		printf("Unsupprted Format\nExiting\n");
		exit(0);
	}
	else if (strcmp(frameArray[choice_int],"AENC")==0)
	{
		fprintf(stderr, "Couldnt edit: %s\n" ,frameArray[choice_int] );
		printf("Unsupprted Format\nExiting\n");
		exit(0);
	}
	else {
		rewind(inputFile);
		rewind(outputFile);

		writeTill = frameOffset[choice_int] ;
		
	
		while(writeTill--){
			putc(fgetc(inputFile), outputFile);
		}
		
		positn = puts_modified(frameArray[choice_int],outputFile,0,ftell(outputFile));
		fseek(outputFile, positn, SEEK_SET);

		printf("Enter modification [max 40 chars and WITHOUT SPACES]: ");
		ch = getchar();
		getchar();
		printf("%d\n", ch);
		scanf("%s",str);


		size = editableFrameDataObj.encoding ? ((strlen(str) + 1 )*2) + 1 : strlen(str) + 1 ;
		temp = (size 		& 0b00000000000000000000000001111111);
		temp = ((size<<1) 	& 0b00000000000000000111111100000000) | temp;
		temp = ((size<<2) 	& 0b00000000011111110000000000000000) | temp; 
		temp = ((size<<3) 	& 0b01111111000000000000000000000000) | temp;
		size = temp;

		putc(*(ptr+3),outputFile);putc(*(ptr+2),outputFile);putc(*(ptr+1),outputFile);putc(*(ptr),outputFile);
		
		fseek(inputFile,frameOffset[choice_int]+4+4, SEEK_SET );
		
		writeTill = 2 ;
		while(writeTill--){
			putc(fgetc(inputFile), outputFile);
		}
		putc(editableFrameDataObj.encoding, outputFile);
				
		positn = puts_modified(str, outputFile,editableFrameDataObj.encoding,ftell(outputFile));
		
		fseek(outputFile, positn, SEEK_SET);
		fseek(inputFile,frameOffset[choice_int] + frameSize[choice_int] + 10,SEEK_SET);
		int tm =10;
		int ch;
		while(!feof(inputFile)){
			putc(fgetc(inputFile),outputFile);
		}
	}
	fclose(outputFile);
	return 0;
}




int main(int argc, char *argv[]){

	char choice_char, outputFileName[50];
	int choice_int, i ;
	clearScreen();
	frameNumber = 0;
	FILE * fileToRead ;
	

	freopen("otherFiles/id3Editor.log","w+",stderr);

	if (argc > 1){
		fileToRead = fopen(argv[1],"rb");
		fprintf(stderr, "Starting log for %s\n", argv[1]);		
		if(fileToRead==NULL){
			printf("No such file exist\n");
			fprintf(stderr, "File to open dosnt exists\n");
			exit(0);
		}
		strcpy(outputFileName,argv[1]);
		for(i=strlen(outputFileName); outputFileName[i] != '.' && i >= 0;i-- );
		if ( i >= 0  )		strcpy(&outputFileName[i],"_modified.mp3");
		else				{printf("File should be of .mp3 format\n"); exit(0);}
	}
	else{
	
		printf("To use the program, specify the file\n");
		exit(0);
	}

	

	if (isSupported(fileToRead)){

		fprintf(stderr, "File is supprted\n");

		struct id3_header headerObj1,headerObj2;
		
		headerObj1 = readHeader(fileToRead);
		printfHeader(headerObj1);

		fprintf(stderr, "Printed header\n");

		if (headerObj1.flag_extendedHeader){
			printf("**Sory, I do not support Extended Headers please try another file\nExiting...\n");
			exit (0);
		}
		
		printfFrame (headerObj1.tag_size , fileToRead, 10);

		fprintf(stderr, "Printed header\n");

		printf("Do you wish to edit the tag ? [y/n] :");
		scanf("%c",&choice_char);
		if (choice_char == 'y' || choice_char == 'Y'){

			do{
				freopen(argv[1],"rb",fileToRead);
				clearScreen();
				frameNumber = 0;
				printfFrame(headerObj1.tag_size, fileToRead, 10);
				printf("Enter the number of the frame to edit [ 0 - %d ] :", frameNumber - 1);
				scanf("%d",&choice_int);
				
				if(choice_int < (frameNumber)){

					clearScreen();
					frameNumber = choice_int;
					printfFrame(frameSize[choice_int]+ frameOffset[choice_int], fileToRead, frameOffset[choice_int]);
					writeToFrame(choice_int,outputFileName,fileToRead);
					clearScreen();
					freopen(outputFileName,"rb",fileToRead);
					headerObj2 = readHeader(fileToRead);
					printfHeader(headerObj2);
					printfFrame(headerObj2.tag_size, fileToRead, 10);
					printf("OK ! Successfully edited that\nOutput file is : %s\nWant to edit again [y/n]?",outputFileName);
					getchar();
					scanf("%c",&choice_char);
				}
			} while (choice_char=='y' || choice_char== 'Y');
		}
		else
		{
			printf("I am terminating :)\n");
			exit(0);
		}
	}
	else{

		printf("%s\n", "File Not Supported" );
	}

	fclose(fileToRead);
	fclose(stderr);
	return 0;
}
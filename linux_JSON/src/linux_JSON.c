/*
 ============================================================================
 Name        : linux_JSON.c
 Author      : Raph
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */


#define FILE_BUFFER_MAXLEN 1024*1024


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "linux_json.h"
#include "lib_json/jRead.h"
#include "lib_json/jWrite.h"


struct mValue{      // our application wants the JSON "Numbers" array data
	char mode[100];
	int value;
};


struct JsonCommand{
	int msgID;
	int msgType;
	int msgParam;
	struct mValue msgValue[20];
};



struct FileBuffer{
	unsigned long length;			// length in bytes
	unsigned char *data;			// malloc'd data, free with freeFileBuffer()
};


unsigned long readFileBuffer( char *filename, struct FileBuffer *pbuf, unsigned long maxlen );
void freeFileBuffer( struct FileBuffer *buf );

void testQuery( char * pJson, char *query );



char * exampleJson=
	"{"
	"  \"astring\": \"This is a string\",\n"
	"  \"number1\": 42,\n"
	"  \"number2\":  -123.45,\n"
	"  \"anObject\":{\"one\":1,\"two\":{\"obj2.1\":21,\"obj2.2\":22},\"three\":333},\n"
	"  \"anArray\":[0, \"one\", {\"two.0\":20,\"two.1\":21}, 3, [4,44,444]],\n"
	"  \"isnull\":null,\n"
	"  \"yes\": true,\n"
	"  \"no\":  false\n"
	"}\n";


int main(void) {
	struct JsonCommand myCommand;
	char myInput;
	system("clear");
	puts("!!!Hello World!!!"); /* prints !!!Hello World!!! */

	while(1){
// BASIC TEST
		/*
		struct jReadElement jElement;
		jRead(exampleJson, "{'anObject'{'one'", &jElement );
		printf("My Value: %*.*s\n", jElement.bytelen,jElement.bytelen, jElement.pValue);

		testQuery(exampleJson, "{'anObject' {0");
		testQuery(exampleJson, "{'anObject' {1");
		testQuery(exampleJson, "{'anArray' [1");
		*/
// END BASIC TEST

// FILE
		char *filename= "esjson.json";
		struct FileBuffer json;
		struct jReadElement MsgElement, arrayElement, NetFrom, NetTo, element, objectElement;
		char *query= "{'To'";
		//char *query= "{'Message'";
		//char *query= "{'Message'{'MessageID'";
		//char *query= "{'Message'{'Value'[0";
		//char *query= "{'Message'{'Value'[0{'step'";

		if( readFileBuffer( filename, &json, FILE_BUFFER_MAXLEN ) == 0 )
		{
				printf("Can't open file: %s\n", filename );
		}
		else{
			printf("ReadJsonData...\n");
			jRead( (char *)json.data, "{'To'", &NetTo );
			char msgTo[32], msgFrom[32];
			jRead_string((char *)json.data, "{'To'", msgTo, 15, NULL );
			jRead_string((char *)json.data, "{'From'", msgFrom, 15, NULL );
			printf("Message to   %s   from   %s\n",msgTo, msgFrom );

// MESSAGE ID
			myCommand.msgID= jRead_int((char *)json.data,  "{'Message'{'MsgID'", NULL);
			printf("Message ID: %d\n",myCommand.msgID);

// MESSAGE TYPE
			int i;
			char myDataString[20];

			// Clear string
			for(i=0;i<20;i++) myDataString[i]=0;
			jRead_string((char *)json.data,  "{'Message'{'MsgType'",myDataString,15, NULL);

			if(!strcmp(myDataString, "command")) myCommand.msgType = COMMAND;
			if(!strcmp(myDataString, "request")) myCommand.msgType = REQUEST;
			if(!strcmp(myDataString, "ack")) myCommand.msgType = ACK;
			if(!strcmp(myDataString, "response")) myCommand.msgType = RESPONSE;
			if(!strcmp(myDataString, "event")) myCommand.msgType = EVENT;
			if(!strcmp(myDataString, "negoc")) myCommand.msgType = NEGOC;
			if(!strcmp(myDataString, "error")) myCommand.msgType = ERROR;

			// Clear string
			for(i=0;i<20;i++) myDataString[i]=0;
			jRead_string((char *)json.data,  "{'Message'{'MsgParam'",myDataString,15, NULL);

			if(!strcmp(myDataString, "avancer")) myCommand.msgParam = FORWARD;
			if(!strcmp(myDataString, "reculer")) myCommand.msgParam = BACK;
			if(!strcmp(myDataString, "gauche")) myCommand.msgParam = LEFT;
			if(!strcmp(myDataString, "droit")) myCommand.msgParam = RIGHT;
			if(!strcmp(myDataString, "rotationGa")) myCommand.msgParam = FORWARD_SPIN_LEFT;
			if(!strcmp(myDataString, "rotationDr")) myCommand.msgParam = FORWARD_SPIN_RIGHT;
			if(!strcmp(myDataString, "stop")) myCommand.msgParam = STOP;

			printf("type: %d  param: %d\n",myCommand.msgType,myCommand.msgParam);

// DATA ARRAY
			  jRead((char *)json.data, "{'Message'{'MsgValue'", &element );
			  if( element.dataType == JREAD_ARRAY )
			  {
			      for( i=0; i<element.elements; i++ )    // loop for no. of elements in JSON
			      {
			    	  jRead_string((char *)json.data, "{'Message'{'MsgValue'[*{'mode'", myCommand.msgValue[i].mode, 15, &i );
			    	  myCommand.msgValue[i].value= jRead_long((char *)json.data, "{'Message'{'MsgValue'[*{'val'", &i );
			    	  printf("Name: %s   Data: %d\n", myCommand.msgValue[i].mode, myCommand.msgValue[i].value);
			      }
			      printf("\n");
			  }
		}

// END FILE

		struct jWriteControl jwc;
		char buffer[1024];
		unsigned int buflen= 1024;

		jwOpen( buffer, buflen, JW_OBJECT, JW_PRETTY );		// start root object
			jwObj_string( "To", "ESMG_xxx" );				// add object key:value pairs
			jwObj_string( "From", "ES_xxx" );				// add object key:value pairs
			jwObj_object( "Message" );
				jwObj_int( "msgID", 1 );
				jwObj_string( "MsgType", "ack" );				// add object key:value pairs
				jwObj_string( "MsgParam", "avancer" );				// add object key:value pairs
				jwObj_array( "msgValue" );
					jwArr_object();							// object in array
						jwObj_string( "obj3_one", "one");
						jwObj_int( "int", 1 );
					jwEnd();
					jwArr_object();							// object in array
						jwObj_string( "obj3_two", "two");
						jwObj_int( "int", 5 );
					jwEnd();
				jwEnd();
			jwEnd();

			jwClose();
			//jRead( (char *)buffer, "{", &MsgElement);
			printf("\n\n RELECTURE: %s", buffer);




		myInput=getchar();
		if(myInput=='q') break;
	}
	printf("bye, bye \n");

	return EXIT_SUCCESS;
}



//-------------------------------------------------
// Do a query and print the results
//anObject
void testQuery( char * pJson, char *query )
{
	struct jReadElement jElement;
	jRead( pJson, query, &jElement );
	printf( "Query: \"%s\"\n", query );
	printf( "return: %d= %s\n", jElement.error, jReadErrorToString(jElement.error) );
	printf( " dataType = %s\n", jReadTypeToString(jElement.dataType) );
	printf( " elements = %d\n", jElement.elements );
	printf( " bytelen  = %d\n", jElement.bytelen );
	printf( " value    = %*.*s\n\n", jElement.bytelen,jElement.bytelen, jElement.pValue );
}


// readFileBuffer
// - reads file into a malloc'd buffer with appended '\0' terminator
// - limits malloc() to maxlen bytes
// - if file size > maxlen then the function fails (returns 0)
//
// returns: length of file data (excluding '\0' terminator)
//          0=empty/failed
//
unsigned long readFileBuffer( char *filename, struct FileBuffer *pbuf, unsigned long maxlen )
{
  FILE *fp;
  int i;

	if( (fp=fopen(filename, "rb")) == NULL )
	{
		printf("Can't open file: %s\n", filename );
		return 0;
	}
	// find file size and allocate buffer for JSON file
	fseek(fp, 0L, SEEK_END);
	pbuf->length = ftell(fp);
	if( pbuf->length >= maxlen )
	{
		fclose(fp);
		return 0;
	}
	// rewind and read file
	fseek(fp, 0L, SEEK_SET);
	pbuf->data= (unsigned char *)malloc( pbuf->length + 1 );
	memset( pbuf->data, 0, pbuf->length+1 );	// +1 guarantees trailing \0

	i= fread( pbuf->data, pbuf->length, 1, fp );
	fclose( fp );
	if( i != 1 )
	{
		freeFileBuffer( pbuf );
		return 0;
	}
	return pbuf->length;
}

// freeFileBuffer
// - free's buffer space and zeros it
//
void freeFileBuffer( struct FileBuffer *buf )
{
	if( buf->data != NULL )
		free( buf->data );
	buf->data= 0;
	buf->length= 0;
}

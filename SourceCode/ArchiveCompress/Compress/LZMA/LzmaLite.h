
#ifndef __SDORICA_H
#define __SDORICA_H


#include "Precomp.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "CpuArch.h"

#include "Alloc.h"
#include "7zFile.h"
#include "7zVersion.h"
#include "LzmaDec.h"
#include "LzmaEnc.h"


static const char * const kCantReadMessage = "Can not read input file";
static const char * const kCantWriteMessage = "Can not write output file";
static const char * const kCantAllocateMessage = "Can not allocate memory";
static const char * const kDataErrorMessage = "Data error";



typedef struct {
	CHAR8*	ArgStr;
	UINT8	ArgNumNeed;
	//EFI_STATUS (*func)(int argc, char ** argv);
	int (*func)(int argc, const char ** argv);
} ARG_INPUT_LIST;

//int MY_CDECL LzmaCompres(int numArgs, const char *args[]);
int LzmaCompres(int numArgs, const char *args[]);

static int main2(int numArgs, const char *args[], char *rs);

static SRes Encode(ISeqOutStream *outStream, ISeqInStream *inStream, UInt64 fileSize, char *rs);

static SRes Decode(ISeqOutStream *outStream, ISeqInStream *inStream);

static SRes Decode2(CLzmaDec *state, ISeqOutStream *outStream, ISeqInStream *inStream, UInt64 unpackSize);

static int PrintUserError(char *buffer);

static int PrintErrorNumber(char *buffer, SRes val);

static int PrintError(char *buffer, const char *message);

static void PrintHelp(char *buffer);

//static void ShowHelpMsg(void);

#endif
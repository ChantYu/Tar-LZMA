
#include "LzmaLite.h"

ARG_INPUT_LIST ArgInputList[] = {
	{"-e",3 ,LzmaCompres}, // 
	{"-d",3 ,LzmaCompres} // 
};
/*
static void ShowHelpMsg(void){
	printf("Commands:\n");
	printf("-e   \n");
	printf(" Compress file in LZMA method \n");
    printf("\n");	
	printf("-d   \n");
	printf(" Decompress file from LZMA method \n");	
	printf("\n");	
	printf("Examples:\n");
    printf("Lzma -e File1 Test.lzma \n");
    printf("  # Compress 'File1' into 'Test.lzma' .\n");
    printf("Lzma -d Test.lzmar File2  \n");
    printf("  # Decompress 'Test.lzma', and get the file named 'File2'. \n");	
	
}
*/
static void PrintHelp(char *buffer)
{
  strcat(buffer,
    "\nLZMA-C " MY_VERSION_CPU " : " MY_COPYRIGHT_DATE "\n\n"
    "Usage:  lzma <e|d> inputFile outputFile\n"
    "  e: encode file\n"
    "  d: decode file\n");
}

static int PrintError(char *buffer, const char *message)
{
  strcat(buffer, "\nError: ");
  strcat(buffer, message);
  strcat(buffer, "\n");
  return 1;
}

static int PrintErrorNumber(char *buffer, SRes val)
{
  sprintf(buffer + strlen(buffer), "\nError code: %x\n", (unsigned)val);
  return 1;
}

static int PrintUserError(char *buffer)
{
  return PrintError(buffer, "Incorrect command");
}

// Root-Cause of compile fail..
// error 2001: unresolved_external_symbol __chkstk
//#define IN_BUF_SIZE (1 << 16)   64K size inputstream
#define IN_BUF_SIZE (1 << 12) // per intput stream must less than 4K
//#define OUT_BUF_SIZE (1 << 16)
#define OUT_BUF_SIZE (1 << 12)


static SRes Decode2(CLzmaDec *state, ISeqOutStream *outStream, ISeqInStream *inStream,
    UInt64 unpackSize)
{
  int thereIsSize = (unpackSize != (UInt64)(Int64)-1);
  Byte inBuf[IN_BUF_SIZE];
  Byte outBuf[OUT_BUF_SIZE];
  size_t inPos = 0, inSize = 0, outPos = 0;
  LzmaDec_Init(state);
  for (;;)
  {
    if (inPos == inSize)
    {
      inSize = IN_BUF_SIZE;
      RINOK(inStream->Read(inStream, inBuf, &inSize));
      inPos = 0;
    }
    {
      SRes res;
      SizeT inProcessed = inSize - inPos;
      SizeT outProcessed = OUT_BUF_SIZE - outPos;
      ELzmaFinishMode finishMode = LZMA_FINISH_ANY;
      ELzmaStatus status;
      if (thereIsSize && outProcessed > unpackSize)
      {
        outProcessed = (SizeT)unpackSize;
        finishMode = LZMA_FINISH_END;
      }
      
      res = LzmaDec_DecodeToBuf(state, outBuf + outPos, &outProcessed,
        inBuf + inPos, &inProcessed, finishMode, &status);
      inPos += inProcessed;
      outPos += outProcessed;
      unpackSize -= outProcessed;
      
      if (outStream)
        if (outStream->Write(outStream, outBuf, outPos) != outPos)
          return SZ_ERROR_WRITE;
        
      outPos = 0;
      
      if (res != SZ_OK || (thereIsSize && unpackSize == 0))
        return res;
      
      if (inProcessed == 0 && outProcessed == 0)
      {
        if (thereIsSize || status != LZMA_STATUS_FINISHED_WITH_MARK)
          return SZ_ERROR_DATA;
        return res;
      }
    }
  }
}


static SRes Decode(ISeqOutStream *outStream, ISeqInStream *inStream)
{
  UInt64 unpackSize;
  int i;
  SRes res = 0;

  CLzmaDec state;

  // header: 5 bytes of LZMA properties and 8 bytes of uncompressed size 
  unsigned char header[LZMA_PROPS_SIZE + 8];

  // Read and parse header 

  RINOK(SeqInStream_Read(inStream, header, sizeof(header)));

  unpackSize = 0;
  for (i = 0; i < 8; i++)
    unpackSize += (UInt64)header[LZMA_PROPS_SIZE + i] << (i * 8);

  LzmaDec_Construct(&state);
  RINOK(LzmaDec_Allocate(&state, header, LZMA_PROPS_SIZE, &g_Alloc));
  res = Decode2(&state, outStream, inStream, unpackSize);
  LzmaDec_Free(&state, &g_Alloc);
  return res;
}


static SRes Encode(ISeqOutStream *outStream, ISeqInStream *inStream, UInt64 fileSize, char *rs)
{
  CLzmaEncHandle enc;
  SRes res;
  CLzmaEncProps props;

  UNUSED_VAR(rs);

  enc = LzmaEnc_Create(&g_Alloc);
  if (enc == 0)
    return SZ_ERROR_MEM;

  LzmaEncProps_Init(&props);
  res = LzmaEnc_SetProps(enc, &props);

  if (res == SZ_OK)
  {
    Byte header[LZMA_PROPS_SIZE + 8];
    size_t headerSize = LZMA_PROPS_SIZE;
    int i;

    res = LzmaEnc_WriteProperties(enc, header, &headerSize);
    for (i = 0; i < 8; i++)
      header[headerSize++] = (Byte)(fileSize >> (8 * i));
    if (outStream->Write(outStream, header, headerSize) != headerSize)
      res = SZ_ERROR_WRITE;
    else
    {
      if (res == SZ_OK)
        res = LzmaEnc_Encode(enc, outStream, inStream, NULL, &g_Alloc, &g_Alloc);
    }
  }
  LzmaEnc_Destroy(enc, &g_Alloc, &g_Alloc);
  return res;
}


static int main2(int numArgs, const char *args[], char *rs)
{
  CFileSeqInStream inStream;
  CFileOutStream outStream;
 // char c;
  int res;
  int encodeMode=0;
  Bool useOutFile = False;

  FileSeqInStream_CreateVTable(&inStream);
  File_Construct(&inStream.file);

  FileOutStream_CreateVTable(&outStream);
  File_Construct(&outStream.file);
/*
  if (numArgs == 1)
  {
    PrintHelp(rs);
    return 0;
  }
*/  
/*
  if (numArgs < 3 || numArgs > 4 || strlen(args[1]) != 1)
    return PrintUserError(rs);

  c = args[1][0];
  encodeMode = (c == 'e' || c == 'E');
  if (!encodeMode && c != 'd' && c != 'D')
    return PrintUserError(rs);
*/
  {
    size_t t4 = sizeof(UInt32);
    size_t t8 = sizeof(UInt64);
    if (t4 != 4 || t8 != 8)
      return PrintError(rs, "Incorrect UInt32 or UInt64");
  }
/*
  if (InFile_Open(&inStream.file, args[2]) != 0)
    return PrintError(rs, "Can not open input file");

  if (numArgs > 3)
  {
    useOutFile = True;
    if (OutFile_Open(&outStream.file, args[3]) != 0)
      return PrintError(rs, "Can not open output file");
  }
  else if (encodeMode)
    PrintUserError(rs);
*/

  if ( strcmp( args[1] , ArgInputList[0].ArgStr ) == 0 ){ // (Encode mode)
    encodeMode = 1;
  }
  useOutFile = True;
  // different from LZMA.efi exchange location argv
  //if (InFile_Open(&inStream.file, args[2]) != 0){
  if (InFile_Open(&inStream.file, args[2]) != 0){
    return PrintError(rs, "Can not open input file");
  }
  
  //if (OutFile_Open(&outStream.file, args[3]) != 0){
  if (OutFile_Open(&outStream.file, args[3]) != 0){
    return PrintError(rs, "Can not open output file");  
  }

  if (encodeMode)
  {
    UInt64 fileSize;
    File_GetLength(&inStream.file, &fileSize);
    res = Encode(&outStream.vt, &inStream.vt, fileSize, rs);
  }
  else
  {
    res = Decode(&outStream.vt, useOutFile ? &inStream.vt : NULL);
  }

  if (useOutFile)
    File_Close(&outStream.file);
  File_Close(&inStream.file);

  if (res != SZ_OK)
  {
    if (res == SZ_ERROR_MEM)
      return PrintError(rs, kCantAllocateMessage);
    else if (res == SZ_ERROR_DATA)
      return PrintError(rs, kDataErrorMessage);
    else if (res == SZ_ERROR_WRITE)
      return PrintError(rs, kCantWriteMessage);
    else if (res == SZ_ERROR_READ)
      return PrintError(rs, kCantReadMessage);
    return PrintErrorNumber(rs, res);
  }
  if (encodeMode){
	//printf(" Compress success!\n %s to %s \n",args[2],args[3]);  
  }
  else{
	//printf(" Decompress success!\n %s to %s \n",args[2],args[3]);  
  }
  return 0;
}


//int MY_CDECL LzmaCompres(int numArgs, const char *args[])
int LzmaCompres(int numArgs, const char *args[])
{
  char rs[800] = { 0 };
  //printf("Inner: %s , %s , %s , %s \n",args[0],args[1],args[2],args[3]);
  int res = main2(numArgs, args, rs);
  fputs(rs, stdout);
  return res;
}

/*
EFI_STATUS
main( int argc , const char *argv[] ){
  //UINT32       ArgInputNumber = 0;
  UINT8        i = 0;
  //EFI_STATUS   Status;	  
  int res=1;
/////////////////////////////////   
  for(i = 0; i < sizeof(ArgInputList)/sizeof(ArgInputList[0]); i++){
	if(strcasecmp(argv[1], ArgInputList[i].ArgStr) == 0){
	  if((argc-1) >= ArgInputList[i].ArgNumNeed){
	    res = ArgInputList[i].func(argc, argv);
	    break;
      }	    
    } // if(strcasecmp(argv[1], ArgInputList[i].ArgStr) == 0)
  } // for(i = 0; i < sizeof(ArgInputList)/sizeof(ArgInputList[0]); i++)
   if(res != 0){
      ShowHelpMsg();	
	 return EFI_INVALID_PARAMETER;
   }   
   return EFI_SUCCESS;
}
*/


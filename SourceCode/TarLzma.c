
#include  "TarLzma.h"



int       ShowLogFlag = 0;

static void
usage(void)
{
	//const char *m = "Usage: untar [-tvx] [-f file] [file]\n";
	printf("Commands:\n");
	printf("-c  \n");
	printf("  Pack the file in archive \n");	
	printf("-x  \n");
	printf("  unPack the archive and get file \n");	
	printf("-v  \n");
	printf("  Show logs while code executing \n");	
	printf("-f  \n");
	printf("  Creaate a folder for pack/unpack file \n");	
	printf("-z  \n");
	printf("  Compress/Decompress with the LZMA \n");		
    printf("\n");	
	printf("Examples:\n");
    printf("tar -zcvf Test.tar.lzma [file1][file2].. \n");
    printf("  # Create Test.tar.lzma and pack [file1][file2].. in archive.\n");
    printf("tar -zxvf Test.tar.lzma [Dirctory]  \n");
    printf("  # Extract all files from Test.tar.lzma under [Dirctory]. \n");
	exit(1);
}

int
main(int argc, char **argv)
{
	char *filename;
	char *fp=NULL;
	int  flags, mode, opt ,i;
	int  prevention=0;
	//int ShowLogFlag = 0;
	int MakFileFlag = 0;
	int CodingFlag = 0;
	int ArchiveFlag = 0;
	FILE *a=NULL;
	int  numSodr = 0;
	const char **TempArgv = argv;
	char *TheOneUndo = argv[2];
	char **chrSodr=NULL;
	char *MakFDirStr=NULL;
	const char *SodrEnc = "-e";
	const char *SodrDec = "-d";
	char *TempArchiveString = "TempArchive.tar";
	const char *TempDecompressResult = "LzmaDecompressResult";
	const char *LzmaTail = ".lzma";	
	long    len = 0;
	int LogicRush = 0;
	
	int res = 1;
/////////////////////////	  
	  
	//TempArgv = argv;
	//printf("1  %s %s \n",TempArgv[2],argv[2]);	
	

   chrSodr        = malloc(100 * sizeof(char*));
   for (i=0 ;i <100 ; i++){   
	 chrSodr[i]   = malloc(100 * sizeof(char));	
   }	 
   //MakFDirStr     = malloc(100 * sizeof(char));
	
	
	mode = 0;
	//verbose = 0;
	//flags = ARCHIVE_EXTRACT_TIME;

	/* Among other sins, getopt(3) pulls in printf(3). */
	while (*++argv != NULL && **argv == '-') {
		const char *p = *argv + 1;

		while ((opt = *p++) != '\0') {
			switch (opt) {
			case 'f':
			    MakFileFlag = 1;
				/*
				if (*p != '\0')
					filename = p;
				else
					filename = *++argv;
				p += strlen(p);
				*/
				break;
			case 'v':
				//verbose++;
				ShowLogFlag = 1;
				break;
			case 'x':
			    ArchiveFlag = 1;
				mode = opt;
				prevention++;
				break;
			case 'c':
			    ArchiveFlag = 1;
				mode = opt;
				prevention++;
				break;
			case 'z':
				CodingFlag = 1;
				break;				
			}
		}
	} // while (*++argv != NULL && **argv == '-')
	
    if ( prevention == 2 ){
      printf("Error: Can not put same operator 'x' and 'c' in the time!\n");
	  if (chrSodr != NULL){
	    for (i=0 ;i <100 ; i++)   
          free(chrSodr[i]);			
	    free(chrSodr);		
	  }		
	  return (0);	  
	}

	if ( ShowLogFlag ){
	  printf("v: ShowLogFlag Assert \n");
	}  
    //PrintLog( "CDYU test!" , ShowLogFlag );

	if ( MakFileFlag && ( mode == 'x' ) ){
	  printf("f: MakFileFlag Assert \n");
	  if ( argc < 4 ){
		printf("User give low few commands\n");	  
	    printf("Can not create dirctory!\n");
	    printf("Unpcak file in current dirctory\n");
	  }
	  else{
	    //memcpy( MakFDirStr , TempArgv[3] , strlen(TempArgv[3])+1 ); 
		MakFDirStr = TempArgv[3]; // NULL or Not NULL
		printf("Create dirctory: %s to catch unpack file\n",MakFDirStr);
	  }
	}  
		
	
	if ( CodingFlag ){
	  *argv++; // THUNDER GOD!!!!
	  ArchiveFlag = 0;	
	  printf("z: CodingFlag Assert\n");		
	  // Bad Logic...check user action inversed...
      if ( argc < 3 ){
		printf("User give low few commands..\nBesides, only suit on fold archiving and compressing\n");  
	  }
      else {
	    switch (mode){

		  case 'c':	
		    printf("c|x : ArchiveFlag Assert\n");		
		    // tar before compress
	        filename = TempArchiveString;
	        //printf(" Get Temp TAR name success \n");		    
		    if(argc < 4){ 
		      printf("For Pack utility \nUser give low few commands\n\n");
		      break;
		    }
		    tarbsd(filename, argv);		
            printf("Pack success! \n");	
			
            numSodr = 3;           
			memcpy( chrSodr[0] , TempArgv[0] , strlen(TempArgv[0])+1 );
			memcpy( chrSodr[1] , SodrEnc , strlen(SodrEnc)+1 );
			memcpy( chrSodr[2] , TempArchiveString , strlen(TempArchiveString)+1 );	 // in
			memcpy( chrSodr[3] , TheOneUndo , strlen(TheOneUndo)+1 );  // out
            //printf("\n %s \n %s  \n %s \n %s\n",chrSodr[0],chrSodr[1],chrSodr[2],chrSodr[3]);			  
			res = LzmaCompres( numSodr , chrSodr );	
            if (res == 0){
			  printf("Compress success!\n");	
			  PrintLog(TempArchiveString,ShowLogFlag);
			  PrintLog(" to ",ShowLogFlag);	
			  PrintLog(TheOneUndo,ShowLogFlag);		
			  PrintLog(" \n",ShowLogFlag);					
			}
            remove(TempArchiveString);			
		    break; 
						
		  case 'x':	
		    printf("c|x : ArchiveFlag Assert\n");	
		    // untar after decompress 
			numSodr = 3;
			memcpy( chrSodr[0] , TempArgv[0] , strlen(TempArgv[0])+1 );
			memcpy( chrSodr[1] , SodrDec , strlen(SodrDec)+1 );
			memcpy( chrSodr[2] , TheOneUndo , strlen(TheOneUndo)+1 );          // in
            memcpy( chrSodr[3] , TempArchiveString , strlen(TempArchiveString)+1 ); 	// out
            //printf("\n %s \n %s  \n %s \n %s\n",chrSodr[0],chrSodr[1],chrSodr[2],chrSodr[3]);
 			res = LzmaCompres( numSodr , chrSodr );	
            if (res == 0){
			  printf("Decompress success!\n");	
			  PrintLog(TheOneUndo,ShowLogFlag);	
			  PrintLog(" to ",ShowLogFlag);	
			  PrintLog(TempArchiveString,ShowLogFlag);	
			  PrintLog(" \n",ShowLogFlag);	
			}					
	        if (res!=0){
			  break;	
			}
			if (argc < 3){
		      printf("For Pack utility \User give low few commands\n\n");	
		      break;	
		    }					
			filename = TempArchiveString;
			a = fopen(filename, "r");
	        //printf(" Get Temp TAR name success \n");	
		    if (a == NULL)
		      fprintf(stderr, "Unable to open %s\n", filename);
		    else {
		      untar(a, filename ,MakFDirStr);  //!!!!
		      fclose(a);
			  printf("Unpack success! \n");	
		    }	
            remove(TempArchiveString);				
		    break; 		
			
		  default:	
            numSodr = 3;
			memcpy( chrSodr[0] , TempArgv[0] , strlen(TempArgv[0])+1 );
			memcpy( chrSodr[1] , SodrDec , strlen(SodrDec)+1 );
			memcpy( chrSodr[2] , TheOneUndo , strlen(TheOneUndo)+1 );          // in
            memcpy( chrSodr[3] , TempDecompressResult , strlen(TempDecompressResult)+1 ); 	// out
            //printf("\n %s \n %s  \n %s \n %s\n",chrSodr[0],chrSodr[1],chrSodr[2],chrSodr[3]);
			res = LzmaCompres( numSodr , chrSodr );
			 if (res == 0){
			   fp = fopen(TempDecompressResult,"r");	 
			   if ( fp != NULL ){
			     fseek(fp, 0, SEEK_END);
	             len = ftell(fp);	
				 if ( len != 0 ){
			       printf("Decompress success!\n");	
			       PrintLog(TheOneUndo,ShowLogFlag);	
			       PrintLog(" to ",ShowLogFlag);	
			       PrintLog(TempDecompressResult,ShowLogFlag);	
			       PrintLog(" \n",ShowLogFlag);	
                   LogicRush =1;				   
				 } //if ( len != 0 )
			   } // if ( fp != NULL )	 			   
			 }
		     if (LogicRush == 0) {
			   remove(TempDecompressResult);				 
			 }
			
			if ( res != 0 || LogicRush == 0 ){
			  
			  memcpy( chrSodr[0] , TempArgv[0] , strlen(TempArgv[0])+1 );
			  memcpy( chrSodr[1] , SodrEnc , strlen(SodrEnc)+1 );
			  memcpy( chrSodr[2] , TheOneUndo , strlen(TheOneUndo)+1 );	 // in
			  strcat ( TheOneUndo , LzmaTail );
			  memcpy( chrSodr[3] , TheOneUndo , strlen(TheOneUndo)+1 );  // out
              //printf("\n %s \n %s  \n %s \n %s\n",chrSodr[0],chrSodr[1],chrSodr[2],chrSodr[3]);			  
			  
			  
              //printf("%s %s %s",chrSodr[1],chrSodr[2],chrSodr[3]);
			  res = LzmaCompres( numSodr , chrSodr );	
 			  if (res == 0){
			    printf("Compress success!\n");	
			    PrintLog("Create Compress File: ",ShowLogFlag);	
			    PrintLog(TheOneUndo,ShowLogFlag);	
			    PrintLog(" \n",ShowLogFlag);					 
			  }          		  
			}
		    break; 			  
		} // switch (mode)	
 	  } // !! if ( argc < 3 )
	} //if ( CodingFlag )

	if ( ArchiveFlag ){
	  printf("c|x : ArchiveFlag Assert\n");		
	  *argv++; // THUNDER GOD!!!!	
	  filename = TempArgv[2]; // use strcpy() testing...
	  //printf("%s %s \n",filename,argv[0]);
	  printf(" Get TAR name success \n");
	  switch (mode) {
	  case 'c':
		if(argc < 4){ 
		  printf("For Pack utility \nUser give low few commands\n\n");
		  //usage();
		  break;
		}
		tarbsd(filename, argv);
		break;
	  case 'x':
	    if (argc < 3){
		  printf("For Pack utility \User give low few commands\n\n");	
		  //usage();
		  break;	
		}
		a = fopen(filename, "r");
		if (a == NULL)
		  fprintf(stderr, "Unable to open %s\n", filename);
		else {
		  untar(a, filename ,MakFDirStr );
		  fclose(a);
		}
		break;
	  default:
		break;
	  }
    } // if ( ArchiveFlag )
	
    if (argc < 2){
	  usage();
	}


	if (chrSodr != NULL){
	  for (i=0 ;i <100 ; i++)   
        free(chrSodr[i]);			
	  free(chrSodr);		
	}	
	
	return (0);
}




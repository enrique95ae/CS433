#include <stdio.h>
#include <fcntl.h>
     


int main(int argc, char *argv[])
{	

	int n, sourceFile, destFile;
	char buffer;	

	//Command arguments validation
	if(argc != 3)
    	{		
     	 printf("Usage: %s <Source> <Destination> \n");
      	return -1;

    	}else{
		sourceFile = open(argv[1], O_RDONLY);
		if(sourceFile == -1)
		{
			printf("ERROR: source file could not be open!");
		}else{//if source file could be open --> time to open the destination file
		
			destFile = open(argv[2], O_WRONLY | O_CREAT, 0671);
			if(destFile == -1)
			{	
				printf("ERROR: destination file could not be open!");
			}else{//both files are open time to read from source and write into destination
				while((n=read(sourceFile, &buffer, 1)) != 0) //while end of file hasn´t been reached.
				{
					write(destFile, &buffer, 1);
				}	
				close(sourceFile);
				close(destFile);				
			}
		}				
	}	
	
return 0;
}

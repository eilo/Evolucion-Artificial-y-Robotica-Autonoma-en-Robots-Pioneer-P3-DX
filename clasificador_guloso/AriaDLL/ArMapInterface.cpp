#include "ArExport.h"
#include "ariaOSDef.h"
#include "ariaInternal.h"

#include "ArMapInterface.h"


AREXPORT const char *ArMapInfoInterface::MAP_INFO_NAME        = "MapInfo:"; 
AREXPORT const char *ArMapInfoInterface::META_INFO_NAME       = "MetaInfo:";
AREXPORT const char *ArMapInfoInterface::TASK_INFO_NAME       = "TaskInfo:";   
AREXPORT const char *ArMapInfoInterface::ROUTE_INFO_NAME      = "RouteInfo:"; 
AREXPORT const char *ArMapInfoInterface::SCHED_TASK_INFO_NAME = "SchedTaskInfo:";
AREXPORT const char *ArMapInfoInterface::SCHED_INFO_NAME      = "SchedInfo:"; 
AREXPORT const char *ArMapInfoInterface::CAIRN_INFO_NAME      = "CairnInfo:";  
AREXPORT const char *ArMapInfoInterface::CUSTOM_INFO_NAME     = "CustomInfo:";

AREXPORT const char *ArMapInterface::MAP_CATEGORY_2D = "2D-Map";
AREXPORT const char *ArMapInterface::MAP_CATEGORY_2D_MULTI_SOURCES = "2D-Map-Ex";
AREXPORT const char *ArMapInterface::MAP_CATEGORY_2D_EXTENDED = "2D-Map-Ex2";



AREXPORT bool ArMapScanInterface::isDefaultScanType(const char *scanType)
{
  bool b = false;
  if ((scanType != NULL) &&
      (ArUtil::isStrEmpty(scanType))) {
    b = true;
  }
  return b;
}

AREXPORT bool ArMapScanInterface::isSummaryScanType(const char *scanType)
{
  bool b = scanType == NULL;
  return b;
}

// ----------------------------------------------------------------------------


/** 
 * Determines what system file path to use based on the contents of @a baseDirectory, @a fileName and
 * @a isIgnoreCase.  If @a fileName is not an absolute path and @a baseDirectory is not null and 
 * not empty, then it is combined with @a baseDirectory to form a full path.
 * An absolute path starts with the '/' or '\' character, or on Windows, with "X:\" where X is any
 * upper or lower case alphabetic character A-Z or a-z.  
 */
AREXPORT std::string ArMapInterface::createRealFileName(const char *baseDirectory,
                                                        const char *fileName,
                                                        bool isIgnoreCase)
{ 

  if (fileName == NULL) {
    return "";
  }
  std::string realFileName;
  
  // If there is no base directory or the filename part is an absolute path, use the filename directly without the base directory
  if ((fileName[0] == '/') || 
      (fileName[0] == '\\') ||
      (strlen(baseDirectory) == 0) ||
      (baseDirectory == NULL)
#ifdef WIN32
	  ||
	  ( fileName[1] == ':' && fileName[2] == '\\' && isalpha(fileName[0]) )
#endif
  )
  {
    realFileName = fileName;
  }
  else // non-empty base directory and fileName is not an absolute path
  {
    int totalLen = strlen(baseDirectory) + strlen(fileName) + 10;
    char *nameBuf = new char[totalLen];
    nameBuf[0] = '\0';

    snprintf(nameBuf, totalLen, baseDirectory);
    ArUtil::appendSlash(nameBuf, totalLen);
    
    realFileName = nameBuf;
    realFileName += fileName;

    delete [] nameBuf;

  } // end else non empty base directory

  // this isn't needed in windows since it ignores case no matter what
#ifndef WIN32
  if (isIgnoreCase)
  {
    char directoryRaw[2048];
    directoryRaw[0] = '\0';
    char fileNamePart[2048];
    fileNamePart[0] = '\0';
    if (!ArUtil::getDirectory(realFileName.c_str(), 
					                    directoryRaw, sizeof(directoryRaw)) ||
	      !ArUtil::getFileName(realFileName.c_str(), 
			                       fileNamePart, sizeof(fileNamePart)))
    {
      ArLog::log(ArLog::Normal, 
		             "ArMap: Problem with filename '%s'", 
		             realFileName.c_str());
      return "";
    }
    

    char directory[2048];
    //printf("DirectoryRaw %s\n", directoryRaw);
    if (strlen(directoryRaw) == 0 || strcmp(directoryRaw, ".") == 0)
    {
      strcpy(directory, ".");
    }
    else if (directoryRaw[0] == '/')
    {
      strcpy(directory, directoryRaw);
    }
    else if (!ArUtil::matchCase(baseDirectory, 
				                        directoryRaw, 
                                directory, 
				                        sizeof(directory)))
    {
	    ArLog::log(ArLog::Normal, 
		             "ArMap: Bad directory for '%s'", 
		              realFileName.c_str());
      return "";
    }

    char tmpDir[2048];
    tmpDir[0] = '\0';
    //sprintf(tmpDir, "%s", tmpDir, directory);
    strcpy(tmpDir, directory);
    ArUtil::appendSlash(tmpDir, sizeof(tmpDir));
    char squashedFileName[2048];
    
    if (ArUtil::matchCase(tmpDir, fileNamePart, 
			                    squashedFileName, 
			                    sizeof(squashedFileName)))
    {
      realFileName = tmpDir;
      realFileName += squashedFileName;
      //printf("squashed from %s %s\n", tmpDir, squashedFileName);
    }
    else
    {
      realFileName = tmpDir;
      realFileName += fileNamePart;
      //printf("unsquashed from %s %s\n", tmpDir, fileNamePart);
    }
    
    ArLog::log(ArLog::Verbose, 
	       "ArMap: %s is %s",
	       fileName, realFileName.c_str());
  }
#endif

  return realFileName;

} // end method createRealFileName

